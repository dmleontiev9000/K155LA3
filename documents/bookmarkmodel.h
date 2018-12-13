#pragma once

#include <QAbstractItemModel>
#include <QIcon>
#include "document.h"

namespace K {
namespace IDE {

class BookmarkModelPrivate;
class DOCUMENTSSHARED_EXPORT BookmarkModel
        : public QAbstractListModel
{
public:
    BookmarkModel(QObject * parent);
    ~BookmarkModel();

    Document * document(const QModelIndex& index,
                        QString * id,
                        int     * line) const;

    bool       addBookmark(Document * doc,
                           int line,
                           const QString& id,
                           const QString& tooltip);
    void       removeBookmark(Document * doc,
                              int line,
                              const QString& id);
private slots:
    void objectRemoved(QObject * obj);
private:
    QVariant    data(const QModelIndex &index, int role) const;
    int         rowCount(const QModelIndex &parent) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;

    BookmarkModelPrivate * d;
};

}
}
