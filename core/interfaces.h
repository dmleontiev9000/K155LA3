#pragma once

#include "core_global.h"
#include "compat.h"
#include <QObject>
#include <QString>
#include <QList>
#include <QStringList>
#include <QCoreApplication>
#include "utils.h"
namespace K {

class IfCore;
class IfPlugin;
class IfAutoConnectPlugin;
class IfWorker;

/*
 * IfCore - загружает плагины и позволяет их находить по имени,
 * создает пулы потоков и удаляет их
 */
class K_CORE_EXPORT IfCore {
public:
    typedef struct {
        QString   name;
        QObject  *object;
        bool      loaded;
    } LoadablePlugin;
    typedef QList<LoadablePlugin> PluginList;

    static IfCore * instance();

    virtual const QString& appPath() const = 0;
    virtual bool           run() = 0;
    virtual void           quit() = 0;
    virtual const QString& errorString() const = 0;

    virtual PluginList     plugins() = 0;
    virtual QObject  *     loadPlugin(const QString& name) = 0;
    virtual void           unloadPlugin(const QString& name) = 0;

    virtual IfWorker *     createWorker(QObject * parent) = 0;
    virtual void           deleteWorker(IfWorker * worker) = 0;
protected:
    IfCore();
    virtual ~IfCore();
};

/*
 * Минимальный интерфейс, который должен экспортировать
 * основной класс плагина. Может быть инициализован,
 * деинициализован, уведомляется о включении и выключении
 * других плагинов. Может требовать для работы графического
 * интерфейса.
 *
 * Может быть улучшен функцией автоподключения к другим
 * плагинам на основе тэгов
 */
class K_CORE_EXPORT IfPlugin {
public:
    virtual bool init() = 0;
    virtual void stop() = 0;    //before unregistering: stop calling others
    virtual void cleanup() = 0; //after unregistering: free resources
    virtual void pluginLoaded(const QString& name, QObject * plugin);
    virtual void pluginUnloaded(QObject * plugin);
    virtual QString errorString() = 0;
    virtual QObject * self() = 0;
    virtual bool requiresGui() const = 0;

protected:
    bool loadPlugins(const QStringList& plugin);
    void unloadPlugins();
    void autoConnect(QObject * peer);

    IfPlugin() {}
    virtual ~IfPlugin();
private:
    QStringList mPlugins;
    QObjectList mInstances;
};

#define K_DECLARE_PLUGIN_METHODS \
    bool init() override; \
    void stop() override; \
    void cleanup() override; \
    QString errorString() override; \
    QObject * self() override; \
    bool requiresGui() const override;    

#define K_DECLARE_PLUGIN_CONNECT \
    void pluginLoaded(const QString& name, QObject * plugin) override; \
    void pluginUnloaded(QObject * plugin) override;

#define K_DECLARE_ALL_PLUGIN_METHODS \
    K_DECLARE_PLUGIN_METHODS \
    K_DECLARE_PLUGIN_CONNECT

class K_CORE_EXPORT IfAutoConnectPlugin : public IfPlugin {
protected:
    void pluginLoaded(const QString& name, QObject * plugin) override final;
    void pluginUnloaded(QObject * plugin) override final;
    IfAutoConnectPlugin() {}
    ~IfAutoConnectPlugin();
};

/*
 * IfWorker - выполняет какую либо работу асинхронно
 *      используется прежде всего для работы с библиотеками
 *      которые не поддерживают параллельную работу.
 *      например, контекст OpenGL либо ГИС Панорама.
 *      в этом случае клиенты ставят в очередь запросы к
 *      данному объекту и получают callbackи.
 */
class K_CORE_EXPORT IfWorker {
public:
    virtual QThread * thread() const = 0;
    virtual void async(QObject * tag, function<bool ()>&& func) = 0;
    virtual void async(QObject * tag, const function<bool ()>& func) = 0;

    virtual void sync(QObject * tag, function<bool ()>&& func) = 0;
    virtual void sync(QObject * tag, const function<bool ()>& func) = 0;

    virtual void blocking(QObject * tag, function<bool ()>&& func) = 0;
    virtual void blocking(QObject * tag, const function<bool ()>& func) = 0;

    virtual void drop(QObject * tag) = 0;
protected:
    IfWorker() {}
    virtual ~IfWorker();
};

} //namespace K;

Q_DECLARE_INTERFACE(K::IfCore,     "vnd.k.core")
Q_DECLARE_INTERFACE(K::IfPlugin,   "vnd.k.plugin")
Q_DECLARE_INTERFACE(K::IfWorker,   "vnd.k.worker")




