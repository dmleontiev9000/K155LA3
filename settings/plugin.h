#pragma once

#include "../core/interfaces.h"
#include "../gui/conventions.h"
#include "conventions.h"

#include <QStackedWidget>
#include <QScopedPointer>
#include <QListWidget>
#include <QHash>


#ifndef IDE_LIBRARY
#error "this header is internal"
#endif

namespace K {
namespace IDE {

class Plugin
        : public QObject
        , public K::IfPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "k.settings")
    Q_INTERFACES(K::IfPlugin)
public:
    Plugin();
    ~Plugin();
signals:
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

    K_GUI_ADD_WIDGET
    void addWidget(QScopedPointer<QWidget>* widget,
                   QScopedPointer<QWidget>* toolbar,
                   const QString& widgetid,
                   int position);

    K_GUI_REMOVE_WIDGET
    void removeWidget(const QString& widgetid);
    K_GUI_ACTIVATE_MODE
    void activateMode(const QString& widgetid,
                      bool exclusive);
public slots:
    K_SETTINGS_ADD_PANEL
    void addPanel(QScopedPointer<QWidget>* panel,
                  const QIcon&   icon,
                  const QString& title,
                  const QString& widgetid);
    K_SETTINGS_REMOVE_PANEL
    void removePanel(const QString& widgetid);
    K_SETTINGS_SHOW_PANEL
    void showPanel(const QString& widgetid);
private:
    K_DECLARE_PLUGIN_METHODS

    QString               mErrorString;

    struct ButtonEntry : public QListWidgetItem {
        ButtonEntry(const QIcon& icon,
                    const QString& text,
                    QListWidget * widget) :
            QListWidgetItem(icon, text, widget, QListWidgetItem::UserType)
        {}
        QWidget * panel;
    };
    QWidget             * mSettingsPanel = nullptr;
    QListWidget         * mPanelsSwitcher= nullptr;
    QStackedWidget      * mPanelsStack   = nullptr;
    QWidget             * mDummy         = nullptr;
    QHash<QString, ButtonEntry*> mPanels;
};

} //namespace IDE
} //namespace K
