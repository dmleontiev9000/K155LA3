#include "panel.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QToolButton>
#include "../coregui/style.h"
#include "../coregui/icons.h"
#include <QReadWriteLock>
using namespace K::Gui::Internal;
using namespace K::Core;

Panel::Panel(int tbh, QWidget *parent) : QWidget(parent)
{
    Style  * style = Style::instance();
    mToolbarHeight = tbh;
    QVBoxLayout * vbox = new QVBoxLayout(this);
    vbox->setSpacing(0);
    vbox->setMargin(0);

    mToolbarStack = new QStackedWidget;
    mWidgetStack  = new QStackedWidget;
    Style::instance()->addPanel(mToolbarStack, Style::Toolbar);

    QHBoxLayout * hbox = new QHBoxLayout;
    hbox->addWidget(mToolbarStack, 1);
    hbox->setMargin(0);
    hbox->setSpacing(0);
    vbox->addLayout(hbox);
    vbox->addWidget(mWidgetStack, 1);
    mToolbarHBox = hbox;

    mNullToolbar = new QWidget;
    mNullToolbar->setFixedHeight(mToolbarHeight);
    mToolbarStack->addWidget(mNullToolbar);
    style->addPanel(mToolbarStack, K::Core::Style::Toolbar);

    mNullWidget  = new QWidget;
    mWidgetStack->addWidget(mNullWidget);
    //style->addPanel(mWidgetStack, K::Core::Style::Reset);

    mCurrent.widget = mNullWidget;
    mCurrent.toolbar= mNullToolbar;
    mCurrent.visible= false;
}
void Panel::addToolWidget(QToolButton *w, bool first) {
    Style  * style = Style::instance();
    style->addPanel(w, Style::Flat);
    w->setIconSize(QSize(mToolbarHeight-2,mToolbarHeight-2));
    mToolbarHBox->insertWidget(first ? 0:-1, w);
}
void Panel::addWidget(const QString &id, QWidget *w, QWidget *t) {
    WidgetPair wp;
    wp.toolbar = t ? t : mNullToolbar;
    wp.widget  = w ? w : mNullWidget;
    wp.visible = true;
    mWidgetStack->addWidget(w);
    if (t) mToolbarStack->addWidget(t);
    mWidgets.insert(id, wp);
}
bool Panel::setCurrent(const QString &id, bool show) {
    auto i = mWidgets.find(id);
    if (i != mWidgets.end()) {
        mCurrentId = id;
        mCurrent = i.value();
        mCurrent.visible |= show;
        mToolbarStack->setCurrentWidget(mCurrent.toolbar);
        mWidgetStack->setCurrentWidget(mCurrent.widget);
        setVisible(mCurrent.visible);
    } else if (show) {
        mCurrentId.clear();
        mCurrent.widget = mNullWidget;
        mCurrent.toolbar= mNullToolbar;
        mCurrent.visible= false;
        setVisible(mCurrent.visible);
    }
    return mCurrent.widget != mNullWidget;
}
void Panel::removeWidget(const QString &id) {
    if (id == mCurrentId) {
        mCurrentId.clear();
        mCurrent.widget = mNullWidget;
        mCurrent.toolbar= mNullToolbar;
        mCurrent.visible= false;
        mToolbarStack->setCurrentWidget(mNullToolbar);
        mWidgetStack->setCurrentWidget(mNullWidget);
        setVisible(false);
    }
    auto i = mWidgets.find(id);
    if (i != mWidgets.end()) {
        if (i.value().widget != mNullWidget) {
            mWidgetStack->removeWidget(i.value().widget);
            delete i.value().widget;
        }
        if (i.value().toolbar != mNullToolbar) {
            mToolbarStack->removeWidget(i.value().toolbar);
            delete i.value().toolbar;
        }
        mWidgets.erase(i);
    }
}
void Panel::setVisibleEx(bool v)
{
    if (mCurrent.widget != mNullWidget) {
        mCurrent.visible = v;
        QWidget::setVisible(v);
    }
}

