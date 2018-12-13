#include "detaileditemdelegate.h"
#include <QStyleOptionViewItem>
#include <QFontMetrics>
#include <QPainter>

namespace K { namespace Core {

class DetailedItemDelegatePrivate {
public:
    int  mFontSize;
    int  mCellHeight;
    QPen mMainPen;
    QPen mAuxPen;
    QWidget * mWidget;
};

DetailedItemDelegate::DetailedItemDelegate(QWidget *widget)
    : QAbstractItemDelegate(widget)
    , d(new DetailedItemDelegatePrivate)
{
    d->mWidget = widget;
    QFontMetrics m(widget->font());
    d->mFontSize = m.height() + m.descent();
    d->mCellHeight = d->mFontSize*2 + 3;
}

DetailedItemDelegate::~DetailedItemDelegate()
{
    delete d;
}

void DetailedItemDelegate::setColors(const QColor &c1, const QColor &c2) {
    d->mMainPen = QPen(c1);
    d->mAuxPen = QPen(c2);
}

QSize DetailedItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const {
    Q_UNUSED(option);
    Q_UNUSED(index);
    int w = d->mWidget->width();
    return QSize(qMax(72, w-30), d->mCellHeight+6);
}

void DetailedItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    static QTextOption opt;
    auto model = index.model();
    QVariant icon = model->data(index, Qt::DecorationRole);
    QVariant data = model->data(index, Qt::DisplayRole);
    QVariant aux  = model->data(index, Qt::UserRole);
    QRect r = option.rect;

    QIcon i = icon.value<QIcon>();
    if (!i.isNull()) {
        QPixmap p = i.pixmap(d->mCellHeight);
        painter->drawPixmap(r.left()+3,
                            r.top() + (r.height()>>1) - (p.height()>>1),
                            p);
    } else {
        QBrush b = icon.value<QBrush>();
        painter->setPen(Qt::NoPen);
        painter->setBrush(b);
        painter->drawRect(r.left()+3,
                          r.top()+3,
                          d->mCellHeight,
                          d->mCellHeight);
    }

    painter->setBrush(Qt::NoBrush);
    int x = r.left() + 6 + d->mCellHeight;
    QRect textRect(x, r.top() + 3,
                   r.right() - x, d->mFontSize);
    QRect auxRect (x, r.bottom() - 3 - d->mFontSize,
                   r.right() - x, d->mFontSize);
    painter->setPen(d->mMainPen);
    painter->drawText(textRect, Qt::TextSingleLine, data.toString());
    painter->setPen(d->mAuxPen);
    painter->drawText(auxRect, Qt::TextSingleLine, aux.toString());
}
}}
