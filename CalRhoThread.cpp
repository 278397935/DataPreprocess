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
#include "CalRhoThread.h"


/****************************************************************************
 * CalRhoThread : constructure function
 *
 */
CalRhoThread::CalRhoThread(MyDatabase *poDatabase, QObject *parent) :
    QThread(parent)
{
    poDb = poDatabase;


}

/****************************************************************************
 * CalRhoThread : unconstructure
 *
 */
CalRhoThread::~CalRhoThread()
{

}

/**********************************************************************************
 * Calculate the WFEM ρ in a single thread.
 * (Translation from MATLAB program)
 *
 */
void CalRhoThread::CalRho(STATION oStation)
{
    /* Read coordinate from coordinate file */
    if(!this->CoorRead(oStation))
    {
        emit SigMsg(QString("线号：%1\n点号：%2\n坐标获取失败！")
                    .arg(oStation.oStrLineId)
                    .arg(oStation.oStrSiteId));

        return;
    }

    /* AB Dipole length */
    double dL = LengthGet(ptA, ptB)/NDIV;

    /* AB length */
    double dLPhi = LengthGet(ptA, ptB);

    /* M--->N length */
    double dMN = LengthGet(ptM, ptN);

    /* Frequency list */
    QList<double> adF;
    adF.clear();
    adF = poDb->getF(oStation);

    /* Rho list */
    QList<RhoResult> aoRhoResult;
    aoRhoResult.clear();

    /* image(i) */
    std::complex<double> IMAGE(0, 1);
    std::complex<double> TWO(2, 0);
    std::complex<double> ONE(1, 0);
    std::complex<double> NEGONE(-1, 0);

    /* Loop1: Frequency count */
    foreach(double dF, adF)
    {
        double dI = poDb->getI(dF);

        double dField = poDb->getField(oStation, dF);

        double dErr = poDb->getErr(oStation, dF);

        double dRho0 = 10;
        double dRho  = 100;

        /* 电场值（单位：伏/米） 2017-03-10 */
        double dE = ( dField*UU )/dMN;

        double dW  = dF*2*M_PI;

        /* 计算Ey视电阻率
         * r: 收发距
         * I: 电流
         * L: AB的距离
         * a: 夹角
         * Ey: 场值，不是电压值，Ey=UMN/MN,场值=振幅/长度
         * 公式：pey=abs(2*pi*r^3*Ey/(3*I*L*(sin(a)*cos(a))));
         */
        if(oStation.oStrTag == "Ey")
        {
            /* AB_Mid --> point length，收发距 */
            double dR = LengthGet(ptTxMid, ptRxMid);

            double dPhi = AngleGet(ptA, ptB, ptTxMid, ptRxMid);

            /* pey= abs( 2*pi  *r^3        *Ey/(3*I.       *L    *(sin(a).  *cos(a)   ))) */
            dRho = qAbs( 2*M_PI*pow(dR, 3) *dE/(3*dI*dLPhi*(sin(dPhi)*cos(dPhi))));
        }
        else
        {
            /* Error upper limit */
            while( qAbs( (dRho - dRho0)/dRho0 ) >= ERR )
            {
                dRho0 = dRho;

                std::complex<double> cca(0, 0);

                if(oStation.oStrTag == "Ex")
                {
                    /* Loop2: ndiv, 100 */
                    for(qint32 j = 1; j < NDIV +1; j++)
                    {
                        //qDebugV0()<<"Loop1/Loop2:"<<i<<"("<<adF.at(i)<<") "<<"/"<<j<<"No./100:"<<j<<"/100";
                        //center_x(ii) =     xa         + (xb         - xa        )/2/nn  *(2*ii-1);
                        QPointF ptDipoleMid( ptA.x() + (ptB.x() - ptA.x())/2/NDIV*(2*j-1),
                                             ptA.y() + (ptB.y() - ptA.y())/2/NDIV*(2*j-1));

                        /* Dipole_Mid --> point length */
                        double dR = LengthGet(ptDipoleMid, ptRxMid);

                        //qDebugV0()<<"Dipole_Mid --> point length:"<<dR;

                        /* DipoleMid_point------>AB angle */
                        double dPhi = AngleGet(ptA, ptB, ptRxMid, ptDipoleMid);

                        //qDebugV0()<<"DipoleMid_point------>AB angle:"<<dPhi;

                        double dCc = (dI*dL)/(2*M_PI*(pow(dR, 3)));

                        std::complex<double> k2( EPSILON*MU*pow(dW, 2), dW*MU/dRho0 );

                        std::complex<double> k = sqrt(k2);

                        //  = cca + cc(ii)*(1-3*sin(phi(ii))*sin(phi(ii))+exp(i    *k*r(ii))- i    *k*r(ii)*exp(i    *k*r(ii)));
                        cca = cca + dCc   *(1-3*sin(dPhi)   *sin(dPhi)   +exp(IMAGE*k*dR   )- IMAGE*k*dR   *exp(IMAGE*k*dR   ));

                    }
                }

                else if(oStation.oStrTag == "E\u03c6")/* Eφ */
                {
                    /* AB_Mid --> point length */
                    double dR = LengthGet(ptTxMid, ptRxMid);

                    /* DipoleMid_point------>AB angle */
                    double dPhi = AngleGet(ptA, ptB, ptTxMid, ptRxMid);

                    std::complex<double> k2 = EPSILON*MU*pow(dW, 2) - IMAGE*dW*MU/dRho0;

                    std::complex<double> k = sqrt(k2);

                    std::complex<double> coef1 = dI*dLPhi*sin(dPhi)/(2*M_PI*pow(dR, 3));
                    std::complex<double> coef2 = TWO - exp(NEGONE*IMAGE*k*dR)*(ONE+IMAGE*k*dR);

                    cca = coef1*coef2;
                }
                dRho = abs(dE/cca);
            }
        }

        RhoResult oRhoResult;

        oRhoResult.oStation = oStation;

        oRhoResult.oAB = oAB;
        oRhoResult.oMN = oMN;

        oRhoResult.dF = dF;
        oRhoResult.dI = dI;
        oRhoResult.dField = dField;
        oRhoResult.dErr = dErr;
        oRhoResult.dRho = dRho;

        aoRhoResult.append(oRhoResult);
    }

    poDb->importRho(aoRhoResult);

    emit SigRhoResult(aoRhoResult);
}

bool CalRhoThread::getAB()
{
    /* TX, AB */
    oAB = poDb->getCoordinate("A", "B");

    if(oAB.dMX == 0 && oAB.dMY == 0 && oAB.dMZ == 0 && oAB.dNX == 0 && oAB.dNY == 0 && oAB.dNZ == 0 )
    {
        emit SigMsg("AB\n坐标获取失败！");
        return false;
    }

    ptA.setX(oAB.dMX);
    ptA.setY(oAB.dMY);

    ptB.setX(oAB.dNX);
    ptB.setY(oAB.dNY);

    ptTxMid.setX( (ptA.x() + ptB.x())/2 );
    ptTxMid.setY( (ptA.y() + ptB.y())/2 );

    qDebugV0()<<fixed<< qSetRealNumberPrecision(3)<<"AB:"<<ptA<<ptB;

    return true;
}

/*****************************************************
 * Get coordinate from coordinate file
 *
 */
bool CalRhoThread::CoorRead(STATION oStation)
{
    /* RX, Line Site*/
    oMN = poDb->getCoordinate(oStation.oStrLineId, oStation.oStrSiteId);
    if(oMN.dMX == 0 && oMN.dMY == 0 && oMN.dMZ == 0 && oMN.dNX == 0 && oMN.dNY == 0 && oMN.dNZ == 0 )
    {
        return false;
    }

    ptM.setX(oMN.dMX);
    ptM.setY(oMN.dMY);

    ptN.setX(oMN.dNX);
    ptN.setY(oMN.dNY);

    ptRxMid.setX( (ptM.x() + ptN.x())/2 );
    ptRxMid.setY( (ptM.y() + ptN.y())/2 );

    return true;
}

/*************************************************************
 * Angle between two lines
 * Line1(p1,p2) Line2(p3, p4)
 *
 */
double CalRhoThread::AngleGet(const QPointF ptPoint1, const QPointF ptPoint2, const QPointF ptPoint3, const QPointF ptPoint4)
{
    double dAngle = 0.0;

    if (ptPoint1.x() == ptPoint2.x())
    {
        if (ptPoint3.x() == ptPoint4.x())
        {
            dAngle = 0.0;
        }
        else
        {
            dAngle = M_PI/2 - atan( (ptPoint3.y()- ptPoint4.y())/(ptPoint3.x() - ptPoint4.x()) );
        }
    }
    else
    {
        if (ptPoint3.x() == ptPoint4.x())
        {
            dAngle = M_PI/2 - atan( (ptPoint2.y() - ptPoint1.y())/(ptPoint2.x() - ptPoint1.x()) );
        }
        else
        {
            qreal rDelta1 = (ptPoint2.y() - ptPoint1.y())/(ptPoint2.x() - ptPoint1.x());
            qreal rDelta2 = (ptPoint3.y() - ptPoint4.y())/(ptPoint3.x() - ptPoint4.x());

            dAngle = qAbs(atan( (rDelta1 - rDelta2)/(1 + rDelta1*rDelta2) ));
        }
    }

    //qDebugV0()<<"Angle:"<<dAngle;

    return dAngle;
}

/*************************************************************
 * The length  between two points
 *
 */
double CalRhoThread::LengthGet(const QPointF ptPoint1, const QPointF ptPoint2)
{
    double dLength = 0;

    dLength = sqrt( pow( (ptPoint2.x() - ptPoint1.x()), 2 ) +
                    pow( (ptPoint2.y() - ptPoint1.y()), 2 ) );

    //qDebugV0()<<"Length:"<<dLength;

    return dLength;
}
