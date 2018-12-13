#include "projectmodel.h"

using namespace K::IDE;

ProjectModel::ProjectModel(int limit, QObject * parent) : QAbstractListModel(parent) {
    mEntryLimit = limit;
}
void ProjectModel::addElement(const QIcon& icon,
                              const QString& name,
                              const QString& aux,
                              const QString& uid) {
    QModelIndex i;
    Entry e;
    e.mIcon = icon;
    e.mName = name;
    e.mAux  = aux;
    e.mAux  = uid;

    if (mEntryLimit > 0 && mEntries.size() >= mEntryLimit) {
        beginRemoveRows(i, mEntries.size()-1, mEntries.size()-1);
        mEntries.pop_back();
        endRemoveRows();
    }
    beginInsertRows(i, mEntries.size(), mEntries.size());
    mEntries.append(e);
    endInsertRows();
}
void ProjectModel::removeElement(const QString& uid) {
    QModelIndex j;
    auto i = mEntries.begin();
    while (i != mEntries.end()) {
        if (i->mUid != uid) {
            ++i;
            continue;
        }

        int n = i-mEntries.begin();
        beginRemoveRows(j, n, n);
        i = mEntries.erase(i);
        endRemoveRows();
    }
}
bool ProjectModel::pokeElementByUid(const QString& uid) {
    auto i = mEntries.begin();
    while (i != mEntries.end()) {
        if (i->mUid != uid) {
            ++i;
            continue;
        }

        int n = i-mEntries.begin();
        QModelIndex j;
        beginMoveRows(j, n, n, j, 0);
        mEntries.move(n, 0);
        endMoveRows();
        return true;
    }
    return false;
}

void ProjectModel::pokeElement(int row) {
    if (row < 0 || row >= mEntries.size())
        return;

    QModelIndex j;
    beginMoveRows(j, row, row, j, 0);
    mEntries.move(row, 0);
    endMoveRows();
}

int ProjectModel::rowCount(const QModelIndex &parent) const {
    if (parent.isValid())
        return 0;
    return mEntries.size();
}
QVariant ProjectModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.column() != 0 ||
         index.row() < 0 || index.row() >= mEntries.size())
        return QVariant();

    switch(role) {
    case Qt::DecorationRole:
        return QVariant(mEntries[index.row()].mIcon);
    case Qt::DisplayRole:
        return QVariant(mEntries[index.row()].mName);
    case Qt::UserRole:
        return QVariant(mEntries[index.row()].mAux);
    default:
        return QVariant();
    }
}
const ProjectModel::Entry* ProjectModel::at(const QModelIndex& index) {
    if (!index.isValid() || index.column() != 0 ||
         index.row() < 0 || index.row() >= mEntries.size())
        return nullptr;

    return &mEntries[index.row()];
}
int ProjectModel::rowCount() const {
    return mEntries.size();
}
const ProjectModel::Entry* ProjectModel::at(int i) const {
    if (i < 0 || i >= mEntries.size())
        return nullptr;

    return &mEntries[i];
}
