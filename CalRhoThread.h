/**********************************************************************
 *(c) Copyright 2016 xxx.  All rights reserved.
 *
 * GDC2DP professional: window rho
 *
 * Created on: 2016-05-04
 * Author: Haiping.Yang
 * Context: First version
 *
 * 本程序为根据实测电场计算准双极源电阻率的程序，先读出实测电场，然后根据其反算电阻率。
 * 假设收发距为R，AB相距L，根据计算的结果，即使R=L的近收发距的情况，如果将AB剖分成100个首尾相连的小电偶极子，
 * 那么不论观测点与AB成什么角度，其小电偶
 * 极子在该处产生的场强与真双极源在该处产生的场强相比，其误差均小于1%。
 * 因此本程序采用将AB电偶极子剖分成100个首尾相连的小电偶极子的方式进行计算。
 */
#ifndef CALRHOTHREAD_H
#define CALRHOTHREAD_H

#include <QThread>
#include <QStringList>
#include <QtMath>
#include <QMessageBox>

#include <complex>

#include "Common/PublicDef.h"

#include "MyDatabase.h"

/* ΔU Unit facter,接收机采样振幅单位约定为毫伏(mV)  2017-03-10 */
#define UU       (pow(10, -3))

/* μ */
#define MU       (4*(M_PI)*(pow(10, -7)))

/* ε */
#define EPSILON  (8.85*(pow(10, -12)))

/* Split AB into 100 small electric dipoles. */
#define NDIV    100

/* Error */
#define ERR     0.0005

class CalRhoThread : public QThread
{
    Q_OBJECT
public:
    explicit CalRhoThread(MyDatabase *poDatabase, QObject *parent = 0);
    ~CalRhoThread();

    MyDatabase *poDb;

    /* TX A&B coordinates */
    QVector3D vA;
    QVector3D vB;

    QPointF ptA;
    QPointF ptB;
    QPointF ptTxMid;

    /* RX MN coordinates */
    QVector3D vM;
    QVector3D vN;

    QPointF ptM;
    QPointF ptN;
    QPointF ptRxMid;

    /* Calculate WFEM ρ in a Single thread for one MN */
    void CalRho(STATION oStation);

    bool getAB();

private:
    /* Get coordinate from coordinate file */
    bool CoorRead(STATION oStation);

    /* Angle between two lines */
    double AngleGet(const QPointF ptPoint1, const QPointF ptPoint2, const QPointF ptPoint3, const QPointF ptPoint4);

    /* The length  between two points */
    double LengthGet(const QPointF ptPoint1, const QPointF ptPoint2);

signals:
    /* Error */
    void SigMsg(QString);

    /* Rho result struct */
    void SigRhoResult(QString oStrTitle, QVector<double> , QVector<double>);

public slots:

};

#endif // CALRHOTHREAD_H
