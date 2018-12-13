#include "fancymenu.h"
#include <QAction>
#include <QDesktopWidget>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QWheelEvent>
#include <QPainter>
#include <QRadialGradient>
#include <QRegion>
#include <cmath>

namespace K { namespace Core {

class FancyMenuPrivate {
public:
    //these variables are defined upon creation of menu
    QDesktopWidget* mDesktop;
    int             mIconSize;
    int             mSize;//половина размера окна
    int             mMaxIcons;
    float           mRadius;
    float           mIconRadius;
    float           mMinPhi;
    QBrush          mBrush;
    //these must be set before menu is used
    QList<Action*>  mAllActions;
    QList<Action*>  mActions;
    //these are valid once menu was shown
    QPoint          mPopupPoint;
    bool            mConstrained;
    float           mPhi0;
    float           mPhi1;
    float           mScroll;
    int             mPacking;
    int             mHovered;
    //these too, and are related to user interaction
    Qt::MouseButton mClicked;
    QPoint          mClickPoint;
    bool            mTouched;

};

FancyMenu::FancyMenu(int iconSize)
    : QWidget(nullptr,
              Qt::Popup|
              Qt::FramelessWindowHint|
              Qt::NoDropShadowWindowHint
              )
    , d(new FancyMenuPrivate)
{
    d->mDesktop    = new QDesktopWidget();
    d->mIconSize   = iconSize;
    d->mSize       = qMax(iconSize-6, 10)*4;
    d->mRadius     = 2.5*qMax(iconSize-6, 10);
    d->mIconRadius = sqrtf(2.0f)*d->mIconSize;

    //sin(180/n) <= sqrt(2)*iconSize
    //для углов < 90 градусов, верно:
    // если x>y, то sin(x)>sin(y), следовательно мы можем
    // применить арксинус.
    float numf  = floorf(M_PI/asinf(d->mIconRadius/d->mRadius));
    d->mMaxIcons   = (int)numf;
    d->mMinPhi     = M_PI/numf;

    setAttribute(Qt::WA_AcceptTouchEvents, true);
    setAttribute(Qt::WA_TranslucentBackground, true);

    setMouseTracking(true);

    QColor T(63,63,63,0), G(63,63,63,230), Y(255,255,255,255);
    QRadialGradient grad1(0.5,0.5,0.5,0.5,0.5);
    grad1.setCoordinateMode(QGradient::ObjectBoundingMode);
    grad1.setColorAt(0.0, T);
    grad1.setColorAt(1.3 / 4.0, T);
    grad1.setColorAt(1.4 / 4.0, Y);
    grad1.setColorAt(1.8 / 4.0, G);
    grad1.setColorAt(3.2 / 4.0, G);
    grad1.setColorAt(3.6 / 4.0, Y);
    grad1.setColorAt(3.7 / 4.0, T);
    grad1.setColorAt(1.0, T);
    d->mBrush = QBrush(grad1);
}
FancyMenu::~FancyMenu() {
    delete d->mDesktop;
    delete d;
}
void FancyMenu::setActions(const QList<Action *> &actions) {
    hide();
    d->mActions.clear();
    d->mAllActions = actions;
    for(Action * action : d->mAllActions) {
        connect(action, SIGNAL(changed()),
                this, SLOT(actionChanged()));
        connect(action, SIGNAL(toggled(bool)),
                this, SLOT(actionChanged()));
        connect(action, SIGNAL(reordered()),
                this, SLOT(reordered()));
    }
    actionChanged();;
}
bool FancyMenu::popup(QWidget *widget, const QPoint &point) {
    return popupInternal(widget, widget->mapToGlobal(point));
}
bool FancyMenu::popupInternal(QWidget *widget, const QPoint &point)
{
    if (isVisible())
        hide();

    d->mActions.clear();
    for(int i = 0; i < d->mAllActions.size(); ++i) {
        d->mAllActions[i]->visibilityChanged();//cache visibility
        if (d->mAllActions[i]->isVisible()) {
            d->mActions.append(d->mAllActions[i]);
        }
    }

    sortActions();

    int n = d->mActions.size();
    if (!n) return false;
    //prepare mask and position
    auto size = d->mSize;
    QRect   geometry(point.x()-size, point.y()-size, size+size, size+size);
    QRegion maskRegion(0, 0, size+size, size+size, QRegion::Ellipse);
    QWidget::setGeometry(geometry);
    QWidget::setMask(maskRegion);
    //check for intersection with screen borders
    //it this case we will have to use mask rectangle
    QRect  screen = d->mDesktop->screenGeometry(widget);
    if (screen.isEmpty())
        return false;

    QRect intersection = screen.intersected(geometry);
    if (intersection.isEmpty())
        return false;
    if (!intersection.contains(geometry.center(), true))
        return false;

    bool constrained_at_left   = intersection.left()   > geometry.left();
    bool constrained_at_right  = intersection.right()  < geometry.right();
    bool constrained_at_top    = intersection.top()    > geometry.top();
    bool constrained_at_bottom = intersection.bottom() < geometry.bottom();
    bool constrained = constrained_at_left |
                       constrained_at_right |
                       constrained_at_top |
                       constrained_at_bottom;

    d->mConstrained = constrained;
    d->mPopupPoint  = point;
    if (!constrained) {
        d->mPhi0    = 2.0f*M_PI/3.0f;
        d->mPhi1    = d->mPhi0 + M_2_PI;
        d->mPacking = qBound(3, n, d->mMaxIcons);
    } else {
        //не работаем на слишком мелких экранах
        if (constrained_at_left && constrained_at_right)
            return false;
        if (constrained_at_bottom && constrained_at_top)
            return false;

        /*
         * вот примерный чертеж того, как мы вычисляем углы
         *
         *    |x_geometry
         *    |
         *    |   |x_intersection
         *    |   |
         *    |   A__  <-------ch2
         *    |  (|  __
         *    | ( |     __
         *  --E(--B-----__C--------------------->x
         *    | ( |  __
         *    |  (|__   <------сh1
         *    |   D
         *    |   |
         *    |
         *
         * угол BCA = углу BCD = ф
         * cos(ф) = BC / mRadius;
         *
         * углы должны идти всегда против часовой стрелки
         */

        //возможно мы слегка уширим радиус
        //если иконки будут налазить на край экрана
        float effRadius = d->mRadius;
        //у стенки если элемент 1, его желательно приподнять
        int minPack = 3;

        float ch1 = -M_2_PI;
        float ch2 =  M_2_PI;
        if (constrained_at_left) {
            //нас теснят слева
            int h = abs(point.x() - intersection.left());
            float f = acosf(float(h)/effRadius);
            ch1 =-M_PI + f;
            ch2 = M_PI - f;
            --minPack;
        } else if (constrained_at_right) {
            //нас теснят справа
            int h = abs(intersection.right() - point.x());
            float f = acosf(float(h)/effRadius);
            ch1 = f;
            ch2 = M_2_PI - f;
            --minPack;
        }

        float cv1a = -M_2_PI;
        float cv2a =  M_2_PI;
        float cv1b = -M_2_PI;
        float cv2b =  M_2_PI;
        if (constrained_at_top) {
            //нас теснят сверху
            int h = abs(point.y() - intersection.top());
            float f = acosf(float(h)/effRadius);
            cv1a = M_PI_2 + f;
            cv2a = M_2_PI + M_PI_2 - f;
            cv1b =-M_PI - M_PI_2 + f;
            cv2b = M_PI - f;
            --minPack;
        } else if (constrained_at_bottom) {
            //нас теснят снизу
            int h = abs(intersection.bottom() - point.y());
            float f = acosf(float(h)/effRadius);
            cv1a = -M_PI_2 + f;
            cv2a = M_PI + M_PI_2 - f;
            cv1b = cv1a;
            cv2b = cv2a;
            --minPack;
        }

        //выберем пересечение интервалов углов, дающее наибольший угол
        //и объединим его в один интервал
        float c1a = qMax(ch1, cv1a);
        float c2a = qMin(ch2, cv2a);
        float c1b = qMax(ch1, cv1b);
        float c2b = qMin(ch2, cv2b);
        if (c2a - c1a > c2b - c1b) {
            d->mPhi0 = c1a;
            d->mPhi1 = c2a;
        } else {
            d->mPhi0 = c1b;
            d->mPhi1 = c2b;
        }
        //теперь расположим его так, чтоб углы лежали в интервале -П...2П
        if (d->mPhi0 > M_PI) {
            d->mPhi0 -= M_2_PI;
            d->mPhi1 -= M_2_PI;
        }
        //выясним, сколько туда влезет элементов
        int maximum = (int)(floorf((d->mPhi1 - d->mPhi0) / d->mMinPhi));
        if (maximum <= 0)
            return false;
        d->mPacking = qBound(minPack, n, maximum);
    }
    //обнуляем перемотку и все флажки
    d->mScroll  = 0.0f;
    d->mClicked = Qt::NoButton;
    d->mHovered = -1;
    d->mTouched = false;
    show();
    update();
    return true;
}
int  FancyMenu::actionAt(const QPoint &pt) {

    float xy0  = QWidget::width()*0.5f;
    float X    = pt.x() - xy0;
    float Y    = xy0 - pt.y();
    float rad  = sqrtf(X*X+Y*Y);

    float br1  = (1.3f/2.5f)*d->mRadius;
    float br2  = (3.7f/2.5f)*d->mRadius;

    if (br1 >= rad || br2 <= rad)
        return -1;

    /*
     *    click              ----phi0
     *    |                 /
     *     \      OOOOOO   /
     *      \   OO      OO/
     *       \OO          OO
     *       O              O
     *      O                O
     *      O                O
     *      /O              O
     *     /  OO          OO
     *    /     OO      OO
     *   |        OOOOOO
     *   phi1
     *
     */
    //найдем угол тыка в интервале
    float angle = atan2f(Y, X);
    if (angle < d->mPhi0 && angle > d->mPhi1) {
        angle += M_2_PI;
        if (angle < d->mPhi0 && angle > d->mPhi1)
            return -1;
    }

    float dphi  = (d->mPhi1 - d->mPhi0) / d->mPacking;
    int   n     = d->mActions.size();
    //при этом мы еще скролимся, и каждая единица mScroll соответствует одному проскролленому объекту
    //скроллинг отключается, если все объекты и так умещаются
    if (n <= d->mPacking) {
        int index = floorf((angle - d->mPhi0) / dphi);
        if (index < 0 || index >= n)
            return -1;
        return index;
    } else {
        float delta = (angle - d->mPhi0) / dphi + d->mScroll;
        int index = (int)floorf(delta);
        int base  = (int)floorf(d->mScroll);
        //если идет скроллинг, то у краев могут быть неполные элементы и их надо
        //сделать невыбираемыми, если они выглядывают менее чем на половину
        delta     = delta - floorf(delta);
        int ignored = delta >= 0.5f ? base : base + d->mPacking;
        if (index == ignored)
            return -1;
        //нам надо как-то подкорректировать index, т.к. он может уйти за пределы [0,n)
        //оценим, можно ли просто взять остаток от деления с учетом отрицательных чисел:
        //остаток определен как: (a/b)*b + a%b == a
        //в делении, The quotient is truncated towards zero (fractional part is discarded).

        //пусть a = -kn+b;
        //    (-kn+b)/n=1-k, (1-k)*n=n-kn;
        //    a%b + n - kn = -kn+b
        //    a%b = b-n
        //значит, если a < 0, модуль надо увеличить на n
        index = index < 0 ? (index%n + n) : (index%n);

        Q_ASSERT(index >= 0);
        Q_ASSERT(index < n);
        return index;
    }
}
void FancyMenu::paintEvent(QPaintEvent *) {
    QPainter painter;
    painter.begin(this);
    //draw circle
    painter.setCompositionMode(QPainter::CompositionMode_Source);
    QSize viewport = QWidget::size();
    painter.fillRect(0, 0, viewport.width(), viewport.height(), d->mBrush);

    float dphi = (d->mPhi1 - d->mPhi0) / d->mPacking;
    int   n    = d->mActions.size();
    if (n <= d->mPacking) {
        float phi = d->mPhi0;
        for(int i = 0; i < n; ++i) {
            drawIcon(painter, phi, dphi, i);
            phi += dphi;
        }
    } else {
        float flor = floorf(d->mScroll);
        float frac = d->mScroll - flor;
        int index = (int)flor;
        index     = index < 0 ? (index%n + n) : (index%n);

        float phi = d->mPhi0 - dphi * frac;
        for(int i = 0; i <= d->mPacking; ++i) {
            float opacity = 1.0f;
            if (!i) {
                opacity -= frac;
            } else if (i == d->mPacking) {
                opacity = frac;
            }
            painter.setOpacity(opacity);
            drawIcon(painter, phi, dphi, index);
            phi  += dphi;
            index = (index+1)%n;
        }
    }
    painter.end();
}
void FancyMenu::drawIcon(QPainter &painter, float phi, float dphi, int index) {
    Action * action = d->mActions.at(index);
    QIcon::State state = QIcon::On;
    QIcon::Mode  mode  = QIcon::Disabled;
    float bg = 0.0f;
    if (action->isEnabled()) {
        mode = QIcon::Normal;
        if (action->isChecked())
            bg = 0.7f;
        else
            state = QIcon::Off;
        if (index == d->mHovered) {
            bg = 0.3f;
            mode = QIcon::Active;
        }
    }

    float xy0 = width()*0.5f;
    if (bg > 0.0f) {
        float br1  = (1.3f/2.5f)*d->mRadius;
        float br2  = (3.7f/2.5f)*d->mRadius;

        float phi0 = phi;
        float phi1 = phi+dphi;

        QPainterPath path;
        QRectF rect1(xy0-br1, xy0-br1, 2.0*br1, 2.0*br1);
        path.arcMoveTo(rect1, phi0);
        path.arcTo(rect1, phi0, phi1-phi0);
        QRectF rect2(xy0-br2, xy0-br2, 2.0*br2, 2.0*br2);
        path.arcTo(rect2, phi1, phi0-phi1);
        path.closeSubpath();
        QColor color = action->background();
        color.setAlphaF(bg);
        painter.setCompositionMode(QPainter::CompositionMode_SourceAtop);
        painter.fillPath(path, QBrush(color));
    }

    phi += 0.5*dphi;
    float x = cosf(phi)*d->mRadius;
    float y = sinf(phi)*d->mRadius;

    const QPixmap pixmap = action->icon().pixmap(d->mIconSize, mode, state);
    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
    painter.drawPixmap(QPointF(x-0.5f*pixmap.width(), y-0.5f*pixmap.height()), pixmap);
}
void FancyMenu::mousePressEvent(QMouseEvent *event) {
    if (!isVisible())
        return;
    if (d->mClicked == Qt::NoButton) {
        d->mClickPoint = event->pos();
        d->mClicked = event->button();
    }
    mouseMoveEvent(event);
}
void FancyMenu::mouseMoveEvent(QMouseEvent *event) {
    if (!isVisible())
        return;
    int action = actionAt(event->pos());
    if (d->mHovered != action) {
        d->mHovered = action;
        update();
    }
}
void FancyMenu::mouseReleaseEvent(QMouseEvent *event) {
    if (!isVisible())
        return;
    if (event->buttons() != 0 || d->mClicked == Qt::NoButton)
    {
        mouseMoveEvent(event);
        return;
    }
    do {
        if (!d->mClicked)
            break;
        QPoint g = event->pos() - d->mClickPoint;
        if (g.manhattanLength() > 3)
            break;

        int action = actionAt(d->mClickPoint);
        if (action < 0)
            break;

        d->mActions.at(action)->activate(QAction::Trigger);
        if (d->mActions.at(action)->isCheckable()) {
            mouseMoveEvent(event);
            return;
        }
    } while(0);
    hide();
}
void FancyMenu::wheelEvent(QWheelEvent *event) {
    int dw = event->angleDelta().y();
    float w = float(dw) * (d->mPhi1 - d->mPhi0) / d->mPacking / 480.0f;
    doScroll(w);
    update();
}
void FancyMenu::keyPressEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Escape) {
        hide();
    }
}
bool FancyMenu::touchPopup(QWidget *widget, const QPointF &point) {
    QPoint  originPoint(point.x(), point.y());
    if (!popupInternal(widget, originPoint))
        return false;

    d->mTouched      = true;
    return true;
}
bool FancyMenu::touchPopup(QWidget *widget, QTouchEvent *ev, int firstTouch) {
    Q_ASSERT(ev->type() == QEvent::TouchBegin);
    Q_ASSERT(ev->isAccepted());

    //для начала найдем, где находится "главный палец"
    const QList<QTouchEvent::TouchPoint>& pts = ev->touchPoints();
    int origin = -1;
    for(int n = 0; n < pts.size(); ++n) {
        if (pts[n].id() == firstTouch) {
            origin = n;
            break;
        }
    }
    if (origin < 0)
        //такого по идее быть не должно, но оно есть и мы обиделись
        return false;

    QPointF originPointF = pts[origin].startScreenPos();
    QPoint  originPoint(originPointF.x(), originPointF.y());
    if (!popupInternal(widget, originPoint))
        return false;

    d->mTouched      = true;
    return true;
}
bool FancyMenu::canHandleTouch(QTouchEvent *event) {
    Q_ASSERT(event->type() == QEvent::TouchBegin);
    if (!isVisible())
        return false;
    const QList<QTouchEvent::TouchPoint>& pts = event->touchPoints();
    for(int i = 0; i < pts.size(); ++i) {
        qreal dx = pts[i].screenPos().x() - d->mPopupPoint.x();
        qreal dy = pts[i].screenPos().y() - d->mPopupPoint.y();
        qreal g  = sqrt(dx*dx+dy*dy);
        if (g < (3.7f/2.5f)*d->mRadius)
            return true;
    }
    return false;
}
bool FancyMenu::touchUpdate(QTouchEvent *ev) {
    Q_ASSERT(ev->isAccepted());
    Q_ASSERT(ev->type() != QEvent::TouchBegin);
    if (!isVisible())
        return false;
    if (!d->mTouched)
        return false;

    switch(ev->type()) {
    case QEvent::TouchUpdate:
        handleTouch(ev);;
        break;
    case QEvent::TouchEnd:
        handleTouch(ev);
        d->mTouched    = false;
        d->mHovered    = -1;
        update();
        break;
    case QEvent::TouchCancel:
        d->mTouched    = false;
        d->mHovered    = -1;
        update();
        break;
    default:
        qFatal("unexpected event type");
    }
    return true;
}
bool FancyMenu::event(QEvent *event) {
    auto type = event->type();
    switch(type) {
    case QEvent::TouchBegin:
        event->accept();
        d->mTouched = true;
        handleTouch(static_cast<QTouchEvent*>(event));
        break;
    case QEvent::TouchUpdate:
    case QEvent::TouchEnd:
    case QEvent::TouchCancel:
        touchUpdate(static_cast<QTouchEvent*>(event));
        break;
    default:
        return QWidget::event(event);
    }
    return true;
}
void FancyMenu::handleTouch(QTouchEvent *event) {
    bool repaint = false;
    QPointF center(d->mPopupPoint);
    const QList<QTouchEvent::TouchPoint>& pts = event->touchPoints();

    float br1    = (1.3f/2.5f)*d->mRadius;
    float br2    = (3.7f/2.5f)*d->mRadius;

    //сперва скроллим
    float scroll = 0.0f;
    for(int i = 0; i < pts.size(); ++i) {
        QPointF prev = pts[i].lastScreenPos()-center;
        QPointF now  = pts[i].screenPos()-center;
        float r1 = prev.x()*prev.x()+prev.y()*prev.y();
        float r2 = now.x()*now.x()+now.y()*now.y();
        auto state = pts[i].state();

        if (state == Qt::TouchPointMoved ||
            state == Qt::TouchPointReleased)
        {
            //если палец внутри круга меню, мы ничего не
            if (r1 > br2 && r2 > br2) {
                float angle1 = atan2(prev.y(), prev.x());
                float angle2 = atan2(now.y(),  now.x());
                //выберем наименьшее направление скроллинга
                //то есть если углы 10 градусов и 20 градусов,
                //то выбор между 10 и 360-10.
                float d = angle2 - angle1;
                if (d <= M_PI)
                    d += M_PI;
                if (d >= M_PI)
                    d -= M_PI;
                scroll += d;
            }
        }
    }
    if (fabsf(scroll) > 1e-3) {
        repaint = true;
        doScroll(scroll);
    }
    //затем тычем
    int   hovered= -1;
    int   prevPointType = 0;
    bool  makeEvent = false;
    for(int i = 0; i < pts.size(); ++i) {
        QPointF now  = pts[i].screenPos()-center;
        float r2 = now.x()*now.x()+now.y()*now.y();
        auto state = pts[i].state();

        if (r2 > br1 && r2 < br2) {
            //нажатая точка "сильнее" сдвинутой точки,
            //нажатая > сдвинутая > стационарная
            int pointType = 0;
            if (state == Qt::TouchPointPressed)
                pointType = 3;
            else if (state == Qt::TouchPointMoved)
                pointType = 2;
            else if (state == Qt::TouchPointReleased)
                pointType = 1;

            if (pointType > prevPointType) {
                int h = actionAt(QPoint(now.x(),now.y()));
                if (h >= 0) {
                    prevPointType = pointType;
                    hovered = h;
                    makeEvent = (state == Qt::TouchPointReleased);
                }
            }
        }
    }
    if (hovered >= 0 && makeEvent && pts.size() == 1) {
        repaint = true;
        d->mActions.at(hovered)->activate(QAction::Trigger);
        if (d->mActions.at(hovered)->isCheckable()) {
            hide();
            return;
        }
    }
    if (hovered != d->mHovered)
        repaint = true;
    d->mHovered = hovered;

    if (repaint)
        update();
}
void FancyMenu::doScroll(float dw) {
    float n = d->mActions.size();
    d->mScroll += dw;
    while (d->mScroll >= n)
        d->mScroll -= n;
    while (d->mScroll < 0)
        d->mScroll += n;
}

void FancyMenu::sortActions() {
    qSort(d->mActions.begin(), d->mActions.end(), Action::compareActions);
}

void FancyMenu::actionChanged() {
    if (!isVisible())
        return;
    QObject * s = sender();
    if (!s)
        return;
    Action  * a = qobject_cast<Action*>(s);
    if (!a)
        return;
    if (a->visibilityChanged()) {
        hide();
        return;
    }
    sortActions();
    update();
}
void FancyMenu::focusOutEvent(QFocusEvent *event) {
    QWidget::focusOutEvent(event);
    hide();
}
void FancyMenu::hide() {
    d->mClicked = Qt::NoButton;
    d->mTouched = false;
    d->mHovered = -1;
    emit hidden();
    QWidget::hide();
}


}}
