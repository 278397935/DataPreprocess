/**********************************************************************
 * (c) Copyright 2016 xxx.  All rights reserved.
 *
 * GDC2 data process: Public define
 *
 *  Created on: 2015-3-5
 *      Author: chenglong.li
 */

#ifndef PUBLICDEF_H
#define PUBLICDEF_H

#include <QtCore>

/****************************************************************/
/* SoftWare Information*/
/****************************************************************/
#define SoftWareName    ("GDC Data Process")
#define SoftWareVer     ("1.0")
#define SoftWareMagic   ("777")

#define SoftAbout       ("<h2>GDC Data Process v1.0</h2><p>Copyright &copy; 2016 Central South University.\n--------------------------------------------------\n<P>Developer: Chenglong.Li, Haiping.Yang, Shangeshi.Wen")
#define SoftSupport     ("<h2>GDC Data Process v1.0</h2><P>Developer: Chenglong.Li, Haiping.Yang<p>Mail: lcl650@qq.com\n")

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

enum FILE_TYPE
{
    FILE_RX = 0,
    FILE_TX,
    FILE_XY
};

enum RX_SRC
{
    RX_ORG = 0,
    RX_TEMP
};

enum EXIST_SWITCH
{
    EXIST_NO = 0,
    EXIST_YES
};

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
