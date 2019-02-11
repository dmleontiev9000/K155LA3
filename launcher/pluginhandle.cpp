#include <QCoreApplication>
#include <QStringList>
#include <QDebug>
#include <QDir>
#include "pluginhandle.h"
#include "worker.h"
static QList<K::Launcher::PluginHandle*> mAll;
K::Launcher::PluginHandle::PluginHandle(const QString& name,
        const QFileInfo &path,
        QSettings &settings,
        bool verbose,
        QTranslator *translator,
        QObject *parent)
    : QPluginLoader(parent)
    , mName(name)
    , mInstance(nullptr)
    , mIfp(nullptr)
    , mUsage(0)
    , mVerbose(verbose)
{
    auto app = QCoreApplication::instance();
    bool havegui = false;
    if (!strcmp(app->metaObject()->className(), "QApplication"))
        havegui = true;

    setFileName(path.absoluteFilePath());
    //QPluginLoader::setLoadHints(QLibrary::ResolveAllSymbolsHint);
    if (QPluginLoader::load()) {
        mInstance = QPluginLoader::instance();
        if (mInstance) {
            mIfp = qobject_cast<IfPlugin*>(mInstance);
            if (!mIfp) {
                mErrorString = app->translate("plugin", "not supported");
                mInstance = nullptr;
            }
            if (mIfp->requiresGui() && !havegui) {
                mErrorString = app->translate("plugin", "plugin requires GUI mode");
                mInstance = nullptr;
            }
            mInstance->setObjectName(name);
            QDir dir(path.absoluteDir());
            dir.cdUp();
            dir.cd("share");
            mInstance->setProperty("path", dir.absolutePath());
            if (dir.cd("i18n"))
                translator->load(QLocale(),
                                 name,
                                 QLatin1String("_"),
                                 dir.absoluteFilePath("tr"));

            settings.beginGroup("plugins");
            settings.beginGroup(name);
            auto ks = settings.allKeys();
            for(QString k : ks) {
                mInstance->setProperty(k.toLatin1().constData(),
                                       settings.value(k));
            }
            settings.endGroup();
            settings.endGroup();

            for(int i = 0; i < mAll.size(); ++i) {
                connect(this, SIGNAL(started(QString,QObject*)),
                        mAll[i], SLOT(otherStarted(QString,QObject*)));
                connect(this, SIGNAL(terminated(QObject*)),
                        mAll[i], SLOT(otherTerminated(QObject*)));
            }
            mAll.append(this);

            return;
        }
    }
    mErrorString = QPluginLoader::errorString();
    mInstance = nullptr;
}
K::Launcher::PluginHandle::~PluginHandle() {
    Q_ASSERT(mUsage == 0);
    mAll.removeOne(this);
    mInstance = nullptr;
}
void K::Launcher::PluginHandle::otherStarted(const QString &name, QObject *obj) {
    if (mInstance && mInstance != obj && mUsage)
        mIfp->pluginLoaded(name, obj);
}
void K::Launcher::PluginHandle::otherTerminated(QObject *obj) {
    if (mInstance && mInstance != obj && mUsage)
        mIfp->pluginUnloaded(obj);
}
bool K::Launcher::PluginHandle::use() {
    if (mInstance == nullptr)
        return false;
    if (mUsage == 0) {
        if (!mIfp->init()) {
            if (mVerbose) {
                qWarning()<<"Failed to load plugin "<<mName;
                qWarning()<<mIfp->errorString();
            }
            return false;
        }
    }
    ++mUsage;

    emit started(mName, mInstance);
    return true;
}
void K::Launcher::PluginHandle::release() {
    if (mInstance == nullptr)
        return;
    Q_ASSERT(mUsage != 0);
    --mUsage;

    if (mUsage)
        return;

    mIfp->stop();
    emit terminated(mInstance);
    mInstance->disconnect();
    mIfp->cleanup();
}
void K::Launcher::PluginHandle::stop() {
    if (mInstance == nullptr || !mUsage)
        return;

    mIfp->stop();
    mInstance->disconnect();
}

