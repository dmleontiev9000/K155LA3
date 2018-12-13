#include <plugin.h>
#include <style.h>
#include <icons.h>
#include <QApplication>
#include <QFontMetrics>
#include <QPainter>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QStackedLayout>
#include <QPushButton>
#include <QLabel>
#include <QDialog>
#include <QFile>
#include <QCloseEvent>
#include <QMoveEvent>
#include <QResizeEvent>
#include <QDebug>
#include <QPainter>
#include <QtPlugin>
#include "../core/interfaces.h"
#include "../core/utils.h"

using namespace K;
using namespace K::Gui::Internal;

using K::Core::Style;
/*****************************************
 * basic functions                       *
 *****************************************/
static const char TRCTX[] = {"gui"};
Plugin::Plugin()
    : mSettings(QSettings::UserScope, "org.k155la3")
{
    mSettings.beginGroup("ide");
}
Plugin::~Plugin()
{
}
QObject * Plugin::self() {
    return this;
}
bool Plugin::requiresGui() const {
    return true;
}
QString Plugin::errorString() {
    return mErrorString;
}
void Plugin::setStyle(const QString &style) {
    mStyle = style;
}
QVariant Plugin::windowPosition() const {
    return QVariant::fromValue(mWindowPosition);
}
QVariant Plugin::windowSize() const {
    return QVariant::fromValue(mWindowSize);
}
void Plugin::setWindowSize(const QVariant &size) {
    QSize s;
    if (size.type() == QVariant::Size) {
        s = size.toSize();
    } else {
        QStringList dim = size.toString().split('x');
        //accept WxH
        if (dim.size() != 2)
            return;

        int width  = dim[0].toInt();
        int height = dim[1].toInt();
        if (width <= 0 || height <= 0)
            return;

        s = QSize(width, height);
    }
    if (s.isEmpty())
        return;
    if (mWindowSize != s) {
        if (mWindow) {
            mWindow->resize(s);
            s = mWindow->size();
        }
        if (mWindowSize != s) {
            mWindowSize = s;
            emit windowSizeChanged(s);
        }
    }
}
/*****************************************
 * closing window                        *
 *****************************************/
namespace {
    class MainWidget : public QWidget {
    public:
        MainWidget(Plugin * g) : gui(g) {
        }
        void closeEvent(QCloseEvent * ev) {
            if (gui->__aboutToClose()) {
                ev->accept();
            } else {
                ev->ignore();
            }
        }
        void resizeEvent(QResizeEvent * ev) {
            QWidget::resizeEvent(ev);
            gui->__geometryUpdated();
        }

        void moveEvent(QMoveEvent * ev) {
            QWidget::moveEvent(ev);
            gui->__geometryUpdated();
        }
        void hideEvent(QHideEvent * ev) {
            gui->__hidePopups();
            QWidget::hideEvent(ev);
        }
        void focusOutEvent(QFocusEvent * ev) {
            gui->__hidePopups();
            QWidget::focusOutEvent(ev);
        }
    private:
        Plugin * gui;
    };
}
bool Plugin::__aboutToClose() {
    QTreeWidget * unsaved_items = new QTreeWidget;
    emit wantClose(unsaved_items);
    if (!unsaved_items->topLevelItemCount()) {
        IfCore::instance()->quit();
        return true;
    }
    __hidePopups();

    QDialog dialog(mWindow);
    dialog.resize(400, 300);
    QVBoxLayout * vbox = new QVBoxLayout;
    vbox->addWidget(new QLabel(tr("The following files are not saved:", TRCTX)));
    vbox->addWidget(unsaved_items, 1);
    QHBoxLayout * hbox = new QHBoxLayout;
    vbox->addLayout(hbox);
    hbox->addStretch(1);
    QPushButton * dontsave = new QPushButton(tr("Don't save", TRCTX));
    QPushButton * cancel   = new QPushButton(tr("Cancel", TRCTX));
    QPushButton * saveall  = new QPushButton(tr("Save everything", TRCTX));
    saveall->setDefault(true);
    hbox->addWidget(dontsave);
    hbox->addWidget(cancel);
    hbox->addWidget(saveall);
    dialog.setLayout(vbox);
    connect(dontsave, SIGNAL(clicked(bool)), &dialog, SLOT(accept()));
    connect(cancel, SIGNAL(clicked(bool)), &dialog, SLOT(reject()));
    connect(saveall, SIGNAL(clicked(bool)), this, SIGNAL(willClose()));
    connect(saveall, SIGNAL(clicked(bool)), &dialog, SLOT(accept()));
    if (dialog.exec() == QDialog::Accepted) {
        IfCore::instance()->quit();
        return true;
    }
    return false;
}
void Plugin::__geometryUpdated() {
    QWidget * w = mCentralPanel;
    auto g  = w->geometry();
    QRect rect(w->mapToGlobal(g.topLeft()), w->mapToGlobal(g.bottomRight()));
    int dx = qBound(0, (rect.width()-60)/2, 20);
    int dy = qBound(0, (rect.height()-60)/2, 20);
    rect.adjust(dx,dy,-dx,-dy);
    emit geometryChanged(rect);
    if (!mSplitter) {
        mLeftPanel->setGeometry(rect);
        mRightPanel->setGeometry(rect);
    }
}
void Plugin::__hidePopups() {
    if (!mSplitter) {
        mLeftPanel->hide();
        mRightPanel->hide();
    }
    emit hidePopups();
}
void Plugin::toggleLeft() {
    if (mSplitter)
        return;
    mLeftPanel->show();
}
void Plugin::toggleRight() {
    if (mSplitter)
        return;
    mRightPanel->show();
}

bool Plugin::eventFilter(QObject * o, QEvent * e) {
    if (e->type() == QEvent::KeyPress ||
        e->type() == QEvent::KeyRelease)
    {
        QKeyEvent * ke = static_cast<QKeyEvent*>(e);
        if (ke->key() == Qt::Key_Escape && e->type() == QEvent::KeyRelease)
        {
            if (mExclusive && !mCentralPanel->isVisible())
            {
                mLeftPanel->popVisibility();
                mRightPanel->popVisibility();
                mCentralPanel->show();
                activateProperOption();
                return true;
            }
        }
    }
    return QObject::eventFilter(o, e);
}

/*****************************************
 * initialization                        *
 *****************************************/
bool Plugin::init() {
    QApplication* app = qApp;
    if (!app) {
        mErrorString = tr("not in GUI mode", TRCTX);
        return false;
    }

    Style  * style = Style::instance();

    mExclusive = false;

    mWindow = new MainWidget(this);
    QPalette  pal(QColor(63,63,63));
    mWindow->setPalette(pal);
    mWindow->setAutoFillBackground(true);
    mWindow->setWindowIcon(Core::getIcon("project"));
    mWindow->setWindowTitle(tr("K155LA3", TRCTX));
    if (!mWindowSize.isNull())
        mWindow->resize(mWindowSize);
    style->addPanel(mWindow, Style::Flat);
    mWindow->installEventFilter(this);

    int fontHeight = app->fontMetrics().height() + 6;

    mWindow->setWindowState(Qt::WindowMaximized);
    mToolbarHeight = qMax(16, fontHeight);
    style->setIconSize(mToolbarHeight);

    QHBoxLayout * hbox = new QHBoxLayout(mWindow);

    mSwitchBar = new Core::SwitchBar(32, Core::SwitchBar::Edge::Left,
                                        QSizePolicy::MinimumExpanding, true);
    mActionBar = new Core::SwitchBar(32, Core::SwitchBar::Edge::Left,
                                        QSizePolicy::Maximum);
    style->addPanel(mSwitchBar, Style::Flat);
    style->addPanel(mActionBar, Style::Flat);

    QVBoxLayout * vb1 = new QVBoxLayout;
    QWidget     * padding = new QWidget;
    padding->setPalette(pal);
    padding->setAutoFillBackground(true);
    style->addPanel(padding, Style::Toolbar);

    vb1->addWidget(padding);
    vb1->addWidget(mSwitchBar, 1);
    vb1->addWidget(mActionBar, 1);
    hbox->addLayout(vb1);

    mSplitter = new QSplitter;
    hbox->addWidget(mSplitter, 1);

    mLeftPanel = new Internal::Panel(mToolbarHeight, true);
    mSplitter->addWidget(mLeftPanel);
    mCentralPanel = new Internal::Panel(mToolbarHeight, false);
    mSplitter->addWidget(mCentralPanel);
    mRightPanel = new Internal::Panel(mToolbarHeight, true);
    mSplitter->addWidget(mRightPanel);
    if (!mSplitter->restoreState(mSettings.value("sw_geom").toByteArray()))
        mSplitter->setStretchFactor(1,1);
    connect(mSplitter, &QSplitter::splitterMoved, [=]() {
        mSettings.setValue("sw_geom", mSplitter->saveState());
    });

    mLeftPanel->hide();
    mRightPanel->hide();
    mWindow->show();
    mErrorString.clear();
    connect(mLeftPanel, SIGNAL(hidePanel()), this, SLOT(hideLeft()));
    connect(mRightPanel, SIGNAL(hidePanel()), this, SLOT(hideRight()));

    Q_ASSERT(mSwitchBar->parentWidget() != nullptr);
    Q_ASSERT(mActionBar->parentWidget() != nullptr);

    mModeIndex0 = 0;
    mModeIndex1 = 3200;
    mModeIndex2 = 6400;

    return true;
}
void Plugin::stop() {
    if (!mActions.isEmpty()) {
        for (auto i = mActions.constBegin();
             i != mActions.constEnd(); ++i)
        {
            mSwitchBar->removeAction(i.value().action);
            delete i.value().action;
        }
        mActions.clear();
    }
}
void Plugin::cleanup() {
    delete mWindow;
    if (!mSplitter) {
        delete mLeftPanel;
        delete mRightPanel;
    }
    mSplitter       = nullptr;
    mLeftPanel      = nullptr;
    mRightPanel     = nullptr;
    mCentralPanel   = nullptr;
    mWindow         = nullptr;
}

/*****************************************
 * user interface api                    *
 *****************************************/
void Plugin::addMode(QObject       *object,
                  const QString &iconname,
                  const QString &title,
                  const QString &tooltip,
                  const QString &widgetid,
                  bool           exclusive,
                  int            order)
{
    if (!mWindow)
        return;
    if (widgetid.isEmpty())
        return;

    switch(order) {
    case GUI_MODE_FIRST:
        order = mModeIndex0++; break;
    case GUI_MODE_MIDDLE:
        order = mModeIndex1++; break;
    case GUI_MODE_LAST:
        order = mModeIndex2++; break;
    case GUI_MODE_TAIL:
        order = 1000000; break;
    default:;
    }

    ActionPair pair;
    pair.action = new Core::Action(Core::getIcon(iconname), title, this);
    pair.action->setCheckable(true);
    pair.action->setOrder(order);
    pair.action->setToolTip(tooltip);
    pair.widgetid = widgetid;
    pair.exclusive = exclusive;

    connect(pair.action, SIGNAL(toggled(bool)), this, SLOT(modeActivated(bool)));
    K::connectSignalToTaggedSlot(pair.action, SIGNAL(toggled(bool)),
                                 object, SLOT_TAG(K_GUI_ACTIVATED));
    mActions.insertMulti(object, pair);
    mSwitchBar->addAction(pair.action, true);
    mSwitchBar->toggleFirst();
}
void Plugin::removeMode(QObject *object) {
    auto i = mActions.find(object);
    while (i != mActions.end()) {
        ActionPair pair = i.value();
        mSwitchBar->removeAction(pair.action);
        mActions.erase(i);
        i = mActions.find(object);
    }
}
void Plugin::activateMode(const QString &widgetid, bool exclusive) {
    bool haveleft = mLeftPanel->setCurrent(widgetid);
    bool haveright = mRightPanel->setCurrent(widgetid);
    bool havecenter = mCentralPanel->setCurrent(widgetid);

    //no exclusive mode on tablets
    if (!mSplitter)
        exclusive = false;

    if (exclusive) {
        if (!mExclusive) {
            mLeftPanel->pushVisibility();
            mRightPanel->pushVisibility();
        }
        if (haveleft) {
            mLeftPanel->show();
            mRightPanel->hide();
            if (mSplitter)
                mCentralPanel->hide();
        } else if (haveright) {
            mLeftPanel->hide();
            mRightPanel->show();
            if (mSplitter)
                mCentralPanel->hide();
        } else if (havecenter) {
            mLeftPanel->hide();
            mRightPanel->hide();
            mCentralPanel->show();
        } else {
            return;
        }
        mExclusive = true;
    } else if (mExclusive) {
        if (haveleft)
            mLeftPanel->setVisibility(true);
        if (haveright)
            mRightPanel->setVisibility(true);
        if (havecenter) {
            mCentralPanel->show();
            mLeftPanel->popVisibility();
            mRightPanel->popVisibility();
            mExclusive = false;
        }
    } else {
        if (haveleft)
            mLeftPanel->show();
        if (haveright)
            mRightPanel->show();
        if (havecenter)
            mCentralPanel->show();
        mExclusive = false;
    }
    if (havecenter) {
        activateProperOption();
    }
}

void Plugin::hideMode(const QString &widgetid) {
    if (!mSplitter) {
        if (mLeftPanel->isCurrent(widgetid))
            mLeftPanel->hide();
        if (mRightPanel->isCurrent(widgetid))
            mRightPanel->hide();
    } else if (mExclusive) {
        if (mLeftPanel->isCurrent(widgetid) ||
            mRightPanel->isCurrent(widgetid))
        {
            mLeftPanel->popVisibility();
            mRightPanel->popVisibility();
            mCentralPanel->show();
            activateProperOption();
        }
    }
}

void Plugin::hideLeft() {
    if (!mSplitter) {
        mLeftPanel->hide();
    } else {
        if (mExclusive) {
            if (mLeftPanel->isVisible()) {
                mLeftPanel->popVisibility();
                mRightPanel->popVisibility();
                mCentralPanel->show();
                activateProperOption();
            } else {
                mLeftPanel->setVisibility(false);
            }
        } else {
            mLeftPanel->hide();
        }
    }
}

void Plugin::hideRight() {
    if (!mSplitter) {
        mRightPanel->hide();
    } else {
        if (mExclusive) {
            if (mRightPanel->isVisible()) {
                mLeftPanel->popVisibility();
                mRightPanel->popVisibility();
                mCentralPanel->show();
                activateProperOption();
            } else {
                mRightPanel->setVisibility(false);
            }
        } else {
            mRightPanel->hide();
        }
    }
}

void Plugin::activateProperOption() {
    QString wid = mCentralPanel->currentId();
    for(auto k = mActions.constBegin();
        k != mActions.end(); ++k)
    {
        if (k.value().widgetid == wid) {
            k.value().action->setChecked(true);
            return;
        }
    }

    //no action was associated with mode, uncheck actions
    for(auto k = mActions.constBegin();
        k != mActions.end(); ++k)
    {
        k.value().action->setChecked(false);
    }

}

void Plugin::modeActivated(bool b) {
    if (!b) return;
    Core::Action * a = qobject_cast<Core::Action*>(sender());
    if (!a) return;
    for (auto i = mActions.constBegin(); i != mActions.constEnd(); ++i) {
        if (i.value().action == a) {
            activateMode(i.value().widgetid,
                         i.value().exclusive);
            break;
        }
    }
}
void Plugin::addAction(Core::Action *action) {
    mActionBar->addAction(action, false);
}
void Plugin::removeAction(Core::Action *action) {
    mActionBar->removeAction(action);
}
void Plugin::addWidget(QScopedPointer<QWidget> *widget,
                    QScopedPointer<QWidget> *toolbar,
                    const QString &widgetid,
                    int position)
{
    if (!mWindow || !widget)
        return;

    Q_ASSERT(widget->data()->parentWidget() == nullptr);
    if (toolbar) {
        Q_ASSERT(toolbar->data()->parentWidget() == nullptr);
    }

    Internal::Panel * p = mCentralPanel;
    if (position < 0) p = mLeftPanel;
    if (position > 0) p = mRightPanel;

    QWidget * w = widget->take();
    QWidget * t = toolbar ? toolbar->take() : nullptr;
    p->addWidget(widgetid, w, t);
}
void Plugin::removeWidget(const QString &widgetid) {
    mCentralPanel->removeWidget(widgetid);
    mLeftPanel->removeWidget(widgetid);
    mRightPanel->removeWidget(widgetid);
}
