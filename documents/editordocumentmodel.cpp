#include "editordocumentmodel.h"

namespace K { namespace IDE {

class EditorDocumentModelPrivate {
public:
    QList<Document *> mDocuments;
};

EditorDocumentModel::EditorDocumentModel(QObject * parent)
    : QAbstractListModel(parent)
    , d(new EditorDocumentModelPrivate)
{
}
EditorDocumentModel::~EditorDocumentModel()
{
    delete d;
}

QVariant EditorDocumentModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (index.column() != 0 || index.row() < 0 || index.row() >= d->mDocuments.size())
        return QVariant();

    switch(role) {
    case Qt::DecorationRole:
        return d->mDocuments[index.row()]->getIcon();
    case Qt::DisplayRole:
        if (d->mDocuments[index.row()]->isModified())
            return d->mDocuments[index.row()]->getFilename()+" *";
        return d->mDocuments[index.row()]->getFilename();
    default:
        return QVariant();
    }
}
int EditorDocumentModel::rowCount() const
{
    return d->mDocuments.size();
}
int EditorDocumentModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return d->mDocuments.size();
}
Qt::ItemFlags EditorDocumentModel::flags(const QModelIndex &index) const {
    Q_UNUSED(index);
    return Qt::ItemIsEnabled|Qt::ItemIsSelectable;
}
void EditorDocumentModel::attachDocument(Document *doc) {
    if (!doc) {
        emit activateDocument(-1);
        return;
    }
    int n = d->mDocuments.indexOf(doc);
    if (n < 0) {
        connect(doc, SIGNAL(iconChanged(Document*)), this, SLOT(docIconChanged(Document*)));
        connect(doc, SIGNAL(renamed(Document*)), this, SLOT(docChanged(Document*)));
        connect(doc, SIGNAL(changed(Document*,bool)), this, SLOT(docChanged(Document*)));
        connect(doc, SIGNAL(closed(Document*)), this, SLOT(removeDocument(Document*)));
        QModelIndex mi;
        beginInsertRows(mi, d->mDocuments.size(), d->mDocuments.size());
        d->mDocuments.append(doc);
        endInsertRows();
        n = d->mDocuments.size()-1;
    }
    emit activateDocument(n);
}
void EditorDocumentModel::removeDocument(Document *doc) {
    int n = d->mDocuments.indexOf(doc);
    if (n >= 0) {
        QModelIndex mi;
        beginRemoveRows(mi, n, n);
        d->mDocuments.removeAt(n);
        disconnect(doc, 0, this, 0);
        endRemoveRows();
    }
}
Document * EditorDocumentModel::at(const QModelIndex &index) const {
    if (!index.isValid())
        return nullptr;
    if (index.column() != 0 || index.row() < 0 || index.row() >= d->mDocuments.size())
        return nullptr;
    return d->mDocuments[index.row()];
}
Document * EditorDocumentModel::at(int index) const {
    if (index < 0 || index > d->mDocuments.size())
        return nullptr;
    return d->mDocuments[index];
}
void EditorDocumentModel::docChanged(Document *doc) {
    int n = d->mDocuments.indexOf(doc);
    if (n >= 0) {
        static const QVector<int> roles({Qt::DisplayRole});
        QModelIndex i = index(n, 0);
        emit dataChanged(i, i, roles);
    }
}
void EditorDocumentModel::docIconChanged(Document *doc) {
    int n = d->mDocuments.indexOf(doc);
    if (n >= 0) {
        static const QVector<int> roles({Qt::DecorationRole});
        QModelIndex i = index(n, 0);
        emit dataChanged(i, i, roles);
    }
}
}}
