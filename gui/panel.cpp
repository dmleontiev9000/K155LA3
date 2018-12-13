#include "panel.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QToolButton>
#include "../coregui/style.h"
#include "../coregui/icons.h"
#include <QReadWriteLock>
using namespace K::Gui::Internal;
using namespace K::Core;
Panel::Panel(int tbh, bool hidebutton, QWidget *parent) : QWidget(parent)
{
    Style  * style = Style::instance();
    mToolbarHeight = tbh;
    QVBoxLayout * vbox = new QVBoxLayout(this);
    vbox->setSpacing(0);
    vbox->setMargin(0);
    mToolbarStack = new QStackedWidget;
    mWidgetStack  = new QStackedWidget;
    Style::instance()->addPanel(mToolbarStack, Style::Toolbar);
    if (hidebutton) {
        QHBoxLayout * hbox = new QHBoxLayout;
        vbox->addLayout(hbox);
        hbox->addWidget(mToolbarStack, 1);
        hbox->setMargin(0);
        hbox->setSpacing(0);
        QToolButton * close = new QToolButton;
        style->addPanel(close, Style::Flat);
        close->setIcon(getIcon("close"));
        close->setIconSize(QSize(mToolbarHeight-2,mToolbarHeight-2));
        hbox->addWidget(close);
        connect(close, SIGNAL(clicked(bool)),
                this, SIGNAL(hidePanel()));
    } else {
        vbox->addWidget(mToolbarStack);
    }
    vbox->addWidget(mWidgetStack, 1);

    mNullToolbar = new QWidget;
    mNullToolbar->setFixedHeight(mToolbarHeight);
    mToolbarStack->addWidget(mNullToolbar);
    style->addPanel(mToolbarStack, K::Core::Style::Toolbar);

    mNullWidget  = new QWidget;
    mWidgetStack->addWidget(mNullWidget);
    //style->addPanel(mWidgetStack, K::Core::Style::Reset);
}

void Panel::addWidget(const QString &id, QWidget *w, QWidget *t) {
    WidgetPair wp;
    wp.toolbar = t;
    wp.widget  = w;
    mWidgetStack->addWidget(w);
    if (t) mToolbarStack->addWidget(t);
    mWidgets.insert(id, wp);
}
bool Panel::setCurrent(const QString &id) {
    auto i = mWidgets.find(id);
    if (i != mWidgets.end()) {
        mToolbarStack->setCurrentWidget(i.value().toolbar ?
                                            i.value().toolbar : mNullToolbar);
        mWidgetStack->setCurrentWidget(i.value().widget ?
                                           i.value().widget : mNullWidget);
        return true;
    }
    return false;
}
void Panel::removeWidget(const QString &id) {
    auto i = mWidgets.find(id);
    if (i != mWidgets.end()) {
        if (i.value().widget == mWidgetStack->currentWidget()) {
            mToolbarStack->setCurrentWidget(mNullToolbar);
            mWidgetStack->setCurrentWidget(mNullWidget);
        }
        mWidgetStack->removeWidget(i.value().widget);
        if (i.value().toolbar)
            mToolbarStack->removeWidget(i.value().toolbar);
        delete i.value().toolbar;
        delete i.value().widget;
        mWidgets.erase(i);
    }
}
void Panel::pushVisibility() {
    mSavedVisibility = isVisible();
}
void Panel::setVisibility(bool v) {
    mSavedVisibility = v;
}
void Panel::popVisibility() {
    setVisible(mSavedVisibility);
}
bool Panel::isCurrent(const QString &id) {
    auto i = mWidgets.find(id);
    if (i != mWidgets.end()) {
        if (i.value().widget == mWidgetStack->currentWidget())
            return true;
    }
    return false;
}
const QString& Panel::currentId() const {
    static QString empty;
    QWidget * w = mWidgetStack->currentWidget();
    for (auto i = mWidgets.constBegin();
         i != mWidgets.constEnd(); ++i) {
        if (i.value().widget == w)
            return i.key();
    }
    return empty;
}
