#include "Data/RX.h"


RX::RX(QString oStrFileName, QObject *parent):QObject(parent)
{
    oStrCSV = oStrFileName;

    this->importRX(oStrFileName);
}

/* Import csv file */
void RX::importRX(QString oStrFileName)
{
    QFileInfo oFileInfo(oStrFileName);

    QString oStrBaseName = oFileInfo.baseName();

    int iPos = oStrBaseName.lastIndexOf(")_L");

    //qDebugV0()<<iPos;

    QString oStrTemp = oStrBaseName.mid(iPos + 2);

    //qDebugV0()<<oStrTemp;

    /* 读取文件名里面的信息，摘取线号点号仪器号通道号~ */
    QStringList aoStrStationInfo = oStrTemp.split('_', QString::SkipEmptyParts );

    //qDebugV0()<<aoStrStationInfo;

    /* LineId */
    QString oStrLineId= aoStrStationInfo.at(0);
    goStrLineId = oStrLineId.remove(0,1);

    /* SiteId */
    QString oStrSiteId= aoStrStationInfo.at(1);
    goStrSiteId = oStrSiteId.remove(0,1);

    /* DevId */
    QString oStrDevId = aoStrStationInfo.at(2);
    oStrDevId.remove(0,1);
    giDevId = oStrDevId.toInt();

    /* DevCh */
    QString oStrDevCh= aoStrStationInfo.at(3);
    oStrDevCh.remove(0,2);
    giDevCh = oStrDevCh.toInt();

    /* Component identifier */
    QString oStrCompTag= aoStrStationInfo.at(4);
    goStrCompTag = oStrCompTag;

    /* 读文件内容，场值 */
    QFile oFile(oStrFileName);
    QString oStrLineCSV;
    oStrLineCSV.clear();

    if(oFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream oStream(&oFile);

        oStream.seek(0);

        QStringList aoStrLineCSV;

        QString oStrLineCSV;

        while(!oStream.atEnd())
        {
            oStrLineCSV.clear();

            /*  */
            oStrLineCSV = oStream.readLine();

            if(!oStrLineCSV.isEmpty())
            {
                aoStrLineCSV.clear();

                aoStrLineCSV = oStrLineCSV.split(',', QString::SkipEmptyParts);

                adF.append(aoStrLineCSV.first().toDouble());

                /* 读取到了频率之后，在QStringList中删除掉 */
                aoStrLineCSV.removeFirst();

                QVector<double> adData;
                adData.clear();

                foreach(QString oStrData, aoStrLineCSV)
                {
                    adData.append(oStrData.toDouble());
                }

                aadScatter.append(adData);

                adE.append(getE(adData));

                adErr.append(getErr(adData));
            }
            else
            {
                break;
            }
        }
    }

    oFile.close();
}

/* 刷新散点图，同时，平均值和相对均方误差也应该对应刷新。 */
void RX::renewScatter(int iIndex)
{
    /*  */
    QFile oFile(oStrCSV);
    QString oStrLineCSV;
    oStrLineCSV.clear();

    if(oFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream oStream(&oFile);

        oStream.seek(0);

        QStringList aoStrLineCSV;

        QString oStrLineCSV;

        for(int i = 0; i < iIndex; i++)
        {
            oStream.readLine();
        }
        oStrLineCSV.clear();

        oStrLineCSV = oStream.readLine();

        if(!oStrLineCSV.isEmpty())
        {
            aoStrLineCSV.clear();

            aoStrLineCSV = oStrLineCSV.split(',', QString::SkipEmptyParts);

//            qDebugV0()<< aoStrLineCSV.first().toDouble();

            aoStrLineCSV.removeFirst();

            QVector<double> adData;
            adData.clear();

            foreach(QString oStrData, aoStrLineCSV)
            {
                adData.append(oStrData.toDouble());
            }

            aadScatter[iIndex] = adData;

            adE[iIndex] = getE(adData);

            adErr[iIndex] = getErr(adData);
        }
    }

    oFile.close();
}

/* 计算场值平均值 */
double RX::getE(QVector<double> adData)
{
    double dAverage = 0;
    double dSum = 0;

    foreach(double dData, adData)
    {
        dSum += dData;
    }

    if(adData.count() != 0)
    {
        dAverage = dSum/adData.count();
    }

    return dAverage;
}

/* 计算相对均方误差 */
double RX::getErr(QVector<double> adData)
{
    double dError = 0;

    double dAvg = 0;
    dAvg = this->getE(adData);

    double dTemp = 0 ;

    for(qint32 i = 0 ; i < adData.count(); i++)
    {
        dTemp += qPow((adData.at(i) - dAvg)/dAvg, 2);
    }

    dError = qSqrt(dTemp/adData.count()) * 100;

    return dError;
}

/***********************************************************
 * 1：Update Scatter,
 * 2：Update E(Field value)
 * 3：Update Error
 */
void RX::updateScatter(int iIndex, QVector<double> adScatter)
{
    aadScatter[iIndex].clear();

    aadScatter[iIndex] = adScatter;

    adE[iIndex] = this->getE(adScatter);

    adErr[iIndex] = this->getErr(adScatter);
}
