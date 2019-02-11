#include <QCoreApplication>
#include <QTranslator>
#include <QLocale>
#include <QDir>
#include <QFileInfo>
#include <QThread>
#include <QSettings>
#include <stdio.h>
#include <string.h>
#include "core.h"
#include "../coregui/icons.h"

namespace {
static void usage() {//======================================
    fprintf(stderr, "Usage: application [args] options\n"
            "Supported arguments:\n\n"
            "  --version                 print version and return\n\n"
            "  --verbose                 print debug information\n\n"
            "  --params  \"file\"        use ini file as source of\n"
            "                            plugin paramaters\n\n"
            "  --load \"name\"           load plugin\n\n"
            "  --no-xxxxx                prevent plugin xxxx from loading\n\n"
            "Parameters format:\n\n"
            "  pluginname.value=string   specify value for a named plugin\n\n");
}

static QString q_string_escape(const QString& value) {
    static const QString hex("_%1");
    QString copy;
    copy.reserve(value.length());

    int i, len = value.length();
    for(i = 0; i < len; ++i) {
       QChar c = value[i];
       if (c.unicode() < 0x1f)
           break;
       if (c.isPunct() || c.isSpace() || c.unicode() > 0x126 || c == '_') {
           copy.append(hex.arg(c.unicode(), 0, 16));
       } else {
           copy.append(c);
       }
    }
    if (i != len)
        copy.clear();
    return copy;
}


}//===================================================================

K::Launcher::Core::Core(QObject * parent)
    : Worker(Worker::Mode::FAKE, parent)
    , mCanLaunch(false)
    , mBlocked(true)
{
    auto app = QCoreApplication::instance();
    Q_ASSERT(QThread::currentThread() == app->thread());

    QDir baseDir(app->applicationDirPath());

    mAppPath   = baseDir.absolutePath();
    mAppBinary = app->applicationFilePath();

    auto translator = new QTranslator(this);
    if (translator->load(QLocale(),
                         QLatin1String("k155la3"),
                         QLatin1String("_"),
                         baseDir.absoluteFilePath("tr")))
    {
        app->installTranslator(translator);
    }

    auto arguments = app->arguments();
    auto iterator  = arguments.constBegin();
    auto end       = arguments.constEnd();

    //parameters accepted:
    //  --version: print version and return
    bool version(false);
    //  --verbose: print debug information
    bool verbose(false);
    //  --params: get parameters out of this file
    QString paramsFile(baseDir.absoluteFilePath("config.ini"));
    //bad parameters flag:
    QString errorMessage;
    QRegExp regex("\\s*((\\w+)\\.){0,1}(\\w+)\\s*=\\s*(\\w+)\\s*");

    //skip first parameter
    if (iterator != end) {
        iterator++;
    }
    while(iterator != end) {
        const QString& str = *iterator;
        if (str.isEmpty())
            break;

        if (str == "--verbose") {
            verbose = true;
        } else if (str == "--version") {
            version = true;
        } else if (str == "--params") {
            if (iterator+1 == end) {
                errorMessage = app->translate("cmdline", "params parameter needs argument");
                break;
            }
            ++iterator;
            paramsFile = *iterator;
        } else
            break;
        ++iterator;
    }

    QSettings settings(paramsFile, QSettings::IniFormat);
    if (settings.status() != QSettings::NoError)
        errorMessage = tr("cmdline", "configuration file contains errors or is not readable");
    else while(iterator != end) {
        const QString& str = *iterator;
        if (str.isEmpty())
            break;

        if (str[0] != '-') {
            if (!regex.exactMatch(str)) {
                errorMessage = app->translate("cmdline", "expected name=value or plugin.name=value");
                break;
            }
            if(regex.captureCount() != 4) {
                errorMessage = app->translate("cmdline", "expected name=value or plugin.name=value");
                break;
            }
            QString pluginName = regex.cap(2);
            QString paramName  = regex.cap(3);
            QString value      = regex.cap(4);
            if (pluginName.isEmpty()) {
                errorMessage = app->translate("cmdline", "must either provide plugin name explicitly or via previous --load option");
                break;
            }
            settings.beginGroup("plugin");
            settings.beginGroup(pluginName);
            settings.setValue(paramName, value);
            settings.endGroup();
            settings.endGroup();
        } else if (str.startsWith("--no-")) {
            if (str.length() <= 5) {
                errorMessage = app->translate("cmdline", "invalid parameter");
                break;
            }
            settings.beginGroup("plugin");
            settings.beginGroup(str.mid(5));
            settings.setValue("load", false);
            settings.endGroup();
            settings.endGroup();
        } else if (str == "--with-") {
            if (str.length() <= 7) {
                errorMessage = app->translate("cmdline", "invalid parameter");
                break;
            }
            settings.beginGroup("plugin");
            settings.beginGroup(str.mid(7));
            settings.setValue("load", true);
            settings.endGroup();
            settings.endGroup();
        } else {
            errorMessage = app->translate("cmdline", "unknown parameter");
            break;
        }
        ++iterator;
    }

    if (!errorMessage.isEmpty()) {
        mErrorString = errorMessage;
        fprintf(stderr, "Error: %s", qPrintable(errorMessage));
        fprintf(stderr, " at %s\n", qPrintable(*iterator));
        usage();
        return;
    }

    if (version) {
        fprintf(stdout, "%s\n", K_VERSION);
        return;
    }

    mVerbose = verbose;

    //add icon theme
    auto iconpaths  = QIcon::themeSearchPaths();
    auto ouricons   = baseDir.absoluteFilePath("share/icons");
    iconpaths.prepend(ouricons);
    QIcon::setThemeSearchPaths(iconpaths);
    K::Core::setIconPath(ouricons);

    //add all plugin directories to list
    auto pluginpath = baseDir.absoluteFilePath("plugins");
    app->addLibraryPath(pluginpath);
    QHash<QString, QString> knownPlugins;
    /*enumerate plugins*/{
        QDir pluginDir(pluginpath);
        QStringList libs = pluginDir.entryList(
                    QStringList(LIB_PREFIX "*" LIB_SUFFIX),
                    QDir::Files|QDir::Readable);
        for(auto i = libs.constBegin(); i != libs.constEnd(); ++i) {
            QString name = i->left(i->indexOf('.'));
            #ifdef Q_OS_UNIX
            if (name.startsWith("lib"))
                name = name.mid(3);
            #endif
            if (name.isEmpty())
                continue;

            name = q_string_escape(name);
            if (name.isEmpty())
                continue;

            settings.beginGroup("plugin");
            settings.beginGroup("name");
            if (!settings.contains("load") || settings.value("load").toBool())
                knownPlugins.insert(name, pluginDir.absoluteFilePath(*i));
            settings.endGroup();
            settings.endGroup();
        }
    }

    if (verbose) {
        for(auto i = knownPlugins.constBegin();
            i != knownPlugins.constEnd(); ++i)
        {
            fprintf(stdout, "Plugin %s -> %s\n",
                    qPrintable(i.key()),
                    qPrintable(i.value()));
        }
    }

    for(auto i = knownPlugins.constBegin();
        i != knownPlugins.constEnd(); ++i)
    {
        QFileInfo info(i.value());
        QString value = info.baseName();
        if (value.isEmpty()) {
            fprintf(stderr,
                    "Plugin %s has invalid path\n",
                    qPrintable(i.key()));
            continue;
        }

        QScopedPointer<PluginHandle> loader(new PluginHandle(
            i.key(),info,settings,mVerbose,translator,this));
        QObject * instance = loader->instance();
        if (!instance) {
            if (verbose) {
                fprintf(stdout, "Plugin %s cannot be loaded:\n\t%s\n",
                        qPrintable(i.key()),
                        qPrintable(loader->errorString()));
            }
            continue;
        }
        //check if this plugin is loaded under other name
        bool isloaded = false;
        for(auto i = mPlugins.constBegin();
            i != mPlugins.constEnd(); ++i) {
            if (i.value()->instance() == instance) {
                isloaded = true;
                break;
            }
        }
        if (isloaded) {
            fprintf(stdout, "Plugin %s cannot be loaded:\n\t%s\n",
                    qPrintable(i.key()),
                    "already loaded as another library");
            mErrorString = "misconfuguration detected, check library paths and symlinks";
            return;
        }
        mPlugins.insert(i.key(), loader.take());
        mAutoLoad.append(i.key());
    }

    mParamsFile  = paramsFile;

    mCanLaunch = true;
    mBlocked = false;
}

K::Launcher::Core::~Core()
{
    auto app = QCoreApplication::instance();
    Q_ASSERT(QThread::currentThread() == app->thread());
    unload();
}
//=================================================
const QString& K::Launcher::Core::errorString() const {
    return mErrorString;
}
const QString& K::Launcher::Core::appPath() const {
    return mAppPath;
}
//=================================================
bool K::Launcher::Core::runApp() {
    auto app = QCoreApplication::instance();
    Q_ASSERT(QThread::currentThread() == app->thread());

    if (!mCanLaunch)
        return false;

    for(QString s : mAutoLoad) {
        if (!loadPlugin(s)) {
            mCanLaunch = false;

            unload();

            return false;
        }
    }

    Worker::run();
    unload();

    return true;
}
void K::Launcher::Core::quit() {
    auto app = QCoreApplication::instance();
    Q_ASSERT(QThread::currentThread() == app->thread());

    if (mCanLaunch)
        Worker::quit();
}
void K::Launcher::Core::unload() {
    auto app = QCoreApplication::instance();
    Q_ASSERT(QThread::currentThread() == app->thread());

    mBlocked = true;
    //safely unload all plugins
    for(auto i = mPlugins.constBegin(); i != mPlugins.constEnd(); ++i) {
        if (i.value()->isLoaded())
            i.value()->stop();
    }
    for(auto i = mPlugins.constBegin(); i != mPlugins.constEnd(); ++i) {
        if (i.value()->isLoaded())
            i.value()->release();
    }
}
//=================================================
QObject * K::Launcher::Core::loadPlugin(const QString& name) {
    auto app = QCoreApplication::instance();
    Q_ASSERT(QThread::currentThread() == app->thread());

    if (!mCanLaunch)
        return nullptr;
    if (mBlocked)
        return nullptr;

    auto i = mPlugins.constFind(name);
    if (i == mPlugins.constEnd()) {
        if (mVerbose)
            qWarning("Cannot find plugin %s", qPrintable(name));
        return nullptr;
    }
    if (i.value()->use()) {
        return i.value()->instance();
    }
    return nullptr;
}

void K::Launcher::Core::unloadPlugin(const QString& name) {
    auto app = QCoreApplication::instance();
    Q_ASSERT(QThread::currentThread() == app->thread());

    if (mBlocked)
        return;

    auto i = mPlugins.find(name);
    if (i == mPlugins.end())
        return;
    i.value()->release();
}
K::IfCore::PluginList K::Launcher::Core::plugins() {
    auto app = QCoreApplication::instance();
    Q_ASSERT(QThread::currentThread() == app->thread());

    PluginList result;
    result.reserve(mPlugins.size());
    for(auto i = mPlugins.constBegin(); i != mPlugins.constEnd(); ++i) {
        result.append({i.key(),
                       i.value()->instance(),
                       i.value()->isLoaded()});
    }
    return result;
}
//=================================================
K::IfWorker * K::Launcher::Core::createWorker(QObject *parent) {
    auto app = QCoreApplication::instance();
    Q_ASSERT(QThread::currentThread() == app->thread());

    if (!parent) return this;

    for(auto i = mPlugins.constBegin(); i != mPlugins.constEnd(); ++i) {
        PluginHandle * ph = i.value();
        if (ph->instance() != parent)
            continue;

        return new Worker(Worker::Mode::REAL, parent);
    }

    qCritical("attempt to create worker for non-plugin");
    return nullptr;
}
void K::Launcher::Core::deleteWorker(IfWorker *worker) {
    auto app = QCoreApplication::instance();
    Q_ASSERT(QThread::currentThread() == app->thread());

    if (worker) {
        Worker * w = dynamic_cast<Worker*>(worker);
        if (w != static_cast<Worker*>(this))
            delete w;
    }
}
