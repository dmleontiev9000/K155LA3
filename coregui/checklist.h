#pragma once
#include "coregui_global.h"
#include <QWidget>

namespace K {
namespace Core {

class CheckListPrivate;
class COREGUISHARED_EXPORT CheckList : public QWidget
{
    Q_OBJECT
public:
    enum State { DISABLED, DONT_CARE, PENDING, BAD, GOOD };
    CheckList(QWidget * parent = nullptr);
    virtual ~CheckList();

    void addCheckPoint(int id,
                       const QString& name,
                       const QString& description,
                       State state = State::DONT_CARE);
    void addWidget(QWidget * widget);
    void addLayout(QLayout * layout);
    void complete();

    void setCheckPointState(int id, int count, State state);
    void gotoCheckPoint(int id);
private:
    CheckListPrivate * d;
};

} //namespace Widgets;
} //namespace K;

