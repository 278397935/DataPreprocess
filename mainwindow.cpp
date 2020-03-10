#include "Mainwindow.h"
#include "ui_mainwindow.h"

#include <QPainter>
#include <QProxyStyle>

class CustomTabStyle : public QProxyStyle
{
public:
    QSize sizeFromContents(ContentsType type, const QStyleOption *option,
                           const QSize &size, const QWidget *widget) const
    {
        QSize s = QProxyStyle::sizeFromContents(type, option, size, widget);
        if (type == QStyle::CT_TabBarTab)
        {
            s.transpose();
            s.rwidth()  = 150; // 设置每个tabBar中item的大小
            s.rheight() = 50;
        }
        return s;
    }

    void drawControl(ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
    {
        if (element == CE_TabBarTabLabel)
        {
            if (const QStyleOptionTab *tab = qstyleoption_cast<const QStyleOptionTab *>(option))
            {
                QRect allRect = tab->rect;

                if (tab->state & QStyle::State_Selected)
                {
                    painter->save();
                    //painter->setPen(Qt::darkBlue);
                    painter->setBrush(QBrush(Qt::blue));
                    painter->drawRect(allRect.adjusted(6, 6, -6, -6));
                    painter->restore();
                }

                QTextOption option;
                option.setAlignment(Qt::AlignVCenter | Qt::AlignHCenter);

                if (tab->state & QStyle::State_Selected)
                {
                    painter->setPen(0xf8fcff);
                }
                else
                {
                    painter->setPen(0x5d5d5d);
                }

                painter->drawText(allRect, tab->text, option);
                return;
            }
        }

        if (element == CE_TabBarTab)
        {
            QProxyStyle::drawControl(element, option, painter, widget);
        }
    }
};
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    this->setWindowTitle("Tang_广域数据预处理工具_" + QDateTime::currentDateTime().toString("yyyy年MM月dd日AP"));

    qDebugV0()<<QThread::currentThreadId();

    qRegisterMetaType<FILE_TYPE>("FILE_TYPE");
    qRegisterMetaType<STATION_INFO>("STATION_INFO");
    qRegisterMetaType< QVector<qreal> >("QVector<qreal>");

    /* Draw marker line */
    ui->actionCutterH->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_H));
    connect(ui->actionCutterH, SIGNAL(triggered()), this, SLOT(drawMarkerH()));

    ui->actionCutterV->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_V));
    connect(ui->actionCutterV, SIGNAL(triggered()), this, SLOT(drawMarkerV()));

    ui->actionSave->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_S));
    ui->actionRecovery->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_R));

    gmapCurveData.clear();

    gmapCurveItem.clear();

    /* Init Global Var */
    gpoScatter = NULL;

    gpoErrorCurve = NULL;

    gpoSelectedCurve = NULL;
    giSelectedIndex = -1;

    gpoSelectedRX = NULL;

    poMarkerPicker = NULL;

    poCurvePicker = NULL;

    poDb = NULL;

    /* Init poCurvePicker */
    poCurvePicker = new CanvasPicker( ui->plotCurve );
    connect(poCurvePicker, SIGNAL(SigSelected(QwtPlotCurve*,int)), this, SLOT(Selected(QwtPlotCurve*,int)));

    ui->plotScatter->hide();

    ui->buttonScatterShift->setFixedHeight(12);
    ui->buttonScatterShift->setIconSize(QSize(10, 10));
    ui->buttonScatterShift->setStyleSheet( "QPushButton{background: rgb(255, 255, 255);"
                                           "border-radius:5px;"
                                           "border-style: solid;"
                                           "border-width: 1px 1px 1px 1px; "
                                           "spacing: 0px;"
                                           "padding: 6px 0px ;}"
                                           "QPushButton::hover{ background: rgb(180, 180, 180);"
                                           "border-radius:5px;"
                                           "border-style: solid;}" );
    ui->buttonScatterShift->setFlat(true);

    this->initPlotCurve();

    this->initPlotScatter();

    connect(ui->buttonScatterShift,SIGNAL(clicked()),this, SLOT(shiftScatter()));


    /* QwtLegend */
    ui->pushButtonLegend->setFixedWidth(12);
    ui->pushButtonLegend->setIconSize(QSize(10, 10));
    ui->pushButtonLegend->setStyleSheet( "QPushButton{background: rgb(255, 255, 255);"
                                         "border-radius:5px;"
                                         "border-style: solid;"
                                         "border-width: 1px 1px 1px 1px; "
                                         "spacing: 0px;"
                                         "padding: 6px 0px ;}"
                                         "QPushButton::hover{ background: rgb(180, 180, 180);"
                                         "border-radius:5px;"
                                         "border-style: solid;}" );
    ui->pushButtonLegend->setFlat(true);

    connect(ui->pushButtonLegend,SIGNAL(clicked()),this, SLOT(shiftLegend()));

    /* Init Marker line */
    sMkList.poTop    = NULL;
    sMkList.poBottom = NULL;
    sMkList.poLeft   = NULL;
    sMkList.poRight  = NULL;

    ui->actionImportRX->setEnabled(false);

    ui->actionExportRho->setEnabled(false);


    poDb = new MyDatabase();
    poDb->connect();

    connect(poDb, SIGNAL(SigModelTX(QSqlTableModel*)), this, SLOT(showTableTX(QSqlTableModel*)));
    connect(poDb, SIGNAL(SigModelRX(QSqlTableModel*)), this, SLOT(showTableRX(QSqlTableModel*)));
    connect(poDb, SIGNAL(SigModelXY(QSqlTableModel*)), this, SLOT(showTableXY(QSqlTableModel*)));

    connect(poDb, SIGNAL(SigModelRho(QSqlTableModel*)), this, SLOT(showTableRho(QSqlTableModel*)));

    ui->tabWidget->setTabText(0, "电流");
    ui->tabWidget->setTabText(1, "场值");
    ui->tabWidget->setTabText(2, "坐标");
    ui->tabWidget->setTabText(3, "广域视电阻率表格");
    ui->tabWidget->setTabText(4, "广域视电阻率曲线");

    /* 根据内容，决定列宽 */
    ui->tableViewTX->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->tableViewRX->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->tableViewXY->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->tableViewRho->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    /* 设置选中时为整行选中 */
    ui->tableViewTX->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableViewRX->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableViewXY->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableViewRho->setSelectionBehavior(QAbstractItemView::SelectRows);

    /* 设置表格的单元为只读属性，即不能编辑 */
    ui->tableViewTX->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableViewRX->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableViewXY->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableViewRho->setEditTriggers(QAbstractItemView::NoEditTriggers);

    ui->tabWidget->setTabPosition(QTabWidget::East);
    ui->tabWidget->tabBar()->setStyle(new CustomTabStyle);

    connect(poDb, SIGNAL(SigMsg(QString)), this, SLOT(showMsg(QString)));

    poCalRho = new CalRhoThread(poDb);
    connect(poCalRho, SIGNAL(SigMsg(QString)), this, SLOT(showMsg(QString)));

    this->initPlotTx();
    this->initPlotRx();
    this->initPlotRho();

    connect(poCalRho, SIGNAL(SigRho(STATION, QVector<double>, QVector<double>)), this, SLOT(drawRho(STATION, QVector<double>, QVector<double>)));

    aoStrExisting.clear();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionImportTX_triggered()
{
    QString oStrFileName = QFileDialog::getOpenFileName(this,
                                                        "打开电流文件",
                                                        QString("%1").arg(this->LastDirRead()),
                                                        "电流文件(FFT_AVG_I_T*.csv)");

    if(oStrFileName.length() != 0)
    {
        poDb->importTX(oStrFileName);

        this->LastDirWrite( oStrFileName );

        ui->actionImportRX->setEnabled(true);
    }

    ui->plotTx->setFooter("电流文件："+oStrFileName);
}

/* 打开接收端场值文件 */
void MainWindow::on_actionImportRX_triggered()
{
    QStringList aoStrRxThisTime = QFileDialog::getOpenFileNames(this,
                                                                "打开电场文件",
                                                                QString("%1").arg(this->LastDirRead()),
                                                                "电场文件(FFT_SEC_V_T*.csv)");

    if(aoStrRxThisTime.isEmpty())
    {
        QMessageBox::warning(this, "警告","未选中有效文件，\n请重新选择！");
        return;
    }

    foreach(QString oStrRxThisTime, aoStrRxThisTime)
    {
        if( aoStrExisting.contains(oStrRxThisTime) )
        {
            //qDebugV0()<<"existing~~~"<<oStrRxThisTime;
        }
        else
        {
            //            qDebugV0()<<"Not existing~~~"<<oStrRxThisTime;

            RX *poRX = new RX(oStrRxThisTime);
            aoStrExisting.append(oStrRxThisTime);
            gapoRX.append(poRX);
        }
    }

    this->LastDirWrite( aoStrRxThisTime.last() );

    //qDebugV0()<<aoStrExisting;

    this->drawCurve();

    QFileInfo oFileInfo(aoStrRxThisTime.last());

    ui->plotRx->setTitle( "场值文件目录：" + oFileInfo.absolutePath() );
}

/*  Close Application */
void MainWindow::on_actionClose_triggered()
{
    this->close();
}

/*  */
void MainWindow::on_actionClear_triggered()
{
    QMessageBox oMsgBox(QMessageBox::Question, "清空数据", "确定清空数据?",
                        QMessageBox::Yes | QMessageBox::No, NULL);

    if(oMsgBox.exec() == QMessageBox::Yes)
    {
        gpoSelectedCurve = NULL;
        giSelectedIndex = -1;

        gpoSelectedRX = NULL;

        gpoErrorCurve = NULL;

        ui->stackedWidget->setCurrentIndex(0);

        QVector<double> xData;xData.clear();
        QVector<double> yData;yData.clear();

        gpoScatter->setSamples(xData, yData);

        /* 散点图，褫干净 */
        ui->plotScatter->replot();

        /* 电压除以电流的图，褫干净 */
        foreach (QwtPlotCurve *poCurve, gmapCurveData.keys())
        {
            if(poCurve != NULL)
            {
                delete poCurve;
                poCurve == NULL;
            }
        }
        gmapCurveData.clear();

        foreach(RX *oRx, gapoRX)
        {
            if(oRx != NULL)
            {
                delete oRx;
                oRx = NULL;
            }
        }

        gapoRX.clear();

        ui->plotCurve->detachItems();

        ui->plotCurve->repaint();


        /* 右手边，tableWidget 褫干净 */
        foreach (QTreeWidgetItem *poItem, gmapCurveItem.values())
        {
            if(poItem != NULL)
            {
                disconnect(ui->treeWidgetLegend, SIGNAL(itemChanged(QTreeWidgetItem*,int)), this, SLOT(shiftCurveSelect(QTreeWidgetItem*,int)));
                delete poItem;
                poItem == NULL;
            }
        }
        gmapCurveItem.clear();

        ui->treeWidgetLegend->clear();

        ui->plotCurve->setTitle("");

        if( !ui->plotScatter->isHidden())
        {
            ui->buttonScatterShift->setIcon(QIcon(":/GDC2/Icon/ScatterShow.png"));
            ui->plotScatter->hide();
        }

        aoStrExisting.clear();
    }
}

/* 显示提示信息，QMessage自动定时关闭。 */
void MainWindow::showMsg(QString oStrMsg)
{
    QMessageBox *poMsgBox = new QMessageBox(QMessageBox::Information,tr("友情提示"),oStrMsg);
    QTimer::singleShot(5000, poMsgBox, SLOT(accept())); //也可将accept改为close
    poMsgBox->exec();//box->show();都可以
}

void MainWindow::recoveryCurve(QwtPlotCurve *poCurve)
{
    QVector<double> adR = this->getR(gpoSelectedRX->adF, gpoSelectedRX->adE);

    poCurve->setSamples(gpoSelectedRX->adF, adR);

    ui->plotCurve->replot();
}

/**********************************************************************
 * Draw Average curve
 *
 */
void MainWindow::drawCurve()
{
    foreach (QwtPlotCurve *poCurve, gmapCurveData.keys())
    {
        delete poCurve;
        poCurve == NULL;
    }

    gmapCurveData.clear();

    foreach (QTreeWidgetItem *poItem, gmapCurveItem.values())
    {
        if(poItem != NULL)
        {
            delete poItem;
            poItem == NULL;
        }
    }
    gmapCurveItem.clear();

    ui->plotCurve->detachItems();

    ui->plotCurve->repaint();

    disconnect(ui->treeWidgetLegend, SIGNAL(itemChanged(QTreeWidgetItem*,int)), this, SLOT(shiftCurveSelect(QTreeWidgetItem*,int)));
    ui->treeWidgetLegend->clear();

    QSet<double> setF;

    foreach(RX *poRX, gapoRX)
    {
        for(int i = 0; i < poRX->adF.count(); i++)
        {
            setF.insert(poRX->adF.at(i));
        }
    }

    QList<double> adF = setF.toList();

    qSort(adF);

    //qDebugV0()<<"F sequence:"<<adF;

    /* Fill ticks */
    QList<double> adTicks[QwtScaleDiv::NTickTypes];
    adTicks[QwtScaleDiv::MajorTick] = adF;
    QwtScaleDiv oScaleDiv( adTicks[QwtScaleDiv::MajorTick].last(),
            adTicks[QwtScaleDiv::MajorTick].first(),
            adTicks );

    ui->plotCurve->setAxisScaleDiv( QwtPlot::xBottom, oScaleDiv );

    foreach (RX* poRX, gapoRX)
    {
        /* Curve title, cut MCSD_ & suffix*/
        const QwtText oTxtTitle( QString(tr("线号:%1__点号:%2__仪器号:%3__通道号:%4(%5)"))
                                 .arg(poRX->goStrLineId)
                                 .arg(poRX->goStrSiteId)
                                 .arg(poRX->giDevId)
                                 .arg(poRX->giDevCh)
                                 .arg(poRX->goStrCompTag) );

        /* Create a curve pointer */
        QwtPlotCurve *poCurve = NULL;
        poCurve = new QwtPlotCurve( oTxtTitle );

        poCurve->setPen( Qt::black, 2, Qt::SolidLine );
        poCurve->setRenderHint( QwtPlotItem::RenderAntialiased, true );

        QwtSymbol *poSymbol = new QwtSymbol( QwtSymbol::Ellipse,
                                             QBrush( Qt::yellow ),
                                             QPen( Qt::blue, 2 ),
                                             QSize( 8, 8 ) );
        poCurve->setSymbol( poSymbol );
        poCurve->setStyle(QwtPlotCurve::Lines);

        QVector<double> adR = this->getR( poRX->adF, poRX->adE);

        poCurve->setSamples( poRX->adF, adR );
        poCurve->setAxes(QwtPlot::xBottom, QwtPlot::yLeft);
        poCurve->attach(ui->plotCurve);
        poCurve->setVisible( true );

        gmapCurveData.insert(poCurve, poRX);

        QTreeWidgetItem *poItem = NULL;
        poItem = new QTreeWidgetItem(ui->treeWidgetLegend, QStringList({poCurve->title().text(),"",""}));

        poItem->setData(1, Qt::CheckStateRole, Qt::Checked);
        poItem->setData(2, Qt::CheckStateRole, Qt::Unchecked);

        gmapCurveItem.insert(poCurve, poItem);
    }

    connect(ui->treeWidgetLegend, SIGNAL(itemChanged(QTreeWidgetItem*,int)), this, SLOT(shiftCurveSelect(QTreeWidgetItem*,int)));


    gpoSelectedCurve = NULL;
    giSelectedIndex = -1;

    gpoSelectedRX = NULL;

    gpoErrorCurve = NULL;

    ui->plotCurve->replot();
}

/**************************************************************
 * Real time set scatter canvas scales & Set aside blank.
 *
 */
void MainWindow::resizeScaleScatter()
{
    /* Real time set  plot Scale and set aside blank. */
    double dXSacle = gpoScatter->maxXValue() - gpoScatter->minXValue();
    double dYSacle = gpoScatter->maxYValue() - gpoScatter->minYValue();

    ui->plotScatter->setAxisScale( QwtPlot::xBottom, gpoScatter->minXValue() - dXSacle*0.03, gpoScatter->maxXValue() + dXSacle*0.03 );
    ui->plotScatter->setAxisScale( QwtPlot::yLeft,   gpoScatter->minYValue() - dYSacle*0.1,  gpoScatter->maxYValue() + dYSacle*0.1 );

    if( sMkList.poTop != NULL && sMkList.poBottom != NULL )
    {
        drawMarkerH();
    }
    else if( sMkList.poLeft != NULL && sMkList.poRight != NULL )
    {
        drawMarkerV();
    }
}

/* 电压除以电流，得到电阻值。实际上就是用电流来归一化电压 */
QVector<double> MainWindow::getR(QVector<double> adF, QVector<double> adE)
{
    QVector<double> adR;
    adR.clear();

    for(int i = 0; i < adF.count(); i++)
    {
        adR.append( adE.at(i)/poDb->getI(adF.at(i)) );
    }

    return adR;
}

/******************************************************************************
 * Get all the points in the scatter diagram
 *
 */
QPolygonF MainWindow::currentScatterPoints()
{
    QPolygonF aoPointF;
    aoPointF.clear();

    qint32 iIndex = 0;

    /* Horizontal Cut */
    if( sMkList.poBottom != NULL && sMkList.poTop != NULL )
    {
        for( qint32 i = 0; i < (qint32)gpoScatter->dataSize(); i++ )
        {
            if( gpoScatter->data()->sample(i).y() >= sMkList.poBottom->yValue() &&  gpoScatter->data()->sample(i).y() <= sMkList.poTop->yValue() )
            {
                QPointF ptTmpPoint( iIndex, gpoScatter->data()->sample(i).y() );
                // QPointF ptTmpPoint( gpoScatter->data()->sample(i) );

                aoPointF.append( ptTmpPoint );

                iIndex++;
            }
        }
    }
    /* Vertical Cut */
    else if( sMkList.poLeft != NULL && sMkList.poRight != NULL )
    {
        for( qint32 i = 0; i < (qint32)gpoScatter->dataSize(); i++ )
        {
            if( gpoScatter->data()->sample(i).x() >= sMkList.poLeft->xValue() &&  gpoScatter->data()->sample(i).x() <= sMkList.poRight->xValue() )
            {
                QPointF ptTmpPoint( iIndex, gpoScatter->data()->sample(i).y() );
                //QPointF ptTmpPoint(  gpoScatter->data()->sample(i) );

                aoPointF.append( ptTmpPoint );

                iIndex++;
            }
        }
    }

    return aoPointF;
}

QPolygonF MainWindow::currentCurvePoints()
{
    QPolygonF aoPoint;
    aoPoint.clear();

    if(gpoSelectedCurve != NULL)
    {
        for(uint i = 0; i < gpoSelectedCurve->dataSize(); i++)
        {
            aoPoint.append(gpoSelectedCurve->sample(i));
        }
    }

    return aoPoint;
}

/**************************************************
 * Init Curve plot
 *
 */
void MainWindow::initPlotCurve()
{
    QFont oFont("Times New Roman", 12, QFont::Thin);

    ui->plotCurve->setFont(oFont);

    /* Nice blue */
    //ui->plotCurve->setCanvasBackground(QColor(55, 100, 141));

    /* Set Log Scale */
    ui->plotCurve->setAxisScaleEngine( QwtPlot::xBottom, new QwtLogScaleEngine() );
    ui->plotCurve->setAxisScaleEngine( QwtPlot::yLeft,   new QwtLogScaleEngine() );
    ui->plotCurve->setAxisScaleEngine( QwtPlot::yRight,  new QwtLinearScaleEngine() );

    ui->plotCurve->enableAxis(QwtPlot::xBottom , true);
    ui->plotCurve->enableAxis(QwtPlot::yLeft   , true);
    ui->plotCurve->enableAxis(QwtPlot::yRight  , true);

    QwtScaleDraw *poScaleDraw  = new QwtScaleDraw();
    poScaleDraw->setLabelRotation( -26 );
    poScaleDraw->setLabelAlignment( Qt::AlignLeft | Qt::AlignVCenter );

    ui->plotCurve->setAxisScaleDraw(QwtPlot::xBottom, poScaleDraw);

    /* Set Axis title */
    //QwtText oTxtXAxisTitle( "频率(Hz)" );
//    QwtText oTxtYAxisTitle( "F/I" );
    QwtText oTxtErrAxisTitle(tr("RMS(%)"));
    //oTxtXAxisTitle.setFont( oFont );
    //oTxtYAxisTitle.setFont( oFont );
    oTxtErrAxisTitle.setFont( oFont );
    //ui->plotCurve->setAxisTitle(QwtPlot::xBottom, oTxtXAxisTitle);
//    ui->plotCurve->setAxisTitle(QwtPlot::yLeft,   oTxtYAxisTitle);
    ui->plotCurve->setAxisTitle(QwtPlot::yRight,  oTxtErrAxisTitle);

    /* Draw the canvas grid */
    QwtPlotGrid *poGrid = new QwtPlotGrid();
    //poGrid->enableX( true );
    poGrid->enableY( true );
    poGrid->setMajorPen( Qt::gray, 0.5, Qt::DotLine );
    poGrid->attach( ui->plotCurve );

    ui->plotCurve->setAutoDelete ( true );

    /* Remove the gap between the data axes */
    for ( int i = 0; i < ui->plotCurve->axisCnt; i++ )
    {
        QwtScaleWidget *poScaleWidget = ui->plotCurve->axisWidget( i);
        if (poScaleWidget)
        {
            poScaleWidget->setMargin( 0 );
        }

        QwtScaleDraw *poScaleDraw = ui->plotCurve->axisScaleDraw( i );
        if ( poScaleDraw )
        {
            poScaleDraw->enableComponent( QwtAbstractScaleDraw::Backbone, false );
        }
    }

    ui->plotCurve->plotLayout()->setAlignCanvasToScales( true );

    ui->plotCurve->setAutoReplot(true);

    ui->treeWidgetLegend->clear();
    ui->treeWidgetLegend->setHeaderLabels(QStringList()<<tr("曲线")<<tr("显示")<<tr("凸显"));

    ui->treeWidgetLegend->setColumnWidth(0, 120);
    ui->treeWidgetLegend->setColumnWidth(1, 40);
    ui->treeWidgetLegend->setColumnWidth(2, 40);
    //ui->treeWidgetLegend->header()->setSectionResizeMode(QHeaderView::ResizeToContents);

    connect(ui->treeWidgetLegend->header(), SIGNAL(sectionDoubleClicked(int)), this, SLOT(shiftAllCurve(int)));

    /* 放大镜 */
    QwtPlotMagnifier *poMagnifier = new QwtPlotMagnifier(ui->plotCurve->canvas());
    poMagnifier->setAxisEnabled(QwtPlot::xBottom, false);
    poMagnifier->setAxisEnabled(QwtPlot::yLeft, true);
    poMagnifier->setAxisEnabled(QwtPlot::yRight, true);

    /* 平移 */
    QwtPlotPanner *poPanner = new QwtPlotPanner(ui->plotCurve->canvas());
    poPanner->setMouseButton(Qt::LeftButton);

    ui->plotCurve->replot();
}

/********************************************************************************
 * Init scatter plot
 *
 */
void MainWindow::initPlotScatter()
{
    ui->plotScatter->setFrameStyle(QFrame::NoFrame);
    ui->plotScatter->enableAxis(QwtPlot::xBottom, true);
    ui->plotScatter->enableAxis(QwtPlot::yLeft,   true);
    ui->plotScatter->setAutoDelete ( true );
    ////
    /* Clean up the Copy items, Before creating new scatter, Even without. */
    if( gpoScatter != NULL )
    {
        delete gpoScatter;
        gpoScatter = NULL;
    }
    /* New a scatter */
    gpoScatter = new QwtPlotCurve;

    if( gpoScatter == NULL )
    {
        return;
    }

    /* New marker picker pointer */
    if(poMarkerPicker != NULL)
    {
        delete poMarkerPicker;
        poMarkerPicker = NULL;
    }

    poMarkerPicker = new MarkerPicker( ui->plotScatter, gpoScatter );
    connect( poMarkerPicker, SIGNAL(SigMarkerMoved()), this , SLOT(markerMoved()));

    gpoScatter->setStyle(QwtPlotCurve::NoCurve);



    QwtSymbol *poSymbol = new QwtSymbol( QwtSymbol::Ellipse,
                                         QBrush( Qt::blue ),
                                         QPen( Qt::blue, 1.0 ),
                                         QSize( 6, 6 ) );
    gpoScatter->setSymbol(poSymbol);

    gpoScatter->attach(ui->plotScatter);

    ui->plotScatter->replot();
}

/**********************************************************************************
 * Show or Hide scatter graph
 *
 */
void MainWindow::shiftScatter()
{
    bool bFlag;
    bFlag = ui->plotScatter->isHidden();
    if(bFlag == true)
    {
        ui->buttonScatterShift->setIcon(QIcon(":/GDC2/Icon/ScatterHide.png"));
        ui->plotScatter->show();
    }
    else
    {
        ui->buttonScatterShift->setIcon(QIcon(":/GDC2/Icon/ScatterShow.png"));
        ui->plotScatter->hide();
    }
}

void MainWindow::shiftLegend()
{
    bool bFlag;
    bFlag = ui->treeWidgetLegend->isHidden();
    if(bFlag == true)
    {
        ui->pushButtonLegend->setIcon(QIcon(":/GDC2/Icon/NavOpen.png"));
        ui->treeWidgetLegend->show();
    }
    else
    {
        ui->pushButtonLegend->setIcon(QIcon(":/GDC2/Icon/NavClose.png"));
        ui->treeWidgetLegend->hide();
    }
}

void MainWindow::shiftAllCurve(int iLogicalIndex)
{
    qDebugV0()<<iLogicalIndex;

    if( iLogicalIndex == 0 )
    {
        return;
    }

    QTreeWidgetItemIterator it(ui->treeWidgetLegend);

    int iSum = 0;

    while (*it)
    {
        iSum += (*it)->data(iLogicalIndex, Qt::CheckStateRole).toUInt();

        ++it;
    }

    Qt::CheckState eCheckState = Qt::Unchecked;

    if( iSum == 0 )
    {
        eCheckState = Qt::Checked;
    }
    else if( iSum > 0 )
    {
        eCheckState = Qt::Unchecked;
    }

    QTreeWidgetItemIterator itWrite(ui->treeWidgetLegend);

    while (*itWrite)
    {
        //qDebugV0()<<"Name:"<<(*itWrite)->text(0);
        (*itWrite)->setData(iLogicalIndex, Qt::CheckStateRole, eCheckState );

        ++itWrite;
    }
}

void MainWindow::shiftCurveSelect(QTreeWidgetItem *poItem, int iCol)
{
    if(poItem == NULL || iCol == 0)
    {
        return;
    }
    else
    {
        QwtPlotCurve *poCurve = gmapCurveItem.key(poItem);

        qDebugV0()<<poCurve->title().text();

        switch (iCol)
        {
        case 1://display
            if(poItem->data(iCol, Qt::CheckStateRole).toUInt() == Qt::Checked)
            {
                poCurve->setVisible(true);
            }
            else if(poItem->data(iCol, Qt::CheckStateRole).toUInt() == Qt::Unchecked)
            {
                poCurve->setVisible(false);
            }

            break;
        case 2://highlights
            if(poItem->data(iCol, Qt::CheckStateRole).toUInt() == Qt::Checked)
            {
                poCurve->setPen( Qt::red, 4, Qt::SolidLine );
            }
            else if(poItem->data(iCol, Qt::CheckStateRole).toUInt() == Qt::Unchecked)
            {
                poCurve->setPen( Qt::black, 2, Qt::SolidLine );
            }
            break;
        default:
            break;
        }

        ui->plotCurve->replot();
    }
}

/* slot 函数。 接收picker对象的signal  SigSelected(poCurve, index) */
void MainWindow::Selected(QwtPlotCurve *poCurve, int iIndex)
{
    if( poCurve->style() == QwtPlotCurve::Lines)
    {
        gpoSelectedCurve = poCurve;
        giSelectedIndex = iIndex;

        gpoSelectedRX = this->gmapCurveData.value(gpoSelectedCurve);

        this->restoreCurve();

        this->drawScatter();
        this->switchHighlightCurve();
        this->drawError();

        QString oStrFooter(QString("%1__频率:%2Hz")
                           .arg(poCurve->title().text())
                           .arg(poCurve->data()->sample(iIndex).x()));
        QwtText oTxt;
        oTxt.setText(oStrFooter);
        QFont oFont("Times New Roman", 12, QFont::Bold);
        oTxt.setFont(oFont);
        oTxt.setColor(Qt::blue);

        ui->plotCurve->setTitle(oTxt);
    }
    else if(poCurve->style() == QwtPlotCurve::Sticks)
    {
        qDebugV0()<<"相对均方误差   quxian玩不得~~~";
        ui->plotCurve->setTitle("");
    }
    else
    {
        qDebugV0()<<"buxiade s shme 玩不得~~~";
        ui->plotCurve->setTitle("");
    }
}

/* Scatter changed, then, curve's point need be change. */
void MainWindow::markerMoved()
{
    /* Scatter Cannot be null and void, Otherwise, return! */
    if( gpoScatter == NULL )
    {
        return;
    }

    /* All points on Scatter */
    QPolygonF apoPointScatter = currentScatterPoints();

    if( apoPointScatter.count() == 0)
    {
        return;
    }

    //gpoSelectedRX = this->gmapCurveData.value(gpoSelectedCurve);

    QVector<qreal> arE;
    arE.clear();

    for(qint32 i = 0; i < apoPointScatter.count(); i++)
    {
        arE.append(apoPointScatter.at(i).y());
    }

    double dE = gpoSelectedRX->getE(arE);

    double dI = poDb->getI(gpoSelectedCurve->data()->sample(giSelectedIndex).x());

    /* All points on selected curve */
    QPolygonF aoPointCurve = this->currentCurvePoints();

    /* Selected point's new value(just Y) */
    QPointF ptPoint( gpoSelectedCurve->data()->sample(giSelectedIndex).x(), dE/dI );

    /* Replace current selected point Y value. */
    aoPointCurve.replace( giSelectedIndex, ptPoint );

    /* Set new samples on Curve */
    gpoSelectedCurve->setSamples( aoPointCurve );

    /* All points on selected curve */
    QPolygonF aoPointFError;

    for(uint i = 0; i < gpoErrorCurve->dataSize(); i++)
    {
        aoPointFError.append(gpoErrorCurve->sample(i));
    }

    /* Selected point's new value(just Y) */
    QPointF oPointF( gpoErrorCurve->data()->sample(giSelectedIndex).x(), gpoSelectedRX->getErr(arE) );

    /* Replace current selected point Y value. */
    aoPointFError.replace( giSelectedIndex, oPointF );

    /* Set new samples on Curve */
    gpoErrorCurve->setSamples( aoPointFError );

    /* Update MSRE */
    QString oStrFooter(QString("相对均方误差: %1%").arg(gpoSelectedRX->getErr(arE)));

    QwtText oTxt;
    QFont oFont("Times New Roman", 12, QFont::Thin);
    oTxt.setFont(oFont);
    oTxt.setColor(Qt::red);

    oTxt.setText(oStrFooter);

    ui->plotScatter->setFooter(oTxt);

    ui->plotCurve->replot();
}

/************************************************************************
 * Draw scatter points and Marker line
 *
 */
void MainWindow::drawScatter()
{
    /* Detach & Delete marker line pointer, plotCurve's curves,set NULL textLabel. */
    if( gpoSelectedCurve == NULL || giSelectedIndex == -1  )
    {
        clearMarker();
        return;
    }

    /* Selected some curve, restore that curve. */
    const QwtPlotItemList& itmList = ui->plotScatter->itemList();

    for( QwtPlotItemIterator it = itmList.begin(); it != itmList.end(); ++it )
    {
        if ( ( *it )->rtti() == QwtPlotItem::Rtti_PlotTextLabel )
        {
            QwtPlotTextLabel *poTextLabel = static_cast<QwtPlotTextLabel *>( *it );

            poTextLabel->detach();
            if(poTextLabel != NULL)
            {
                delete poTextLabel;
                poTextLabel = NULL;
            }
        }
    }

    if( !ui->plotScatter->autoDelete() )
    {
        qDebugV5()<<"Auto delete all items not work!";
    }

    gpoScatter->setTitle(QString("%1Hz(%2)")
                         .arg(gpoSelectedCurve->sample(giSelectedIndex).x())
                         .arg(gpoSelectedCurve->title().text()));


    RX *poRxSelected = gmapCurveData.value(gpoSelectedCurve);

    QVector<double> adScatter = poRxSelected->aadScatter.at(giSelectedIndex);
    QVector<double> adX;
    adX.clear();

    for(int i = 0; i < adScatter.count(); i++)
    {
        adX.append(i);
    }

    gpoScatter->setSamples( adX, adScatter );

    QwtSymbol *poSymbol = new QwtSymbol( QwtSymbol::Ellipse,
                                         QBrush( Qt::blue ),
                                         QPen( Qt::blue, 1.0 ),
                                         QSize( 6, 6 ) );
    gpoScatter->setSymbol(poSymbol);


    /* Real time set plot canvas Scale & Set aside blank. */
    this->resizeScaleScatter();

    /* Read Excel file(copy), display MSRE on PlotCurve canvas(Top||Right) */
    /* Display MSRE on footer */
    QString oStrFooter(QString("相对均方误差: %1%").arg( poRxSelected->adErr.at(giSelectedIndex) ));
    QwtText oTxt;
    QFont oFont("Times New Roman", 12, QFont::Thin);
    oTxt.setFont(oFont);
    oTxt.setColor(Qt::red);
    oTxt.setText(oStrFooter);
    ui->plotScatter->setFooter(oTxt);
    ui->plotScatter->replot();

    if( ui->plotScatter->isHidden())
    {
        ui->buttonScatterShift->setIcon(QIcon(":/GDC2/Icon/ScatterHide.png"));
        ui->plotScatter->show();
    }
}

void MainWindow::drawError()
{
    if( gpoErrorCurve != NULL)
    {
        gpoErrorCurve->detach();

        delete gpoErrorCurve;
         gpoErrorCurve = NULL;
    }

    /* Draw new error bar */
    gpoErrorCurve = new QwtPlotCurve(tr("相对均方误差"));
    if( gpoErrorCurve == NULL )
    {
        return;
    }

    qDebugV0()<<gpoErrorCurve->title().text();

    gpoErrorCurve->setLegendAttribute( QwtPlotCurve::LegendNoAttribute, true );

    QwtSymbol *PoSymbol = new QwtSymbol( QwtSymbol::Diamond,
                                         QBrush( Qt::red ),
                                         QPen( Qt::gray, 1 ),
                                         QSize( 4, 4 ) );

    gpoErrorCurve->setSymbol( PoSymbol );

    gpoErrorCurve->setAxes( QwtPlot::xBottom, QwtPlot::yRight );
    gpoErrorCurve->setPen( Qt::black, 0.5, Qt::DotLine );
    gpoErrorCurve->setStyle( QwtPlotCurve::Sticks );


    RX *poRX = gmapCurveData.value(gpoSelectedCurve);

    gpoErrorCurve->setSamples( poRX->adF, poRX->adErr );
    gpoErrorCurve->attach( ui->plotCurve );


    ui->plotCurve->replot();
}

void MainWindow::switchHighlightCurve()
{
    foreach(QwtPlotCurve * poC, gmapCurveData.keys())
    {
        poC->setPen( Qt::black, 2, Qt::SolidLine );
    }
    if( gpoSelectedCurve != NULL )
    {
        gpoSelectedCurve->setPen( Qt::red, 4, Qt::SolidLine );

        // poDb->refineError(gmapCurveData.value(poCurve));
    }
}
/* 绘制发射端电流曲线 */
void MainWindow::drawTx(const QVector<double> adF, const QVector<double> adI)
{
    ui->plotTx->detachItems();
    ui->plotTx->replot();

    QSet<double> setF;

    for(int i = 0; i < adF.count(); i++)
    {
        setF.insert(adF.at(i));
    }

    QList<double> listF = setF.toList();

    qSort(listF);

    qDebugV0()<<listF;

    /* Fill ticks */
    QList<double> adTicks[QwtScaleDiv::NTickTypes];
    adTicks[QwtScaleDiv::MajorTick] = listF;
    QwtScaleDiv oScaleDiv( adTicks[QwtScaleDiv::MajorTick].last(),
            adTicks[QwtScaleDiv::MajorTick].first(),
            adTicks );

    ui->plotTx->setAxisScaleDiv( QwtPlot::xBottom, oScaleDiv );
    /* Create a curve pointer */
    QwtPlotCurve *poCurve = new QwtPlotCurve();

    poCurve->setPen( Qt::red, 2, Qt::SolidLine );
    poCurve->setRenderHint( QwtPlotItem::RenderAntialiased, true );

    QwtSymbol *poSymbol = new QwtSymbol( QwtSymbol::Ellipse,
                                         QBrush( Qt::yellow ),
                                         QPen( Qt::blue, 2 ),
                                         QSize( 8, 8 ) );
    poCurve->setSymbol( poSymbol );
    poCurve->setStyle(QwtPlotCurve::Lines);

    poCurve->setSamples( adF, adI );
    poCurve->setAxes(QwtPlot::xBottom, QwtPlot::yLeft);
    poCurve->attach(ui->plotTx);
    poCurve->setVisible( true );

    ui->plotTx->replot();
}

void MainWindow::initPlotRx()
{
    //    ui->plotRx->setCanvasBackground(QColor(29, 100, 141)); // nice blue

    //ui->plotCurve->setTitle(QwtText("Curve Plot"));
    QFont oFont("Times New Roman", 12, QFont::Thin);

    QwtText oTxtTitle( "接收端_场值曲线" );
    oTxtTitle.setFont( oFont );

    ui->plotRx->setTitle(oTxtTitle);

    ui->plotRx->setFont(oFont);

    /* Nice blue */
    //ui->plotCurve->setCanvasBackground(QColor(55, 100, 141));

    /* Set Log Scale */
    ui->plotRx->setAxisScaleEngine( QwtPlot::xBottom, new QwtLogScaleEngine() );
    ui->plotRx->setAxisScaleEngine( QwtPlot::yLeft,   new QwtLogScaleEngine() );

    ui->plotRx->enableAxis(QwtPlot::xBottom , true);
    ui->plotRx->enableAxis(QwtPlot::yLeft   , true);

    QwtScaleDraw *poScaleDraw  = new QwtScaleDraw();
    poScaleDraw->setLabelRotation( -26 );
    poScaleDraw->setLabelAlignment( Qt::AlignLeft | Qt::AlignVCenter );

    ui->plotRx->setAxisScaleDraw(QwtPlot::xBottom, poScaleDraw);

    /* Set Axis title */
    QwtText oTxtXAxisTitle( "F/Hz" );
    QwtText oTxtYAxisTitle( "Field" );
    oTxtXAxisTitle.setFont( oFont );
    oTxtYAxisTitle.setFont( oFont );

    ui->plotRx->setAxisTitle(QwtPlot::xBottom,  oTxtXAxisTitle);
    ui->plotRx->setAxisTitle(QwtPlot::yLeft,    oTxtYAxisTitle);

    /* Draw the canvas grid */
    QwtPlotGrid *poGrid = new QwtPlotGrid();
    poGrid->enableX( false );
    poGrid->enableY( true );
    poGrid->setMajorPen( Qt::gray, 0.5, Qt::DotLine );
    poGrid->attach( ui->plotRx );

    ui->plotRx->setAutoDelete ( true );

    /* Remove the gap between the data axes */
    for ( int i = 0; i < ui->plotRx->axisCnt; i++ )
    {
        QwtScaleWidget *poScaleWidget = ui->plotRx->axisWidget( i);
        if (poScaleWidget)
        {
            poScaleWidget->setMargin( 0 );
        }

        QwtScaleDraw *poScaleDraw = ui->plotRx->axisScaleDraw( i );
        if ( poScaleDraw )
        {
            poScaleDraw->enableComponent( QwtAbstractScaleDraw::Backbone, false );
        }
    }

    ui->plotRx->plotLayout()->setAlignCanvasToScales( true );

    ui->plotRx->setAutoReplot(true);
}

void MainWindow::drawRx()
{
    ui->plotRx->detachItems();
    ui->plotRx->replot();

    QSet<double> setF;

    foreach(RX *poRX, gapoRX)
    {
        for(int i = 0; i < poRX->adF.count(); i++)
        {
            setF.insert(poRX->adF.at(i));
        }
    }

    QList<double> adF = setF.toList();

    qSort(adF);

    qDebugV0()<<adF;

    /* Fill ticks */
    QList<double> adTicks[QwtScaleDiv::NTickTypes];
    adTicks[QwtScaleDiv::MajorTick] = adF;
    QwtScaleDiv oScaleDiv( adTicks[QwtScaleDiv::MajorTick].last(),
            adTicks[QwtScaleDiv::MajorTick].first(),
            adTicks );

    ui->plotRx->setAxisScaleDiv( QwtPlot::xBottom, oScaleDiv );

    foreach (RX* poRX, gapoRX)
    {
        /* Curve title, cut MCSD_ & suffix*/
        /* Curve title, cut MCSD_ & suffix*/
        const QwtText oTxtTitle( QString(tr("线号:%1_点号:%2_仪器号:%3_通道号:%4(%5)"))
                                 .arg(poRX->goStrLineId)
                                 .arg(poRX->goStrSiteId)
                                 .arg(poRX->giDevId)
                                 .arg(poRX->giDevCh)
                                 .arg(poRX->goStrCompTag) );

        /* Create a curve pointer */
        QwtPlotCurve *poCurve = new QwtPlotCurve( oTxtTitle );

        poCurve->setPen( Qt::red, 2, Qt::SolidLine );
        poCurve->setRenderHint( QwtPlotItem::RenderAntialiased, true );

        QwtSymbol *poSymbol = new QwtSymbol( QwtSymbol::Ellipse,
                                             QBrush( Qt::yellow ),
                                             QPen( Qt::blue, 2 ),
                                             QSize( 8, 8 ) );
        poCurve->setSymbol( poSymbol );
        poCurve->setStyle(QwtPlotCurve::Lines);

        QVector<double> adR = this->getR( poRX->adF, poRX->adE);

        poCurve->setSamples( poRX->adF, adR );
        poCurve->setAxes(QwtPlot::xBottom, QwtPlot::yLeft);
        poCurve->attach(ui->plotRx);
        poCurve->setVisible( true );
    }

    QwtPlotMagnifier *poM = new QwtPlotMagnifier(ui->plotRx->canvas());
    poM->setAxisEnabled(QwtPlot::xBottom, false);
    poM->setAxisEnabled(QwtPlot::yLeft, true);
    poM->setAxisEnabled(QwtPlot::yRight, true);

    QwtPlotPanner *poP = new QwtPlotPanner(ui->plotRx->canvas());
    poP->setMouseButton(Qt::LeftButton);

    ui->plotRx->replot();
}

void MainWindow::initPlotRho()
{
    ui->plotRho->setCanvasBackground(QColor(29, 100, 141)); // nice blue

    //ui->plotCurve->setTitle(QwtText("Curve Plot"));
    QFont oFont("Times New Roman", 12, QFont::Thin);

    QwtText oTxtTitle( "广域视电阻率曲线" );
    oTxtTitle.setFont( oFont );

    ui->plotRho->setTitle(oTxtTitle);

    ui->plotRho->setFont(oFont);

    /* Nice blue */
    //ui->plotCurve->setCanvasBackground(QColor(55, 100, 141));

    /* Set Log Scale */
    ui->plotRho->setAxisScaleEngine( QwtPlot::xBottom, new QwtLogScaleEngine() );
    ui->plotRho->setAxisScaleEngine( QwtPlot::yLeft,   new QwtLogScaleEngine() );

    ui->plotRho->enableAxis(QwtPlot::xBottom , true);
    ui->plotRho->enableAxis(QwtPlot::yLeft   , true);

    QwtScaleDraw *poScaleDraw  = new QwtScaleDraw();
    poScaleDraw->setLabelRotation( -26 );
    poScaleDraw->setLabelAlignment( Qt::AlignLeft | Qt::AlignVCenter );

    ui->plotRho->setAxisScaleDraw(QwtPlot::xBottom, poScaleDraw);

    /* Set Axis title */
    QwtText oTxtXAxisTitle( "F/Hz" );
    QwtText oTxtYAxisTitle( "Rho" );
    oTxtXAxisTitle.setFont( oFont );
    oTxtYAxisTitle.setFont( oFont );

    ui->plotRho->setAxisTitle(QwtPlot::xBottom,  oTxtXAxisTitle);
    ui->plotRho->setAxisTitle(QwtPlot::yLeft,    oTxtYAxisTitle);

    /* Draw the canvas grid */
    QwtPlotGrid *poGrid = new QwtPlotGrid();
    poGrid->enableX( false );
    poGrid->enableY( true );
    poGrid->setMajorPen( Qt::gray, 0.5, Qt::DotLine );
    poGrid->attach( ui->plotRho );

    ui->plotRho->setAutoDelete ( true );

    /* Remove the gap between the data axes */
    for ( int i = 0; i < ui->plotRho->axisCnt; i++ )
    {
        QwtScaleWidget *poScaleWidget = ui->plotRho->axisWidget( i);
        if (poScaleWidget)
        {
            poScaleWidget->setMargin( 0 );
        }

        QwtScaleDraw *poScaleDraw = ui->plotRho->axisScaleDraw( i );
        if ( poScaleDraw )
        {
            poScaleDraw->enableComponent( QwtAbstractScaleDraw::Backbone, false );
        }
    }

    ui->plotRho->plotLayout()->setAlignCanvasToScales( true );

    //ui->plotRho->setTitle(("-------"));

    ui->plotRho->setAutoReplot(true);
}

/*****************************************************************************
 * Restore Curve when release this point or curve.
 *
 */
void MainWindow::restoreCurve()
{
    RX *poRX = gmapCurveData.value(gpoSelectedCurve);

    if( gpoSelectedCurve != NULL)
    {
        QVector<double> adR = this->getR(poRX->adF, poRX->adE);

        gpoSelectedCurve->setSamples( poRX->adF, adR );

        ui->plotCurve->setTitle("");

        ui->plotCurve->replot();
    }

    if( gpoErrorCurve != NULL )
    {
        gpoErrorCurve->setSamples( poRX->adF, poRX->adErr  );

        ui->plotScatter->setFooter("");

        ui->plotScatter->replot();
    }
}

/*****************************************************************************
 * Insert 2 Horizontal Marker line
 *
 */
void MainWindow::drawMarkerH()
{
    /* No Scatter, Don't draw the Marker line! */
    if( gpoScatter == NULL )
    {
        qDebugV5()<<"No scatter!";
        return;
    }

    /* Real time set  plot Scale and set aside blank. */
    double dYSacle = gpoScatter->maxYValue() - gpoScatter->minYValue();

    /* Detach & Delete marker line pointer */
    clearMarker();

    /* Top Marker line */
    sMkList.poTop = new QwtPlotMarker("Top");
    sMkList.poTop->setLineStyle( QwtPlotMarker::HLine );
    sMkList.poTop->setLabelAlignment(Qt::AlignLeft | Qt::AlignTop);
    sMkList.poTop->setYAxis( QwtPlot::yLeft );
    sMkList.poTop->setYValue( gpoScatter->maxYValue() + dYSacle*0.075 );
    sMkList.poTop->setLabel(QwtText(QString("Top: Y = %1").arg(sMkList.poTop->yValue())));
    sMkList.poTop->setLinePen(Qt::black, 1.0, Qt::SolidLine);

    sMkList.poTop->attach( ui->plotScatter );

    /* Bottom Marker line */
    sMkList.poBottom = new QwtPlotMarker("Bottom");
    sMkList.poBottom->setLineStyle( QwtPlotMarker::HLine );
    sMkList.poBottom->setLabelAlignment(Qt::AlignLeft | Qt::AlignBottom);
    sMkList.poBottom->setYAxis( QwtPlot::yLeft );
    sMkList.poBottom->setYValue( gpoScatter->minYValue() - dYSacle*0.075 );
    sMkList.poBottom->setLabel(QwtText(QString("Bottom: Y = %1").arg(sMkList.poBottom->yValue())));
    sMkList.poBottom->setLinePen(Qt::black, 1.0, Qt::SolidLine);

    sMkList.poBottom->attach( ui->plotScatter );

    QwtPlotCanvas *poCanvas = qobject_cast<QwtPlotCanvas *>( ui->plotScatter->canvas() );

    poCanvas->setCursor( Qt::SplitVCursor );

    ui->plotScatter->replot();
}

/**********************************************************
 * Insert 2 Vertical Marker line
 *
 */
void MainWindow::drawMarkerV()
{
    /* No Scatter, Don't draw the Marker line! */
    if( gpoScatter == NULL )
    {
        qDebugV5()<<"No scatter!";
        return;
    }

    /* X Axis scale */
    double dXSacle = gpoScatter->maxXValue() - gpoScatter->minXValue();

    /* Detach & Delete marker line pointer */
    clearMarker();

    /* Left Marker line */
    sMkList.poLeft = new QwtPlotMarker("Left");
    sMkList.poLeft->setLineStyle( QwtPlotMarker::VLine );
    sMkList.poLeft->setXAxis( QwtPlot::xBottom );
    sMkList.poLeft->setLabel(sMkList.poLeft->title());
    sMkList.poLeft->setLabelOrientation(Qt::Vertical);
    sMkList.poLeft->setLabelAlignment(Qt::AlignLeft | Qt::AlignTop);
    sMkList.poLeft->setXValue( gpoScatter->minXValue() - dXSacle*0.02 );
    sMkList.poLeft->setLabel(QwtText(QString("Left: X = %1").arg(sMkList.poLeft->xValue())));
    sMkList.poLeft->setLinePen(Qt::black, 1.0, Qt::SolidLine);

    sMkList.poLeft->attach( ui->plotScatter );

    /*  Right Marker line */
    sMkList.poRight = new QwtPlotMarker("Right");
    sMkList.poRight->setLineStyle( QwtPlotMarker::VLine );
    sMkList.poRight->setXAxis( QwtPlot::xBottom );
    sMkList.poRight->setLabel(sMkList.poRight->title());
    sMkList.poRight->setLabelOrientation(Qt::Vertical);
    sMkList.poRight->setLabelAlignment(Qt::AlignRight | Qt::AlignTop);
    sMkList.poRight->setXValue( gpoScatter->maxXValue() + 0.02 );
    sMkList.poRight->setLabel(QwtText(QString("Right: X = %1").arg(sMkList.poRight->xValue())));
    sMkList.poRight->setLinePen(Qt::black, 1.0, Qt::SolidLine);

    sMkList.poRight->attach( ui->plotScatter );

    QwtPlotCanvas *poCanvas = qobject_cast<QwtPlotCanvas *>( ui->plotScatter->canvas() );

    poCanvas->setCursor( Qt::SplitHCursor );

    ui->plotScatter->replot();
}

/***************************************************************
 * Detach & Delete marker line pointer
 *
 */
void MainWindow::clearMarker()
{
    QwtPlotCanvas *poCanvas = qobject_cast<QwtPlotCanvas *>( ui->plotScatter->canvas() );

    poCanvas->setCursor( Qt::ArrowCursor );

    /* Detach & Delete marker line pointer */
    if(sMkList.poTop != NULL)
    {
        sMkList.poTop->detach();
        delete sMkList.poTop;
        sMkList.poTop = NULL;
    }

    if(sMkList.poBottom != NULL)
    {
        sMkList.poBottom->detach();
        delete sMkList.poBottom;
        sMkList.poBottom = NULL;
    }

    if(sMkList.poLeft != NULL)
    {
        sMkList.poLeft->detach();
        delete sMkList.poLeft;
        sMkList.poLeft = NULL;
    }

    if(sMkList.poRight != NULL)
    {
        sMkList.poRight->detach();
        delete sMkList.poRight;
        sMkList.poRight = NULL;
    }
}

/*******************************************************************
 * Read last Dir log file, get last Dir(Previous directory)
 */
QString MainWindow::LastDirRead()
{
    QString oStrLastDir;
    oStrLastDir.clear();

    //qDebugV0()<<"Read last time Dir.";

    QFile oFileLastDir(LASTDIR);

    if( oFileLastDir.open(QIODevice::ReadOnly | QIODevice::Text) )
    {
        QTextStream oTextStreamIn(&oFileLastDir);
        oStrLastDir = oTextStreamIn.readLine();

        if(oStrLastDir.isNull())
        {
            /* Default Dir */
            oStrLastDir = "D:/";
        }

        //qDebugV0()<<"Last Dir:"<<oStrLastDir;
    }
    else
    {
        qDebugV0()<<"Can't open the file! Return default Dir.";

        /* Default Dir */
        oStrLastDir = "D:/";
    }

    oFileLastDir.close();

    return oStrLastDir;
}

/***********************************************************************
 * Save the current project Dir as the most recently opened directory
 */
void MainWindow::LastDirWrite(QString oStrFileName)
{
    QFileInfo oFileInfoLastDir(oStrFileName);

    //qDebugV0()<<"Current project Dir:"<<oFileInfoLastDir.absoluteDir().absolutePath();

    QDir oDir = oFileInfoLastDir.absoluteDir();

    //qDebugV0()<<oDir;

    if( !oDir.cdUp() )
    {
        //qDebugV0()<<"After switching to the first level Dir:"<<oDir.absolutePath();
        qDebugV0()<<"The previous directory of the current directory does not exist!";
        return;
    }

    QFile oFileLastDir(LASTDIR);

    if( !oFileLastDir.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text) )
    {
        qDebugV0()<<"Open last dir file false!";
        return;
    }

    QTextStream oTextStreamOut(&oFileLastDir);

    QDir oDirRslt = oFileInfoLastDir.absoluteDir();

    //qDebugV0()<<oDirRslt.absolutePath();

    oTextStreamOut<<oDirRslt.absolutePath();

    oFileLastDir.close();
}

/* 发射电流曲线 */
void MainWindow::initPlotTx()
{
    //ui->plotCurve->setTitle(QwtText("Curve Plot"));
    QFont oFont("Times New Roman", 12, QFont::Thin);

    //    ui->plotTx->setCanvasBackground(QColor(29, 100, 141)); // nice blue

    ui->plotTx->setFont(oFont);

    QwtText oTxtTitle( "发射端_电流曲线" );
    oTxtTitle.setFont( oFont );

    ui->plotTx->setTitle(oTxtTitle);

    /* Nice blue */
    //ui->plotCurve->setCanvasBackground(QColor(55, 100, 141));

    /* Set Log Scale */
    ui->plotTx->setAxisScaleEngine( QwtPlot::xBottom, new QwtLogScaleEngine() );
    ui->plotTx->setAxisScaleEngine( QwtPlot::yLeft,   new QwtLogScaleEngine() );

    ui->plotTx->enableAxis(QwtPlot::xBottom , true);
    ui->plotTx->enableAxis(QwtPlot::yLeft   , true);

    QwtScaleDraw *poScaleDraw  = new QwtScaleDraw();
    poScaleDraw->setLabelRotation( -26 );
    poScaleDraw->setLabelAlignment( Qt::AlignLeft | Qt::AlignVCenter );

    ui->plotTx->setAxisScaleDraw(QwtPlot::xBottom, poScaleDraw);

    /* Set Axis title */
    QwtText oTxtXAxisTitle( "F/Hz" );
    QwtText oTxtYAxisTitle( "I" );
    oTxtXAxisTitle.setFont( oFont );
    oTxtYAxisTitle.setFont( oFont );

    ui->plotTx->setAxisTitle(QwtPlot::xBottom,  oTxtXAxisTitle);
    ui->plotTx->setAxisTitle(QwtPlot::yLeft,    oTxtYAxisTitle);

    /* Draw the canvas grid */
    QwtPlotGrid *poGrid = new QwtPlotGrid();
    poGrid->enableX( false );
    poGrid->enableY( true );
    poGrid->setMajorPen( Qt::gray, 0.5, Qt::DotLine );
    poGrid->attach( ui->plotTx );

    ui->plotTx->setAutoDelete ( true );

    /* Remove the gap between the data axes */
    for ( int i = 0; i < ui->plotTx->axisCnt; i++ )
    {
        QwtScaleWidget *poScaleWidget = ui->plotTx->axisWidget( i);
        if (poScaleWidget)
        {
            poScaleWidget->setMargin( 0 );
        }

        QwtScaleDraw *poScaleDraw = ui->plotTx->axisScaleDraw( i );
        if ( poScaleDraw )
        {
            poScaleDraw->enableComponent( QwtAbstractScaleDraw::Backbone, false );
        }
    }

    ui->plotTx->plotLayout()->setAlignCanvasToScales( true );

    ui->plotTx->setAutoReplot(true);
}

/* 导出广域视电阻率计算结果到csv文档。 */
void MainWindow::on_actionExportRho_triggered()
{
    QString oStrDefault = QDateTime::currentDateTime().toString("yyyy年MM月dd日HH时mm分ss秒_广域视电阻率结果");

    QString oStrFileName = QFileDialog::getSaveFileName(this, tr("保存广域视电阻率计算结果"),
                                                        QString("%1/%2.csv").arg(this->LastDirRead()).arg(oStrDefault),
                                                        "(*.csv *.txt *.dat)");

    QFile oFile(oStrFileName);
    bool openOk = oFile.open(QIODevice::WriteOnly|QIODevice::Truncate|QIODevice::Text);
    if(!openOk)
    {
        return ;
    }

    QTextStream outStream(&oFile);

    QAbstractItemModel *poModel = ui->tableViewRho->model();

    /* 列头 */
    for(int i = 0; i < poModel->columnCount(); i++)
    {
        outStream<<poModel->headerData(i, Qt::Horizontal, Qt::DisplayRole).toString()<<",";
    }
    outStream<<endl;

    for(int i = 0; i < poModel->rowCount(); i++)
    {
        for(int j = 0 ; j < poModel->columnCount(); j++)
        {
            outStream<<poModel->data(poModel->index(i,j), Qt::DisplayRole).toString()<<",";
        }
        outStream<<endl;
    }

    outStream.flush();
    oFile.close();

    this->showMsg(QString("数据导出成功\n\n%1").arg(oStrFileName));
}

/***************************************************************
 * Read data from RX, update Scatter\Error\Curve
 *
 */
void MainWindow::on_actionRecovery_triggered()
{
    if( gpoSelectedCurve == NULL || giSelectedIndex == -1)
    {
        this->showMsg("未选中曲线上的点！");
        return;
    }

    qDebugV0()<<gpoSelectedCurve->title().text()<<giSelectedIndex;


    /* Read scatter from Sctatter table, then update curve && error Table */
    gpoSelectedRX->renewScatter(giSelectedIndex);

    /* Draw scatter(Point from scatter) */
    this->drawScatter();

    /* Get points from curve table, replace current curve's points, repplote. */
    this->recoveryCurve(gpoSelectedCurve);

    /* Draw error, get point from Error table */
    this->drawError();
}

/* 做了裁剪之后， 点击保存， 保存的是散点（detail）， 接着更新Curve */
void MainWindow::on_actionSave_triggered()
{
    QPolygonF aoPoint = this->currentScatterPoints();

    if( aoPoint.count() == 0)
    {
        return;
    }

    QVector<double> adY;
    adY.clear();

    for(qint32 i = 0; i < aoPoint.count(); i++)
    {
        adY.append(aoPoint.at(i).y());
    }

    RX *poRX = gmapCurveData.value(gpoSelectedCurve);

    poRX->updateScatter(giSelectedIndex, adY);

    gpoScatter->setSamples( aoPoint );

    this->resizeScaleScatter();

    ui->plotScatter->replot();
}

void MainWindow::on_actionCalRho_triggered()
{
    ui->stackedWidget->setCurrentIndex(1);

    /* Import RX */
    poDb->importRX(gapoRX);

    QString oStrFileName = QFileDialog::getOpenFileName(this,
                                                        "打开坐标文件",
                                                        QString("%1").arg(this->LastDirRead()),
                                                        "坐标文件(*.dat *.txt *.csv)");

    if(oStrFileName.length() != 0)
    {
        poDb->importXY(oStrFileName);
    }

    poCalRho->getAB();

    QList<STATION> aoStation = poDb->getStation();

    foreach(STATION oStation, aoStation)
    {
        poCalRho->CalRho(oStation);
    }

    ui->actionCalRho->setEnabled(false);
    //ui->actionClear->setEnabled(false);
    ui->actionCutterH->setEnabled(false);
    ui->actionCutterH->setEnabled(false);
    ui->actionCutterV->setEnabled(false);
    ui->actionExportRho->setEnabled(true);
    ui->actionImportRX->setEnabled(false);
    ui->actionImportTX->setEnabled(false);
    ui->actionRecovery->setEnabled(false);
    ui->actionSave->setEnabled(false);
}

void MainWindow::showTableTX(QSqlTableModel *poModel)
{
    ui->tableViewTX->setModel(poModel);

    ui->tableViewTX->repaint();

    ui->tabWidget->setCurrentIndex(0);

    QVector<double> adF, adI;

    for(int i = 0; i < poModel->rowCount(); i++)
    {
        adF.append( poModel->data(poModel->index(i,0), Qt::DisplayRole).toDouble() );
        adI.append( poModel->data(poModel->index(i,1), Qt::DisplayRole).toDouble() );
    }

    this->drawTx(adF, adI);
}

void MainWindow::showTableRX(QSqlTableModel *poModel)
{
    ui->tableViewRX->setModel(poModel);

    ui->tableViewRX->repaint();

    this->drawRx();

    ui->tabWidget->setCurrentIndex(1);
}

void MainWindow::showTableXY(QSqlTableModel *poModel)
{
    ui->tableViewXY->setModel(poModel);

    ui->tableViewXY->repaint();

    ui->tabWidget->setCurrentIndex(2);
}

void MainWindow::showTableRho(QSqlTableModel *poModel)
{
    ui->tableViewRho->setModel(poModel);

    ui->tableViewRho->repaint();

    ui->tabWidget->setCurrentIndex(3);
}

void MainWindow::drawRho(STATION oStation, QVector<double> adF, QVector<double> adRho)
{
    ui->plotRho->detachItems();
    ui->plotRho->replot();

    QSet<double> setF;

    foreach(double dF, adF)
    {
        setF.insert(dF);
    }

    QList<double> listF = setF.toList();

    qSort(listF);

    qDebugV0()<<listF;

    /* Fill ticks */
    QList<double> adTicks[QwtScaleDiv::NTickTypes];
    adTicks[QwtScaleDiv::MajorTick] = listF;
    QwtScaleDiv oScaleDiv( adTicks[QwtScaleDiv::MajorTick].last(), adTicks[QwtScaleDiv::MajorTick].first(), adTicks );

    //QwtScaleDiv (double lowerBound, double upperBound, QList< double >[NTickTypes])
    qDebugV0()<<ui->plotRho->axisScaleDiv(QwtPlot::xBottom).lowerBound()
             <<adTicks[QwtScaleDiv::MajorTick].last()
            <<ui->plotRho->axisScaleDiv(QwtPlot::xBottom).upperBound()
           <<adTicks[QwtScaleDiv::MajorTick].first();

    if( ui->plotRho->axisScaleDiv(QwtPlot::xBottom).lowerBound() == adTicks[QwtScaleDiv::MajorTick].last() ||
            ui->plotRho->axisScaleDiv(QwtPlot::xBottom).upperBound() == adTicks[QwtScaleDiv::MajorTick].first() )
    {
        qDebugV0()<<"==";
    }
    {
        ui->plotRho->setAxisScaleDiv( QwtPlot::xBottom, oScaleDiv );
    }


    /* Curve title, cut MCSD_ & suffix*/
    const QwtText oTxtTitle( QString("L%1-%2_D%3-%4_%5")
                             .arg(oStation.oStrLineId)
                             .arg(oStation.oStrSiteId)
                             .arg(oStation.iDevId)
                             .arg(oStation.iDevCh)
                             .arg(oStation.oStrTag) );

    /* Create a curve pointer */
    QwtPlotCurve *poCurve = new QwtPlotCurve( oTxtTitle );

    poCurve->setPen( Qt::red, 2, Qt::SolidLine );
    poCurve->setRenderHint( QwtPlotItem::RenderAntialiased, true );

    QwtSymbol *poSymbol = new QwtSymbol( QwtSymbol::Ellipse,
                                         QBrush( Qt::yellow ),
                                         QPen( Qt::blue, 2 ),
                                         QSize( 8, 8 ) );
    poCurve->setSymbol( poSymbol );
    poCurve->setStyle(QwtPlotCurve::Lines);

    poCurve->setSamples( adF, adRho );
    poCurve->setAxes(QwtPlot::xBottom, QwtPlot::yLeft);
    poCurve->attach(ui->plotRho);
    poCurve->setVisible( true );

    ui->plotRho->replot();
}
