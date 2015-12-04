#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDesktopServices>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    createThreads();
    createActions();
    createMenus();
    makeConnections();

    setWindowTitle(QString(APP_NAME));
    //showMaximized();
}

MainWindow::~MainWindow()
{
    pt_videoThread->exit();
        pt_videoThread->wait();
    pt_stasmThread->exit();
        pt_stasmThread->wait();
    pt_opencvThread->exit();
        pt_opencvThread->wait();
    delete ui;
}

void MainWindow::createActions()
{
    pt_fileAct = new QAction(tr("&File"), this);
    pt_fileAct->setStatusTip(tr("Open video file"));
    connect(pt_fileAct, SIGNAL(triggered()), this, SLOT(callFileSelectDialog()));

    pt_resumeAct = new QAction(tr("&Resume"), this);
    pt_resumeAct->setStatusTip("Resume");
    connect(pt_resumeAct, SIGNAL(triggered()), pt_videocapture, SLOT(resume()));

    pt_pauseAct = new QAction(tr("&Pause"), this);
    pt_pauseAct->setStatusTip("Pause");
    connect(pt_pauseAct, SIGNAL(triggered()), pt_videocapture, SLOT(pause()));

    pt_backwardAct = new QAction(this);
    connect(pt_backwardAct, SIGNAL(triggered()), pt_videocapture, SLOT(stepBackward()));

    pt_forwardAct = new QAction(this);
    connect(pt_forwardAct, SIGNAL(triggered()), pt_videocapture, SLOT(stepForward()));

    pt_speedupAct = new QAction(tr("Speedx2.0"), this);
    pt_speedupAct->setStatusTip("Increase speed of playback by two times");
    connect(pt_speedupAct, SIGNAL(triggered(bool)), pt_videocapture, SLOT(speedUp()));

    pt_speeddownAct = new QAction(tr("Speedx0.5"), this);
    pt_speeddownAct->setStatusTip("Decrease speed of playback by two times");
    connect(pt_speeddownAct, SIGNAL(triggered(bool)), pt_videocapture, SLOT(speedDown()));

    pt_aboutAct = new QAction(tr("&About"), this);
    pt_aboutAct->setStatusTip("Show about");
    connect(pt_aboutAct, SIGNAL(triggered()), this, SLOT(about()));

    pt_helpAct = new QAction(tr("&Help"), this);
    pt_helpAct->setStatusTip("Show help");
    connect(pt_helpAct, SIGNAL(triggered()), this, SLOT(help()));

    pt_numAct = new QAction(tr("&Numbers"), this);
    pt_numAct->setStatusTip("Toogle show numbers");
    pt_numAct->setCheckable(true);
    pt_numAct->setChecked(true);
    connect(pt_numAct, SIGNAL(triggered(bool)), ui->display, SLOT(setNumbersVisualization(bool)));

    pt_imageAct = new QAction(tr("&Image"), this);
    pt_imageAct->setStatusTip("Toogle show image");
    pt_imageAct->setCheckable(true);
    pt_imageAct->setChecked(true);
    connect(pt_imageAct, SIGNAL(triggered(bool)), ui->display, SLOT(setImageVisualization(bool)));

    pt_selectionAct = new QAction(tr("&Select"), this);
    pt_selectionAct->setStatusTip("Toogle show selection");
    pt_selectionAct->setCheckable(true);
    pt_selectionAct->setChecked(true);
    connect(pt_selectionAct, SIGNAL(triggered(bool)), ui->display, SLOT(setSelectionVisualization(bool)));

    pt_deviceAct = new QAction(tr("&Device"),this);
    pt_deviceAct->setStatusTip(tr("Open video device"));
    connect(pt_deviceAct, SIGNAL(triggered()), this, SLOT(callDeviceSelectDialog()));

    pt_plotAct = new QAction(tr("&Plot"),this);
    pt_plotAct->setStatusTip(tr("New plot"));
    connect(pt_plotAct, SIGNAL(triggered()), this, SLOT(addPlot()));
}

void MainWindow::createMenus()
{
    pt_sourceMenu = menuBar()->addMenu(tr("&Source"));
    pt_sourceMenu->addAction(pt_fileAct);
    pt_sourceMenu->addAction(pt_deviceAct);

    pt_optionsMenu = menuBar()->addMenu(tr("&Options"));
    pt_optionsMenu->addAction(pt_numAct);
    pt_optionsMenu->addAction(pt_imageAct);
    pt_optionsMenu->addAction(pt_selectionAct);
    pt_optionsMenu->addSeparator();
    pt_optionsMenu->addAction(pt_plotAct);

    pt_helpMenu = menuBar()->addMenu(tr("&Help"));
    pt_helpMenu->addAction(pt_aboutAct);
    pt_helpMenu->addAction(pt_helpAct);
}

void MainWindow::createThreads()
{
    pt_videoThread = new QThread(this);
    pt_videocapture = new QVideoCapture();
    pt_videocapture->moveToThread(pt_videoThread);
    connect(pt_videoThread, SIGNAL(started()), pt_videocapture, SLOT(initializeTimer()));
    connect(pt_videoThread, SIGNAL(finished()), pt_videocapture, SLOT(close()));
    connect(pt_videoThread, SIGNAL(finished()), pt_videocapture, SLOT(deleteLater()));

    pt_stasmThread = new QThread(this);
    pt_stasm = new QStasm();
    pt_stasm->moveToThread(pt_stasmThread);
    connect(pt_stasmThread, SIGNAL(finished()), pt_stasm, SLOT(deleteLater()));

    pt_opencvThread = new QThread(this);
    pt_opencv = new QOpencvProcessor();
    pt_opencv->moveToThread(pt_opencvThread);
    connect(pt_opencvThread, SIGNAL(finished()), pt_opencv, SLOT(deleteLater()));

    qRegisterMetaType<cv::Mat>("cv::Mat");
    connect(pt_videocapture, SIGNAL(frameUpdated(cv::Mat)), pt_opencv, SLOT(custom_algorithm(cv::Mat)));
    connect(pt_videocapture,SIGNAL(frameUpdated(cv::Mat)), pt_stasm, SLOT(search_single(cv::Mat)), Qt::BlockingQueuedConnection);
    connect(pt_stasm, SIGNAL(landmarksUpdated(cv::Mat,float*,uint)), ui->display, SLOT(updateImage(cv::Mat,float*,uint)), Qt::BlockingQueuedConnection);

    pt_videoThread->start(QThread::LowPriority);
    pt_stasmThread->start(QThread::HighPriority);
    pt_opencvThread->start();
}

void MainWindow::makeConnections()
{
    connect(pt_videocapture, SIGNAL(positionUpdated(int)), ui->positionSlider, SLOT(setValue(int)));
    connect(ui->positionSlider, SIGNAL(sliderPressed()), pt_videocapture, SLOT(pause()));
    connect(ui->positionSlider, SIGNAL(sliderReleased(int)), pt_videocapture, SLOT(setPosition(int)));
    connect(ui->positionSlider, SIGNAL(sliderReleased(int)), pt_videocapture, SLOT(resume()));
    connect(pt_videocapture, SIGNAL(framesInFile(int)), ui->positionSlider, SLOT(setMaxValue(int)));

    connect(ui->resumeButton, SIGNAL(pressed()), pt_resumeAct, SLOT(trigger()));
    connect(ui->pauseButton, SIGNAL(pressed()), pt_pauseAct, SLOT(trigger()));
    connect(ui->backwardButton, SIGNAL(pressed()), pt_backwardAct, SLOT(trigger()));
    connect(ui->forwardButton, SIGNAL(pressed()), pt_forwardAct, SLOT(trigger()));
    connect(ui->speeddownButton, SIGNAL(pressed()), pt_speeddownAct, SLOT(trigger()));
    connect(ui->speedupButton, SIGNAL(pressed()), pt_speedupAct, SLOT(trigger()));

    connect(pt_videocapture, SIGNAL(positionUpdated(int)), ui->frameLCD, SLOT(display(int)));
    connect(pt_videocapture, SIGNAL(framesInFile(int)), ui->totalframesLCD, SLOT(display(int)));

    connect(pt_stasm, SIGNAL(frametimeUpdated(double)), ui->frametimeLCD, SLOT(display(double)));

    connect(pt_opencv, SIGNAL(snrUpdated(double)), ui->snrLCD, SLOT(display(double)));
    connect(pt_opencv, SIGNAL(contrastUpdated(double)), ui->contrastLCD, SLOT(display(double)));
    connect(pt_stasm, SIGNAL(eyesdistanceUpdated(double)), ui->eyesLCD, SLOT(display(double)));   
    qRegisterMetaType<cv::Rect>("cv::Rect");
    connect(pt_stasm, SIGNAL(facerectUpdated(cv::Rect)), ui->display, SLOT(updateSelection(cv::Rect)));
}

void MainWindow::updateStatus(const QString &str)
{
    this->statusBar()->showMessage(str);
}

void MainWindow::updateStatus(int value)
{
    this->statusBar()->showMessage(QString::number(value));
}

void MainWindow::callFileSelectDialog()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"), "", tr("Video (*.avi *.mp4 *.wmf)"));
    if(!pt_videocapture->openfile(fileName)) {
        QMessageBox msg(QMessageBox::Information, tr("Info"), tr("Can not open file"), QMessageBox::Cancel);
        msg.exec();
    } else
        pt_resumeAct->trigger();
}

void MainWindow::about()
{
   QDialog *aboutdialog = new QDialog();
   int pSize = aboutdialog->font().pointSize();
   aboutdialog->setWindowTitle("About");
   aboutdialog->setFixedSize(pSize*27,pSize*17);

   QVBoxLayout *templayout = new QVBoxLayout();
   templayout->setMargin(5);

   QLabel *projectname = new QLabel(QString(APP_NAME) +"\t"+ QString(APP_VERSION));
   projectname->setFrameStyle(QFrame::Box | QFrame::Raised);
   projectname->setAlignment(Qt::AlignCenter);
   QLabel *projectauthors = new QLabel(QString(APP_DESIGNER) + "\n\nBMSTU\n\nNovember of 2015");
   projectauthors->setWordWrap(true);
   projectauthors->setAlignment(Qt::AlignCenter);
   QLabel *hyperlink = new QLabel("<a href='mailto:biometric-doc@yandex.ru?subject=Pointmetry'>Contact us at biometric-doc@yandex.ru");
   hyperlink->setOpenExternalLinks(true);
   hyperlink->setAlignment(Qt::AlignCenter);

   templayout->addWidget(projectname);
   templayout->addWidget(projectauthors);
   templayout->addWidget(hyperlink);

   aboutdialog->setLayout(templayout);
   aboutdialog->exec();

   delete hyperlink;
   delete projectauthors;
   delete projectname;
   delete templayout;
   delete aboutdialog;
}

void MainWindow::help()
{
    if (!QDesktopServices::openUrl(QUrl(QString("http://biometric.bmstu.ru/category/kontakti"), QUrl::TolerantMode))) // runs the ShellExecute function on Windows
    {
        QMessageBox msgBox(QMessageBox::Information, this->windowTitle(), tr("Can not find help"), QMessageBox::Ok, this, Qt::Dialog);
        msgBox.exec();
    }
}

void MainWindow::callDeviceSelectDialog()
{
    pt_videocapture->open_deviceSelectDialog();
    if( pt_videocapture->opendevice() )
        QTimer::singleShot(500, pt_videocapture, SLOT(resume()));
}

void MainWindow::addPlot()
{
    QEasyPlot *plot = new QEasyPlot();
    plot->setWindowFlags(Qt::Window);
    v_plots.push_back(plot);
    plot->show();
}

void MainWindow::closeEvent(QCloseEvent*)
{
    QEasyPlot *temp;
    for(uint i = 0; i < v_plots.size(); i++)    {
        temp = v_plots[i];
        temp->close();
    }
    disconnect(pt_videocapture,0,0,0);
}
