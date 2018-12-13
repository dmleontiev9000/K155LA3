#pragma once

#include "coregui_global.h"
#include <QAbstractItemDelegate>
#include <QPen>

namespace K {
namespace Core {

class DetailedItemDelegatePrivate;
class COREGUISHARED_EXPORT DetailedItemDelegate : public QAbstractItemDelegate
{
public:
    DetailedItemDelegate(QWidget * widget);
    virtual ~DetailedItemDelegate();
    void setColors(const QColor& c1, const QColor& c2);
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
private:
    DetailedItemDelegatePrivate* d;
};

} //namespace Widgets
} //namespace K

