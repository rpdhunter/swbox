#ifndef ASSETMODEL_H
#define ASSETMODEL_H

#include <QAbstractItemModel>
#include "assetsql.h"

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
    bool insertRows(int row, int count, const QModelIndex &parent);
    bool removeRows(int row, int count, const QModelIndex &parent);
    bool setData(const QModelIndex &index, const QVariant &value, int role);


    void save_node(const QModelIndex &index);       //保存节点至sql
    void show_tree();

signals:

public slots:

private:
    AssetSql *sql;

    Node *node_from_index(const QModelIndex &index) const;
    Node *rootNode;     //虚拟根节点
};

#endif // ASSETMODEL_H
