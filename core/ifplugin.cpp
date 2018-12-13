#include "interfaces.h"
#include <QDebug>
K::IfPlugin::~IfPlugin()
{

}

void K::IfPlugin::pluginLoaded(const QString &, QObject *)
{

}

void K::IfPlugin::pluginUnloaded(QObject *plugin)
{
    QObject::disconnect(self(), 0, plugin, 0);
    QObject::disconnect(plugin, 0, self(), 0);
}

bool K::IfPlugin::loadPlugins(const QStringList &plugins) {
    IfCore * core = IfCore::instance();
    bool fail = false;

    for(int n = 0; n < plugins.size(); ++n) {
        if (plugins[n].endsWith(QChar('?')))
        {
            QString p = plugins[n].left(plugins[n].length()-1);
            if (p.isEmpty())
                continue;

            QObject * o = core->loadPlugin(plugins[n]);
            if (o) {
                mPlugins.append(p);
                mInstances.append(o);
            }
        } else {
            QObject * o = core->loadPlugin(plugins[n]);
            if (!o) {
                fail = true;
                break;
            }
            mPlugins.append(plugins[n]);
            mInstances.append(o);
        }
    }

    if (!fail) {
        for(QObject * o : mInstances)
            autoConnect(o);
        return true;
    } else {
        QObject * s = self();
        for(QObject * o : mInstances) {
            QObject::disconnect(s, 0, o, 0);
            QObject::disconnect(o, 0, s, 0);
        }
        for(QString p : mPlugins) {
            core->unloadPlugin(p);
        }
        mInstances.clear();
        mPlugins.clear();
        return false;
    }
}
void K::IfPlugin::unloadPlugins() {
    QObject * s = self();
    for(QObject * o : mInstances) {
        QObject::disconnect(s, 0, o, 0);
        QObject::disconnect(o, 0, s, 0);
    }
    IfCore * core = IfCore::instance();
    for(int n = 0; n < mPlugins.size(); ++n) {
        core->unloadPlugin(mPlugins[n]);
    }
}

void K::IfPlugin::autoConnect(QObject *peer) {
    autoConnectByTags(self(), peer, Qt::DirectConnection);
    autoConnectByTags(peer, self(), Qt::DirectConnection);
}
