#pragma once

#include <QWidget>
#include <QStackedWidget>
#include <QHBoxLayout>
#include <QToolButton>
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
    explicit Panel(int tbh, QWidget *parent = 0);
    void addWidget(const QString& id, QWidget * w, QWidget * t);
    void removeWidget(const QString& id);
    bool setCurrent(const QString& id, bool show);
    const QString& currentId() const { return mCurrentId; }
    void addToolWidget(QToolButton* w, bool first);
    void setVisibleEx(bool v);
    bool actuallyVisible() const { return mCurrent.visible; }
private:
    struct WidgetPair { QWidget * toolbar;
                        QWidget * widget;
                        bool      visible;};
    typedef QHash<QString, WidgetPair> WidgetMap;
    QStackedWidget     * mWidgetStack    = nullptr;
    QStackedWidget     * mToolbarStack   = nullptr;
    QHBoxLayout        * mToolbarHBox    = nullptr;
    QWidget            * mNullToolbar    = nullptr;
    QWidget            * mNullWidget     = nullptr;
    WidgetMap            mWidgets;
    int                  mToolbarHeight;
    QString              mCurrentId;
    WidgetPair           mCurrent;
};

} //namespace Internal
} //namespace Gui
} //namespace K
