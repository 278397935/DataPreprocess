#include "MyDatabase.h"

MyDatabase::MyDatabase(QObject *parent) : QObject(parent)
{

}

void MyDatabase::connect()
{
    poDb = new QSqlDatabase(QSqlDatabase::addDatabase("QSQLITE"));

    poDb->setDatabaseName("MyDb.db");

    if(!poDb->open())
    {
        qDebugV5()<<poDb->lastError().text();
    }
    else
    {
        qDebugV0()<<"connect DB ok!";
    }

    QSqlQuery oQuery;


    if(!oQuery.exec("DELETE FROM RX"))
    {
        qDebugV5()<<oQuery.lastError().text();
    }
    if(!oQuery.exec("DELETE FROM Coordinate"))
    {
        qDebugV5()<<oQuery.lastError().text();
    }
    if(!oQuery.exec("DELETE FROM Rho"))
    {
        qDebugV5()<<oQuery.lastError().text();
    }
}

/* 导入电流文件 */
void MyDatabase::importTX(QString oStrFileName)
{
    QFile oFile(oStrFileName);

    if( !oFile.open( QFile::ReadOnly | QFile::Text ) )
    {
        QMessageBox::critical(NULL, tr("错误"),
                              QString("打开:\n%1\n失败").arg(oStrFileName),
                              QMessageBox::Yes | QMessageBox::Yes);
        return;
    }

    QString::SectionFlag flag = QString::SectionSkipEmpty;

    QTextStream stream(&oFile);

    QSqlQuery oQuery(*poDb);
    poDb->transaction();

    /* 在写之前，就将原来的数据清除掉。 */
    if(!oQuery.exec("DELETE FROM TX"))
    {
        qDebugV5()<<oQuery.lastError().text();
    }

    stream.seek(0);
    while (!stream.atEnd())
    {
        QString oStrLine = stream.readLine();

        bool bOkF = false, bOkI = false;

        double dF = oStrLine.section(",",0,0, flag).toDouble(&bOkF);
        double dI = oStrLine.section(",",1,1, flag).toDouble(&bOkI);

        if(bOkF && bOkI)
        {
            /* LineID, SiteID, DevID, DevCH, CompTag, F, Amplitude, Phase */
            if( !oQuery.exec(QString("INSERT INTO TX VALUES(%1, %2)")
                             .arg(dF)
                             .arg(dI)))

            {
                qDebugV5()<<oQuery.lastError().text();
            }
        }
    }

    poDb->commit();

    /* 更新model */
    QSqlTableModel *poModel = new QSqlTableModel(this, *poDb);
    poModel->setTable("TX");
    poModel->setEditStrategy(QSqlTableModel::OnManualSubmit);

    poModel->setHeaderData(0, Qt::Horizontal, QStringLiteral("频率"));
    poModel->setHeaderData(1, Qt::Horizontal, QStringLiteral("电流"));
    poModel->select();

    emit SigModelTX(poModel);
}

/* 将接收端场值写入到数据库中 */
void MyDatabase::importRX(QVector<RX*> apoRX)
{
    QSqlQuery oQuery(*poDb);
    poDb->transaction();

    if(!oQuery.exec("DELETE FROM RX"))
    {
        qDebugV5()<<oQuery.lastError().text();
    }

    foreach(RX *poRX, apoRX)
    {
        foreach(double dF, poRX->adF)
        {
            /* LineID, SiteID, DevID, DevCH, CompTag, F, Amplitude, Phase */
            if( !oQuery.exec(QString("INSERT INTO RX VALUES('%1', '%2', %3, %4, '%5', %6, %7, %8, %9)")
                             .arg(poRX->goStrLineId)
                             .arg(poRX->goStrSiteId)
                             .arg(poRX->giDevId)
                             .arg(poRX->giDevCh)
                             .arg(poRX->goStrCompTag)
                             .arg(dF)
                             .arg(this->getI(dF))
                             .arg(poRX->mapAvg.value(dF))
                             .arg(poRX->mapErr.value(dF))))
            {
                qDebugV5()<<oQuery.lastError().text();
            }
        }
    }
    poDb->commit();

    QSqlTableModel *poModel = new QSqlTableModel(this, *poDb);
    poModel->setTable("RX");
    poModel->setEditStrategy(QSqlTableModel::OnManualSubmit);
    poModel->setHeaderData(0, Qt::Horizontal, QStringLiteral("线号"));
    poModel->setHeaderData(1, Qt::Horizontal, QStringLiteral("点号"));
    poModel->setHeaderData(2, Qt::Horizontal, QStringLiteral("仪器号"));
    poModel->setHeaderData(3, Qt::Horizontal, QStringLiteral("通道号"));
    poModel->setHeaderData(4, Qt::Horizontal, QStringLiteral("分量标识"));
    poModel->setHeaderData(5, Qt::Horizontal, QStringLiteral("频率"));
    poModel->setHeaderData(6, Qt::Horizontal, QStringLiteral("电流"));
    poModel->setHeaderData(7, Qt::Horizontal, QStringLiteral("场值"));
    poModel->setHeaderData(8, Qt::Horizontal, QStringLiteral("相对均方误差"));
    poModel->select();

    emit SigModelRX(poModel);
}

void MyDatabase::importXY(QString oStrFileName)
{
    QFile oFile(oStrFileName);

    if( !oFile.open( QFile::ReadOnly | QFile::Text ) )
    {
        QMessageBox::critical(NULL, tr("错误"),
                              QString("打开:\n%1\n失败").arg(oStrFileName),
                              QMessageBox::Yes | QMessageBox::Yes);
        return;
    }

    QString::SectionFlag flag = QString::SectionSkipEmpty;

    QTextStream stream(&oFile);

    QSqlQuery oQuery(*poDb);

    poDb->transaction();

    if(!oQuery.exec("DELETE FROM Coordinate"))
    {
        qDebugV5()<<oQuery.lastError().text();
    }

    if( stream.readLine().split(",").count() != 8)
    {
        emit SigMsg("坐标文件有误，请核实！");
    }

    stream.seek(0);

    while (!stream.atEnd())
    {
        QString oStrLine = stream.readLine();

        QString oStrLineId = oStrLine.section(",",0,0, flag);
        QString oStrSiteId = oStrLine.section(",",1,1, flag);

        QString oStrMX = oStrLine.section(",",2,2, flag);
        QString oStrMY = oStrLine.section(",",3,3, flag);
        QString oStrMH = oStrLine.section(",",4,4, flag);

        QString oStrNX = oStrLine.section(",",5,5, flag);
        QString oStrNY = oStrLine.section(",",6,6, flag);
        QString oStrNH = oStrLine.section(",",7,7, flag);

        /* LineId, SiteId, X, Y */
        if( !oQuery.exec(QString("INSERT INTO Coordinate VALUES('%1', '%2', '%3', '%4', '%5', '%6', '%7', '%8')")
                         .arg(oStrLineId)
                         .arg(oStrSiteId)
                         .arg(oStrMX)
                         .arg(oStrMY)
                         .arg(oStrMH)
                         .arg(oStrNX)
                         .arg(oStrNY)
                         .arg(oStrNH)))

        {
            qDebugV5()<<oQuery.lastError().text();
        }
    }

    poDb->commit();

    QSqlTableModel *poModel = new QSqlTableModel(this, *poDb);
    poModel->setTable("Coordinate");
    poModel->setEditStrategy(QSqlTableModel::OnManualSubmit);
    poModel->setHeaderData(0, Qt::Horizontal, QStringLiteral("线号"));
    poModel->setHeaderData(1, Qt::Horizontal, QStringLiteral("点号"));
    poModel->setHeaderData(2, Qt::Horizontal, QStringLiteral("MX"));
    poModel->setHeaderData(3, Qt::Horizontal, QStringLiteral("MY"));
    poModel->setHeaderData(4, Qt::Horizontal, QStringLiteral("MH"));
    poModel->setHeaderData(5, Qt::Horizontal, QStringLiteral("NX"));
    poModel->setHeaderData(6, Qt::Horizontal, QStringLiteral("NY"));
    poModel->setHeaderData(7, Qt::Horizontal, QStringLiteral("NH"));
    poModel->select();

    emit SigModelXY(poModel);
}

void MyDatabase::importRho(QList<RhoResult> aoRhoResult)
{
    QSqlQuery oQuery(*poDb);

    poDb->transaction();

    foreach(RhoResult oRhoResult, aoRhoResult)
    {
        /* LineID, SiteID, DevID, DevCH, CompTag, F, I, Field, Rho */
        if( !oQuery.exec(QString("INSERT INTO Rho VALUES('%1', '%2', %3, %4, '%5',"
                                 "%6, %7, %8, '%9', %10, "
                                 "'%11', '%12', '%13', '%14', '%15', '%16', "
                                 "'%17', '%18', '%19', '%20', '%21', '%22')")
                         .arg(oRhoResult.oStation.oStrLineId)
                         .arg(oRhoResult.oStation.oStrSiteId)
                         .arg(oRhoResult.oStation.iDevId)
                         .arg(oRhoResult.oStation.iDevCh)
                         .arg(oRhoResult.oStation.oStrTag)
                         .arg(oRhoResult.dF)
                         .arg(oRhoResult.dI)
                         .arg(oRhoResult.dField)
                         .arg(QString("%1%").arg(QString::number(oRhoResult.dErr, 10, 3)))
                         .arg(oRhoResult.dRho)
                         .arg(QString::number(oRhoResult.oAB.dMX, 10, FloatPrecision))
                         .arg(QString::number(oRhoResult.oAB.dMY, 10, FloatPrecision))
                         .arg(QString::number(oRhoResult.oAB.dMZ, 10, FloatPrecision))
                         .arg(QString::number(oRhoResult.oAB.dNX, 10, FloatPrecision))
                         .arg(QString::number(oRhoResult.oAB.dNY, 10, FloatPrecision))
                         .arg(QString::number(oRhoResult.oAB.dNZ, 10, FloatPrecision))
                         .arg(QString::number(oRhoResult.oMN.dMX, 10, FloatPrecision))
                         .arg(QString::number(oRhoResult.oMN.dMY, 10, FloatPrecision))
                         .arg(QString::number(oRhoResult.oMN.dMZ, 10, FloatPrecision))
                         .arg(QString::number(oRhoResult.oMN.dNX, 10, FloatPrecision))
                         .arg(QString::number(oRhoResult.oMN.dNY, 10, FloatPrecision))
                         .arg(QString::number(oRhoResult.oMN.dNZ, 10, FloatPrecision))))

        {
            qDebugV5()<<oQuery.lastError().text();
        }
    }

    poDb->commit();

    QSqlTableModel *poModel = new QSqlTableModel(this, *poDb);
    poModel->setTable("Rho");
    poModel->setEditStrategy(QSqlTableModel::OnManualSubmit);
    poModel->setHeaderData(0, Qt::Horizontal, QStringLiteral("线号"));
    poModel->setHeaderData(1, Qt::Horizontal, QStringLiteral("点号"));
    poModel->setHeaderData(2, Qt::Horizontal, QStringLiteral("仪器号"));
    poModel->setHeaderData(3, Qt::Horizontal, QStringLiteral("通道号"));
    poModel->setHeaderData(4, Qt::Horizontal, QStringLiteral("分量标识"));

    poModel->setHeaderData(5, Qt::Horizontal, QStringLiteral("频率"));
    poModel->setHeaderData(6, Qt::Horizontal, QStringLiteral("电流"));
    poModel->setHeaderData(7, Qt::Horizontal, QStringLiteral("场值"));
    poModel->setHeaderData(8, Qt::Horizontal, QStringLiteral("相对均方误差"));
    poModel->setHeaderData(9, Qt::Horizontal, QStringLiteral("视电阻率"));

    poModel->setHeaderData(10, Qt::Horizontal, QStringLiteral("AX"));
    poModel->setHeaderData(11, Qt::Horizontal, QStringLiteral("AY"));
    poModel->setHeaderData(12, Qt::Horizontal, QStringLiteral("AH"));
    poModel->setHeaderData(13, Qt::Horizontal, QStringLiteral("BX"));
    poModel->setHeaderData(14, Qt::Horizontal, QStringLiteral("BY"));
    poModel->setHeaderData(15, Qt::Horizontal, QStringLiteral("BH"));
    poModel->setHeaderData(16, Qt::Horizontal, QStringLiteral("MX"));
    poModel->setHeaderData(17, Qt::Horizontal, QStringLiteral("MY"));
    poModel->setHeaderData(18, Qt::Horizontal, QStringLiteral("MH"));
    poModel->setHeaderData(19, Qt::Horizontal, QStringLiteral("NX"));
    poModel->setHeaderData(20, Qt::Horizontal, QStringLiteral("NY"));
    poModel->setHeaderData(21, Qt::Horizontal, QStringLiteral("NH"));
    poModel->select();

    emit SigModelRho(poModel);
}

QVector<double> MyDatabase::getF(STATION oStation)
{
    QSqlQuery oQuery(*poDb);
    QVector<double> adF;
    adF.clear();

    if( ! oQuery.exec(QString("SELECT DISTINCT F FROM RX WHERE "
                              "LineId = '%1' AND SiteId = '%2' AND "
                              "DevId = %3 AND DevCh = %4")
                      .arg(oStation.oStrLineId)
                      .arg(oStation.oStrSiteId)
                      .arg(oStation.iDevId)
                      .arg(oStation.iDevCh)) )
    {
        qDebugV5()<<oQuery.lastError().text();
    }
    while(oQuery.next())
    {
        adF.append(oQuery.value("F").toDouble());
    }
    qDebugV0()<<oStation.oStrLineId
             <<oStation.oStrSiteId
            <<oStation.iDevId
           <<oStation.iDevCh
          <<adF;

    return adF;
}

/* 获取对应频点的电流值 */
double MyDatabase::getI(double dF)
{
    QSqlQuery oQuery(*poDb);

    /* Default = 1， 电流要用来做除数，所以不能为0。 2020年03月06日 */
    double dI = 1;

    if( !oQuery.exec(QString("SELECT I FROM TX WHERE F = %1")
                     .arg(dF)) )
    {
        qDebugV5()<<oQuery.lastError().text();
    }
    if(oQuery.first())
    {
        dI = oQuery.value("I").toDouble();
    }
    else
    {
        emit SigMsg(QString("未找到频点%1Hz的电流值！/n请确认。").arg(dF));
    }

    return dI;
}

double MyDatabase::getField(STATION oStation, double dF)
{
    QSqlQuery oQuery(*poDb);
    double dField = 0;

    if( ! oQuery.exec(QString("SELECT Field FROM RX WHERE "
                              "LineId = '%1' AND SiteId = '%2' AND "
                              "DevId  =  %3  AND DevCh  = %4   AND F = %5")
                      .arg(oStation.oStrLineId)
                      .arg(oStation.oStrSiteId)
                      .arg(oStation.iDevId)
                      .arg(oStation.iDevCh)
                      .arg(dF)) )
    {
        qDebugV5()<<oQuery.lastError().text();
    }
    if(oQuery.first())
    {
        dField = oQuery.value("Field").toDouble();
    }

    //    qDebugV0()<<oStation.oStrLineId
    //             <<oStation.oStrSiteId
    //            <<oStation.iDevId
    //           <<oStation.iDevCh
    //          <<dF
    //         <<dField;

    return dField;
}

double MyDatabase::getErr(STATION oStation, double dF)
{
    QSqlQuery oQuery(*poDb);
    double dErr = 0;

    if( ! oQuery.exec(QString("SELECT Err FROM RX WHERE "
                              "LineId = '%1' AND SiteId = '%2' AND "
                              "DevId  =  %3  AND DevCh  = %4   AND F = %5")
                      .arg(oStation.oStrLineId)
                      .arg(oStation.oStrSiteId)
                      .arg(oStation.iDevId)
                      .arg(oStation.iDevCh)
                      .arg(dF)) )
    {
        qDebugV5()<<oQuery.lastError().text();
    }
    if(oQuery.first())
    {
        dErr = oQuery.value("Err").toDouble();
    }

    //    qDebugV0()<<oStation.oStrLineId
    //             <<oStation.oStrSiteId
    //            <<oStation.iDevId
    //           <<oStation.iDevCh
    //          <<dF
    //         <<dErr;

    return dErr;
}

Position MyDatabase::getCoordinate(QString oStrLineId, QString oStrSiteId)
{
    QSqlQuery oQuery(*poDb);

    Position aoPt;

    aoPt.dMX = 0;
    aoPt.dMY = 0;
    aoPt.dMZ = 0;

    aoPt.dNX = 0;
    aoPt.dNY = 0;
    aoPt.dNZ = 0;

    if( ! oQuery.exec(QString("SELECT MX, MY, MH, NX, NY, NH FROM Coordinate WHERE "
                              "LineId = '%1' AND SiteId = '%2'")
                      .arg(oStrLineId)
                      .arg(oStrSiteId)))
    {
        qDebugV5()<<oQuery.lastError().text();
    }
    if(oQuery.first())
    {
        aoPt.dMX = oQuery.value("MX").toDouble();
        aoPt.dMY = oQuery.value("MY").toDouble();
        aoPt.dMZ = oQuery.value("MH").toDouble();

        aoPt.dNX = oQuery.value("NX").toDouble();
        aoPt.dNY = oQuery.value("NY").toDouble();
        aoPt.dNZ = oQuery.value("NH").toDouble();
    }

    return aoPt;
}

QList<STATION> MyDatabase::getStation()
{
    QList< STATION > aoStation;

    QSqlQuery oQuery(*poDb);

    if( ! oQuery.exec(QString("SELECT LineId, SiteId, DevId, DevCh, CompTag FROM RX")) )
    {
        qDebugV5()<<oQuery.lastError().text();
    }
    while(oQuery.next())
    {
        STATION oStation;

        oStation.oStrLineId = oQuery.value("LineId").toString();
        oStation.oStrSiteId = oQuery.value("SiteId").toString();
        oStation.iDevId     = oQuery.value("DevId").toInt();
        oStation.iDevCh     = oQuery.value("DevCh").toInt();
        oStation.oStrTag    = oQuery.value("CompTag").toString();

        aoStation.append( oStation );
    }

    for (int i = 0; i < aoStation.count(); i++)
    {
        for (int k = i + 1; k <  aoStation.count(); k++)
        {
            if ( aoStation.at(i).oStrLineId == aoStation.at(k).oStrLineId &&
                 aoStation.at(i).oStrSiteId == aoStation.at(k).oStrSiteId &&
                 aoStation.at(i).iDevId == aoStation.at(k).iDevId &&
                 aoStation.at(i).iDevCh == aoStation.at(k).iDevCh)
            {
                aoStation.removeAt(k);
                k--;
            }
        }
    }

    foreach(STATION oStation, aoStation)
    {
        qDebugV0()<<oStation.oStrLineId
                 <<oStation.oStrSiteId
                <<oStation.iDevId
               <<oStation.iDevCh;
    }
    return aoStation;
}

