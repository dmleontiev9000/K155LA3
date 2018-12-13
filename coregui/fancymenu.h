#pragma once
#include "coregui_global.h"
#include <QWidget>
#include <QDesktopWidget>
#include <QTouchEvent>
#include "action.h"
namespace K {
namespace Core {

class FancyMenuPrivate;
class COREGUISHARED_EXPORT FancyMenu : public QWidget
{
    Q_OBJECT
public:
    FancyMenu(int iconSize=16);
    virtual ~FancyMenu();

    //provides menu with actions
    void setActions(const QList<Action*>& actions);
    //sets allowed regions for menu
    void setRegions(int regions);
    //shows popup in normal way(on click, etc)
    bool popup(QWidget * widget, const QPoint& point);
    //touch event interface: external widget may request
    //menu on touch event, but we won't be able to receive
    //these events. the following api makes possible
    //external widget to teleport touch events into our menu
    bool touchPopup(QWidget *widget, const QPointF& point);
    bool touchPopup(QWidget * widget, QTouchEvent * event, int firstTouch);
    bool canHandleTouch(QTouchEvent * event);
    bool touchUpdate(QTouchEvent * event);
public slots:
    void hide();
    void sortActions();
signals:
    void hidden();
private slots:
    void actionChanged();
private:
    bool event(QEvent * event);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void keyPressEvent(QKeyEvent *event);
    void paintEvent(QPaintEvent *event);
    void focusOutEvent(QFocusEvent *event);
    void wheelEvent(QWheelEvent *event);

    bool popupInternal(QWidget * widget, const QPoint& point);
    int  actionAt(const QPoint& pt);
    void drawIcon(QPainter& painter, float phi, float dphi, int index);
    void handleTouch(QTouchEvent * event);
    void doScroll(float dw);

    FancyMenuPrivate * d;
};

} //namespace Widgets;
} //namespace K;
