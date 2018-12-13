#pragma once

#include <QHash>
#include <QPair>
#include <QObject>
#include <QWidget>
#include <QScopedPointer>
#include <QActionGroup>
#include <QStackedWidget>
#include <QTreeWidget>
#include <QSplitter>
#include <QKeySequence>
#include <QSettings>
#include "../core/interfaces.h"
#include "../coregui/style.h"
#include "../coregui/switchbar.h"
#include "panel.h"
#include "conventions.h"
#include <QOffscreenSurface>

#ifndef GUI_LIBRARY
#error "this header is internal"
#endif


namespace K {
namespace Gui {
namespace Internal {
class Plugin
        : public QObject
        , public IfPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "k.gui")
    Q_INTERFACES(K::IfPlugin)
    Q_PROPERTY(QString style
               READ style
               WRITE setStyle
               FINAL)
    Q_PROPERTY(QVariant windowSize
               READ windowSize
               WRITE setWindowSize
               NOTIFY windowSizeChanged)
    Q_PROPERTY(QVariant windowPosition
               READ windowPosition
               NOTIFY windowPositionChanged)
public:
    Plugin();
    ~Plugin();

    const QString& style() const { return mStyle; }
    void setStyle(const QString& style);

    QVariant windowSize() const;
    void setWindowSize(const QVariant& size);

    QVariant windowPosition() const;

    bool __aboutToClose();
    void __geometryUpdated();
    void __hidePopups();
public slots:
    K_GUI_ADD_MODE
    void addMode(QObject * object,
                 const QString& iconname,
                 const QString& title,
                 const QString& tooltip,
                 const QString& widgetid,
                 bool exclusive,
                 int order);
    K_GUI_REMOVE_MODE
    void removeMode(QObject* object);

    K_GUI_ACTIVATE_MODE
    void activateMode(const QString& widgetid,
                      bool exclusive);
    K_GUI_HIDE_MODE
    void hideMode(const QString& widgetid);

    K_GUI_ADD_WIDGET
    void addWidget(QScopedPointer<QWidget>* widget,
                   QScopedPointer<QWidget>* toolbar,
                   const QString& widgetid,
                   int position);

    K_GUI_REMOVE_WIDGET
    void removeWidget(const QString& widgetid);

    K_GUI_ADD_ACTION
    void addAction(K::Core::Action * action);
    K_GUI_REMOVE_ACTION
    void removeAction(K::Core::Action * action);
signals:
    K_GUI_WANT_CLOSE
    void wantClose(QTreeWidget * tree);
    K_GUI_SAVE_DATA
    void willClose();
    K_GUI_GEOMETRY_CHANGE
    void geometryChanged(const QRect& pos);
    K_GUI_HIDE_POPUPS
    void hidePopups();

    void windowSizeChanged(const QVariant& size);
    void windowPositionChanged(const QVariant& pos);
private slots:
    void modeActivated(bool);
    void toggleLeft();
    void toggleRight();
    void hideLeft();
    void hideRight();
private:
    K_DECLARE_PLUGIN_METHODS

    QWidget            * mWindow   = nullptr;
    Core::SwitchBar    * mSwitchBar;
    Core::SwitchBar    * mActionBar;
    int                  mToolbarHeight;

    struct ActionPair { Core::Action * action;
                        QString   widgetid;
                        bool      exclusive; };
    typedef QHash<QObject*,ActionPair> ActionMap;

    void activateProperOption();
    bool eventFilter(QObject *, QEvent *);

    QSplitter          * mSplitter;
    Panel              * mLeftPanel;
    Panel              * mRightPanel;
    Panel              * mCentralPanel;
    bool                 mExclusive;

    ActionMap            mActions;
    int                  mModeIndex0;
    int                  mModeIndex1;
    int                  mModeIndex2;

    QString              mStyle;
    QSize                mWindowSize;
    QPoint               mWindowPosition;
    QString              mErrorString;
    QSettings            mSettings;
};
} //namespace Internal;
} //namespace Gui;
} //namespace K;



