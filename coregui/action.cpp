
#include "action.h"

namespace K { namespace Core {

class ActionPrivate {
public:
    QColor mBackground = QColor(Qt::white);
    int    mOrder = 0;
    bool   mLastVisible;

};

Action::Action(const QIcon& icon,
       const QString& text,
       QObject * parent)
    : QAction(icon,
              text,
              parent)
    , d(new ActionPrivate)
{
    d->mLastVisible = isVisible();
}
Action::Action(const QIcon& icon,
       const QString& text,
       QColor bg,
       QObject * parent)
    : QAction(icon,
              text,
              parent)
    , d(new ActionPrivate)
{
    d->mBackground = bg;
    d->mLastVisible = isVisible();

}
Action::~Action()
{
    delete d;
}

QColor Action::background() const { return d->mBackground; }
int    Action::order() const { return d->mOrder; }

void Action::setBackground(const QColor& color) {
    d->mBackground = color;
    emit changed();
}
void Action::setOrder(int order) {
    d->mOrder = order;
    emit reordered();
}
bool Action::visibilityChanged() {
    if (d->mLastVisible != isVisible()) {
        d->mLastVisible = !d->mLastVisible;
        return true;
    }
    return false;
}
bool Action::compareActions(const Action *ac1, const Action *ac2) {
    return ac1->d->mOrder < ac2->d->mOrder;
}

void Action::activateWithContext(void *context) {
    //do nothing
    Q_UNUSED(context);
}

}}
