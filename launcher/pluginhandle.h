#pragma once

#include "../core/interfaces.h"
#include <QPluginLoader>
#include <QSettings>
#include <QHash>
#include <QFileInfo>
#include <QSettings>
#include <QTranslator>

//windows defines "interface" for some reason
#undef interface

namespace K {
namespace Launcher {

class PluginHandle : public QPluginLoader
{
    Q_OBJECT
public:
    PluginHandle(const QString& name,
                 const QFileInfo& path,
                 QSettings& settings,
                 bool verbose,
                 QTranslator * translator,
                 QObject * parent);
    ~PluginHandle();
    bool use();
    void release();
    void stop();
    bool isLoaded() const {
        return mUsage > 0;
    }
    const QString& name() const {
        return mName;
    }

    const QString& errorString() const {
        return mErrorString;
    }
    QObject * instance() const {
        return mInstance;
    }
    IfPlugin * plugin() const {
        return mIfp;
    }
signals:
    void started(const QString& name, QObject * self);
    void terminated(QObject * self);
private slots:
    void otherStarted(const QString& name, QObject * obj);
    void otherTerminated(QObject * obj);
private:
    QString   mName;
    QObject * mInstance;
    IfPlugin* mIfp;
    QString   mErrorString;
    unsigned  mUsage;
    bool      mVerbose;

};

} //namespace Launcher;
} //namespace K;
