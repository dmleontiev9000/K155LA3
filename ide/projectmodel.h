#pragma once
#include <QAbstractListModel>
#include <QIcon>

#ifndef IDE_LIBRARY
#error "this header is internal"
#endif

namespace K {
namespace IDE {

class ProjectModel : public QAbstractListModel {
public:
    struct Entry {
        QIcon   mIcon;
        QString mName;
        QString mAux;
        QString mUid;
    };

    ProjectModel(int limit, QObject * parent);
    void addElement(const QIcon& icon,
                    const QString& name,
                    const QString& aux,
                    const QString& uid);
    void removeElement(const QString& uid);
    bool pokeElementByUid(const QString& uid);
    void pokeElement(int row);

    int          rowCount(const QModelIndex &parent) const;
    QVariant     data(const QModelIndex &index, int role) const;
    const Entry* at(const QModelIndex& index);
    int          rowCount() const;
    const Entry* at(int i) const;
private:
    QList<Entry> mEntries;
    int mEntryLimit;
};

} //namespace IDE;
} //namespace K;
