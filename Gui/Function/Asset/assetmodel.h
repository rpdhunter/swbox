#ifndef ASSETMODEL_H
#define ASSETMODEL_H

#include <QAbstractItemModel>

class AssetModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit AssetModel(QObject *parent = nullptr);
    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    QModelIndex index(int row, int column, const QModelIndex &parent) const;
    QModelIndex parent(const QModelIndex &child) const;

signals:

public slots:
};

#endif // ASSETMODEL_H
