#pragma once

#include "libide_global.h"
#include <action.h>
#include <QObject>
#include <QIODevice>
#include <QSharedPointer>

namespace K {
namespace IDE {

class DocumentPrivate;

class LIBIDESHARED_EXPORT Document : public QObject
{
    Q_OBJECT
public:
    void    setAutosaveInterval(int timeout_sec);
    bool    finishIO();
    virtual ~Document();
protected:
    //initialization
    Document(const QString &path, bool canRename, QObject *parent = nullptr);
    bool             createFile();
    bool             checkFile();
public:
    enum class IOResult { Error, Complete, WantMore };

    virtual QList<QAction*> actions() const;
    //loading
    bool             loadFile(bool reload);
    //saving
    bool             saveFile(const QString * newpath = nullptr);
    bool             rename(const QString& newname);
    //unloading documents
    virtual void     unload();
    virtual bool     unloadIfSaved();

    bool             setChanged();

    bool             canRename() const;
    bool             isReadonly() const;
    bool             isModified() const;
    bool             isRecovered() const;

    const QString&   getPath() const;
    const QString&   getFilename() const;

    const QIcon& getIcon() const;
    void setIcon(const QIcon& icon);

    virtual const QStringList& editorIds() const = 0;
protected:
    //IO=================
    virtual bool makeSnapshot();

    virtual IOResult beginCreate(QIODevice * dev, QString * errorString) = 0;
    virtual IOResult beginLoading(QIODevice * dev, QString * errorString) = 0;
    virtual IOResult beginSaving(QIODevice * dev, QString * errorString) = 0;
    virtual IOResult continueIO(QString * errorString) = 0;
    virtual void     endIO(bool ok) = 0;
signals:
    void changed(Document * doc, bool modified);
    void closed(Document * doc);
    void renamed(Document * doc);
    void iconChanged(Document * doc);
private:
    DocumentPrivate* d;
    bool handleIOResult(IOResult ior);
    void timerEvent(QTimerEvent *event);

    Q_DISABLE_COPY(Document)
};

} //namespace IDE
} //namespace K
