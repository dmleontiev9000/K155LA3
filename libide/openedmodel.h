#pragma once

#include <QAbstractItemModel>
#include <QIcon>
#include "document.h"

namespace K {
namespace IDE {

class OpenedDocumentModelPrivate;
class LIBIDESHARED_EXPORT OpenedDocumentModel
        : public QAbstractItemModel
{
public:
    OpenedDocumentModel(QObject * parent);
    ~OpenedDocumentModel();

    Document    * document(const QModelIndex& index) const;
    Document    * document(const QString& location) const;
    QModelIndex   index(Document * document) const;
    void          addDocument(Document * doc);
private slots:
    void          removeDocument(QObject* document);
private:
    QModelIndex   index(int row, int column, const QModelIndex &parent) const;
    QModelIndex   parent(const QModelIndex &child) const;
    QVariant      data(const QModelIndex &index, int role) const;
    int           rowCount(const QModelIndex &parent) const;
    int           columnCount(const QModelIndex &parent) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    bool          setData(const QModelIndex &index, const QVariant &value, int role);
    QModelIndex   indexFor(void * e) const;
    void          deleteItem(void * e, bool recurse = false);

    OpenedDocumentModelPrivate * d;
};

}
}
