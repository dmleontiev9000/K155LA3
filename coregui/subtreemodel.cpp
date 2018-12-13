#include "subtreemodel.h"
#include <QDebug>

namespace K { namespace Core {

enum {
    NOOP = 0,
    ADD_COL,
    MOVE_COL,
    REMOVE_COL,
    ADD_ROW,
    MOVE_ROW,
    REMOVE_ROW,
    RESET
};

class SubTreeModelPrivate {
public:
    QAbstractItemModel    * mModel;
    QPersistentModelIndex   mDirIndex;
    int                     mAvailableRows = 0;
    int                     mAvailableCols = 0;
    int                     mUpdateBehavior= 0;
    bool                    mWritable = false;
};


SubtreeModel::SubtreeModel(QAbstractItemModel *src, const QModelIndex& root, QObject *parent)
    : QAbstractItemModel(parent)
    , d(new SubTreeModelPrivate)
{
    d->mModel = src;
    d->mDirIndex = root;
    d->mModel->fetchMore(root);
    d->mAvailableRows = d->mModel->rowCount(root);
    d->mAvailableCols = d->mModel->columnCount(root);
    connect(d->mModel, SIGNAL(columnsAboutToBeInserted(QModelIndex,int,int)),
            this, SLOT(colsWillBeInserted(QModelIndex,int,int)));
    connect(d->mModel, SIGNAL(columnsInserted(QModelIndex,int,int)),
            this, SLOT(complete()));

    connect(d->mModel, SIGNAL(columnsAboutToBeMoved(QModelIndex,int,int,QModelIndex,int)),
            this, SLOT(colsWillBeMoved(QModelIndex,int,int,QModelIndex,int)));
    connect(d->mModel, SIGNAL(columnsMoved(QModelIndex,int,int,QModelIndex,int)),
            this, SLOT(complete()));

    connect(d->mModel, SIGNAL(columnsAboutToBeRemoved(QModelIndex,int,int)),
            this, SLOT(colsWillBeRemoved(QModelIndex,int,int)));
    connect(d->mModel, SIGNAL(columnsRemoved(QModelIndex,int,int)),
            this, SLOT(complete()));

    connect(d->mModel, SIGNAL(rowsAboutToBeInserted(QModelIndex,int,int)),
            this, SLOT(rowsWillBeInserted(QModelIndex,int,int)));
    connect(d->mModel, SIGNAL(rowsInserted(QModelIndex,int,int)),
            this, SLOT(complete()));

    connect(d->mModel, SIGNAL(rowsAboutToBeMoved(QModelIndex,int,int,QModelIndex,int)),
            this, SLOT(rowsWillBeMoved(QModelIndex,int,int,QModelIndex,int)));
    connect(d->mModel, SIGNAL(rowsMoved(QModelIndex,int,int,QModelIndex,int)),
            this, SLOT(complete()));

    connect(d->mModel, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
            this, SLOT(rowsWillBeRemoved(QModelIndex,int,int)));
    connect(d->mModel, SIGNAL(rowsRemoved(QModelIndex,int,int)),
            this, SLOT(complete()));

    connect(d->mModel, SIGNAL(modelAboutToBeReset()),
            this, SLOT(modelWillBeReset()));
    connect(d->mModel, SIGNAL(modelReset()),
            this, SLOT(complete()));

    connect(d->mModel, SIGNAL(dataChanged(QModelIndex,QModelIndex,QVector<int>)),
            this, SLOT(modelDataChanged(QModelIndex,QModelIndex,QVector<int>)));
}
SubtreeModel::~SubtreeModel() {
    delete d;
}

QModelIndex SubtreeModel::rootIndex() const {
    return d->mDirIndex;
}
QModelIndex SubtreeModel::indexOf(const QModelIndex& i) const {
    if (!d->mDirIndex.isValid())
        return QModelIndex();
    return d->mModel->index(i.row(), i.column(), d->mDirIndex);
}
void SubtreeModel::setWritable(bool en) {
    d->mWritable = en;
}
//============ROWS===============
void SubtreeModel::rowsWillBeInserted(const QModelIndex &parent, int first, int last)
{
    Q_ASSERT(d->mUpdateBehavior == 0);
    if (!d->mDirIndex.isValid()) {
        return;
    }
    if (d->mDirIndex != parent) {
        return;
    }

    if (first > d->mAvailableRows) {
        d->mUpdateBehavior = RESET;
        beginResetModel();
    } else {
        d->mUpdateBehavior = ADD_ROW;
        beginInsertRows(QModelIndex(), first, last);
    }
}
void SubtreeModel::rowsWillBeMoved(const QModelIndex &src, int first, int last, const QModelIndex &dst, int dfirst)
{
    Q_ASSERT(d->mUpdateBehavior == 0);
    if (!d->mDirIndex.isValid())
        return;
    QModelIndex mi = d->mDirIndex;
    if (mi == src && mi == dst) {
        if (first < d->mAvailableRows &&
            last < d->mAvailableRows &&
            dfirst <= d->mAvailableRows)
        {
            d->mUpdateBehavior = MOVE_ROW;
            QModelIndex inv;
            beginMoveRows(inv, first, last, inv, dfirst);
            return;
        }
    }
    if (mi == src || mi == dst) {
        d->mUpdateBehavior = RESET;
        beginResetModel();
        return;
    }
}
void SubtreeModel::rowsWillBeRemoved(const QModelIndex &parent, int first, int last) {
    Q_ASSERT(d->mUpdateBehavior == 0);
    if (!d->mDirIndex.isValid())
        return;
    if (d->mDirIndex != parent) {
        if (d->mDirIndex.parent() == parent &&
            d->mDirIndex.row() >= first &&
            d->mDirIndex.row() <= last)
        {
            //we are being removed!
            d->mUpdateBehavior = RESET;
            beginResetModel();
        }
        return;
    }

    if (last > d->mAvailableRows) {
        d->mUpdateBehavior = RESET;
        beginResetModel();
    } else {
        d->mUpdateBehavior = REMOVE_ROW;
        beginRemoveRows(QModelIndex(), first, last);
    }
}
//============COLS===============
void SubtreeModel::colsWillBeInserted(const QModelIndex &parent, int first, int last)
{
    Q_ASSERT(d->mUpdateBehavior == 0);
    if (!d->mDirIndex.isValid())
        return;
    if (d->mDirIndex != parent)
        return;

    if (first > d->mAvailableCols) {
        d->mUpdateBehavior = RESET;
        beginResetModel();
    } else {
        d->mUpdateBehavior = ADD_COL;
        beginInsertColumns(QModelIndex(), first, last);
    }
}
void SubtreeModel::colsWillBeMoved(const QModelIndex &src, int first, int last, const QModelIndex &dst, int dfirst)
{
    Q_ASSERT(d->mUpdateBehavior == 0);
    if (!d->mDirIndex.isValid())
        return;
    QModelIndex mi = d->mDirIndex;
    if (mi == src && mi == dst) {
        if (first < d->mAvailableCols &&
            last < d->mAvailableCols &&
            dfirst <= d->mAvailableCols)
        {
            d->mUpdateBehavior = MOVE_COL;
            QModelIndex inv;
            beginMoveColumns(inv, first, last, inv, dfirst);
            return;
        }
    }
    if (mi == src || mi == dst) {
        d->mUpdateBehavior = RESET;
        beginResetModel();
        return;
    }
}
void SubtreeModel::colsWillBeRemoved(const QModelIndex &parent, int first, int last) {
    Q_ASSERT(d->mUpdateBehavior == 0);
    if (!d->mDirIndex.isValid())
        return;
    if (d->mDirIndex != parent)
        return;

    if (last > d->mAvailableCols) {
        d->mUpdateBehavior = RESET;
        beginResetModel();
    } else {
        d->mUpdateBehavior = REMOVE_COL;
        beginRemoveColumns(QModelIndex(), first, last);
    }
}
//=========================================
void SubtreeModel::modelWillBeReset() {
    Q_ASSERT(d->mUpdateBehavior == 0);
    d->mUpdateBehavior = RESET;
    beginResetModel();
}
void SubtreeModel::complete() {
    d->mAvailableRows = 0;
    d->mAvailableCols = 0;

    //if (d->mDirIndex.isValid())
    //    emit indexLost();

    if (d->mDirIndex.isValid()) {
        QModelIndex mi = d->mDirIndex;
        d->mAvailableRows = d->mModel->rowCount(mi);
        d->mAvailableCols = d->mModel->columnCount(mi);
    }

    switch(d->mUpdateBehavior) {
    case ADD_COL:
        endInsertColumns();
        break;
    case REMOVE_COL:
        endRemoveColumns();
        break;
    case MOVE_COL:
        endMoveColumns();
        break;
    case ADD_ROW:
        endInsertRows();
        break;
    case REMOVE_ROW:
        endRemoveRows();
        break;
    case MOVE_ROW:
        endMoveRows();
        break;
    case RESET:
        endResetModel();
        break;
    default:;
    }

    d->mUpdateBehavior = NOOP;
}
//=========================================
void SubtreeModel::setRoot(const QModelIndex &index) {
    if (index.isValid()) {
        if (d->mDirIndex.isValid() && d->mDirIndex == index)
            return;
        beginResetModel();
        d->mDirIndex = index;
        d->mAvailableRows = d->mModel->rowCount(index);
        d->mAvailableCols = d->mModel->columnCount(index);
        endResetModel();
        d->mModel->fetchMore(index);

        if (index.isValid()) {
            emit folderChanged(d->mModel->data(index).toString());
        } else {
            emit folderChanged(tr("<<<root element>>>"));
        }
    }
}
bool SubtreeModel::cdUp() {
    if (!d->mDirIndex.isValid())
        return false;

    QModelIndex mi = d->mDirIndex.parent();
    beginResetModel();
    d->mDirIndex = mi;
    d->mAvailableRows = d->mModel->rowCount(mi);
    d->mAvailableCols = d->mModel->columnCount(mi);
    endResetModel();

    if (mi.isValid()) {
        emit folderChanged(d->mModel->data(mi).toString());
    } else {
        emit folderChanged(tr("<<<root element>>>"));
    }
    return true;
}
QModelIndex SubtreeModel::parent(const QModelIndex &child) const {
    Q_UNUSED(child);
    return QModelIndex();
}
QModelIndex SubtreeModel::index(int row, int col, const QModelIndex &parent) const {
    if (parent.isValid())
        return QModelIndex();
    if (row < 0 || col < 0 || row >= d->mAvailableRows || col >= d->mAvailableCols)
        return QModelIndex();
    return createIndex(row, col);
}
bool SubtreeModel::hasChildren(const QModelIndex &parent) const {
    if (parent.isValid())
        return false;
    return d->mAvailableRows != 0;
}
int  SubtreeModel::rowCount(const QModelIndex &parent) const {
    if (parent.isValid())
        return 0;
    return d->mAvailableRows;
}
int  SubtreeModel::columnCount(const QModelIndex &parent) const {
    if (parent.isValid())
        return 0;
    return d->mAvailableCols;
}
bool SubtreeModel::canFetchMore(const QModelIndex &parent) const {
    if (parent.isValid())
        return false;
    if (!d->mDirIndex.isValid())
        return false;
    return d->mModel->canFetchMore(d->mDirIndex);
}
void SubtreeModel::fetchMore(const QModelIndex &parent) {
    if (parent.isValid())
        return;
    if (!d->mDirIndex.isValid())
        return;
    d->mModel->fetchMore(d->mDirIndex);
}
QVariant SubtreeModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || !d->mDirIndex.isValid())
        return QVariant();

    QModelIndex mi = d->mModel->index(index.row(), index.column(), d->mDirIndex);
    return d->mModel->data(mi, role);
}
Qt::ItemFlags SubtreeModel::flags(const QModelIndex &index) const {
    if (!index.isValid() || !d->mDirIndex.isValid())
        return Qt::NoItemFlags;

    QModelIndex mi = d->mModel->index(index.row(), index.column(), d->mDirIndex);
    Qt::ItemFlags f = d->mModel->flags(mi);
    f = (Qt::ItemFlags)(f | Qt::ItemIsSelectable | Qt::ItemNeverHasChildren);
    if (!d->mWritable)
        f = (Qt::ItemFlags)(f &~ Qt::ItemIsEditable);
    return f;
}
bool SubtreeModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    if (!d->mWritable)
        return false;

    if (!index.isValid() || !d->mDirIndex.isValid())
        return false;

    if (index.parent() != d->mDirIndex)
        return false;

    QModelIndex mi = d->mModel->index(index.row(), index.column(), d->mDirIndex);
    return d->mModel->setData(mi, value, role);
}
void SubtreeModel::modelDataChanged(const QModelIndex &tl, const QModelIndex &br, const QVector<int> &roles) {
    if (!d->mDirIndex.isValid())
        return;

    QModelIndex mi = d->mDirIndex;
    if (tl.parent() != mi ||
        br.parent() != mi)
        return;

    emit dataChanged(createIndex(tl.row(), tl.column()),
                     createIndex(br.row(), br.column()),
                     roles);
}

}}
