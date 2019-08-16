/**********************************************************************
 *(c) Copyright 2016 xxx.  All rights reserved.
 *
 * GDC2DP professional: MarkerPicker
 *
 * Created on: 2016-05-04
 * e-mail box: 278397935@qq.com
 * Context: First version
 */
#ifndef SCATTERPICKER_H
#define SCATTERPICKER_H

#include <QObject>
#include "Common/PublicDef.h"
#include <QApplication>
#include <qevent.h>
#include <qwhatsthis.h>
#include <qpainter.h>
#include <qwt_symbol.h>
#include <qwt_scale_map.h>
#include <qwt_plot.h>
#include <qwt_plot_canvas.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_directpainter.h>
#include <qwt_plot_textlabel.h>
#include <qwt_plot_marker.h>

class QPoint;
class QCustomEvent;
class QwtPlot;
class QwtPlotCurve;

class MarkerPicker : public QObject
{
    Q_OBJECT
public:
    MarkerPicker(QwtPlot *plot, QwtPlotCurve *scatter );

    virtual bool eventFilter( QObject *, QEvent * );

    virtual bool event( QEvent * );

private:
    void Select( const QPoint & );

    /* Move the selected Marker line */
    void Move( const QPoint & );

    void MoveBy( int dx, int dy );

    void release();

    /* Set Selected && Unselected marker line's color. */
    void CursorShow( bool bOn );

    /* Get current scatter */
    QwtPlotCurve *gpoScatter;

    QwtPlot *plot();
    const QwtPlot *plot() const;

    /* Global variable */
    QwtPlotMarker *gpoSelectedMarker;
    QwtPlotMarker *gpoUnselectedMarker;

signals:
    void SigMarkerMoved();
};

#endif // SCATTERPICKER_H
