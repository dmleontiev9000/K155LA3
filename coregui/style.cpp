#include "style.h"
#include <QWidget>
#include <QPainter>
#include <QBrush>
#include <QStyleOption>
#include <QApplication>
#include <QHash>
#include <QFile>
#include <QFileIconProvider>
#include <QFontMetrics>
#include <QDebug>
using namespace K::Core;

Style::Style() {
}

Style * Style::instance() {
    static Style * s = nullptr;
    if (!s) {
        s = new Style();
        qApp->setStyle(s);
    }
    return s;
}

void Style::setIconSize(int is) {
    if (is < 24)
        is = 16;
    else if (is < 32)
        is = 24;
    else if (is < 48)
        is = 32;
    else if (is < 64)
        is = 48;
    else is = 64;
    iconsize = is;
}
QSize Style::getIconSize() const {
    return QSize(iconsize, iconsize);
}
bool Style::copyOption(const QStyleOption *opt, char *buffer) const {
    int size = 0;
    switch(opt->type) {
    case QStyleOption::SO_Default:
        size = sizeof(QStyleOption);break;
    case QStyleOption::SO_Button:
        size = sizeof(QStyleOptionButton); break;
    case QStyleOption::SO_ComboBox:
        size = sizeof(QStyleOptionComboBox); break;
    case QStyleOption::SO_Complex:
        size = sizeof(QStyleOptionComplex); break;
    case QStyleOption::SO_ToolButton:
        size = sizeof(QStyleOptionToolButton); break;
    default:;
    }
    if (size) {
        Q_ASSERT(size < 1024);
        memcpy(buffer, opt, size);
        return true;
    }

    return false;
}

int Style::pixelMetric(PixelMetric metric,
                       const QStyleOption *option,
                       const QWidget *widget) const
{
    switch (metric) {
    case PM_LayoutHorizontalSpacing:
    case PM_LayoutVerticalSpacing:
    case PM_LayoutBottomMargin:
    case PM_LayoutLeftMargin:
    case PM_LayoutRightMargin:
    case PM_LayoutTopMargin:
        return 0;
    case PM_ToolBarFrameWidth:
    case PM_ToolBarHandleExtent:
    case PM_ToolBarItemSpacing:
    case PM_ToolBarItemMargin:
        return 2;
    default:;
    }

    const QWidget * wi  = widget;
    while(wi) {
        if (panels.contains(wi))
            break;
        wi  = wi->parentWidget();
    }

    if (!wi)
        return QProxyStyle::pixelMetric(metric, option, widget);

    const Features& m = panels[wi];
    switch (metric) {
    case PM_ToolBarIconSize:
    case PM_ButtonIconSize:
    case PM_IconViewIconSize:
        return m.iconsize;
    default:
        return QProxyStyle::pixelMetric(metric, option, widget);
    }
}

void Style::drawComplexControl(ComplexControl control,
                               const QStyleOptionComplex *option,
                               QPainter *painter,
                               const QWidget *widget) const
{
    const QObject * obj = static_cast<const QObject*>(widget);
    auto i   = panels.constFind(obj);
    if (i == panels.constEnd()) {
        QProxyStyle::drawComplexControl(control, option, painter, widget);
        return;
    }

    char overrideopt[1024];
    if (i.value().mode & DontShade) {
        if (copyOption(option, overrideopt)) {
            QStyleOption* so = (QStyleOption*)overrideopt;
            so->state &= ~(
                    QStyle::State_MouseOver|
                    QStyle::State_Raised|
                    QStyle::State_Selected|
                    QStyle::State_Sunken|
                    QStyle::State_AutoRaise
                        );
            option = (const QStyleOptionComplex*)so;
        }
    }

    QProxyStyle::drawComplexControl(control, option, painter, widget);
}

void Style::drawPrimitive(PrimitiveElement element,
                          const QStyleOption *option,
                          QPainter *painter,
                          const QWidget *widget) const
{
    const QObject * obj = static_cast<const QObject*>(widget);
    auto i   = panels.constFind(obj);
    bool hit = true;
    const QWidget * wi  = widget;

    while(i == panels.constEnd())
    {
        if (i.value().mode & Reset) {
            QProxyStyle::drawPrimitive(element,
                                       option,
                                       painter,
                                       widget);
            return;
        }

        hit = false;
        wi  = wi->parentWidget();
        if (!wi) {
            QProxyStyle::drawPrimitive(element,
                                       option,
                                       painter,
                                       widget);
            return;
        }
        i = panels.find(static_cast<const QObject*>(wi));
    }

    char overrideopt[1024];
    if (i.value().mode & DontShade) {
        if (copyOption(option, overrideopt)) {
            QStyleOption* so = (QStyleOption*)overrideopt;
            so->state &= ~(
                    QStyle::State_MouseOver|
                    QStyle::State_Raised|
                    QStyle::State_Selected|
                    QStyle::State_Sunken|
                    QStyle::State_AutoRaise
                        );
            option = (const QStyleOptionComplex*)so;
        }
    }

    switch(element) {
    case PE_Widget:
    case PE_PanelButtonTool:
    case PE_PanelButtonBevel:
    case PE_PanelButtonCommand:
    case PE_PanelMenu:
    case PE_PanelMenuBar:
    case PE_PanelToolBar:
    case PE_PanelStatusBar:
    case PE_PanelScrollAreaCorner:
        drawBackground(option, painter, i.value(), hit);
        break;
    case PE_FrameLineEdit:
        element = PE_PanelButtonCommand;goto dfl;
    default:dfl:
        QProxyStyle::drawPrimitive(element,
                                   option,
                                   painter,
                                   widget);
    }
}
void Style::addPanel(QWidget *panel,
                     Mode mode,
                     int is) {
    if (is < 0)
        is = iconsize;
    if (!panels.contains(panel)) {
        connect(panel, SIGNAL(destroyed(QObject*)),
                this, SLOT(objectRemoved(QObject*)));
        Features f;
        f.color = QColor(Qt::white);
        f.iconsize = is;
        f.mode = mode;
        panels.insert(panel, f);

        if (mode & Toolbar)
            panel->setFixedHeight(is+8);
    }
}
void Style::setColor(QWidget *panel, QColor color) {
    auto i = panels.find(panel);
    if (i != panels.end()) {
        i.value().color = color;
    }
}

void Style::objectRemoved(QObject *object) {
    panels.remove(object);
}

void Style::drawBackground(const QStyleOption * option,
                           QPainter * painter,
                           const Features &f,
                           bool hit) const
{
    bool enabled = option->state & State_Enabled;
    bool on      = option->state & State_On;
    bool hover   = option->state & State_MouseOver;
    on    &= enabled;
    hover &= enabled;
    hover &= !on;

    bool toolbar = (f.mode & Toolbar) != 0;

    if (!on && !hover && !hit && !toolbar)
        return;

    if (f.mode & DontShade)
        hover = false;

    QPalette::ColorGroup cg = QPalette::Normal;
    QPalette::ColorRole  cr = QPalette::Window;
    if (toolbar) {
        if (!hit) {
            cr = QPalette::Button;
            cg = QPalette::Disabled;
            if (enabled) {
                cg = on ? QPalette::Active : QPalette::Inactive;
                if (hover)
                    cr = QPalette::Light;
            }
        }
    }

    painter->fillRect(option->rect, option->palette.brush(cg, cr));
}

Style::~Style()
{
//    qWarning("style was destroyed");
}
