#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    createThreads();
    createActions();
    createMenus();
    makeConnections();
}

MainWindow::~MainWindow()
{
    pt_videoThread->exit();
        pt_videoThread->wait();
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
}

void MainWindow::createMenus()
{
    pt_sourceMenu = this->menuBar()->addMenu(tr("Source"));
    pt_sourceMenu->addAction(pt_fileAct);
}

void MainWindow::createThreads()
{
    pt_videoThread = new QThread(this);
    pt_videocapture = new QVideoCapture();
    pt_videocapture->moveToThread(pt_videoThread);
    connect(pt_videoThread, SIGNAL(started()), pt_videocapture, SLOT(initializeTimer()));
    connect(pt_videoThread, SIGNAL(finished()), pt_videocapture, SLOT(deleteLater()));



    qRegisterMetaType<cv::Mat>("cv::Mat");
    connect(pt_videocapture,SIGNAL(frameUpdated(cv::Mat)), ui->display, SLOT(updateImage(cv::Mat)));

    pt_videoThread->start();
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
