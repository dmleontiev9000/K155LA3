#pragma once
#include "coregui_global.h"
#include <QAction>
#include <QColor>
namespace K {
namespace Core {

class ActionPrivate;
class COREGUISHARED_EXPORT Action : public QAction
{
    Q_OBJECT
    Q_PROPERTY(int order READ order WRITE setOrder NOTIFY reordered)
    Q_PROPERTY(QColor background READ background WRITE setBackground)
public:
    Action(const QIcon& icon,
           const QString& text,
           QObject * parent = 0);
    Action(const QIcon& icon,
           const QString& text,
           QColor bg,
           QObject * parent = 0);
    virtual ~Action();
    QColor background() const;
    void   setBackground(const QColor& color);
    int    order() const;
    void   setOrder(int order);
    bool   visibilityChanged();
    static bool compareActions(const Action * ac1, const Action * ac2);

    virtual void activateWithContext(void * context);
signals:
    void   reordered();
private:
    ActionPrivate * d;
};

} //namespace Widgets;
} //namespace K;
