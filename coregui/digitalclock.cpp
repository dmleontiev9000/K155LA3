#include "digitalclock.h"
#include "style.h"
#include <QPainter>
#include <QLinearGradient>
#include <QWheelEvent>
#include <QMouseEvent>
#include <math.h>
namespace K { namespace Core {

class DigitalClockPrivate {
public:
    int             mSize;
    bool            mShowRate     = false;
    char            mState        = 's';
    QBrush          mBgBrush;

    QPoint          mClickPoint;
    Qt::MouseButton mClickButton  = Qt::NoButton;
    bool            mTouchActive  = false;
    char            mTouchScroll  = 0;
    int             mTouchId;
    qreal           mTouchPos;
    QElapsedTimer   mElapsed;

    int             mRate         = 100;
    qreal           mEffRate      = NAN;
    qint64          mTime         = 0;
};

DigitalClock::DigitalClock(int iss, QWidget *parent)
    : QWidget(parent)
    , d(new DigitalClockPrivate)
{
    d->mSize = iss;
    setAttribute(Qt::WA_AcceptTouchEvents, true);
    setAttribute(Qt::WA_TranslucentBackground, true);

    QLinearGradient linear(0.0, 0.0, 0.0, 1.0);
    linear.setColorAt(0.0,        QColor(0,  0,  0,  0));
    linear.setColorAt(0.5,        QColor(1,  1,  1,  1));
    linear.setColorAt(1.0,        QColor(1,  1,  1,  1));
    linear.setCoordinateMode(QGradient::ObjectBoundingMode);
    d->mBgBrush = QBrush(linear);
}

DigitalClock::~DigitalClock()
{
    delete d;
}
qreal DigitalClock::rate() const { return d->mRate; }
qreal DigitalClock::effectiveRate() const { return d->mEffRate; }

void DigitalClock::setShowRate(bool show) {
    d->mShowRate = show;
    updateGeometry();
}
QSize DigitalClock::sizeHint() const {
    int iss = d->mSize;
    int letterWidth = (iss/2)+(iss/4);
    int dotWidth    = (iss/2);
    int timeWidth   = letterWidth*2 /*hours*/
                    + dotWidth
                    + letterWidth*2 /*minutes*/
                    + dotWidth
                    + letterWidth*2 /*seconds*/
                    + dotWidth
                    + letterWidth*3 /*milliseconds*/
                    ;
    //2+2+2+3 = 9
    //3/2 iss + 9/2 iss + 9/4 iss =
    //6iss + 2iss + iss/4

    if (d->mShowRate) {
        /*2*/
        timeWidth += iss /*padding*/
                   + 4*letterWidth /*X+target rate*/
                  ;
    }
    return QSize(iss+dotWidth+/*play icon + spacing*/
                 timeWidth+8 /*indicators + spacing*/,
                 iss+12);
}
static void drawNumber(QPainter * p, int x, int y, int iss, int n) {
    //шаблоны семисегментного индикатора
    static const int patterns[10] = {
        0b1110111, //0
        0b0100100, //1
        0b1011101, //2
        0b1101101, //3
        0b0101110, //4
        0b1101011, //5
        0b1111011, //6
        0b0100101, //7
        0b1111111, //8
        0b1101111, //9
    };
    if (n < 0 || n > 9)
        return;

    int is2 = iss/2;
    int i = patterns[n];
    if (i & 1)
        p->drawLine(x,     y,     x+is2, y);
    if (i & 2)
        p->drawLine(x,     y,     x,     y+is2);
    if (i & 4)
        p->drawLine(x+is2, y,     x+is2, y+is2);
    if (i & 8)
        p->drawLine(x,     y+is2, x+is2, y+is2);
    if (i & 16)
        p->drawLine(x,     y+is2, x,     y+iss);
    if (i & 32)
        p->drawLine(x+is2, y+is2, x+is2, y+iss);
    if (i & 64)
        p->drawLine(x,     y+iss, x+is2, y+iss);
}
void DigitalClock::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);
    int iss = d->mSize;
    int is2 = (iss/2);
    int is4 = (iss/4);
    int is8 = (iss/8);
    int xoffset = 4, yoffset = 4;

    QPainter p;
    p.begin(this);

    QBrush redb(QColor(255,128,128));
    QPen red(QColor(255,128,128));
    red.setWidth(is8);
    p.setPen(red);
    p.setBrush(redb);
    /* draw state indicator */
    switch(d->mState) {
    case 'p':
        p.drawRect(xoffset+is4-is8,
                   yoffset+is8,
                   is8, iss-is4);
        p.drawRect(xoffset+is2+is8,
                   yoffset+is8,
                   is8, iss-is4);
        break;
    case '>': {
        QPoint pt[3];
        pt[0].setX(xoffset+is8); pt[0].setY(yoffset+is8);
        pt[1].setX(xoffset+is8); pt[1].setY(yoffset+iss-is8);
        pt[2].setX(xoffset+iss-is8); pt[2].setY(yoffset+is2);
        p.drawPolygon(pt, 3);
        break;}
    case 's':
        p.drawRect(xoffset+is8,
                   yoffset+is8,
                   iss-is4, iss-is4);
        break;
    default:;
    }
    xoffset += iss + is2;

    int indicatorbase = xoffset;
    /* draw time indicator */
    if (d->mTime >= 0) {
        int h2 = (int)(d->mTime/36000000);
        qint64 z = d->mTime % 36000000;
        int h1 = (int)(z/3600000);
        z      = z % 3600000;

        int m2 = (int)(z/600000);
        z      = z % 600000;
        int m1 = (int)(z/60000);
        z      = z % 60000;

        int s2 = (int)(z/10000);
        z      = z % 10000;
        int s1 = (int)(z/1000);
        z      = z % 1000;

        int u3 = (int)(z/100);
        z      = z % 100;
        int u2 = (int)(z/10);
        int u1 = (int)(z%10);

        drawNumber(&p, xoffset, yoffset, iss, h2); xoffset+= is2+is4;
        drawNumber(&p, xoffset, yoffset, iss, h1); xoffset+= is2+is4;
        p.drawPoint(xoffset+is4, yoffset+is4);
        p.drawPoint(xoffset+is4, yoffset+is2+is4);
        xoffset += is2;

        drawNumber(&p, xoffset, yoffset, iss, m2); xoffset+= is2+is4;
        drawNumber(&p, xoffset, yoffset, iss, m1); xoffset+= is2+is4;
        p.drawPoint(xoffset+is4, yoffset+is4);
        p.drawPoint(xoffset+is4, yoffset+is2+is4);
        xoffset += is2;

        drawNumber(&p, xoffset, yoffset, iss, s2); xoffset+= is2+is4;
        drawNumber(&p, xoffset, yoffset, iss, s1); xoffset+= is2+is4;
        p.drawPoint(xoffset+is4, yoffset+iss);
        xoffset += is2;

        drawNumber(&p, xoffset, yoffset, iss, u3); xoffset+= is2+is4;
        drawNumber(&p, xoffset, yoffset, iss, u2); xoffset+= is2+is4;
        drawNumber(&p, xoffset, yoffset, iss, u1); xoffset+= is2+is4;
    } else {
        xoffset += 8*iss+is4;
    }
    /* draw rate input */
    if (d->mShowRate) {
        xoffset += iss;
        p.drawLine(xoffset+is4-is8,
                   yoffset+is2-is8,
                   xoffset+is4+is8,
                   yoffset+is2+is8);
        p.drawLine(xoffset+is4-is8,
                   yoffset+is2+is8,
                   xoffset+is4+is8,
                   yoffset+is2-is8);
        xoffset += is2+is4;

        int r3 = d->mRate / 100;
        int r2 = (d->mRate % 100) / 10;
        int r1 = d->mRate % 10;
        drawNumber(&p, xoffset, yoffset, iss, r3); xoffset+= is2+is4;
        drawNumber(&p, xoffset, yoffset, iss, r2); xoffset+= is2+is4;
        drawNumber(&p, xoffset, yoffset, iss, r1); xoffset+= is2+is4;
    }

    /* draw rate indicator */
    if (!std::isnan(d->mEffRate)) {
        int w = (int)((xoffset-indicatorbase-10)*d->mEffRate)+10;
        p.setCompositionMode(QPainter::CompositionMode_Source);
        p.setBrush(d->mBgBrush);
        p.drawRect(indicatorbase, yoffset+iss, w, 8);
        p.setCompositionMode(QPainter::CompositionMode_ColorBurn);
        if (d->mEffRate == 1.0) {
            p.setBrush(QColor(128,255,128));
        } else {
            p.setBrush(QColor(255,96+(int)(95.0*d->mEffRate),96));
        }
        p.drawRect(indicatorbase, yoffset+iss, w, 8);
    }
}
void DigitalClock::displayTime(qint64 time, qreal rate) {
    d->mTime = qMax(0LL, time);
    d->mEffRate = rate;
    update();
}
void DigitalClock::playing(char newstate) {
    d->mState = newstate;
    update();
}
//=================================================
void DigitalClock::focusOutEvent(QFocusEvent *event) {
    QWidget::focusOutEvent(event);
    d->mClickButton = Qt::NoButton;
    d->mTouchActive = false;
}
void DigitalClock::mousePressEvent(QMouseEvent *event) {
    if (event->buttons() != event->button()) {
        mouseMoveEvent(event);
        return;
    }

    event->accept();
    d->mClickButton = event->button();
    d->mClickPoint  = event->pos();
}
void DigitalClock::mouseMoveEvent(QMouseEvent *event) {
    event->accept();
    if (d->mClickButton == Qt::NoButton)
        return;
    if ((event->pos()-d->mClickPoint).manhattanLength() > 3) {
        d->mClickButton = Qt::NoButton;
        return;
    }
}
void DigitalClock::mouseReleaseEvent(QMouseEvent *event) {
    event->accept();
    if (d->mClickButton == Qt::NoButton)
        return;
    if ((event->pos()-d->mClickPoint).manhattanLength() > 3) {
        d->mClickButton = Qt::NoButton;
        return;
    }
    if (d->mClickButton == event->button()) {
        switch(d->mState) {
        case 's':
        case 'p':
            emit play(true);
            break;
        case '>':
            emit play(false);
            break;
        default:;
        }
    }
}
//=================================================
bool DigitalClock::event(QEvent *event) {
    QTouchEvent * t;
    switch(event->type()) {
    case QEvent::TouchBegin:
        event->accept();
        t = static_cast<QTouchEvent*>(event);
        if (t->touchPoints().isEmpty())
            break;
        d->mTouchActive = true;
        d->mTouchScroll  = 0;
        d->mTouchId = t->touchPoints()[0].id();
        d->mTouchPos = t->touchPoints()[0].startPos().x();
        d->mElapsed.start();
        break;
    case QEvent::TouchUpdate:
        if (!d->mTouchActive)
            break;
        goto _update;
    case QEvent::TouchEnd:
        if (!d->mTouchActive)
            break;
        d->mTouchActive = false;
    _update:
        t = static_cast<QTouchEvent*>(event);
        for(int i = 0; i < t->touchPoints().size(); ++i)
        {
            const QTouchEvent::TouchPoint& pt = t->touchPoints()[i];
            if (pt.id() == d->mTouchId)
            {
                if (!d->mTouchScroll)
                {
                    QPointF dp = pt.pos() - pt.startPos();
                    if (dp.manhattanLength() > 3.0)
                    {
                        d->mTouchScroll = pt.startPos().x() > (4.0+d->mSize*7.75) ?
                                    'r':'t';
                    }
                    else if (pt.state() == Qt::TouchPointReleased)
                    {
                        d->mTouchActive = false;
                        switch(d->mState) {
                        case 's':
                        case 'p':
                            emit play(true);
                            break;
                        case '>':
                            emit play(false);
                            break;
                        default:;
                        }
                    }
                }
                if (d->mTouchScroll) {
                    qreal dx = pt.pos().x() - d->mTouchPos;
                    if (d->mTouchScroll == 'r')
                    {
                        d->mRate = qBound(0, d->mRate+int(dx*0.33), 999);
                        emit setRate(d->mRate);
                    }
                    else
                    {
                        d->mTouchPos = pt.pos().x();
                        qint64 dt = d->mElapsed.restart();
                        qreal dtf = dt < 1 ? 1.0 : (qreal)dt;
                        dx /= dtf;
                        qreal adx = fabs(dx);
                        if (adx > 10.0)
                            dx *= (adx-9.0);
                        qint64 n = qBound(0LL, d->mTime + qint64(dx), 100000000LL);
                        emit setTime(n);
                    }
                }
                return true;
            }
        }
        d->mTouchActive = false;
        break;
    case QEvent::TouchCancel:
        d->mTouchActive = false;
        break;
    default:
        return QWidget::event(event);
    }
    return true;
}
//=================================================
void DigitalClock::wheelEvent(QWheelEvent *event) {
    int iss = d->mSize;
    int is2 = (iss/2);
    int is4 = (iss/4);
    //int is8 = (iss/8);
    int x = event->pos().x()-4;

    int dw = event->angleDelta().y();

    qint64 rate = 0;
    x -= 7*iss+is4+is2+4;
    if (x < 0) {
        rate = 1;
        do {
            //msec
            x += is2+is4;
            if (x > 0)break;
            rate*=10;
            x += is2+is4;
            if (x > 0)break;
            rate*=10;
            x += is2+is4;
            if (x > 0)break;
            x += is2;
            rate*=10;

            x += is2;
            if (x > 0) { rate = 0; break; }
            //sec
            x += is2+is4;
            if (x > 0)break;
            rate*=10;
            x += is2+is4;
            if (x > 0)break;
            rate*=60;

            x += is2;
            if (x > 0) { rate = 0; break; }
            //min
            x += is2+is4;
            if (x > 0)break;
            rate*=10;
            x += is2+is4;
            if (x > 0)break;
            rate*=60;

            x += is2;
            if (x > 0) { rate = 0; break; }
            //hr
            x += is2+is4;
            if (x > 0)break;
            rate*=10;
            x += is2+is4;
            if (x > 0)break;
            rate = 0;
        } while(false);

        if (!rate) return;
        qint64 n = qBound(0LL, d->mTime + (rate * dw / 120), 100000000LL);
        emit setTime(n);
    } else if (x > iss && x < 4*iss){
        d->mRate = qBound(0, d->mRate + dw/120, 999);
        emit setRate(qreal(d->mRate)*0.01);
    }
}

}}
