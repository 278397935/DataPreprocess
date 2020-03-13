#ifndef RX_H
#define RX_H

#include <QObject>

#include <QVector>

#include <QtMath>

#include <QFile>

#include <QFileInfo>

#include <QTextStream>

#include "Common/PublicDef.h"

class RX : public QObject
{
    Q_OBJECT
public:
    explicit RX(QString oStrFileName, QObject *parent = 0);

    /* csv 文件名 */
    QString oStrCSV;

    //QVector<QVector<double> > aadScatter;
    QMap<double, QVector<double> > map_F_Scatter;

    QVector<double> adF;
    QVector<double> adE;
    QVector<double> adErr;

    QString goStrLineId, goStrSiteId;

    int giDevId, giDevCh;

    QString goStrCompTag;

    void importRX(QString oStrFileName);

    void renewScatter(int iIndex );

    double getE(QVector<double> adData);

    double getErr(QVector<double> adData);

    void updateScatter(double dF, QVector<double> adScatter);

signals:

public slots:
};

#endif // RX_H
