#include "bookmarkmodel.h"

namespace K { namespace IDE {

class BookmarkModelPrivate {
public:
    struct Entry {
        QString text;
        QString id;
        int     line;
        Document * document;
    };
    QList<Entry> entries;
};

BookmarkModel::BookmarkModel(QObject * parent)
    : QAbstractListModel(parent)
    , d(new BookmarkModelPrivate)
{
}
BookmarkModel::~BookmarkModel()
{
    delete d;
}

Document * BookmarkModel::document(const QModelIndex &index,
                                   QString *id,
                                   int * line) const
{
    if (!index.isValid())
        return nullptr;

    int r = index.row();
    if (r >= d->entries.size())
        return nullptr;

    if (id)
        *id = d->entries[r].id;
    if (line)
        *line = d->entries[r].line;
    return d->entries[r].document;
}
bool BookmarkModel::addBookmark(Document *doc, int line, const QString &id, const QString &tooltip) {
    int pos = 0;
    for(pos = 0; pos < d->entries.size(); ++pos) {
        const BookmarkModelPrivate::Entry& e = d->entries.at(pos);
        if (e.document == doc) {
            if (e.line == line &&
                e.id   == id)
                return false;
            if (e.line >= 0 && line > 0 && e.line > line)
                break;
            if (e.id.compare(id) > 0)
                break;
        } else if (e.document->getPath().compare(doc->getPath()) > 0)
            break;
    }
    BookmarkModelPrivate::Entry e;
    e.document = doc;
    e.line = line;
    e.id = id;
    e.text = tooltip;
    beginInsertRows(QModelIndex(), pos, pos);
    d->entries.insert(pos, e);
    endInsertRows();
    connect(doc, SIGNAL(destroyed(QObject*)), this, SLOT(objectRemoved(QObject*)));
    return true;
}
void BookmarkModel::removeBookmark(Document *doc, int line, const QString &id) {
    int numentries = 0;
    int pos = 0;
    for(pos = 0; pos < d->entries.size(); ++pos) {
        const BookmarkModelPrivate::Entry& e = d->entries.at(pos);
        if (e.document != doc)
            continue;
        ++numentries;
        if (e.line == line && e.id == id)
            break;
    }
    if (pos == d->entries.size())
        return;

    beginRemoveRows(QModelIndex(), pos, pos);
    d->entries.removeAt(pos);
    endRemoveRows();
    if (numentries > 1)
        disconnect(doc, 0, this, 0);
}
void BookmarkModel::objectRemoved(QObject *obj) {
    int pos = 0;
    beginResetModel();
    for(;;) {
        const BookmarkModelPrivate::Entry& e = d->entries.at(pos);
        if (static_cast<QObject*>(e.document) == obj) {
            d->entries.removeAt(pos);
            continue;
        } else ++pos;

        if (pos == d->entries.size())
            break;
    }
    endResetModel();
}

QVariant BookmarkModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid())
        return QVariant();

    if (index.row() >= d->entries.size())
        return QVariant();

    switch(role) {
    case Qt::DisplayRole:
        return d->entries[index.row()].text;
    case Qt::UserRole:
        return d->entries[index.row()].document->getFilename();
    case Qt::UserRole+1:
        if (d->entries[index.row()].line >= 0)
            return QString::number(d->entries[index.row()].line);
        return QVariant();
    default:
        return QVariant();
    }
}
int BookmarkModel::rowCount(const QModelIndex &parent) const {
    if (parent.isValid())
        return 0;
    return d->entries.size();
}
Qt::ItemFlags BookmarkModel::flags(const QModelIndex &index) const {
    Q_UNUSED(index);
    return Qt::ItemIsSelectable|Qt::ItemIsEnabled;
}

}}
