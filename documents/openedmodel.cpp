#include "openedmodel.h"
#include <style.h>
#include <icons.h>
#include <QHash>
#include <QFileInfo>
#include <QDir>

namespace K { namespace IDE {


class OpenedDocumentModelPrivate {
public:
    struct Entry {
        Entry * parent;
        QString abspath;
        QString path;
        QList<Entry*> childs;
        Document * document;
        QIcon documentIcon;
    };
    QHash<Document*,Entry*> mDocuments;
    QList<Entry*> mRoot;
};

OpenedDocumentModel::OpenedDocumentModel(QObject * parent)
    : QAbstractItemModel(parent)
    , d(new OpenedDocumentModelPrivate)
{

}
OpenedDocumentModel::~OpenedDocumentModel()
{
    for(OpenedDocumentModelPrivate::Entry * e : d->mRoot) {
        delete e;
    }
    delete d;
}
Document * OpenedDocumentModel::document(const QModelIndex &index) const {
    if (!index.isValid())
        return nullptr;

    auto e = (OpenedDocumentModelPrivate::Entry*)index.internalPointer();
    return e->document;
}
Document * OpenedDocumentModel::document(const QString &location) const {
    for(auto i = d->mDocuments.constBegin();
        i != d->mDocuments.constEnd(); ++i)
    {
        if ((*i)->abspath==location)
            return (*i)->document;
    }
    return nullptr;
}

QModelIndex OpenedDocumentModel::index(Document *document) const {
    auto i = d->mDocuments.find(document);
    if (i == d->mDocuments.end())
        return QModelIndex();

    return indexFor(i.value());
}
void OpenedDocumentModel::addDocument(K::IDE::Document *doc)
{
    Q_ASSERT(d->mDocuments.contains(doc) == false);

    Document * parent = nullptr;
    if (doc->parent()) parent = qobject_cast<Document*>(doc->parent());

    OpenedDocumentModelPrivate::Entry * base_entry = nullptr;
    if (parent) {
        auto parent_loc = d->mDocuments.constFind(parent);
        if (parent_loc != d->mDocuments.constEnd())
            base_entry = parent_loc.value();
    }

    QString rel;

    if (base_entry) {
        auto dir = QFileInfo(base_entry->abspath).dir();
        rel = dir.relativeFilePath(doc->getPath());
        int idx = rel.lastIndexOf('/');
        if (idx > 1) rel.resize(idx-1);
        else base_entry = nullptr;
    }

    if (base_entry) {
        auto i = base_entry->childs.begin();
        int at = 0;
        bool match = false;
        for(; i != base_entry->childs.end(); ++i, ++at) {
            OpenedDocumentModelPrivate::Entry * e = (*i);
            if (e->document) break;
            int cmp = e->path.compare(rel);
            if (cmp < 0) continue;
            if (cmp > 0) break;

            base_entry = e;
            match = true;
            break;
        }
        if (!match) {
            OpenedDocumentModelPrivate::Entry * dir_entry  = new OpenedDocumentModelPrivate::Entry;
            dir_entry->parent       = base_entry;
            dir_entry->path         = rel;
            dir_entry->documentIcon = Core::getIconForFile(dir_entry->abspath, "file");

            QModelIndex mi = indexFor(base_entry);
            beginInsertRows(mi, at, at);
            base_entry->childs.insert(i, dir_entry);
            endInsertRows();

            base_entry = dir_entry;
        }
    }

    QModelIndex mi;
    int         at = 0;
    if (base_entry) {
        mi = indexFor(base_entry);
        auto i = base_entry->childs.begin();
        for(; i != base_entry->childs.end(); ++i, ++at) {
            OpenedDocumentModelPrivate::Entry * e = (*i);
            if (!e->document) continue;
            if (e->path.compare(rel) > 0) break;
        }
    } else {
        auto i = d->mRoot.begin();
        for(; i != d->mRoot.end(); ++i, ++at) {
            OpenedDocumentModelPrivate::Entry * e = (*i);
            if (e->path.compare(rel) > 0) break;
        }
    }

    OpenedDocumentModelPrivate::Entry * entry  = new OpenedDocumentModelPrivate::Entry;
    entry->parent       = base_entry;
    entry->document     = doc;
    entry->path         = doc->getFilename();
    entry->abspath      = doc->getPath();
    entry->documentIcon = doc->getIcon();
    beginInsertRows(mi, at, at);
    base_entry->childs.insert(at, entry);
    endInsertRows();

    connect(doc, SIGNAL(destroyed(QObject*)), this, SLOT(removeDocument(QObject*)));
}
QModelIndex OpenedDocumentModel::indexFor(void *_e) const {
    auto e = (OpenedDocumentModelPrivate::Entry *)_e;

    int row;
    if (e->parent) {
        row = e->parent->childs.indexOf(e);
    } else {
        row = d->mRoot.indexOf(e);
    }
    Q_ASSERT(row >= 0);

    return createIndex(row, 0 , e);
}
void OpenedDocumentModel::removeDocument(QObject *obj) {
    Document * oldptr = static_cast<Document*>(obj);
    auto i = d->mDocuments.find(oldptr);
    if (i == d->mDocuments.end())
        return;

    OpenedDocumentModelPrivate::Entry * e = i.value();
    auto p = e->parent;
    if (p) {
        if (e->parent->document == nullptr &&
            e->childs.size() == 1)
        {
            e = p;
            p = p->parent;
        }
    }
    if (p) {
        int nth = p->childs.indexOf(e);
        Q_ASSERT(nth >= 0);

        beginRemoveRows(indexFor(p), nth, nth);
        p->childs.removeAt(nth);
        d->mDocuments.erase(i);
        endRemoveRows();
    } else {
        int nth = d->mRoot.indexOf(e);
        Q_ASSERT(nth >= 0);

        beginRemoveRows(QModelIndex(), nth, nth);
        d->mRoot.removeAt(nth);
        d->mDocuments.erase(i);
        endRemoveRows();
    }

    auto next = e->childs;
    delete e->document;
    delete e;
    while (!next.isEmpty()) {
        e = next.takeLast();
        next.append(e->childs);
        delete e;
    }
}
QModelIndex OpenedDocumentModel::index(int row, int column, const QModelIndex &parent) const {
    OpenedDocumentModelPrivate::Entry * e;
    if (parent.isValid()) {
        e = (OpenedDocumentModelPrivate::Entry*)parent.internalPointer();
        if (row >= e->childs.size())
            return QModelIndex();
        e = e->childs.at(row);
    } else {
        if (row >= d->mRoot.size())
            return QModelIndex();
        e = d->mRoot.at(row);
    }
    return createIndex(row, column, e);
}
QModelIndex OpenedDocumentModel::parent(const QModelIndex &child) const {
    if (!child.isValid())
        return QModelIndex();
    auto e = (OpenedDocumentModelPrivate::Entry*)child.internalPointer();
    if (!e->parent)
        return QModelIndex();
    return indexFor(e->parent);
}
int OpenedDocumentModel::rowCount(const QModelIndex &parent) const {
    if (parent.isValid()) {
        auto e = (OpenedDocumentModelPrivate::Entry*)parent.internalPointer();
        return e->childs.size();
    }
    return d->mRoot.size();
}
int OpenedDocumentModel::columnCount(const QModelIndex &parent) const {
    Q_UNUSED(parent);
    return 1;
}
QVariant OpenedDocumentModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid())
        return QVariant();
    auto e = (OpenedDocumentModelPrivate::Entry*)index.internalPointer();
    switch(role) {
    case Qt::DisplayRole:
        return e->path;
    case Qt::DecorationRole:
        return e->documentIcon;
    default:
        return QVariant();
    }
}
bool OpenedDocumentModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role != Qt::DisplayRole)
        return false;

    Document * doc = document(index);
    if (!doc) return false;

    return doc->rename(value.toString());
}

Qt::ItemFlags OpenedDocumentModel::flags(const QModelIndex &index) const {
    Q_UNUSED(index);
    return Qt::ItemIsSelectable|Qt::ItemIsEnabled;
}

}}
