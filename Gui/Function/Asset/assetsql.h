#ifndef ASSETSQL_H
#define ASSETSQL_H

#include <QObject>

class AssetSql : public QObject
{
    Q_OBJECT
public:
    explicit AssetSql(QObject *parent = nullptr);

signals:

public slots:

private:
    static bool createConnection();
};

#endif // ASSETSQL_H
