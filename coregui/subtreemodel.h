#pragma once

#include "coregui_global.h"
#include <QAbstractProxyModel>

namespace K {
namespace Core {

class SubTreeModelPrivate;
class COREGUISHARED_EXPORT SubtreeModel : public QAbstractItemModel {
    Q_OBJECT

public:
    SubtreeModel(QAbstractItemModel * src, const QModelIndex &root, QObject * parent = nullptr);
    virtual ~SubtreeModel();
    void setRoot(const QModelIndex& index);
    void setWritable(bool en);
    QModelIndex indexOf(const QModelIndex &i) const;
    QModelIndex rootIndex() const ;
    bool cdUp();
private:
    bool canFetchMore(const QModelIndex &parent) const final override;
    bool hasChildren(const QModelIndex &parent) const final override;
    int  rowCount(const QModelIndex &parent) const final override;
    int  columnCount(const QModelIndex &parent) const final override;
    QModelIndex parent(const QModelIndex &child) const final override;
    QModelIndex index(int row, int column, const QModelIndex &parent) const final override;
    void fetchMore(const QModelIndex &parent) final override;
    QVariant data(const QModelIndex &index, int role) const final override;
    Qt::ItemFlags flags(const QModelIndex &index) const final override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) final override;

    SubTreeModelPrivate   * d;
    bool update();
signals:
    void folderChanged(const QString& folder);
    void indexLost();
private slots:
    void colsWillBeInserted(const QModelIndex& parent, int first, int last);
    void colsWillBeMoved   (const QModelIndex& src, int first, int last,
                            const QModelIndex& dst, int dfirst);
    void colsWillBeRemoved (const QModelIndex& parent, int first, int last);

    void rowsWillBeInserted(const QModelIndex& parent, int first, int last);
    void rowsWillBeMoved   (const QModelIndex& src, int first, int last,
                            const QModelIndex& dst, int dfirst);
    void rowsWillBeRemoved (const QModelIndex& parent, int first, int last);

    void modelWillBeReset();
    void complete();
    void modelDataChanged(const QModelIndex& tl,
                          const QModelIndex& br,
                          const QVector<int>& roles);

};

} //namespace Widgets;
} //namespace K;
