#include "checklist.h"
#include "style.h"
#include "icons.h"
#include <QScrollArea>
#include <QToolButton>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
namespace K {
namespace Core {

class CheckListPrivate {
public:
    CheckListPrivate(){}

    struct Check {
        int              id;
        CheckList::State state;
        QToolButton    * button;
        QList<QWidget*>  widgets;
        QList<QLayout*>  layouts;
    };

    QVector<Check>           checkpoints;
    QIcon                    yes, no, xz;
    QScrollArea            * selector;
    QVBoxLayout            * selectorbox;

    QScrollArea            * sheet;
    QVBoxLayout            * sheetbox;
    int                      timer = -1;
};
CheckList::CheckList(QWidget * parent)
    : QWidget(parent)
{
    auto style  = Style::instance();
    style->addPanel(this, Style::Flat);

    d           = new CheckListPrivate;
    d->yes      = getIcon("yes");
    d->no       = getIcon("no");
    d->xz       = getIcon("right");
    auto box    = new QHBoxLayout(this);
    d->selector = new QScrollArea;
    d->selector->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    d->sheet    = new QScrollArea;
    box->addWidget(d->selector);
    box->addWidget(d->sheet, 1);
    auto selector_widget = new QWidget;
    d->selector->setWidget(selector_widget);
    d->selector->setWidgetResizable(true);
    d->selectorbox = new QVBoxLayout(selector_widget);
    auto sheet_widget = new QWidget;
    d->sheet->setWidget(sheet_widget);
    d->sheet->setWidgetResizable(true);
    d->sheetbox = new QVBoxLayout(sheet_widget);
}
CheckList::~CheckList()
{
    delete d;
}
void CheckList::addCheckPoint(int id,
                              const QString &name,
                              const QString &description,
                              State state)
{
    auto button = new QToolButton;
    button->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    button->setText(name);
    d->selectorbox->addWidget(button);

    auto label = new QLabel(description);
    d->sheetbox->addWidget(label);

    int index = d->checkpoints.size();

    CheckListPrivate::Check check;
    check.id      = id;
    check.state   = state;
    check.button  = button;
    check.widgets.append(label);
    d->checkpoints.append(check);

    connect(button, &QToolButton::clicked, [=] () {
        if (d->checkpoints[index].state != State::DISABLED) {
            d->selector->ensureWidgetVisible(d->checkpoints[index].button);
            QPoint delta = d->checkpoints[index].widgets.first()->mapTo(
                        d->sheet, QPoint(0,0));
            d->sheet->scroll(-delta.x(), -delta.y());
        }
    });
}
void CheckList::addWidget(QWidget *widget)
{
    Q_ASSERT(d->checkpoints.isEmpty() == false);
    CheckListPrivate::Check& check = d->checkpoints.last();
    check.widgets.append(widget);
    d->sheetbox->addWidget(widget);
}
void CheckList::addLayout(QLayout *layout)
{
    Q_ASSERT(d->checkpoints.isEmpty() == false);
    CheckListPrivate::Check& check = d->checkpoints.last();
    check.layouts.append(layout);
    d->sheetbox->addLayout(layout);
}
void CheckList::complete()
{
    d->selectorbox->addStretch(1);
    d->sheetbox->addStretch(1);
}
void CheckList::gotoCheckPoint(int id)
{
    for(int i = 0; i < d->checkpoints.size(); ++i) {
        if (d->checkpoints[i].id != id) continue;

        const CheckListPrivate::Check& check = d->checkpoints.at(id);
        auto pb = check.button->mapTo(this, QPoint(0,0));
        auto pl = check.widgets.first()->mapTo(this, QPoint(0,0));
        d->selector->scroll(-pb.x(), -pb.y());
        d->sheet->scroll(-pl.x(), -pl.y());
        break;
    }
}
void CheckList::setCheckPointState(int id, int count, State state)
{
    for(int i = 0; i < d->checkpoints.size(); ++i) {
        if (d->checkpoints[i].id < id) continue;
        if (d->checkpoints[i].id >= id+count) continue;

        const CheckListPrivate::Check& check = d->checkpoints.at(i);
        if (check.state != state) break;

        //get current relative position of focus widget
        QWidget * focus = focusWidget();
        QPoint pt(0,0), before(0,0), after(0,0);
        if (focus) before = focus->mapTo(this, pt);

        if (check.state == State::DISABLED || state == State::DISABLED) {
            bool on = state != State::DISABLED;
            check.button->setVisible(on);
            for(auto i = check.widgets.constBegin(); i != check.widgets.constEnd(); ++i)
                (*i)->setVisible(on);
            for(auto i = check.layouts.constBegin(); i != check.layouts.constEnd(); ++i)
                (*i)->setEnabled(on);
        }
        switch(state) {
        case State::BAD:
            check.button->setIcon(d->no);
            break;
        case State::GOOD:
            check.button->setIcon(d->yes);
            break;
        default:
            check.button->setIcon(d->xz);
            break;
        }

        d->sheetbox->update();
        if (focus) after = focus->mapTo(this, pt);
        pt = after - before;
        d->sheet->updateGeometry();
        d->sheet->scroll(pt.x(), pt.y());
    }
}


}
}
