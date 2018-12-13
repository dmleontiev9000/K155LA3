#pragma once

#include <QWidget>
#include <QStackedWidget>
#include <QHash>
#include <QPair>

#ifndef GUI_LIBRARY
#error "this header is internal"
#endif

namespace K {
namespace Gui {
namespace Internal {

class Panel : public QWidget
{
    Q_OBJECT
public:
    explicit Panel(int tbh, bool hidebutton, QWidget *parent = 0);
    void addWidget(const QString& id, QWidget * w, QWidget * t);
    void removeWidget(const QString& id);
    bool setCurrent(const QString& id);
    bool isCurrent(const QString& id);
    void setVisibility(bool);
    void pushVisibility();
    void popVisibility();
    const QString& currentId() const;
signals:
    void hidePanel();
private:
    struct WidgetPair { QWidget * toolbar;
                        QWidget * widget;};
    typedef QHash<QString, WidgetPair> WidgetMap;
    QStackedWidget     * mWidgetStack    = nullptr;
    QStackedWidget     * mToolbarStack   = nullptr;
    QWidget            * mNullToolbar    = nullptr;
    QWidget            * mNullWidget     = nullptr;
    WidgetMap            mWidgets;
    int                  mToolbarHeight;
    bool                 mSavedVisibility = false;
};

} //namespace Internal
} //namespace Gui
} //namespace K
