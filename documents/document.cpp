#include "document.h"
#include <QMessageBox>
#include <QTimerEvent>
#include <QMessageBox>
#include <QEventLoop>
#include <QElapsedTimer>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QSet>
#include <QSaveFile>
#include <QFileInfo>
#include <QTimerEvent>
#include <QAbstractButton>
#include <style.h>

namespace K { namespace IDE {

enum class State {
    Invalid,                //cannot be loaded
    Unloaded,               //exists but not loaded
    Valid,                  //exists and loaded
    Creating,               //now creating
    Loading,                //now loading
    Saving,                 //now saving
    BackupSaving,           //now saving a backup copy
};

class DocumentPrivate {
public:
    DocumentPrivate() {}

    State      mState       = State::Invalid;
    bool       mCanRename   = false;
    bool       mRecovered   = false;
    bool       mReadonly    = false;
    bool       mChanged     = false;
    QString    mFilePath;
    QString    mFileName;
    QString    mErrorString;
    QIcon      mIcon;
    qint64     mLastChanged = 0;
    qint64     mLastSaved   = 0;

    QFile     *mLoadDevice  = nullptr;
    QSaveFile *mSaveDevice  = nullptr;
    int        mTimerAS     = -1;
    int        mTimerIO     = -1;
    int        mAutosaveTimeout = -1;
};
struct DocumentTimer : public QElapsedTimer {
    DocumentTimer() { start(); }
};
static DocumentTimer elapsedTimer;

Document::Document(const QString &path,
                   bool           canRename,
                   QObject       *parent)
    : QObject(parent)
    , d(new DocumentPrivate)
{
    QFileInfo info(path);
    d->mFilePath  = info.absoluteFilePath();
    d->mFileName  = info.fileName();
    d->mCanRename = canRename;
}
bool Document::createFile() {
    if (d->mState != State::Invalid)
        return false;

    Q_ASSERT(d->mLoadDevice == nullptr);
    Q_ASSERT(d->mSaveDevice == nullptr);
    auto dev = new QSaveFile(d->mFilePath);
    if (!dev->open(QIODevice::WriteOnly)) {
        delete dev;
        return false;
    }

    d->mState = State::Creating;
    d->mSaveDevice = dev;

    return handleIOResult(beginCreate(dev, &d->mErrorString));
}
bool Document::checkFile() {
    if (d->mState != State::Invalid)
        return false;

    Q_ASSERT(d->mLoadDevice == nullptr);
    Q_ASSERT(d->mSaveDevice == nullptr);

    QFileInfo info1(d->mFilePath);
    QFileInfo info2(d->mFilePath + ".autosave");

    if (!info1.exists())
        return false;

    if (!info1.isWritable())
        d->mReadonly = true;

    //autosave file exists
    if (info2.exists()) {
        if (info2.lastModified() >= info1.lastModified()) {
            d->mRecovered = true;
            d->mChanged   = true;
        } else {
            info2.dir().remove(info2.fileName());
        }
    }
    d->mState = State::Unloaded;
    return true;
}
Document::~Document() {
    emit closed(this);
    delete d->mLoadDevice;
    delete d->mSaveDevice;
    delete d;
}
/*********************************************
 * LOADING
 *********************************************/
bool Document::loadFile(bool reload)
{
    switch(d->mState) {
    //states in which document must be in some valid state
    case State::Saving:
        if (!reload)
            return true;
        if (!finishIO())
            return false;
        goto __unloaded;

    case State::BackupSaving:
        if (!reload)
            return true;
        delete d->mSaveDevice;
        d->mSaveDevice = nullptr;
        d->mState = State::Valid;
        goto __unloaded;

    case State::Valid:
        if (!reload) return true;
        goto __unloaded;

    case State::Unloaded: __unloaded: {
        Q_ASSERT(d->mLoadDevice == nullptr);
        Q_ASSERT(d->mSaveDevice == nullptr);
        QString path = d->mFilePath;
        if (d->mRecovered) path += ".autosave";
        auto dev = new QFile(path);
        if (!dev->open(QIODevice::ReadOnly)) {
            d->mErrorString = dev->errorString();
            delete dev;
            return false;
        }

        d->mState = State::Loading;
        d->mLoadDevice = dev;
        handleIOResult(beginLoading(dev, &d->mErrorString)); }
        return d->mState == State::Valid;

    default:
        return false;
    }
}
bool Document::saveFile(const QString *newpath)
{
    QString path = d->mFilePath;

    if (!newpath) {
        if (d->mReadonly)
            return false;
    } else {
        if (!d->mCanRename)
            return false;
        if (d->mState == State::Unloaded && !loadFile(false))
            return false;
        if (!finishIO())
            return false;
        path = d->mFilePath;
        d->mChanged = true;
    }

    switch(d->mState) {
    case State::Valid:
    case State::Saving:
    case State::BackupSaving: {
        Q_ASSERT(d->mLoadDevice == nullptr);
        if (!d->mChanged)
            return true;

        delete d->mSaveDevice;
        d->mSaveDevice = nullptr;

        auto dev = new QSaveFile(path);
        if (!dev->open(QIODevice::WriteOnly)) {
            d->mErrorString = dev->errorString();
            delete dev;
            return false;
        }

        d->mSaveDevice = dev;
        d->mState      = State::Saving;
        bool ret = handleIOResult(beginSaving(dev, &d->mErrorString));
        if (ret && newpath) {
            ret = finishIO();
            if (ret) d->mFileName = path;
        }
        return ret;}
    default:
        return false;
    }
}
bool Document::rename(const QString &newname) {
    if (newname.isEmpty())
        return false;
    if (newname[0] == '.')
        return false;
    for(int i = 0; i < newname.length(); ++i) {
        QChar c = newname[i];
        if (c.isLetterOrNumber())
            continue;
        switch(c.unicode()) {
        case '_':
        case '.':
            break;
        default:
            return false;
        }
    }

    auto dir = QFileInfo(d->mFilePath).dir();
    QString newpath = dir.absoluteFilePath(newname);
    return saveFile(&newpath);
}
void Document::unload() {
    switch(d->mState) {
    case State::Creating:
    case State::BackupSaving:
    case State::Saving:
    case State::Loading:
        finishIO();
        d->mState = State::Unloaded;
        break;
    case State::Valid:
        d->mState = State::Unloaded;
        break;

    default:;
    }
    if (d->mState == State::Unloaded)
        emit closed(this);
}
bool Document::unloadIfSaved()
{
    if (d->mChanged)
        return false;
    if (!finishIO())
        return false;
    Q_ASSERT(d->mLoadDevice == nullptr);
    Q_ASSERT(d->mSaveDevice == nullptr);
    if (d->mState == State::Valid) {
        d->mState = State::Unloaded;
        emit closed(this);
    }
    return true;
}
bool Document::setChanged()
{
    switch(d->mState) {
    case State::Valid:
    case State::Saving:
    case State::BackupSaving:
        if (!d->mChanged) {
            d->mLastChanged = elapsedTimer.elapsed();
            d->mChanged     = true;
            emit changed(this, true);
        }
        if (d->mAutosaveTimeout > 0 && d->mTimerAS < 0)
            d->mTimerAS = startTimer(d->mAutosaveTimeout*1000);
        return true;
    default:
        return false;
    }
}
bool Document::canRename() const {
    return d->mCanRename;
}
bool Document::isReadonly() const {
    return d->mReadonly;
}
bool Document::isModified() const {
    return d->mChanged;
}
bool Document::isRecovered() const {
    return d->mRecovered;
}
const QString& Document::getFilename() const {
    return d->mFileName;
}
const QString& Document::getPath() const {
    return d->mFilePath;
}
const QIcon& Document::getIcon() const {
    return d->mIcon;
}
void Document::setIcon(const QIcon &icon) {
    d->mIcon = icon;
    emit iconChanged(this);
}
QList<QAction*> Document::actions() const {
    return QList<QAction*>();
}
bool Document::makeSnapshot() {
    return false;
}

void Document::timerEvent(QTimerEvent *event) {
    if (event->timerId() == d->mTimerIO) {
        State badstate;
        switch(d->mState) {
        case State::Creating:
            badstate = State::Invalid;
            break;
        case State::BackupSaving:
        case State::Saving:
            badstate = State::Valid;
            break;
        case State::Loading:
            badstate = State::Unloaded;
            break;
        default:
            Q_ASSERT(d->mLoadDevice == nullptr);
            Q_ASSERT(d->mSaveDevice == nullptr);
            return;
        }
        auto result = continueIO(&d->mErrorString);
        switch(result) {
        case IOResult::Complete:
            d->mState = State::Valid;
            break;
        case IOResult::Error:
            d->mState = badstate;
            break;
        default:
            return;
        }

        emit changed(this, d->mChanged);
        if (d->mState != State::Valid)
            emit closed(this);

        killTimer(d->mTimerIO);
        d->mTimerIO = -1;
        return;
    }

    if (event->timerId() == d->mTimerAS) {
        killTimer(d->mTimerAS);
        d->mTimerAS = -1;

        if (!d->mChanged)
            return;

        do {
            if (d->mState != State::Valid)
                break;
            if (!makeSnapshot())
                break;

            auto dev = new QSaveFile(d->mFilePath + ".autosave");
            if (!dev->open(QIODevice::WriteOnly)) {
                delete dev;
                break;
            }

            d->mState = State::BackupSaving;
            d->mSaveDevice = dev;

            if (!handleIOResult(beginSaving(dev, &d->mErrorString)))
                break;

            return;
        } while(0);

        if (d->mAutosaveTimeout > 0)
            d->mTimerAS = startTimer(d->mAutosaveTimeout*1000);
        return;
    }
}
bool Document::finishIO() {
    State badstate;
    switch(d->mState) {
    case State::Creating:
        badstate = State::Invalid;
        break;
    case State::BackupSaving:
    case State::Saving:
        badstate = State::Valid;
        break;
    case State::Loading:
        badstate = State::Unloaded;
        break;
    default:
        Q_ASSERT(d->mLoadDevice == nullptr);
        Q_ASSERT(d->mSaveDevice == nullptr);
        return true;
    }

    qint64        starttime = elapsedTimer.elapsed();
    QEventLoop    loop;
    QMessageBox * box  = nullptr;
    bool          end  = false;
    bool          ok   = false;
    for(;;) {
        if (!box && (elapsedTimer.elapsed() - starttime > 500)) {
            box  = new QMessageBox(QMessageBox::Information,
                                   tr("Saving files..."),
                                   getPath(),
                                   QMessageBox::Cancel);
            connect(box, &QMessageBox::buttonClicked, [&end] () {end = true;});
        } else loop.processEvents();
        //user cancelled operation
        if (end) {
            endIO(false);
            ok = handleIOResult(IOResult::Error);
            break;
        }
        //operation completed?
        auto result = continueIO(&d->mErrorString);
        ok = result == IOResult::Complete;
        if (result != IOResult::WantMore)
            break;
    }
    delete box;
    delete d->mLoadDevice;
    delete d->mSaveDevice;
    if (d->mTimerAS >= 0) killTimer(d->mTimerAS);
    if (d->mTimerIO >= 0) killTimer(d->mTimerIO);
    d->mLoadDevice = nullptr;
    d->mSaveDevice = nullptr;
    d->mState = ok ? State::Valid : badstate;
    d->mTimerAS = -1;
    d->mTimerIO = -1;
    if (d->mState != State::Valid)
        emit closed(this);
    emit changed(this, d->mChanged);

    return ok;
}
bool Document::handleIOResult(IOResult ior) {
    switch(ior) {
    case IOResult::Complete:
        switch(d->mState) {
        case State::BackupSaving:
            //backup saved, successfully or not - we don't care
            Q_ASSERT(d->mLoadDevice == nullptr);
            d->mSaveDevice->commit();
            delete d->mSaveDevice;
            d->mSaveDevice = nullptr;
            d->mState = State::Valid;
            break;

        case State::Saving:
        case State::Creating:
            //if saving successfuly record save time
            Q_ASSERT(d->mLoadDevice == nullptr);
            if (!d->mSaveDevice->commit()) {
                d->mChanged   = false;
                d->mRecovered = false;
                d->mReadonly  = false;
                d->mLastSaved = elapsedTimer.elapsed();
                d->mErrorString.clear();
                //remove autosave file
                QFileInfo info(d->mFilePath + ".autosave");
                info.dir().remove(info.fileName());
            }
            delete d->mSaveDevice;
            d->mSaveDevice = nullptr;
            d->mState = State::Valid;
            break;

        case State::Loading:
            Q_ASSERT(d->mSaveDevice == nullptr);
            delete d->mLoadDevice;
            d->mLoadDevice = nullptr;
            d->mChanged    = false;
            d->mState      = State::Valid;
            break;

        default:
            Q_ASSERT(d->mLoadDevice == nullptr);
            Q_ASSERT(d->mSaveDevice == nullptr);
        }
        break;

    case IOResult::Error:
        delete d->mSaveDevice;
        d->mSaveDevice = nullptr;
        delete d->mLoadDevice;
        d->mLoadDevice = nullptr;

        switch(d->mState) {
        case State::BackupSaving:
        case State::Saving:
            d->mState = State::Valid;
            break;
        case State::Creating:
            d->mState = State::Invalid;
            break;
        case State::Loading:
            d->mState = State::Unloaded;
            break;
        default:
            Q_ASSERT(d->mLoadDevice == nullptr);
            Q_ASSERT(d->mSaveDevice == nullptr);
        }
        break;

    case IOResult::WantMore:
        switch(d->mState) {
        case State::Saving:
        case State::Loading:
        case State::Creating:
        case State::BackupSaving:
            if (d->mTimerIO < 0)
                d->mTimerIO = startTimer(0);
            break;
        default:;
        }
        break;
    }
    return ior != IOResult::Error;
}
void Document::setAutosaveInterval(int timeout_sec) {
    d->mAutosaveTimeout = timeout_sec;
    if (d->mTimerAS >= 0) {
        killTimer(d->mTimerAS);
        d->mTimerAS = -1;
    }

    if (d->mChanged && timeout_sec > 0)
        d->mTimerAS = startTimer(timeout_sec*1000);
}

}//namespace K
}//namespace IDE
