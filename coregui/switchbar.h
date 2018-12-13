#pragma once

#include "coregui_global.h"
#include "action.h"
#include <QWidget>

namespace K {
namespace Core {

class SwitchBarPrivate;
class COREGUISHARED_EXPORT SwitchBar : public QWidget
{
    Q_OBJECT
public:
    enum class Edge {
        Left, Right, Top, Bottom
    };

    SwitchBar(int iconsize,
              Edge edge,
              QSizePolicy::Policy policy,
              bool exclusive = false);
    virtual ~SwitchBar();

    void toggleFirst();
    bool isEmpty() const;
    void addAction(Action * action, bool exclusive);
    void removeAction(Action *action);
private slots:
    void actionChanged();
    void actionChecked();
    void actionReordered();
    void actionDeleted(QObject *o);
private:
    QSize sizeHint() const override;
    bool event(QEvent *);
    void resizeEvent(QResizeEvent * event);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void focusOutEvent(QFocusEvent *event);
    void paintEvent(QPaintEvent *);
    void leaveEvent(QEvent * event);
    void touchEvent(QEvent * event);
    void doScroll(const QPointF& delta);
    void makeVisible(Action * action);
    Action*itemAtPoint(const QPointF& pt, bool autoupdate);

    SwitchBarPrivate *d;
};

} //namespace Widgets;
} //namespace K;
