#ifndef PUBLICDEF_H
#define PUBLICDEF_H

#include <QtCore>


/****************************************************************/
/* Debug define*/
/****************************************************************/
#define qDebugV0() (qDebug()<<"[INFO]["<<__FUNCTION__<<":"<<__LINE__<<"] ")

/* Some debug info in test */
//#define qDebugV1() (qWarning()<<"[Debug]["<<__FUNCTION__<<":"<<__LINE__<<"] ")

/* Critical */
#define qDebugV5() (qCritical()<<"[ERR]["<<__FILE__"|"<<__FUNCTION__<<":"<<__LINE__<<"] ")

/****************************************************************/
/* Wigdet property*/
/****************************************************************/
#define WGT_OPACITY         0.98

/* For MainMenu */
#define WGT_BAR_WIDTH       30

/* For shadow */
#define WGT_PADDING         0
#define WGT_BAR_PADDING     6

/* Current running dir */
#define RunningDir QCoreApplication::applicationDirPath()

/* Last Dir log */
#define LASTDIR    "LastDirLog.ini"

#define FloatPrecision 3

struct STATION_INFO
{
    quint32 uiLineId;
    quint32 uiSiteId;
    quint32 uiDevId;
    quint32 uiDevCh;
    QString oStrCompTag;
    quint32 uiFCnt;
};

struct Position
{
    double dMX;
    double dMY;
    double dMZ;

    double dNX;
    double dNY;
    double dNZ;
};
#endif // PUBLICDEF_H
