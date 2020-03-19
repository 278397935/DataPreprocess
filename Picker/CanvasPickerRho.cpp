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
#include "CanvasPickerRho.h"

CanvasPickerRho::CanvasPickerRho( QwtPlot *plot ):
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

    const char *text =
            "All points can be moved using the left mouse button "
            "or with these keys:\n\n"
            "- Up:\t\tSelect next curve\n"
            "- Down:\t\tSelect previous curve\n"
            "- Left, '-':\tSelect next point\n"
            "- Right, '+':\tSelect previous point\n"
            "- 7, 8, 9, 4, 6, 1, 2, 3:\tMove selected point";
    canvas->setWhatsThis( text );

    shiftCurveCursor( true );
}

QwtPlot *CanvasPickerRho::plot()
{
    return qobject_cast<QwtPlot *>( parent() );
}

const QwtPlot *CanvasPickerRho::plot() const
{
    return qobject_cast<const QwtPlot *>( parent() );
}

bool CanvasPickerRho::event( QEvent *ev )
{
    if ( ev->type() == QEvent::User )
    {
        showCursor( true );
        return true;
    }
    return QObject::event( ev );
}

void CanvasPickerRho::setNULL()
{
    d_selectedCurve = NULL;
}

bool CanvasPickerRho::eventFilter( QObject *object, QEvent *event )
{
    if ( plot() == NULL || object != plot()->canvas() )
        return false;

    switch( event->type() )
    {
    case QEvent::FocusIn:
    {
        showCursor( true );
        break;
    }
    case QEvent::FocusOut:
    {
        showCursor( false );
        break;
    }
    case QEvent::Paint:
    {
        QApplication::postEvent( this, new QEvent( QEvent::User ) );
        break;
    }
    case QEvent::MouseButtonPress:
    {
        const QMouseEvent *mouseEvent = static_cast<QMouseEvent *>( event );
        select( mouseEvent->pos() );
        return true;
    }
    case QEvent::MouseMove:
    {
        const QMouseEvent *mouseEvent = static_cast<QMouseEvent *>( event );
        move( mouseEvent->pos() );
        return true;
    }
    default:
        break;
    }

    return QObject::eventFilter( object, event );
}

// Select the point at a position. If there is no point
// deselect the selected point

void CanvasPickerRho::select( const QPoint &pos )
{
    QwtPlotCurve *curve = NULL;
    double dist = 10e10;
    int index = -1;

    const QwtPlotItemList& itmList = plot()->itemList();
    for ( QwtPlotItemIterator it = itmList.begin();
          it != itmList.end(); ++it )
    {
        if ( ( *it )->rtti() == QwtPlotItem::Rtti_PlotCurve )
        {
            QwtPlotCurve *c = static_cast<QwtPlotCurve *>( *it );

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

    showCursor( false );
    d_selectedCurve = NULL;
    d_selectedPoint = -1;

    if ( curve && dist < 10 ) // 10 pixels tolerance
    {
        d_selectedCurve = curve;
        d_selectedPoint = index;
        showCursor( true );

        emit SigSelectedRho(curve, index);
    }
}

// Move the selected point
void CanvasPickerRho::move( const QPoint &pos )
{
    if ( !d_selectedCurve )
        return;

    QVector<double> xData( d_selectedCurve->dataSize() );
    QVector<double> yData( d_selectedCurve->dataSize() );

    for ( int i = 0; i < static_cast<int>( d_selectedCurve->dataSize() ); i++ )
    {
        const QPointF sample = d_selectedCurve->sample( i );
        xData[i] = sample.x();

        if ( i == d_selectedPoint )
        {
            //xData[i] = plot()->invTransform( d_selectedCurve->xAxis(), pos.x() );
            yData[i] = plot()->invTransform( d_selectedCurve->yAxis(), pos.y() );
        }
        else
        {
            yData[i] = sample.y();
        }
    }
    d_selectedCurve->setSamples( xData, yData );

    emit SigMoved();

    /*
       Enable QwtPlotCanvas::ImmediatePaint, so that the canvas has been
       updated before we paint the cursor on it.
     */
    QwtPlotCanvas *plotCanvas = qobject_cast<QwtPlotCanvas *>( plot()->canvas() );

    plotCanvas->setPaintAttribute( QwtPlotCanvas::ImmediatePaint, true );
    plot()->replot();
    plotCanvas->setPaintAttribute( QwtPlotCanvas::ImmediatePaint, false );

    showCursor( true );
}

// Hightlight the selected point
void CanvasPickerRho::showCursor( bool showIt )
{
    if ( !d_selectedCurve )
        return;

    QwtSymbol *symbol = const_cast<QwtSymbol *>( d_selectedCurve->symbol() );

    const QBrush brush = symbol->brush();
    if ( showIt )
        symbol->setBrush( symbol->brush().color().dark( 180 ) );

    QwtPlotDirectPainter directPainter;
    directPainter.drawSeries( d_selectedCurve, d_selectedPoint, d_selectedPoint );

    if ( showIt )
        symbol->setBrush( brush ); // reset brush
}

// Select the next/previous curve
void CanvasPickerRho::shiftCurveCursor( bool up )
{
    QwtPlotItemIterator it;

    const QwtPlotItemList &itemList = plot()->itemList();

    QwtPlotItemList curveList;
    for ( it = itemList.begin(); it != itemList.end(); ++it )
    {
        if ( ( *it )->rtti() == QwtPlotItem::Rtti_PlotCurve )
            curveList += *it;
    }
    if ( curveList.isEmpty() )
        return;

    it = curveList.begin();

    if ( d_selectedCurve )
    {
        for ( it = curveList.begin(); it != curveList.end(); ++it )
        {
            if ( d_selectedCurve == *it )
                break;
        }
        if ( it == curveList.end() ) // not found
            it = curveList.begin();

        if ( up )
        {
            ++it;
            if ( it == curveList.end() )
                it = curveList.begin();
        }
        else
        {
            if ( it == curveList.begin() )
                it = curveList.end();
            --it;
        }
    }

    showCursor( false );
    d_selectedPoint = 0;
    d_selectedCurve = static_cast<QwtPlotCurve *>( *it );
    showCursor( true );
}

// Select the next/previous neighbour of the selected point
void CanvasPickerRho::shiftPointCursor( bool up )
{
    if ( !d_selectedCurve )
        return;

    int index = d_selectedPoint + ( up ? 1 : -1 );
    index = ( index + d_selectedCurve->dataSize() ) % d_selectedCurve->dataSize();

    if ( index != d_selectedPoint )
    {
        showCursor( false );
        d_selectedPoint = index;
        showCursor( true );
    }
}
