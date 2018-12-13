#pragma once

#include "../core/interfaces.h"
#include "worker.h"
#include "pluginhandle.h"

namespace K {
namespace Launcher {

class Core : public Worker, public IfCore
{
    Q_OBJECT
public:
    explicit Core(QObject * parent = 0);
    virtual ~Core();

    //IfCore
    const QString& appPath() const override;
    const QString& errorString() const override;
    bool           run() override;
    void           quit() override;
    PluginList     plugins() override;
    QObject *      loadPlugin(const QString& name) override;
    void           unloadPlugin(const QString& name) override;
private:
    IfWorker *     createWorker(QObject * parent) override;
    void           deleteWorker(IfWorker * worker) override;

    QString            mAppBinary;
    QString            mAppPath;
    QString            mParamsFile;
    QStringList        mAutoLoad;

    QString            mErrorString;
    bool               mCanLaunch;
    //prevents reentering core
    volatile bool      mBlocked;
    bool               mVerbose = false;

    QHash<QString, PluginHandle*> mPlugins;

    void     unload();
};

} //namespace Launcher;
} //namespace K;
