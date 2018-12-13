#pragma once

#include "coregui_global.h"
#include <QWidget>
#include <QTouchEvent>
#include <QElapsedTimer>
#include "fancymenu.h"

namespace K {
namespace Core {

class ScrollWidgetPrivate;
class COREGUISHARED_EXPORT ScrollWidget : public QWidget
{
    Q_OBJECT
public:
    ScrollWidget(QWidget * parent = nullptr);
    ~ScrollWidget();

    void setEnabled(bool);
    void setBoundary(const QRectF& boundary);
    void changeZoom(char action, qreal change);
    void setScrollAdjustment(qreal adj);
    void zoomToRegion(const QRectF& region);
    void zoomToPoint(const QPointF& point, qreal zoom);
protected:
    virtual void positionChanged(const QPointF& dpos, qreal expansion, qreal rotation) = 0;
    virtual void scrollActive(bool) = 0;
    virtual bool beginClick(const QPointF& point, Qt::KeyboardModifiers mod) = 0;
    virtual bool completeClick() = 0;
    virtual void cancelClick() = 0;
    virtual QList<Action*> menuClick(const QPointF& point) = 0;
    virtual void move(const QPointF& point, bool clicked) = 0;
    virtual void release(const QPointF& point) = 0;
    virtual void scrollStarted() = 0;
    virtual void scrollFinished() = 0;

    void setZoomLimits(qreal minzoom, qreal maxzoom, qreal step);

    QTransform getTransform() const;
protected:
    bool event(QEvent *event) override;
    void timerEvent(QTimerEvent *) override;
    void resizeEvent(QResizeEvent *) override;
    void focusOutEvent(QFocusEvent *) override;
    void mousePressEvent(QMouseEvent *) override;
    void mouseMoveEvent(QMouseEvent *) override;
    void mouseReleaseEvent(QMouseEvent *) override;
    void wheelEvent(QWheelEvent *) override;
private:
    ScrollWidgetPrivate * d;
    //=================================================
    //current parameters
    QPointF translate(const QPointF& pt);
    QPointF scrollDir(const QPointF& dp);
    void clampCurrentPos();
    //=================================================
    //touch parameters
    bool shouldIgnore(QEvent * event);
    void beginTouch(QEvent * event);
    void updateTouch(QEvent * event);
    //=================================================
    //mouse parameters
    void updateMouse(QMouseEvent * ev);
    //=================================================
    //click parameters
    void endAction();
};

} //namespace Widgets;
} //namespace K;

