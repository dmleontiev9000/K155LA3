#pragma once
#include "coregui_global.h"
#include "clock.h"
#include <QWidget>
#include <QElapsedTimer>

namespace K {
namespace Core {

class DigitalClockPrivate;
class COREGUISHARED_EXPORT DigitalClock : public QWidget
{
    Q_OBJECT
public:
    DigitalClock(int iconSize=16, QWidget * parent = nullptr);
    virtual ~DigitalClock();

    void setShowRate(bool show);
    qreal rate() const;
    qreal effectiveRate() const;
signals:
    void setTime(qint64 time);
    void setRate(qreal rate);
    void play(bool on);
public slots:
    void playing(char newstate);
    void displayTime(qint64 time, qreal rate);
private:
    bool event(QEvent * event);
    void focusOutEvent(QFocusEvent *event);
    void wheelEvent(QWheelEvent *event);
    void paintEvent(QPaintEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    QSize sizeHint() const override;

    DigitalClockPrivate * d;
};

} //namespace Widgets;
} //namespace K;

