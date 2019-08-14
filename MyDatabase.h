#ifndef MYDATABASE_H
#define MYDATABASE_H

#include "Common/PublicDef.h"

#include <QObject>

#include <QFileDialog>
#include <QMessageBox>

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>

#include <QVector3D>

#include <QSqlTableModel>


#include "Data/RX.h"

typedef struct _STATION
{
    QString oStrLineId;
    QString oStrSiteId;

    int iDevId;
    int iDevCh;

    QString oStrTag;
}STATION;

/* Rho result struct */
typedef struct _RhoResult
{
    STATION oStation;

    Position oAB;
    Position oMN;

    double dF;
    double dI;
    double dField;
    double dErr;
    double dRho;
}RhoResult;


class MyDatabase : public QObject
{
    Q_OBJECT
public:
    explicit MyDatabase(QObject *parent = 0);

    void connect();

    void importTX(QString oStrFileName);
    void importRX(QVector<RX *> apoRX);
    void importXY(QString oStrFileName);

    void importRho( QList<RhoResult> aoRhoResult);

    QList<double> getF(STATION oStation);

    double getI(double dF);

    double getField(STATION oStation, double dF);

    double getErr(STATION oStation, double dF);

    Position getCoordinate(QString oStrLineId, QString oStrSiteId);

    QList<STATION> getStation();

    QSqlDatabase *poDb;

signals:    
    void SigMsg(QString);

    void SigModelTX(QSqlTableModel *);
    void SigModelRX(QSqlTableModel *);
    void SigModelXY(QSqlTableModel *);

    void SigModelRho(QSqlTableModel *);

public slots:

};

#endif // MYDATABASE_H
