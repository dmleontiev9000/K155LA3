#include "switchbar.h"
#include <math.h>
#include <QRectF>
#include <QBrush>
#include <QPen>
#include <QPair>
#include <QPainter>
#include <QFont>
#include <QLinearGradient>
#include <QMouseEvent>
#include <QTouchEvent>
#include <QFocusEvent>
#include <QFontMetrics>
#include <QStaticText>
#include <QTextOption>
#include <QActionGroup>
#include <QDebug>


namespace K { namespace Core {

static QPointF o(0.0, 0.0), x(1.0, 0.0), y(0.0, 1.0);
static QLinearGradient bg(SwitchBar::Edge edge) {
    QPointF start, end;
    switch(edge) {
    case SwitchBar::Edge::Left:
        start = o;
        end   = x;
        break;
    case SwitchBar::Edge::Right:
        start = x;
        end   = o;
        break;
    case SwitchBar::Edge::Top:
        start = o;
        end   = y;
        break;
    default:
        start = y;
        end   = o;
    }


    QLinearGradient grad(start, end);
    grad.setCoordinateMode(QGradient::ObjectBoundingMode);
    grad.setColorAt(0, QColor(44,44,44));
    grad.setColorAt(1, QColor(89,89,89));
    return grad;
}
static QLinearGradient scrollfill(Qt::Orientation orientation, bool tail) {
    QPointF start, end;
    if (orientation == Qt::Horizontal) {
        start = o;
        end   = x;
    } else {
        start = o;
        end   = y;
    }
    if (!tail)
        std::swap(start, end);
    QLinearGradient grad(start, end);
    grad.setColorAt(0,   QColor(119,137,231,0));
    grad.setColorAt(0.5, QColor(119,137,231,255));
    grad.setColorAt(1,   QColor(119,137,231,255));
    return grad;
}
static Qt::Orientation orientationFromEdge(SwitchBar::Edge e) {
    switch(e) {
    case SwitchBar::Edge::Left:
    case SwitchBar::Edge::Right:
        return Qt::Vertical;
    default:
        return Qt::Horizontal;
    }
}

class SwitchBarPrivate {
public:
    SwitchBarPrivate(int iconsize, SwitchBar::Edge edge)
        : mOrientation(orientationFromEdge(edge))
        , mBackground(bg(edge))
        , mHeadFill(scrollfill(mOrientation, false))
        , mTailFill(scrollfill(mOrientation, false))
        , mScrollPen(QColor(119,137,231))
        , mBorderPen(QColor(0,0,0))
        , mFont("Hevletica", ceil(72.0f*3.5f/25.4f), QFont::Bold)
        , mMetrics(mFont)
        , mTitleOption(Qt::AlignCenter)
        , mIconSize(iconsize)
        , mTotalSize(0)
    {
        mScrollPen.setWidth(2);
        mBorderPen.setWidth(0);
    }

    Qt::Orientation   mOrientation;
    QBrush            mBackground;
    QBrush            mHeadFill;
    QBrush            mTailFill;
    QPen              mScrollPen;
    QPen              mBorderPen;
    QFont             mFont;
    QFontMetrics      mMetrics;
    QTextOption       mTitleOption;
    int               mIconSize;
    int               mTotalSize;
    int               mWidthHint;
    int               mHeightHint;
    qreal             mScroll       = 0.0;
    Action*           mHighlight    = nullptr;
    QActionGroup*     mExclusive    = nullptr;
    //touch data
    bool              mTouchPt      = false;
    int               mTouchId;
    //mouse data
    Qt::MouseButton   mButton       = Qt::NoButton;
    QPointF           mLastPos;

    struct Element {
        int          size;
        Action     * action;
        QStaticText* text;
    };
    QVector<Element>  mElements;
private:
    Q_DISABLE_COPY(SwitchBarPrivate)
};

SwitchBar::SwitchBar(int iconsize, Edge edge, QSizePolicy::Policy policy, bool exclusive)
    : d(new SwitchBarPrivate(iconsize, edge))
{
    setAttribute(Qt::WA_AcceptTouchEvents, true);
    setAttribute(Qt::WA_NoMousePropagation);
    setAttribute(Qt::WA_TranslucentBackground, true);
    setAttribute(Qt::WA_OpaquePaintEvent, true);

    d->mWidthHint  = /*free space before*/3+
                     /*font size*/d->mMetrics.averageCharWidth()*9+
                     /*free space after*/3;
    d->mHeightHint = /*free space above*/3+
                     /*icon size*/d->mIconSize+
                     /*space between icon and text*/3+
                     /*font size*/d->mMetrics.ascent()+
                     /*free space below*/qMax(d->mMetrics.descent(), 2)+4;
    if (d->mOrientation == Qt::Horizontal) {
        d->mTitleOption.setWrapMode(QTextOption::NoWrap);
        setMinimumWidth(d->mWidthHint);
        setFixedHeight(d->mHeightHint);
        setSizePolicy(policy, QSizePolicy::Fixed);
    } else {
        d->mTitleOption.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
        setFixedWidth(d->mWidthHint);
        setMinimumHeight(d->mHeightHint);
        setSizePolicy(QSizePolicy::Fixed, policy);
    }
    if (exclusive) {
        d->mExclusive = new QActionGroup(this);
        d->mExclusive->setExclusive(true);
    }
}
SwitchBar::~SwitchBar() {
    for(auto i = d->mElements.constBegin();
        i != d->mElements.constEnd(); ++i)
    {
        if (i->text)
            delete i->text;
    }
    delete d;
}

void SwitchBar::addAction(Action *action, bool exclusive) {
    int width;
    if (d->mOrientation == Qt::Horizontal) {
        width = qMax(d->mMetrics.width(action->text()), d->mIconSize);
    } else {
        width = d->mMetrics.averageCharWidth()*10;
    }
    SwitchBarPrivate::Element elt;
    elt.action = action;
    elt.text = new QStaticText(action->text());
    elt.text->setTextOption(d->mTitleOption);
    elt.text->setTextWidth(width);
    elt.text->prepare(QTransform(), d->mFont);
    if (d->mOrientation == Qt::Horizontal) {
        elt.size = /*free space before*/3+
                   /*font size*/elt.text->size().width()+
                   /*free space after*/3;
    } else {
        elt.size = /*free space above*/3+
                   /*icon size*/d->mIconSize+
                   /*space between icon and text*/3+
                   /*text size*/elt.text->size().height()+d->mMetrics.descent()+
                   /*free space below*/4;
    }
    d->mTotalSize += elt.size;

    int index;
    for(index = 0; index < d->mElements.size(); ++index) {
        if (d->mElements[index].action->order() >= action->order())
            break;
    }
    d->mElements.insert(index, elt);

    connect(action, SIGNAL(changed()), this, SLOT(actionChanged()));
    connect(action, SIGNAL(reordered()), this, SLOT(actionReordered()));
    connect(action, SIGNAL(toggled(bool)), this, SLOT(actionChecked()));
    QWidget::updateGeometry();

    if (d->mExclusive && exclusive && action->isCheckable()) {
        if (d->mExclusive->actions().isEmpty())
            action->setChecked(true);
        d->mExclusive->addAction(action);
    }
    update();
}
void SwitchBar::removeAction(Action *action) {
    int index;
    for(index = 0; index < d->mElements.size(); ++index) {
        if (d->mElements[index].action == action)
            break;
    }
    if(index == d->mElements.size())
        return;
    disconnect(action, 0, this, 0);

    if (d->mExclusive) {
        bool that = d->mExclusive->checkedAction()==action;
        d->mExclusive->removeAction(action);

        if (that) {
            auto actions = d->mExclusive->actions();
            if (!actions.isEmpty())
                actions.first()->setChecked(true);
        }
    }

    d->mTotalSize -= d->mElements[index].size;
    delete d->mElements[index].text;
    d->mElements.remove(index);
    d->mHighlight = nullptr;
    makeVisible(nullptr);
    QWidget::updateGeometry();
    update();
}
void SwitchBar::toggleFirst() {
    if (d->mElements.isEmpty())
        return;

    d->mElements[0].action->setChecked(true);
}

QSize SwitchBar::sizeHint() const {
    if (d->mOrientation == Qt::Horizontal) {
        return QSize(qMax(d->mWidthHint, d->mTotalSize), d->mHeightHint);
    } else {
        return QSize(d->mWidthHint, qMax(d->mHeightHint, d->mTotalSize));
    }
}
bool SwitchBar::isEmpty() const {
    return d->mElements.isEmpty();
}
void SwitchBar::actionChanged() {
    update();
}
void SwitchBar::actionChecked() {
    Action * action = qobject_cast<Action*>(sender());
    if (!action) return;

    if (action->isChecked())
        makeVisible(action);
    update();
}
void SwitchBar::makeVisible(Action *action) {
    //makesure it is visible
    qreal pos = d->mScroll, end = d->mScroll;
    for(int index = 0; index < d->mElements.size(); ++index) {
        if (d->mElements[index].action == action) {
            end = pos + d->mElements[index].size;
            break;
        }
        pos += d->mElements[index].size;
    }
    qreal s = d->mScroll;
    if (pos < 0.0) {
        s += pos;
    } else {
        int max = d->mOrientation == Qt::Horizontal ? width() : height();
        end -= max;
        if (end > 0.0)
            s -= end;
    }
    if (s != d->mScroll) {
        d->mScroll = s;
        update();
    }
}
void SwitchBar::actionDeleted(QObject * o) {
    int index;
    for(index = 0; index < d->mElements.size(); ++index) {
        QObject * oo = static_cast<QObject*>(d->mElements[index].action);
        if (o == oo)
            break;
    }
    if(index == d->mElements.size())
        return;

    if (d->mExclusive) {
        auto actions = d->mExclusive->actions();
        for(auto i = actions.constBegin(); i != actions.constEnd(); ++i) {
            QObject * oo = static_cast<QObject*>(*i);
            if (o == oo)
                continue;
            (*i)->setChecked(true);
            break;
        }
    }

    d->mTotalSize -= d->mElements[index].size;
    delete d->mElements[index].text;
    d->mElements.remove(index);
    d->mHighlight = nullptr;
    makeVisible(nullptr);
    QWidget::updateGeometry();
    //cancel input
    d->mButton = Qt::NoButton;
    d->mTouchPt = false;
    update();

}
void SwitchBar::actionReordered() {
    Action * action = qobject_cast<Action*>(sender());
    if (!action) return;

    int oldindex = -1, newindex = d->mElements.size(), index;
    for(index = d->mElements.size()-1; index >= 0; --index) {
        if (d->mElements[index].action == action) {
            oldindex = index;
            continue;
        }
        if (d->mElements[index].action->order() >= action->order())
            newindex = index;
    }
    Q_ASSERT(oldindex >= 0);

    SwitchBarPrivate::Element e = d->mElements[oldindex];
    d->mElements.remove(oldindex);
    if (oldindex < newindex)
        --newindex;
    d->mElements.insert(newindex, e);
    d->mHighlight = nullptr;
    update();
}
void SwitchBar::resizeEvent(QResizeEvent *event) {
    QWidget::resizeEvent(event);
    int     size;
    if (d->mOrientation == Qt::Horizontal) {
        size     = event->size().width();
    } else {
        size     = event->size().height();
    }
    if (size >= d->mTotalSize) {
        d->mScroll = 0.0;
    } else {
        d->mScroll = qBound(0.0, d->mScroll, qreal(d->mTotalSize - size));
    }
    d->mHighlight = nullptr;
    update();
}
void SwitchBar::doScroll(const QPointF &delta) {
    qreal ns = d->mScroll;
    int   size;
    if (d->mOrientation == Qt::Horizontal) {
        ns   += delta.x();
        size  = width();
    } else {
        ns   += delta.y();
        size  = height();
    }
    if (size >= d->mTotalSize) {
        ns = 0.0;
    } else {
        ns = qBound(0.0, ns, qreal(d->mTotalSize - size));
    }
    if (ns != d->mScroll) {
        d->mScroll = ns;
        update();
    }
}
Action *SwitchBar::itemAtPoint(const QPointF &pt, bool autoupdate) {
    qreal coord;
    if (d->mOrientation == Qt::Horizontal) {
        coord = pt.x();
    } else {
        coord = pt.y();
    }

    Action * ac = nullptr;
    qreal position = d->mScroll;
    auto iter = d->mElements.constBegin();
    for(;iter != d->mElements.constEnd();++iter)
    {
        qreal fsize = iter->size;
        if (fsize + position < coord) {
            position += fsize;
            continue;
        }
        ac = iter->action;
        break;
    }
    if (autoupdate && ac != d->mHighlight) {
        update();
    }
    d->mHighlight = ac;
    return ac;
}
//===================================
void SwitchBar::focusOutEvent(QFocusEvent *event) {
    QWidget::focusOutEvent(event);
    d->mButton = Qt::NoButton;
    d->mHighlight = nullptr;
    update();
}
void SwitchBar::mousePressEvent(QMouseEvent *event) {
    event->accept();
    d->mButton    = event->button();
    d->mLastPos   = QPointF(event->pos());
    itemAtPoint(d->mLastPos, true);
}
void SwitchBar::mouseMoveEvent(QMouseEvent *event) {
    event->accept();
    QPointF pt(event->pos());
    itemAtPoint(pt, true);
    if (d->mButton != Qt::NoButton)
        doScroll(pt - d->mLastPos);
    d->mLastPos = pt;
}
void SwitchBar::mouseReleaseEvent(QMouseEvent *event) {
    event->accept();
    QPointF pt(event->pos());
    Action * a = itemAtPoint(pt, true);
    if (d->mButton != Qt::NoButton) {
        QPointF delta = pt - d->mLastPos;
        if (event->button() == d->mButton)
        {
            d->mButton = Qt::NoButton;
            if (a != nullptr &&
                delta.manhattanLength() < 3.0)
            {
                a->activate(QAction::Trigger);
                return;
            }
        }
        doScroll(delta);
    }
    d->mLastPos = pt;
}
void SwitchBar::leaveEvent(QEvent *event) {
    QWidget::leaveEvent(event);
    if (d->mHighlight) {
        d->mHighlight = nullptr;
        update();
    }
}

void SwitchBar::touchEvent(QEvent *event) {
    QTouchEvent * te = static_cast<QTouchEvent*>(event);
    const QList<QTouchEvent::TouchPoint> & pts = te->touchPoints();
    if (pts.isEmpty() == false) {
        d->mTouchPt = false;
        d->mHighlight = nullptr;
        update();
    }

    switch(event->type()) {
    case QEvent::TouchBegin: {
        QSize size = QWidget::size();
        d->mTouchPt = false;
        for(auto iter = pts.constBegin(); iter != pts.constEnd(); ++iter) {
            if (iter->state() == Qt::TouchPointPressed) {
                QPointF pt = iter->pos();
                if (pt.x() < 0.0 || pt.y() < 0.0 ||
                    pt.x() >= size.width() ||
                    pt.y() >= size.height())
                    continue;

                d->mTouchPt = true;
                d->mTouchId = iter->id();
                itemAtPoint(iter->pos(), true);
                break;
            }
        }
        break; }
    case QEvent::TouchUpdate:
    case QEvent::TouchEnd: {
        if (!d->mTouchPt)
            break;

        //no more touch pts
        if (event->type() == QEvent::TouchEnd)
            d->mTouchPt = false;

        auto sel = pts.constEnd();
        for(auto iter = pts.constBegin(); iter != pts.constEnd(); ++iter) {
            if (iter->id() == d->mTouchId) {
                sel = iter;
                break;
            }
        }
        if (sel == pts.constEnd()) {
            d->mTouchPt = false;
            break;
        }
        doScroll(sel->pos() - sel->lastPos());
        Action * a = itemAtPoint(sel->pos(), true);
        if (sel->state() != Qt::TouchPointReleased)
            break;
        //released the finger
        d->mTouchPt = false;
        QPointF delta = sel->pos() - sel->startPos();
        if (a != nullptr && delta.manhattanLength() < 3.0)
        {
            a->activate(QAction::Trigger);
        }
        break; }
    default:
        d->mTouchPt = false;
    }
}
bool SwitchBar::event(QEvent * event) {
    switch(event->type()) {
    case QEvent::Show:
        setMouseTracking(true);
        return QWidget::event(event);
    case QEvent::TouchBegin:
    case QEvent::TouchUpdate:
    case QEvent::TouchEnd:
    case QEvent::TouchCancel:
        event->accept();
        touchEvent(event);
        break;
    default:
        return QWidget::event(event);
    }
    return true;
}
void SwitchBar::paintEvent(QPaintEvent *) {
    QSize size = QWidget::size();
    QPainter painter;
    painter.begin(this);

    QPen   graypen(Qt::gray);
    QPen   whitepen(Qt::white);
    QPen   blackpen(Qt::black);

    painter.setPen(graypen);
    painter.setBrush(d->mBackground);
    painter.drawRect(0, 0, size.width(), size.height());
    //wow! leaving gradient brush attached affects static text drawing
    painter.setBrush(Qt::NoBrush);

    qreal position = d->mScroll;
    auto iter = d->mElements.constBegin();
    for(;iter != d->mElements.constEnd();++iter)
    {
        qreal fsize = iter->size;
        if (fsize + position < 0.0) {
            position += fsize;
            continue;
        }
        if (position >= size.height())
            break;

        Action * a = iter->action;
        bool     ischecked = a->isChecked() & a->isEnabled();
        if (ischecked || a == d->mHighlight) {
            QColor bg = a->background();
            if (ischecked) {
                bg.setAlpha(120);
                painter.setPen(blackpen);
            } else {
                bg.setAlpha(20);
                painter.setPen(whitepen);
            }
            QBrush brush(bg);
            painter.setBrush(brush);
            if (d->mOrientation == Qt::Horizontal) {
                QRectF r(position, 0.0, fsize, size.height());
                painter.drawRect(r);
            } else {
                QRectF r(0.0, position, size.width(), fsize);
                painter.drawRect(r);
            }

        }
        if (ischecked) {
            painter.setPen(blackpen);
        } else if (a->isEnabled()){
            painter.setPen(whitepen);
        } else {
            painter.setPen(graypen);
        }

        QIcon::State state = QIcon::Off;
        QIcon::Mode  mode  = QIcon::Disabled;
        if (a->isEnabled()) {
            mode = QIcon::Normal;
            if (a->isChecked())
                state = QIcon::On;
            else if (a == d->mHighlight)
                mode = QIcon::Active;
        }
        QPixmap pixmap = a->icon().pixmap(d->mIconSize, mode, state);
        qreal x0, y0;
        QPointF textpos, iconpos;
        if (d->mOrientation == Qt::Horizontal) {
            x0 = position + 0.5*fsize;
            y0 = 0.0;
        } else {
            x0 = 0.5*size.width();
            y0 = position;
        }
        textpos.setX(x0 - 0.5 * iter->text->size().width());
        textpos.setY(y0 + 6.0 + d->mIconSize);
        iconpos.setX(x0 - 0.5 * pixmap.width());
        iconpos.setY(y0 + 3.0);
        painter.drawPixmap(iconpos, pixmap);
        painter.drawStaticText(textpos, *iter->text);
        position += fsize;
    }

    if (d->mOrientation == Qt::Horizontal) {
        if (d->mScroll < 0.0) {
            QRectF r(0.0, 0.0, 20.0, size.height());
            painter.setBrush(d->mHeadFill);
            painter.setPen(Qt::NoPen);
            painter.drawRect(r);
            painter.setPen(d->mScrollPen);

            int h = size.height();
            painter.drawLine(8, 4, 4, h>>1);
            painter.drawLine(4, h>>1, 8, h-4);
            painter.drawLine(16, 4, 12, h>>1);
            painter.drawLine(12, h>>1, 16, h-4);
        }
        if (iter != d->mElements.constEnd()) {
            QRectF r(size.width()-20.0, 0.0, 20.0, size.height());
            painter.setBrush(d->mHeadFill);
            painter.setPen(Qt::NoPen);
            painter.drawRect(r);
            painter.setPen(d->mScrollPen);

            int w = size.width();
            int h = size.height();
            painter.drawLine(w-8, 4, w-4, h>>1);
            painter.drawLine(w-4, h>>1, w-8, h-4);
            painter.drawLine(w-16, 4, w-12, h>>1);
            painter.drawLine(w-12, h>>1, w-16, h-4);
        }
    } else {
        if (d->mScroll < 0.0) {
            QRectF r(0.0, 0.0, size.width(), 20.0);
            painter.setBrush(d->mHeadFill);
            painter.setPen(Qt::NoPen);
            painter.drawRect(r);
            painter.setPen(d->mScrollPen);

            int w = size.width();
            painter.drawLine(4, 8, w>>1, 4);
            painter.drawLine(w>>1, 4, w-4, 8);
            painter.drawLine(4, 16, w>>1, 12);
            painter.drawLine(w>>1, 12, w-4, 16);
        }
        if (iter != d->mElements.constEnd()) {
            QRectF r(0.0, size.height()-20.0, size.width(), 20.0);
            painter.setBrush(d->mHeadFill);
            painter.setPen(Qt::NoPen);
            painter.drawRect(r);
            painter.setPen(d->mScrollPen);

            int w = size.width();
            int h = size.height();
            painter.drawLine(4, h-8, w>>1, h-4);
            painter.drawLine(w>>1, h-4, w-4, h-8);
            painter.drawLine(4, h-16, w>>1, h-12);
            painter.drawLine(w>>1, h-12, w-4, h-16);
        }
    }
    painter.end();
}

}}
