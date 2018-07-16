#ifndef ASSETSQL_H
#define ASSETSQL_H

#include <QObject>
#include <QDir>

class QSqlQuery;
class QSqlTableModel;

class Node
{
public:
    enum Type{
        Equipment,
        Substation,
        Area,
        Root,
        Normal,
    };

    enum NodeRole{
        TypeRole = Qt::UserRole,
        CurrentRole,
        PathRole,

    };

    Node(Type type = Root, int id = -1, QString str = "");
    ~Node();

    Type type;          //TypeRole
    int id;             //系统自动分配，不需要设置
    QString name;       //DisplayRole
    bool isCurrent;     //CurrentRole
    QString path;       //PathRole,资产文件路径

    Node *parent;
    QList<Node *> children;
    QString type_to_string();

    //Equipment所特有
    int substation_id;

    //Substation所特有
    int area_id;
};


class AssetSql : public QObject
{
    Q_OBJECT
public:
    explicit AssetSql(QObject *parent = nullptr);

    Node *rootNode;
    void update_node(Node *n);          //保存节点
    QVariant insert_node(Node *n);      //插入节点(返回插入id)
    bool delete_node(Node *n);          //删除节点
    void show_tree();                   //打印树
    void dir_sync();                    //树结构和文件夹系统需要一一对应

    QString current_dir;


signals:

public slots:

private:
    QSqlQuery *q;

    //sql操作
    bool create_connection();       //创建SQL
    QVariant add_area(int id, QString name);
    QVariant add_substation(int id, const QString &name, const QVariant area_id);
    void add_equipment(int id, const QString &name, const QVariant &substation_id );


    //链表操作
    QSqlTableModel *model_area, *model_substation, *model_equipment;
    void create_tree();         //生成树
    Node *create_node_area(int row, Node *parent);
    Node *create_node_substation(int row, Node *parent);
    Node *create_node_equipment(int row, Node *parent);

//    bool rename_dir(QString old_path, QString new_path)
//    {
//        QDir dir;
//        dir.setPath(old_path);
//        return dir.rename(old_path,new_path);
//    }                  //重命名文件夹

    QString node_to_path(Node *n);      //从链表节点生成对应的资产路径

};

#endif // ASSETSQL_H
















