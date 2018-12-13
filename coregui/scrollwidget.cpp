#include "scrollwidget.h"
#include <math.h>
#include <QTouchEvent>
#include <QMouseEvent>
#include <QTimerEvent>
#include <QDebug>

namespace K { namespace Core {

class ScrollWidgetPrivate {
public:
    //=================================================
    //current parameters
    bool    mEnabled = false;
    QPointF mWindowSize;
    QPointF mCurrentPos         = QPointF(0,0);
    QRectF  mBoundary           = QRectF(0,0,1,1);
    bool    mEnabledRotation    = false;
    qreal   mCurrentRotation    = 0.0;
    qreal   mCurrentZoom        = 1.0;
    qreal   mMinZoom            = 1.0;
    qreal   mMaxZoom            = 1.0;
    qreal   mScrollAdj          = 1.0;
    qreal   mZoomStep           = 1.0;
    qreal   mDesiredFps         = 30.0;
    //=================================================
    //touch parameters
    QPointF mTouchCenter;
    int     mTouchId;
    //=================================================
    //mouse parameters
    Qt::MouseButton mMouseButton = Qt::NoButton;
    QPointF         mMousePrevPoint;
    //=================================================
    //click parameters
    enum class InputState { NONE,
                            MOUSE, MOUSE_CLICK, MOUSE_SCROLL,
                            TOUCH, TOUCH_CLICK, TOUCH_SCROLL,
                            TOUCH_MENU
                          };
    InputState mInputState = InputState::NONE;
    int     mClickTimeout       = 1000;
    int     mClickTimer         = -1;
    QElapsedTimer mClickClock;
    QPoint  mStartClickPos;
    //=================================================
    //touch parameters
    FancyMenu * mMenu           = nullptr;

    //=================================================
    //scrolling
    int     mScrollTimer     = -1;
    QElapsedTimer mScrollClock;
    QPointF mScrollStart;
    QPointF mScrollDirection    = QPointF(0,0);
};

static const double ScrollDuration = 3000.0;
//============================================================
ScrollWidget::ScrollWidget(QWidget * parent)
    : QWidget(parent)
    , d(new ScrollWidgetPrivate)
{
    setAttribute(Qt::WA_AcceptTouchEvents, true);
    setAttribute(Qt::WA_NoMousePropagation);
    d->mWindowSize = QPointF(size().width(), size().height());
    d->mMenu       = new FancyMenu(24);
}
ScrollWidget::~ScrollWidget()
{
    delete d->mMenu;
    delete d;
}

void ScrollWidget::setScrollAdjustment(qreal adj) {
    d->mScrollAdj = adj;
}

void ScrollWidget::zoomToRegion(const QRectF &region) {
    d->mCurrentPos = region.center();
    if (d->mWindowSize.x() > 0.0 && d->mWindowSize.y() > 0.0)
    {
        qreal dx = region.width()  / d->mWindowSize.x();
        qreal dy = region.height() / d->mWindowSize.y();
        d->mCurrentZoom = qBound(d->mMinZoom, qMin(dx,dy), d->mMaxZoom);
        d->mCurrentRotation = 0.0;
    }
    clampCurrentPos();
    positionChanged(d->mCurrentPos, d->mCurrentZoom, d->mCurrentRotation);
    update();
}
void ScrollWidget::zoomToPoint(const QPointF &point, qreal zoom) {
    d->mCurrentPos = point + 0.5*d->mWindowSize;
    d->mCurrentZoom = qBound(d->mMinZoom, zoom, d->mMaxZoom);
    clampCurrentPos();
    positionChanged(d->mCurrentPos, d->mCurrentZoom, d->mCurrentRotation);
    update();
}

void ScrollWidget::setZoomLimits(qreal minzoom, qreal maxzoom, qreal step) {
    Q_ASSERT(minzoom <= maxzoom);
    d->mMinZoom = minzoom;
    d->mMaxZoom = maxzoom;
    d->mZoomStep = step;
    qreal z = qBound(d->mMinZoom, d->mCurrentZoom, d->mMaxZoom);
    if (z != d->mCurrentZoom) {
        d->mCurrentZoom = z;
        update();
    }
}
void ScrollWidget::setEnabled(bool e) {
    if (!e) endAction();
    d->mEnabled = e;
}
void ScrollWidget::setBoundary(const QRectF &boundary) {
    d->mBoundary = boundary;
    clampCurrentPos();
    update();
}
void ScrollWidget::changeZoom(char action, qreal change) {
    qreal z;
    switch(action) {
    case '*':
        z = d->mCurrentZoom * change;
        break;
    case '+':
        z = d->mCurrentZoom + change;
        break;
    case '-':
        z = d->mCurrentZoom - change;
        break;
    case '=':
        z = change;
    default:
        return;
    }
    z = qBound(d->mMinZoom, z, d->mMaxZoom);
    if (z != d->mCurrentZoom) {
        d->mCurrentZoom = z;
        update();
    }
}
//============================================================
bool ScrollWidget::shouldIgnore(QEvent *event)
{
    QMouseEvent * ev = static_cast<QMouseEvent*>(event);
    return (ev->flags() != Qt::MouseEventNotSynthesized);
}
bool ScrollWidget::event(QEvent *event)
{
    switch(event->type()) {
    case QEvent::MouseButtonPress:
    case QEvent::MouseButtonRelease:
    case QEvent::MouseMove:
        if (shouldIgnore(event)) {
            event->accept();
            return true;
        } else {
            event->accept();
            return QWidget::event(event);
        }
    case QEvent::Hide:
    case QEvent::Close:
        d->mMenu->hide();
        endAction();
        return QWidget::event(event);
    case QEvent::TouchBegin:
        beginTouch(event);
        break;
    case QEvent::TouchUpdate:
        updateTouch(event);
        break;
    case QEvent::TouchEnd:
        updateTouch(event);
        endAction();
        break;
    case QEvent::TouchCancel:
        endAction();
        break;
    default:
        return QWidget::event(event);
    }
    return true;
}
void ScrollWidget::resizeEvent(QResizeEvent * ev)
{
    d->mWindowSize = QPointF(ev->size().width(), ev->size().height());
    QWidget::resizeEvent(ev);
}
//============================================================
QPointF ScrollWidget::translate(const QPointF &pt)
{
    qreal a = M_PI*d->mCurrentRotation/180.0;
    qreal c = cos(a);
    qreal s = sin(a);
    QPointF adjusted = pt - 0.5*d->mWindowSize;
    QPointF rotated(
       adjusted.x()*c - adjusted.y()*s,
       adjusted.x()*s + adjusted.y()*c
    );
    return d->mCurrentPos + rotated*d->mCurrentZoom;
}
QPointF ScrollWidget::scrollDir(const QPointF &dp)
{
    qreal a = M_PI*d->mCurrentRotation/180.0;
    qreal c = cos(a);
    qreal s = sin(a);
    QPointF adjusted = dp * d->mCurrentZoom;
    QPointF rotated(
       adjusted.x()*c - adjusted.y()*s,
       adjusted.x()*s + adjusted.y()*c
    );
    qDebug()<<"S"<<d->mCurrentZoom<<sqrt(dp.x()*dp.x()+dp.y()*dp.y())<<sqrt(rotated.x()*rotated.x()+rotated.y()*rotated.y());
    return rotated;
}
QTransform ScrollWidget::getTransform() const
{
    qreal a = M_PI*d->mCurrentRotation/180.0;
    qreal c = cos(a);
    qreal s = sin(a);
    return QTransform(/*m11 m12 m13*/
                      c,   -s,   0.0,
                      /*m21 m22 m23*/
                      s,    c,   0.0,
                      /*dx*/
                      -d->mCurrentPos.x()*c-d->mCurrentPos.y()*s,
                      /*dy*/
                      d->mCurrentPos.x()*s-d->mCurrentPos.x()*c,
                      /*w*/
                      d->mCurrentZoom);
}
//============================================================
void ScrollWidget::timerEvent(QTimerEvent * event)
{
    if (event->timerId() == d->mClickTimer)
    {
        killTimer(d->mClickTimer);
        d->mClickTimer = -1;
        if (!d->mEnabled)
            return;
        //если тыкаем пальцем слишком долго, это вызов меню
        if (d->mInputState == ScrollWidgetPrivate::InputState::TOUCH_CLICK) {
            QList<Action*> actions = menuClick(translate(d->mStartClickPos));
            if (!actions.isEmpty()) {
                d->mMenu->setActions(actions);
                if (d->mMenu->touchPopup(this, d->mStartClickPos))
                    d->mInputState = ScrollWidgetPrivate::InputState::TOUCH_MENU;
            }
        }
    }
    else if (event->timerId() == d->mScrollTimer)
    {
        if (!d->mEnabled) {
            killTimer(d->mScrollTimer);
            d->mScrollTimer = -1;
            return;
        }
        qint64 dt = d->mScrollClock.elapsed();
        Q_ASSERT(dt >= 0);
        qreal k = qreal(dt)/ScrollDuration;
        if (k >= 1.0) {
            k = 1.0;
            killTimer(d->mScrollTimer);
            d->mScrollTimer = -1;
        }
        k = k*k;

        d->mCurrentPos = d->mScrollStart + d->mScrollDirection * k;
        clampCurrentPos();
        positionChanged(d->mCurrentPos, d->mCurrentZoom, d->mCurrentRotation);
        update();
    }
}
void ScrollWidget::clampCurrentPos()
{
    if (std::isnan(d->mCurrentPos.x()) || std::isnan(d->mCurrentPos.y()))
    {
        d->mCurrentPos = d->mBoundary.center();
    }
    else
    {
        if (d->mCurrentPos.x() < d->mBoundary.left())
            d->mCurrentPos.setX(d->mBoundary.left());
        if (d->mCurrentPos.x() > d->mBoundary.right())
            d->mCurrentPos.setX(d->mBoundary.right());
        if (d->mCurrentPos.y() < d->mBoundary.top())
            d->mCurrentPos.setY(d->mBoundary.top());
        if (d->mCurrentPos.y() > d->mBoundary.bottom())
            d->mCurrentPos.setY(d->mBoundary.bottom());
    }
}
//============================================================
void ScrollWidget::focusOutEvent(QFocusEvent * event)
{
    endAction();
    QWidget::focusOutEvent(event);
}
//============================================================
void ScrollWidget::mousePressEvent(QMouseEvent * event)
{
    if (!d->mEnabled) {
        return;
    }
    switch (d->mInputState) {
    case ScrollWidgetPrivate::InputState::NONE:
        if (d->mScrollTimer >= 0) {
            killTimer(d->mScrollTimer);
           d-> mScrollTimer = -1;
        }
        if (event->buttons() == event->button()) {
            d->mClickClock.start();
            d->mStartClickPos  = event->pos();
            d->mMouseButton    = event->button();
            d->mMousePrevPoint = event->pos();
            if (beginClick(d->mMousePrevPoint, event->modifiers()))
                d->mInputState = ScrollWidgetPrivate::InputState::MOUSE_CLICK;
            else {
                d->mInputState = ScrollWidgetPrivate::InputState::MOUSE_SCROLL;
                scrollActive(true);
            }
            event->accept();
            break;
        }
        /*fall thru*/
    case ScrollWidgetPrivate::InputState::MOUSE:
    case ScrollWidgetPrivate::InputState::MOUSE_CLICK:
    case ScrollWidgetPrivate::InputState::MOUSE_SCROLL:
        if (d->mScrollTimer >= 0) {
            killTimer(d->mScrollTimer);
            d->mScrollTimer = -1;
        }
        mouseMoveEvent(event);
        break;
    default:;
    }
}
void ScrollWidget::mouseMoveEvent(QMouseEvent * event)
{
    if (!d->mEnabled) {
        return;
    }
    QPointF pp = event->pos();
    QPointF pt = translate(pp);
    event->accept();
    switch (d->mInputState) {
    case ScrollWidgetPrivate::InputState::NONE:
        move(pt, false);
        break;
    case ScrollWidgetPrivate::InputState::MOUSE_CLICK:
        if ((event->pos() - d->mStartClickPos).manhattanLength() < 3)
            break;
        d->mInputState = ScrollWidgetPrivate::InputState::MOUSE;
        /*fall thru*/
    case ScrollWidgetPrivate::InputState::MOUSE:
        move(pt, true);
        break;
    case ScrollWidgetPrivate::InputState::MOUSE_SCROLL:
        d->mCurrentPos -= scrollDir(pp-d->mMousePrevPoint)*d->mScrollAdj;
        d->mMousePrevPoint = event->pos();
        clampCurrentPos();
        positionChanged(d->mCurrentPos, d->mCurrentZoom, d->mCurrentRotation);
        update();
        break;
    default:;
    }
}
void ScrollWidget::mouseReleaseEvent(QMouseEvent * event)
{
    if (!d->mEnabled) {
        return;
    }
    QPointF pp = event->pos();
    QPointF pt = translate(pp);
    event->accept();

    switch (d->mInputState) {
    case ScrollWidgetPrivate::InputState::NONE:
        move(pt, false);
        break;
    case ScrollWidgetPrivate::InputState::MOUSE_CLICK:
        if ((event->pos() - d->mStartClickPos).manhattanLength() < 3)
        {
            if (event->button() == d->mMouseButton) {
                d->mInputState = ScrollWidgetPrivate::InputState::NONE;
                completeClick();
            }
            break;
        }
        d->mInputState = ScrollWidgetPrivate::InputState::MOUSE;
        /*fall thru*/
    case ScrollWidgetPrivate::InputState::MOUSE:
        if (event->button() == d->mMouseButton) {
            d->mInputState = ScrollWidgetPrivate::InputState::NONE;
            release(pt);
        } else {
            move(pt, true);
        }
        break;
    case ScrollWidgetPrivate::InputState::MOUSE_SCROLL:
        d->mCurrentPos -= scrollDir(pp-d->mMousePrevPoint)*d->mScrollAdj;
        clampCurrentPos();
        positionChanged(d->mCurrentPos, d->mCurrentZoom, d->mCurrentRotation);
        if (event->button() == d->mMouseButton) {
            d->mInputState = ScrollWidgetPrivate::InputState::NONE;
            scrollActive(false);
        }
        update();
        break;
    default:;
    }
}
void ScrollWidget::wheelEvent(QWheelEvent * event)
{
    if (!d->mEnabled) {
        return;
    }
    event->accept();
    qreal da = 0.0, dz = 0.0;
    QPoint dp = event->angleDelta()*(15.0/120.0);
    da = dp.x();
    dz = dp.y();
    if (event->modifiers() & Qt::AltModifier) {
        da = dz;
        dz = 0.0;
    }
    dz = d->mZoomStep*dz/15.0;
    if (event->modifiers() & Qt::ShiftModifier)
        dz *= 10.0;
    if (event->modifiers() & Qt::ControlModifier)
        dz *= 0.1;
    if (d->mEnabledRotation)
        d->mCurrentRotation = fmod(d->mCurrentRotation+(180.0/M_PI)*da, 360.0);
    d->mCurrentZoom = qBound(d->mMinZoom, d->mCurrentZoom*(1.0+dz), d->mMaxZoom);
    positionChanged(d->mCurrentPos, d->mCurrentZoom, d->mCurrentRotation);
    update();
}
//============================================================
void ScrollWidget::beginTouch(QEvent *event)
{
    if (!d->mEnabled) {
        return;
    }
    QTouchEvent * ev = static_cast<QTouchEvent*>(event);
    if (d->mInputState != ScrollWidgetPrivate::InputState::NONE) {
        ev->ignore();
        return;
    }
    //если меню видно и оно может забрать себе касание, то отдадим касание меню
    if (d->mMenu->canHandleTouch(ev)) {
        ev->ignore();
        return;
    }
    //иначе прячем меню если оно есть
    d->mMenu->hide();
    //принимаем событие
    event->accept();
    const QList<QTouchEvent::TouchPoint>& pts = ev->touchPoints();

    if (d->mClickTimer >= 0) {
        killTimer(d->mClickTimer);
        d->mClickTimer = -1;
    }
    if (d->mScrollTimer >= 0) {
        killTimer(d->mScrollTimer);
        d->mScrollTimer = -1;
    }

    d->mTouchCenter   = pts[0].pos();
    scrollActive(true);
    d->mInputState = ScrollWidgetPrivate::InputState::TOUCH_SCROLL;
    d->mClickClock.start();
    //если точка касания только одна, это может быть вызов меню
    if (pts.size() == 1) {
        if (beginClick(translate(d->mTouchCenter), ev->modifiers())) {
            scrollActive(false);
            d->mInputState = ScrollWidgetPrivate::InputState::TOUCH_CLICK;
            d->mTouchId = pts[0].id();
            //запустим таймер на полторы секунды
            d->mClickTimer = startTimer(1500);
            d->mStartClickPos = pts[0].screenPos().toPoint();
        }
    } else {
        for(int i = 1; i < pts.size(); ++i) {
            d->mTouchCenter += pts[i].pos();
        }
        d->mTouchCenter*= 1.0/pts.size();
    }
}
void ScrollWidget::updateTouch(QEvent *event)
{
    if (!d->mEnabled) {
        return;
    }
    QTouchEvent * ev = static_cast<QTouchEvent*>(event);
    //если меню видно и оно может забрать себе касание, то отдадим касание меню
    if (d->mMenu->canHandleTouch(ev)) {
        ev->ignore();
        return;
    }

    if (d->mScrollTimer >= 0) {
        killTimer(d->mScrollTimer);
        d->mScrollTimer = -1;
    }

    const QList<QTouchEvent::TouchPoint>& pts = ev->touchPoints();
    int clickid = -1;

    if (d->mInputState == ScrollWidgetPrivate::InputState::TOUCH_CLICK ||
        d->mInputState == ScrollWidgetPrivate::InputState::TOUCH)
    {
        for(int i = 0; i < pts.size(); ++i) {
            if (pts[i].id() == d->mTouchId) {
                clickid = i;
                break;
            }
        }
    }

    switch (d->mInputState) {
    case ScrollWidgetPrivate::InputState::TOUCH_CLICK:
        if (clickid < 0)
        {
            if (d->mClickTimer >= 0) {
                killTimer(d->mClickTimer);
                d->mClickTimer = -1;
            }
            cancelClick();
            scrollActive(true);
            d->mInputState = ScrollWidgetPrivate::InputState::TOUCH_SCROLL;
            goto __scroll;
        }
        if ((pts[clickid].screenPos() - pts[clickid].startScreenPos()).manhattanLength() >= 3.0)
        {
            if (d->mClickTimer >= 0) {
                killTimer(d->mClickTimer);
                d->mClickTimer = -1;
            }
            d->mInputState = ScrollWidgetPrivate::InputState::TOUCH;
            goto __moved;
        }
        if (pts[clickid].state() == Qt::TouchPointReleased)
        {
            if (d->mClickTimer >= 0) {
                killTimer(d->mClickTimer);
                d->mClickTimer = -1;
            }
            completeClick();
            if (pts.size() == 1) {
                d->mInputState = ScrollWidgetPrivate::InputState::NONE;
                break;
            }
            scrollActive(true);
            d->mInputState = ScrollWidgetPrivate::InputState::TOUCH_SCROLL;
            goto __scroll;
        }
        break;
    case ScrollWidgetPrivate::InputState::TOUCH:
        if (clickid < 0) {
            if (d->mClickTimer >= 0) {
                killTimer(d->mClickTimer);
                d->mClickTimer = -1;
            }
            cancelClick();
            scrollActive(true);
            d->mInputState = ScrollWidgetPrivate::InputState::TOUCH_SCROLL;
            goto __scroll;
        }
    __moved:
        if (pts[clickid].state() == Qt::TouchPointReleased)
        {
            release(translate(pts[clickid].pos()));
            if (pts.size() == 1) {
                d->mInputState = ScrollWidgetPrivate::InputState::NONE;
                break;
            }
            scrollActive(true);
            d->mInputState = ScrollWidgetPrivate::InputState::TOUCH_SCROLL;
        }
        else
        {
            move(translate(pts[clickid].pos()), true);
        }
        goto __scroll;
    case ScrollWidgetPrivate::InputState::TOUCH_SCROLL:
    __scroll:
        if (clickid >= 0 && pts.size() == 1)
            break;
        if (pts.size() == 1) {
            d->mTouchCenter = pts[0].pos();
            //if there is only one point, and it is released, apply speed to screen
            QPointF dpos = scrollDir(pts[0].pos() - pts[0].lastPos());
            d->mCurrentPos += dpos*d->mScrollAdj;
            clampCurrentPos();
            positionChanged(d->mCurrentPos, d->mCurrentZoom, d->mCurrentRotation);
            if (pts[0].state() == Qt::TouchPointReleased) {
                qint64 dt = d->mClickClock.restart();
                d->mScrollClock.start();
                d->mScrollStart = d->mCurrentPos;
                d->mScrollDirection =
                    (ScrollDuration / (1+dt)) * scrollDir(pts[0].pos() - pts[0].lastPos());
                d->mScrollTimer = startTimer(1000.0 / qMax(1.0, d->mDesiredFps));
            }
        } else {
            if (d->mClickTimer >= 0) {
                killTimer(d->mClickTimer);
                d->mClickTimer = -1;
            }
            QPointF pc = QPointF(0.0, 0.0);
            qreal   expansion = 0.0;
            qreal   rotation  = 0.0;
            qreal   divisor   = 0.0;
            for(int i = 0; i < pts.size(); ++i) {
                if (i == clickid)
                    continue;
                pc += pts[i].pos();
                divisor += 1.0;
                QPointF g  = pts[i].pos() - pts[i].lastPos();
                QPointF v  = pts[i].pos() - d->mTouchCenter;
                qreal   l  = v.x()*v.x()+v.y()*v.y();
                if (l > 1.0) {
                    expansion += (v.x()*g.x()+v.y()+g.y())/l;
                    rotation  -= asin((v.x()*g.y()-v.y()*g.x())/l);
                }
            }
            pc *= (1.0/divisor);
            QPointF dpos = scrollDir(pc - d->mTouchCenter);
            d->mTouchCenter = pc;
            d->mCurrentPos += dpos*d->mScrollAdj;
            clampCurrentPos();

            d->mCurrentZoom = qBound(d->mMinZoom, d->mCurrentZoom*expansion, d->mMaxZoom);
            if (d->mEnabledRotation)
                d->mCurrentRotation = fmod(d->mCurrentRotation+(180.0/M_PI)*rotation, 360.0);
            positionChanged(d->mCurrentPos, d->mCurrentZoom, d->mCurrentRotation);
            update();
        }
        break;
    default:;
    }
}
void ScrollWidget::endAction()
{
    scrollActive(false);
    d->mInputState    = ScrollWidgetPrivate::InputState::NONE;
    d->mMouseButton   = Qt::NoButton;
    if (d->mClickTimer >= 0) {
        killTimer(d->mClickTimer);
        d->mClickTimer = -1;
    }
}

}}
