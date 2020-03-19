/**********************************************************************
 *(c) Copyright 2016 xxx.  All rights reserved.
 *
 * GDC2DP professional: MarkerPicker
 *
 * Created on: 2016-05-04
 * e-mail box: 278397935@qq.com
 * Context: First version
 */
#include "MarkerPicker.h"

/****************************************************************************
 * MarkerPicker : constructure function
 *
 */
MarkerPicker::MarkerPicker( QwtPlot *plot, QwtPlotCurve *scatter ):
    QObject( plot )
{
    QwtPlotCanvas *poCanvas = qobject_cast<QwtPlotCanvas *>( plot->canvas() );
    poCanvas->installEventFilter( this );

    /* Get current csatter pointer */
    gpoScatter = scatter;
    gpoSelectedMarker   = NULL;
    gpoUnselectedMarker = NULL;

    poCanvas->setFocusPolicy( Qt::StrongFocus );
    //#ifndef QT_NO_CURSOR
    //poCanvas->setCursor( Qt::PointingHandCursor );
    //#endif
    poCanvas->setFocusIndicator( QwtPlotCanvas::ItemFocusIndicator );
    //poCanvas->setFocus();
}

/**********************************************
 * Convert parent pointer to QwtPlot
 *
 */
QwtPlot *MarkerPicker::plot()
{
    return qobject_cast<QwtPlot *>( parent() );
}

/**********************************************
 * Convert parent pointer to QwtPlot
 *
 */
const QwtPlot *MarkerPicker::plot() const
{
    return qobject_cast<const QwtPlot *>( parent() );
}

/***********************************************
 * Event
 *
 */
bool MarkerPicker::event( QEvent *ev )
{
    if ( ev->type() == QEvent::User )
    {
        //qDebugV0()<<"event.";
        return true;
    }
    return QObject::event( ev );
}
/******************************************************************
 * event filter
 *
 */
bool MarkerPicker::eventFilter( QObject *object, QEvent *event )
{
    if ( plot() == NULL || object != plot()->canvas() )
    {
        return false;
    }

    switch( event->type() )
    {
    case QEvent::FocusIn:
    {
        plot()->canvas()->setFocus();

        //CursorShow( true );
        break;
    }
    case QEvent::FocusOut:
    {
        //CursorShow( false );
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

        Select( mouseEvent->pos() );
        return true;
    }
    case QEvent::MouseMove:
    {
        const QMouseEvent *mouseEvent = static_cast<QMouseEvent *>( event );
        Move( mouseEvent->pos() );
        return true;

    }
    case QEvent::MouseButtonRelease:
    {
        //qDebugV0()<<"Mouse button(Marker Line) release";
        return true;
    }
    default:
        break;
    }

    return QObject::eventFilter( object, event );
}

/*******************************************************
* Select the one marker line
*
*/
void MarkerPicker::Select( const QPoint &pos )
{
    /* List struct for stage QwtPlotmarker line pointer and etc. */
    QMap<QwtPlotMarker*, double> oMapMarker;
    oMapMarker.clear();

    const QwtPlotItemList& itmList = plot()->itemList();

    for ( QwtPlotItemIterator it = itmList.begin(); it != itmList.end(); ++it )
    {
        if(( *it )->rtti() == QwtPlotItem::Rtti_PlotMarker )
        {
            QwtPlotMarker *poTmpMarker = static_cast<QwtPlotMarker *>( *it );

            switch( poTmpMarker->lineStyle() )
            {
            case QwtPlotMarker::HLine:

                oMapMarker.insert( poTmpMarker, qAbs( poTmpMarker->yValue() - plot()->invTransform(poTmpMarker->yAxis(), pos.y()) ) );
                break;
            case QwtPlotMarker::VLine:

                oMapMarker.insert( poTmpMarker, qAbs( poTmpMarker->xValue() - plot()->invTransform( poTmpMarker->xAxis(), pos.x()) ) );
                break;
            default:break;
            }
        }
    }

    /* Check the Marker line number is 2, otherwise, return! */
    if(oMapMarker.count() != 2 )
    {
        qDebugV5()<<"Marker line number is wrong";
        return;
    }

    /* To the global variable assignment, the first set is NULL. */
    gpoSelectedMarker   = NULL;
    gpoUnselectedMarker = NULL;

    /* Selected the first Marker */
    if ( oMapMarker.value( oMapMarker.firstKey() ) < 20 &&
         oMapMarker.value( oMapMarker.firstKey() ) < oMapMarker.value( oMapMarker.lastKey() ) )
    {
        gpoSelectedMarker   = oMapMarker.firstKey();
        gpoUnselectedMarker = oMapMarker.lastKey();
    }
    /* Selected the secend Marker */
    else if( oMapMarker.value( oMapMarker.lastKey() ) < 20 &&
             oMapMarker.value( oMapMarker.lastKey() ) < oMapMarker.value( oMapMarker.firstKey() ) )
    {
        gpoSelectedMarker   = oMapMarker.lastKey();
        gpoUnselectedMarker = oMapMarker.firstKey();
    }
    else
    {
        return;
    }

    CursorShow( true );
}

/*******************************************************
 * Move the selected Marker line
 *
 */
void MarkerPicker::MoveBy( int dx, int dy )
{
    if ( dx == 0 && dy == 0 )
    {
        return;
    }

    if ( !gpoSelectedMarker )
    {
        return;
    }

    const double x = plot()->transform( gpoSelectedMarker->xAxis(), gpoSelectedMarker->xValue() );
    const double y = plot()->transform( gpoSelectedMarker->yAxis(), gpoSelectedMarker->yValue()  );

    //qDebugV0()<<"x,y:"<<x<<y;
    Move( QPoint( qRound( x + dx ), qRound( y + dy ) ) );
}

/***************************************************************************
 * Set Selected && Unselected marker line's color.
 *
 */
void MarkerPicker::CursorShow(bool bOn)
{
    /* Non marker line, then return. */
    if( gpoSelectedMarker == NULL || gpoUnselectedMarker == NULL )
    {
        return;
    }

    if(bOn)
    {
        gpoSelectedMarker->setLinePen(   Qt::red,   2.0, Qt::SolidLine );
        gpoUnselectedMarker->setLinePen( Qt::black, 2.0, Qt::SolidLine );
    }
    else
    {
        gpoSelectedMarker->setLinePen(   Qt::black, 2.0, Qt::SolidLine );
        gpoUnselectedMarker->setLinePen( Qt::black, 2.0, Qt::SolidLine );
    }
    plot()->replot();
}

/**********************************************
 * Move the selected Marker line
 *
 */
void MarkerPicker::Move( const QPoint &pos )
{
    /* Real time set  plot Scale and set aside blank. */
    double dXSacle = gpoScatter->maxXValue() - gpoScatter->minXValue();
    double dYSacle = gpoScatter->maxYValue() - gpoScatter->minYValue();

    if( gpoSelectedMarker == NULL && gpoUnselectedMarker == NULL )
    {
        //qDebugV5()<<"gpoSelectedMarker or gpoUnselectedMarker is NULL!";
        return;
    }

    /* Marker line is A vertical line(Left && Right) */
    if( gpoSelectedMarker->lineStyle() == QwtPlotMarker::VLine )
    {
        double moveX = plot()->invTransform(gpoSelectedMarker->xAxis(), pos.x());

        /* Left */
        if(gpoSelectedMarker->xValue() < gpoUnselectedMarker->xValue())
        {
            if( !( gpoScatter->minXValue()- dXSacle*0.02 <= moveX && moveX < gpoUnselectedMarker->xValue() ) )
            {
                return;
            }

            gpoSelectedMarker->setLabel( QString("Left:X= %1").arg( QString::number(moveX, 'f', 0) ));
        }
        else/* Right */
        {
            if( !(gpoUnselectedMarker->xValue() < moveX && moveX <= gpoScatter->maxXValue() + dXSacle*0.02 ) )
            {
                return;
            }

            gpoSelectedMarker->setLabel( QString("Right:X = %1").arg( QString::number(moveX, 'f', 0)) );
        }

        /* Assign a new value */
        gpoSelectedMarker->setXValue( moveX );
    }
    /* Marker line is A horizontal line(Bottom||Top) */
    else if(gpoSelectedMarker->lineStyle() == QwtPlotMarker::HLine )
    {
        double moveY = plot()->invTransform(gpoSelectedMarker->yAxis(), pos.y());

        /* Bottom */
        if(gpoSelectedMarker->yValue() < gpoUnselectedMarker->yValue() )
        {
            if( !(gpoScatter->minYValue() - dYSacle*0.075<= moveY && moveY < gpoUnselectedMarker->yValue()) )
            {
                return;
            }

            gpoSelectedMarker->setLabel( QString("Bottom:Y = %1").arg( QString::number(moveY, 'f', 2)) );
        }
        else/* Top */
        {
            if( !(gpoUnselectedMarker->yValue() < moveY && moveY <= gpoScatter->maxYValue() + dYSacle*0.075 ) )
            {
                return;
            }

            gpoSelectedMarker->setLabel( QString("Top:Y = %1").arg( QString::number(moveY, 'f', 2)) );
        }

        /* Assign a new value */
        gpoSelectedMarker->setYValue( moveY );
    }

    plot()->replot();

    emit SigMarkerMoved();

    //plot()->canvas()->setPaintAttribute( QwtPlotCanvas::ImmediatePaint, false );
}
