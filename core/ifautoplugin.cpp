#include "interfaces.h"
#include "utils.h"
#include <QMetaObject>
#include <QMetaMethod>


//make sure compiler will create a vtable for interfaces
K::IfAutoConnectPlugin::~IfAutoConnectPlugin()
{

}

void K::IfAutoConnectPlugin::pluginLoaded(const QString& name, QObject * plugin) {
    Q_UNUSED(name);
    IfPlugin::autoConnect(plugin);
}

void K::IfAutoConnectPlugin::pluginUnloaded(QObject * plugin) {
    QObject::disconnect(self(), 0, plugin, 0);
    QObject::disconnect(plugin, 0, self(), 0);
}

