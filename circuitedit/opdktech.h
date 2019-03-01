#pragma once

#include "tech.h"
#include <QFuture>

namespace K {
namespace CE {
namespace OPDK {

class Tech : public CE::Tech
{
    Q_OBJECT
public:
    static Tech * import(const QString &path, const QString& original, QObject *parent);
    Tech(const QString &path, QObject *parent);
    ~Tech();

    const QString&   name() const { return mName; }
    const QString&   description() const { return mDesc; }
    const QIcon&     icon() const;
    const QFileInfo& info() const { return mFileInfo; }

    bool  modified() const { return mModified; }
    bool  isValid() const { return mOk; }

    bool  load();
    bool  save(const QString * as = nullptr);
signals:
    void techChanged(Tech * tech);
    void techRemoved(Tech * tech);
    void techUsable(Tech * tech, bool);
private:
    bool           mOk       = false;
    bool           mModified = false;
    bool           mReadonly = false;
    QFileInfo      mFileInfo;
    QString        mAbsPath;
    QString        mDesc;
    QString        mName;

    Element* createLayer(const char * name);
    Element* createSublayer(const char * name);
    Element* createDevice(const char * name);
    Element* createRule(const char * name);
    Element* createParameter(const char * param);
};
class LambdaBasedTech : public Tech {
public:
};
class OPDKBasedTech : public Tech {
public:
    OPDKBasedTech(const QFileInfo& path, QString *errorString, QObject * parent);
};

} //namespace OPDK
} //namespace CE
} //namespace K
