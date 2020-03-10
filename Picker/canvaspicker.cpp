#include <qapplication.h>
#include <qevent.h>
#include <qwhatsthis.h>
#include <qpainter.h>
#include <qwt_plot.h>
#include <qwt_symbol.h>
#include <qwt_scale_map.h>
#include <qwt_plot_canvas.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_directpainter.h>
#include "Canvaspicker.h"

CanvasPicker::CanvasPicker( QwtPlot *plot ):
    QObject( plot ),
    d_selectedCurve( NULL ),
    d_selectedPoint( -1 )
{
    QwtPlotCanvas *canvas = qobject_cast<QwtPlotCanvas *>( plot->canvas() );
    canvas->installEventFilter( this );

    // We want the focus, but no focus rect. The
    // selected point will be highlighted instead.

    canvas->setFocusPolicy( Qt::StrongFocus );
#ifndef QT_NO_CURSOR
    canvas->setCursor( Qt::PointingHandCursor );
#endif
    canvas->setFocusIndicator( QwtPlotCanvas::ItemFocusIndicator );
    canvas->setFocus();
}

QwtPlot *CanvasPicker::plot()
{
    return qobject_cast<QwtPlot *>( parent() );
}

const QwtPlot *CanvasPicker::plot() const
{
    return qobject_cast<const QwtPlot *>( parent() );
}

bool CanvasPicker::event( QEvent *ev )
{
//    if ( ev->type() == QEvent::User )
//    {
//        showCursor( true );
//        return true;
//    }
    return QObject::event( ev );
}

bool CanvasPicker::eventFilter( QObject *object, QEvent *event )
{
    if ( plot() == NULL || object != plot()->canvas() )
        return false;

    switch( event->type() )
    {
    //    case QEvent::FocusIn:
    //    {
    //        showCursor( true );
    //        break;
    //    }
    //    case QEvent::FocusOut:
    //    {
    //        showCursor( false );
    //        break;
    //    }
    case QEvent::Paint:
    {
        QApplication::postEvent( this, new QEvent( QEvent::User ) );
        break;
    }
    case QEvent::MouseButtonDblClick:
    {
        const QMouseEvent *mouseEvent = static_cast<QMouseEvent *>( event );
        select( mouseEvent->pos() );
        return true;
    }

    default:
        break;
    }

    return QObject::eventFilter( object, event );
}

/* Select the point at a position. If there is no point
   deselect the selected point */
void CanvasPicker::select( const QPoint &pos )
{
    QwtPlotCurve *curve = NULL;
    double dist = 10e10;
    int index = -1;

    const QwtPlotItemList& itmList = plot()->itemList();
    foreach( QwtPlotItem *poItem, itmList )
    {
        if ( poItem->rtti() == QwtPlotItem::Rtti_PlotCurve)
        {
            if( (static_cast<QwtPlotCurve *>( poItem ))->style() == QwtPlotCurve::Lines &&
                    (static_cast<QwtPlotCurve *>( poItem ))->isVisible() )
            {
                QwtPlotCurve *c = static_cast<QwtPlotCurve *>(poItem);

                double d;
                int idx = c->closestPoint( pos, &d );
                if ( d < dist )
                {
                    curve = c;
                    index = idx;
                    dist = d;
                }
            }
        }
    }

//    showCursor( false );
    d_selectedCurve = NULL;
    d_selectedPoint = -1;

    qDebug()<<"~~~look    how:"<< curve->title().text();

    if ( curve && dist < 10 ) // 10 pixels tolerance
    {
        d_selectedCurve = curve;
        d_selectedPoint = index;
//        showCursor( true );

        emit SigSelected(d_selectedCurve, d_selectedPoint);
    }
}

// Hightlight the selected point
void CanvasPicker::showCursor( bool showIt )
{
    qDebug()<<"only~~~:"<<showIt;
    if ( d_selectedCurve == NULL )
    {
        return;
    }
    //  qDebug()<<"showCursor"<<d_selectedCurve->title().text()<<"bool"<<showIt;
    QwtSymbol *symbol = const_cast<QwtSymbol *>( d_selectedCurve->symbol() );

    const QBrush brush = symbol->brush();

    if ( showIt )
    {
        symbol->setBrush( symbol->brush().color().dark( 10 ) );
    }

    QwtPlotDirectPainter directPainter;
    directPainter.drawSeries( d_selectedCurve, d_selectedPoint, d_selectedPoint );

    if ( showIt )
    {
        symbol->setBrush( brush ); // reset brush
    }
}
