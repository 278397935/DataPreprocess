#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "Common/PublicDef.h"

#include <QMainWindow>
#include <QFileDialog>

#include <QTreeWidgetItem>
#include <QMessageBox>

#include <qwt_scale_engine.h>
#include <qwt_scale_draw.h>
#include <qwt_plot_grid.h>
#include <qwt_scale_widget.h>
#include <qwt_plot_layout.h>
#include <qwt_plot_magnifier.h>
#include <qwt_plot_panner.h>

#include <QInputDialog>

#include "Data/RX.h"
#include "Picker/Canvaspicker.h"
#include "Picker/MarkerPicker.h"

#include "Picker/CanvasPickerRho.h"

#include "CalRhoThread.h"

#include "MyDatabase.h"


/* Marker line list */
typedef struct _MARKER_LIST
{
    QwtPlotMarker *poTop;
    QwtPlotMarker *poBottom;
    QwtPlotMarker *poLeft;
    QwtPlotMarker *poRight;

}MARKER_LIST;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    MyDatabase *poDb;

    CalRhoThread *poCalRho;

    QStringList gaoRx;

    QVector<RX*> gapoRX;

    /* Curve canvas picker object pointer */
    CanvasPicker *poCurvePicker;

    /* Draw Curve */
    void recoveryCurve(QwtPlotCurve *poCurve);

    void store();

private:
    Ui::MainWindow *ui;

    QMap<QwtPlotCurve*, RX*> gmapCurveData;

    QMap<QwtPlotCurve*, QTreeWidgetItem*> gmapCurveItem;

    /* Scatter canvas picker object pointer */
    MarkerPicker *poMarkerPicker;

    /* Global Var: scatter pointer */
    QwtPlotCurve *gpoScatter;
    QwtPlotCurve *gpoErrorCurve;

    QwtPlotCurve *gpoSelectedCurve;
    int giSelectedIndex;

    RX *gpoSelectedRX;




    //QwtPlotTextLabel *gpoErrorTextLabel;

    /* marker lines list */
    MARKER_LIST sMkList;

    /* Remove files in dir */
    void FilesInDirRemove(const QString& path);

    /* Init QwtPlot(Curve) */
    void initPlotCurve();

    /* Init QwtPlot(Curve) */
    void initPlotScatter();

    /* Get all the points in the scatter diagram  */
    QPolygonF currentScatterPoints();

    QPolygonF currentCurvePoints();

    /* Detach Marker line */
    void clearMarker();

    /* Real time set plot canvas Scale & Set aside blank. */
    void resizeScaleScatter();

    QPolygonF getR(QMap<double, double> oMap);

    /* Read last Dir log file, get last Dir(Previous directory) */
    QString LastDirRead();

    /* Save the current project Dir as the most recently opened directory */
    void LastDirWrite(QString oStrFileName);

    void initPlotTx();

    void drawTx( const QVector< double > adF,  const QVector< double > adI );

    void initPlotRx();

    void drawRx();

    void initPlotRho();


    QStringList aoStrExisting;

    QMap<QwtPlotCurve*, STATION> gmapCurveStation;

    /*"Shift + Ctrl + R",恢复选中的Rho整条曲线 */
    void keyPressEvent(QKeyEvent *event);

private slots:
    void on_actionImportTX_triggered();
    void on_actionImportRX_triggered();
    void on_actionExportRho_triggered();

    void on_actionClose_triggered();

    void on_actionClear_triggered();

    /* Read data from RX, update Scatter\Error\Curve */
    void on_actionRecovery_triggered();

    void showMsg(QString oStrMsg);

    /* 保存到Rx类中 */
    void on_actionSave_triggered();

    void on_actionStore_triggered();


    void on_actionCalRho_triggered();

    void showTableTX(QSqlTableModel*poModel);
    void showTableRX(QSqlTableModel*poModel);
    void showTableXY(QSqlTableModel*poModel);
    void showTableRho(QSqlTableModel*poModel);

    void drawRho(STATION oStation, QVector<double> adF, QVector<double>adRho);


    /* Insert Vertical Marker line */
    void on_actionCutterV_triggered();

    /* Insert Horizontal Marker line */
    void on_actionCutterH_triggered();

public slots:
    /* Draw Curve */
    void drawCurve();

    /* Scatter changed, then, curve's point need be change. */
    void markerMoved();

    /* Restore Curve */
    void restoreCurve();

    /* Scatter plot Show || Hide */
    void shiftScatter();

    /* QwtPlotCurve Legend Show || Hide */
    void shiftLegend();

    void shiftAllCurve(int iLogicalIndex);

    void shiftCurveSelect(QTreeWidgetItem* poItem ,int iCol);

    void Selected(QwtPlotCurve*poCurve, int iIndex);

    /* 手动拖动广域视电阻率，调整曲线形态功能。 */
    void SelectedRho(QwtPlotCurve *poCurve, int iIndex);

    /* When Select point changed, then show scatter plot! */
    void drawScatter();

    void drawError();

    void switchHighlightCurve();

signals:
    void SigSaveScatter(RX*, qreal , QVector<qreal>);
};

#endif // MAINWINDOW_H
