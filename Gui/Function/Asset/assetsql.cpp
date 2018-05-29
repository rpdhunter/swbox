#include "assetsql.h"
#include <QtDebug>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlTableModel>
#include <QSqlError>
#include <QDir>
#include "IO/Data/data.h"
#include "Gui/Common/common.h"

AssetSql::AssetSql(QObject *parent) : QObject(parent)
{
    create_connection();    //建立数据库
    create_tree();          //数据库生成链表
//    show_tree();            //显示链表
    dir_sync();             //同步文件夹
}


/*******************************************************
 * 以id为唯一关键词修改记录
 * 一个完整的节点操作包含4部分：
 * 1.视图-模型操作(AssetWidget AssetView AssetModel)
 * 2.链表操作(AssetModel AssetSql)
 * 3.sql操作(AssetSql)
 * 4.文件夹操作(AssetSql)
 * *****************************************************/

void AssetSql::update_node(Node *n)
{
    switch (n->type) {
    case Node::Area:
        q->exec(QString("UPDATE area SET name = '%2' WHERE id = %1").arg(n->id).arg(n->name));
        break;
    case Node::Substation:
        q->exec(QString("UPDATE substation SET name = '%2', areaid = %3 WHERE id = %1").arg(n->id).arg(n->name).arg(n->area_id));
        break;
    case Node::Equipment:
        q->exec(QString("UPDATE equipment SET name = '%2', substationid = %3 WHERE id = %1").arg(n->id).arg(n->name).arg(n->substation_id));
        break;
    default:
        break;
    }

    QString new_path = node_to_path(n);
    Common::rename_dir(n->path, new_path);
    n->path = new_path;
}

//插入新记录，必须提供除id以外的所有信息
//插入的节点，id字段是无效的，需要sqlite自动分配
QVariant AssetSql::insert_node(Node *n)
{
    n->path = node_to_path(n);
    Common::mk_dir(n->path);

    switch (n->type) {
    case Node::Area:
        q->exec(QString("INSERT INTO area (name) VALUES ('%1')").arg(n->name));
        return q->lastInsertId();
    case Node::Substation:
        q->exec(QString("INSERT INTO substation (name,areaid) VALUES ('%1',%2)").arg(n->name).arg(n->area_id));
        return q->lastInsertId();
    case Node::Equipment:
        q->exec(QString("INSERT INTO equipment (name,substationid) VALUES ('%1',%2)").arg(n->name).arg(n->substation_id));
        return q->lastInsertId();
    default:
        return QVariant();
    }
}

//以id为唯一关键词删除记录
bool AssetSql::delete_node(Node *n)
{
//    qDebug()<<"to be del id ="<<n->id;
    Common::del_dir(node_to_path(n));

    switch (n->type) {
    case Node::Area:
        return q->exec(QString("DELETE FROM area WHERE id=%1").arg(n->id));
    case Node::Substation:
        return q->exec(QString("DELETE FROM substation WHERE id=%1").arg(n->id));
    case Node::Equipment:
        return q->exec(QString("DELETE FROM equipment WHERE id=%1").arg(n->id));
    default:
        return false;
    }   
}

bool AssetSql::create_connection()
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("asset.db");

    if (!db.open()) {
        return false;
    }

    QStringList tables = db.tables();
    q = new QSqlQuery(db);
    QVariant area_id1, substation_id1, substation_id2;

    if(!tables.contains("area", Qt::CaseInsensitive)){
        bool f = q->exec(QLatin1String("create table area(id integer primary key, name varchar)"));

        if(f){
            qDebug()<<"create table area successed";
        }
        else{
            qDebug()<<"create table area failed";
        }

        area_id1 = add_area( 1, QLatin1String("TEST_AREA1") );
        add_area( 2, QLatin1String("TEST_AREA2") );
    }
    if(!tables.contains("substation", Qt::CaseInsensitive)){
        bool f = q->exec(QLatin1String("create table substation(id integer primary key, name varchar, "
                                       "areaid integer )"));
//                                 "FOREIGN KEY (areaid) REFERENCES area )"));

        if(f){
            qDebug()<<"create table substation successed";
        }
        else{
            qDebug()<<"create table substation failed";
        }

        substation_id1 = add_substation(1, "TEST_SUBTATION1", area_id1);
        substation_id2 = add_substation(88, "TEST_SUBTATION2", area_id1);
    }
    if(!tables.contains("equipment", Qt::CaseInsensitive)){
        bool f = q->exec(QLatin1String("create table equipment(id integer primary key, name varchar, "
                                       "substationid integer )"));
//                                 "FOREIGN KEY (substationid) REFERENCES substation(id) )"));

        if(f){
            qDebug()<<"create table equipment successed";
        }
        else{
            qDebug()<<"create table equipment failed";
        }
        add_equipment(4, "E1", substation_id1);
        add_equipment(2, "E2", substation_id2);
        add_equipment(3, "E3", substation_id2);
    }

    model_area = new QSqlTableModel(this,db);
    model_substation = new QSqlTableModel(this,db);
    model_equipment = new QSqlTableModel(this,db);

    model_area->setTable("area");
    model_area->select();
    qDebug()<<"\n area data:"<<model_area->rowCount();

    model_substation->setTable("substation");
    model_substation->select();
    qDebug()<<"substation data:"<<model_substation->rowCount();

    model_equipment->setTable("equipment");
    model_equipment->select();
    qDebug()<<"equipment data:"<<model_equipment->rowCount();

    qDebug()<<"tables are:"<<db.tables();

    return true;
}

QVariant AssetSql::add_area(int id, QString name)
{
    bool f  =  q->exec(QString("insert into area values(%1, '%2')").arg(id).arg(name));;
    if(f){
        qDebug()<<"insert area successed!";
    }
    else{
        qDebug()<<"insert area failed!";
    }
    return q->lastInsertId();
}

QVariant AssetSql::add_substation(int id, const QString &name, const QVariant area_id)
{
    bool f  =  q->exec(QString("insert into substation values(%1, '%2', %3)").arg(id).arg(name).arg(area_id.toInt()));;
    if(f){
        qDebug()<<"insert sub successed!";
    }
    else{
        qDebug()<<"insert sub failed!";
    }
    return q->lastInsertId();
}

void AssetSql::add_equipment(int id, const QString &name, const QVariant &substation_id)
{
    bool f  =  q->exec(QString("insert into equipment values(%1, '%2', %3)").arg(id).arg(name).arg(substation_id.toInt()));;
    if(f){
        qDebug()<<"insert equ successed!";
    }
    else{
        qDebug()<<"insert equ failed!";
    }
}

void AssetSql::create_tree()
{
    model_area->select();       //读取片区
    rootNode = new Node;

    for (int i = 0; i < model_area->rowCount(); ++i) {
        rootNode->children.append(create_node_area(i,rootNode) );
    }
}

Node *AssetSql::create_node_area(int row, Node *parent)
{
    Node *n = new Node;
    n->id = model_area->data( model_area->index(row,0) ).toInt();
    n->name = model_area->data( model_area->index(row,1) ).toString();
    n->type = Node::Area;
    n->parent = parent;
    model_substation->setFilter(QString("areaid=%1").arg(n->id) );
    model_substation->select();     //读取站所
    for (int i = 0; i < model_substation->rowCount(); ++i) {
        n->children.append(create_node_substation(i, n) );
    }
    return n;
}

Node *AssetSql::create_node_substation(int row, Node *parent)
{
    Node *n = new Node;
    n->id = model_substation->data( model_substation->index(row,0) ).toInt();
    n->name = model_substation->data( model_substation->index(row,1) ).toString();
    n->type = Node::Substation;
    n->parent = parent;
    n->area_id = model_substation->data( model_substation->index(row,2) ).toInt();
    model_equipment->setFilter(QString("substationid=%1").arg(n->id) );
    model_equipment->select();        //读取设备
    for (int i = 0; i < model_equipment->rowCount(); ++i) {
        n->children.append(create_node_equipment(i, n) );
    }
    return n;
}

Node *AssetSql::create_node_equipment(int row, Node *parent)
{
    Node *n = new Node;
    n->id = model_equipment->data( model_equipment->index(row,0) ).toInt();
    n->name = model_equipment->data( model_equipment->index(row,1) ).toString();
    n->type = Node::Equipment;
    n->parent = parent;
    n->substation_id = model_equipment->data( model_equipment->index(row,2) ).toInt();
    return n;
}

void AssetSql::show_tree()
{
    Node *n_s, *n_e;
    for (int i = 0; i < rootNode->children.count(); ++i) {
        qDebug()<< QString("[%1]").arg(rootNode->children.at(i)->id) <<rootNode->children.at(i)->name;
        for (int j = 0; j < rootNode->children.at(i)->children.count(); ++j) {
            n_s = rootNode->children.at(i)->children.at(j);
            qDebug()<< QString("[%1]").arg(n_s->id) << n_s->name << "\t" << n_s->area_id;
            for (int k = 0; k < n_s->children.count(); ++k) {
                n_e = n_s->children.at(k);
                qDebug()<< QString("[%1]").arg(n_e->id) << n_e->name << "\t" << n_e->substation_id;
            }
        }
    }
}

void AssetSql::dir_sync()
{
    QString area_path, sub_path, equ_path;
    Node *n_s, *n_e;
    Common::mk_dir(QString(DIR_ASSET));
    rootNode->path = QString(DIR_ASSET);
    for (int i = 0; i < rootNode->children.count(); ++i) {
        area_path = DIR_ASSET"/" + rootNode->children.at(i)->name.toUtf8();
        Common::mk_dir(area_path);
        rootNode->children.at(i)->path = area_path;
        for (int j = 0; j < rootNode->children.at(i)->children.count(); ++j) {
            n_s = rootNode->children.at(i)->children.at(j);
            sub_path = area_path + "/" + n_s->name;
            Common::mk_dir(sub_path);
            n_s->path = sub_path;
            for (int k = 0; k < n_s->children.count(); ++k) {
                n_e = n_s->children.at(k);
                equ_path = sub_path + "/" + n_e->name;
                Common::mk_dir(equ_path);
                n_e->path = equ_path;
            }
        }
    }
}





QString AssetSql::node_to_path(Node *n)
{
    QString e_name, s_name, a_name, path;

    switch (n->type) {
    case Node::Equipment:
        e_name = n->name;
        s_name = n->parent->name;
        a_name = n->parent->parent->name;
        path = QString("%1/%2/%3/%4").arg(DIR_ASSET).arg(a_name).arg(s_name).arg(e_name);
        break;
    case Node::Substation:
        s_name = n->name;
        a_name = n->parent->name;
        path = QString("%1/%2/%3").arg(DIR_ASSET).arg(a_name).arg(s_name);
        break;
    case Node::Area:
        a_name = n->name;
        path = QString("%1/%2").arg(DIR_ASSET).arg(a_name);
        break;
    case Node::Root:
        path = QString("%1").arg(DIR_ASSET);
        break;
    default:
        break;
    }

    return path;
}


Node::Node(Node::Type type, int id, QString str)
{
    this->type = type;
    this->id = id;
    this->name = str;
    this->parent = NULL;
    this->substation_id = -1;
    this->area_id = -1;
    this->isCurrent = false;
}

Node::~Node()
{
    qDebug()<<"qDeleteAll";
    qDeleteAll(children);
}

QString Node::type_to_string()
{
    switch (type) {
    case Equipment:
        return QString("Equipment");
        break;
    case Substation:
        return QString("Substation");
        break;
    case Area:
        return QString("Area");
        break;
    case Root:
        return QString("Root");
        break;
    default:
        return QString("");
        break;
    }
}
