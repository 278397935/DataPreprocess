#include <qobject.h>
#include <qwt_plot_textlabel.h>

class QPoint;
class QCustomEvent;
class QwtPlot;
class QwtPlotCurve;


class CanvasPicker: public QObject
{
    Q_OBJECT
public:
    CanvasPicker( QwtPlot *plot );

    virtual bool eventFilter( QObject *, QEvent * );

    virtual bool event( QEvent * );

private:
    void select( const QPoint & );

    void showCursor( bool enable );

    QwtPlot *plot();
    const QwtPlot *plot() const;

    QwtPlotCurve *d_selectedCurve;
    int d_selectedPoint;

signals:
    void SigSelected( QwtPlotCurve *, int );
};
