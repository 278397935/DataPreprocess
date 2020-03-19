#include <qobject.h>

#include "Common/PublicDef.h"


class QPoint;
class QCustomEvent;
class QwtPlot;
class QwtPlotCurve;

class CanvasPickerRho: public QObject
{
    Q_OBJECT
public:
    CanvasPickerRho( QwtPlot *plot );
    virtual bool eventFilter( QObject *, QEvent * );

    virtual bool event( QEvent * );

private:
    void select( const QPoint & );
    void move( const QPoint & );

    void release();

    void showCursor( bool enable );
    void shiftPointCursor( bool up );
    void shiftCurveCursor( bool up );

    QwtPlot *plot();
    const QwtPlot *plot() const;

    QwtPlotCurve *d_selectedCurve;
    int d_selectedPoint;
signals:
    void SigSelectedRho(QwtPlotCurve *, int);

    void SigMoved();
};
