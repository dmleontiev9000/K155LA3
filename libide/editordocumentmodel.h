#pragma once

#include "libide_global.h"
#include "document.h"
#include <QAbstractListModel>

namespace K {
namespace IDE {

class EditorDocumentModelPrivate;
class EditorDocumentModel
        : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit EditorDocumentModel(QObject * parent = 0);
    ~EditorDocumentModel();

    void attachDocument(Document * doc);
    Document  * at(const QModelIndex& index) const;
    Document  * at(int index) const;
    QVariant    data(const QModelIndex &index, int role) const;
    int         rowCount(const QModelIndex &parent) const;
    int         rowCount() const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
signals:
    void activateDocument(int);
private slots:
    void removeDocument(Document * doc);
    void docChanged(Document * doc);
    void docIconChanged(Document * doc);

private:
    EditorDocumentModelPrivate * d;
};

} //namespace IDE
} //namespace K
