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

    /* 工具选定的频率，更新Rx类中的变量。原来是工具index来检索，有一定的耦合性，所以改过来了。 */
    void renewScatter(double dF);

    double getAvg(QVector<double> adData);

    double getErr(QVector<double> adData);

    void updateScatter(double dF, QVector<double> adScatter);

signals:

public slots:
};

#endif // RX_H
