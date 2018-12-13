#include "plugin.h"
#include "../gui/conventions.h"
#include "../coregui/style.h"
#include <QHBoxLayout>
using namespace K::IDE;
using K::Core::Style;

Plugin::Plugin()
{

}
Plugin::~Plugin()
{
}
QObject * Plugin::self()
{
    return this;
}
bool Plugin::requiresGui() const
{
    return true;
}
QString Plugin::errorString()
{
    return mErrorString;
}
static const QString widgetId("k.ide.settings");
bool Plugin::init()
{
    if (!loadPlugins({"gui"})) {
        mErrorString = "failed to load gui plugin";
        return false;
    }
    auto style = Style::instance();

    /*create settings panel*/ {
        mSettingsPanel = new QWidget;
        style->addPanel(mSettingsPanel, Style::Reset);
        mSettingsPanel->setPalette(QPalette(Qt::white));
        mSettingsPanel->setAutoFillBackground(true);

        connect(mSettingsPanel, &QObject::destroyed,
        [=] () {
            mSettingsPanel  = nullptr;
            mPanelsSwitcher = nullptr;
            mPanelsStack    = nullptr;
            mDummy          = nullptr;
        });
        auto layout = new QHBoxLayout(mSettingsPanel);
        mPanelsSwitcher = new QListWidget;
        mPanelsSwitcher->setMinimumWidth(150);
        mPanelsStack = new QStackedWidget;
        mDummy       = new QWidget;
        mPanelsStack->addWidget(mDummy);
        layout->addWidget(mPanelsSwitcher);
        layout->addWidget(mPanelsStack, 1);
        connect(mPanelsSwitcher, &QListWidget::currentRowChanged,
        [=] (int row) {
            Q_UNUSED(row);
            auto item = mPanelsSwitcher->currentItem();
            if (item) {
                auto item1 = static_cast<ButtonEntry*>(item);
                mPanelsStack->setCurrentWidget(item1->panel);
            } else {
                mPanelsStack->setCurrentWidget(mDummy);
            }
        });

        QScopedPointer<QWidget> pWindow(mSettingsPanel);
        emit addWidget(&pWindow, nullptr,
                       widgetId,
                       GUI_POSITION_CENTER);
        emit addMode(this,
                     "settings",
                     tr("Settings"),
                     tr("Switch to configuration window"),
                     widgetId, true, GUI_MODE_LAST);
    }
    return true;
}
void Plugin::stop()
{
    emit removeMode(this);
    emit removeWidget(widgetId);
    mSettingsPanel  = nullptr;
    mPanelsSwitcher = nullptr;
    mPanelsStack    = nullptr;
    mDummy          = nullptr;
    mPanels.clear();
}
void Plugin::addPanel(QScopedPointer<QWidget> *panel,
                              const QIcon &icon,
                              const QString &title,
                              const QString &widgetid)
{
    if (!mSettingsPanel)
        return;
    auto entry = new ButtonEntry(icon, title, mPanelsSwitcher);
    mPanels.insert(widgetid, entry);
    entry->panel = panel->data();
    mPanelsStack->addWidget(panel->take());
}
void Plugin::removePanel(const QString &widgetid)
{
    auto i = mPanels.constFind(widgetid);
    if (i != mPanels.constEnd()) {
        auto entry = i.value();
        delete entry->panel;
        delete entry;
        mPanels.erase(i);
    }
}
void Plugin::showPanel(const QString &widgetid)
{
    auto i = mPanels.constFind(widgetid);
    if (i != mPanels.constEnd()) {
        auto entry = i.value();
        mPanelsSwitcher->setCurrentItem(entry);
    }
}

//======================================================
void Plugin::cleanup()
{
    unloadPlugins();
}
