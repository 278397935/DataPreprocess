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

    QVector<double> adF;

    QMap<double, QVector<double> > mapScatterList;

    QMap<double, double> mapAvg;

    QMap<double, double> mapErr;


    QString goStrLineId, goStrSiteId;

    int giDevId, giDevCh;

    QString goStrCompTag;

    void importRX(QString oStrFileName);

    void renewScatter(int iIndex );

    double getAvg(QVector<double> adData);

    double getErr(QVector<double> adData);

    void updateScatter(double dF, QVector<double> adScatter);

signals:

public slots:
};

#endif // RX_H
