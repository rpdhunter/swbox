#include "assetmodel.h"
#include <QtDebug>
//#include "assetsql.h"

AssetModel::AssetModel(QObject *parent) : QAbstractItemModel(parent)
{
    sql = new AssetSql;
}

int AssetModel::rowCount(const QModelIndex &parent) const
{
    if(parent.column() > 0 ){
        return 0;
    }
    Node *parentNode = node_from_index(parent);
    if(!parentNode){
        return 0;
    }
    return parentNode->children.count();
}

int AssetModel::columnCount(const QModelIndex &parent) const
{
    return 2;
}

QVariant AssetModel::data(const QModelIndex &index, int role) const
{
    if(role != Qt::DisplayRole)
        return QVariant();

    Node *node = node_from_index(index);
    if(!node){
        return QVariant();
    }
    if(index.column() == 0){
        return node->name;
    }
    else if(index.column() == 1){
        return node->type_to_string();
    }
    return QVariant();
}

QModelIndex AssetModel::index(int row, int column, const QModelIndex &parent) const
{
    if(sql->rootNode->children.isEmpty() || row<0 || column<0){
        return QModelIndex();
    }
    Node *parentNode = node_from_index(parent);
    Node *childNode = parentNode->children.value(row);
    if(!childNode){
        return QModelIndex();
    }
    return createIndex(row,column,childNode);
}

QModelIndex AssetModel::parent(const QModelIndex &child) const
{
    Node *node = node_from_index(child);
    if(!node){
        return QModelIndex();
    }
    Node *parentNode = node->parent;
    if(!parentNode){
        return QModelIndex();
    }
    Node *grandparentNode = parentNode->parent;
    if(!grandparentNode){
        return QModelIndex();
    }

    int row = grandparentNode->children.indexOf(parentNode);
    return createIndex(row, 0, parentNode);
}

//插入操作不涉及到数据库修改，这部分工作留到setData函数
//所以使用时insertRows 和 setData 要绑定出现
bool AssetModel::insertRows(int row, int count, const QModelIndex &parent)
{
    if(row >= 0 && count > 0){      //有效插入
        beginInsertRows(parent,row,row+count-1);
        Node *p;        //父节点
        if(parent.isValid() ){      //非根节点插入
            p = static_cast<Node *>(parent.internalPointer() );
        }
        else {                      //根节点插入
            p = sql->rootNode;
        }
        for (int i = 0; i < count; ++i) {
            Node *n = new Node;
            n->parent = p;
            p->children.insert(row + i,n);            
        }

//        qDebug()<<QString("add %1 new Node!").arg(count);

        endInsertRows();
        return true;
    }


    return false;
}

bool AssetModel::removeRows(int row, int count, const QModelIndex &parent)
{
    if(row >= 0 && count > 0){      //有效删除
        beginRemoveRows(parent,row,row+count-1);
        Node *p,*n;

        if(parent.isValid() ){      //非根节点删除
            p = static_cast<Node *>(parent.internalPointer() );
        }
        else {                      //根节点删除
            p = sql->rootNode;
        }
        if(p != NULL && row+count <= p->children.count()){
            for (int i = 0; i < count; ++i) {
                n = p->children.at(row);
                sql->delete_node(n);        //删除保存至sql
                p->children.removeAt(row);
            }
        }

        endRemoveRows();
        return true;
    }


    return false;
}

bool AssetModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if(index.isValid()){
        Node *n = static_cast<Node *>(index.internalPointer() );
        switch (role) {
        case Qt::UserRole:          //设置节点类型时，应当完善areaid,substationid等信息
            n->type = (Node::Type)value.toInt();
            switch (n->type) {
            case Node::Area:

                break;
            case Node::Substation:
                n->area_id = n->parent->id;
                break;
            case Node::Equipment:
                n->substation_id = n->parent->id;
                break;
            default:
                break;
            }
            break;
        case Qt::EditRole:
            n->name = value.toString();            
            break;
        default:
            break;
        }
        return true;
    }
    return false;
}

//之所以不把保存操作分散到insertRows()和setData()函数中去，是因为整条记录一起操作，比较有效率
//所以增加/修改记录需要手动执行本函数，但删除操作不需要
void AssetModel::save_node(const QModelIndex &index)
{
    if(index.isValid()){
        Node *n = static_cast<Node *>(index.internalPointer() );
        if(n->id < 0){        //说明这是一个新加入的节点，执行插入操作
            int i = sql->insert_node(n).toInt();        //保存至sql文件
//            qDebug()<<"insert id:"<<i;
            n->id = i;
        }
        else{                   //执行修改操作
            sql->update_node(n);        //保存至sql
        }
    }
}

void AssetModel::show_tree()
{
    sql->show_tree();
}

Node *AssetModel::node_from_index(const QModelIndex &index) const
{
    if(index.isValid()){
        return static_cast<Node *>(index.internalPointer());
    }
    else{
        return sql->rootNode;
    }
}











