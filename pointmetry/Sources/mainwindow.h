#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMessageBox>
#include <QFileDialog>
#include <QAction>
#include <QThread>
#include <QMenu>

#include "qimagewidget.h"
#include "qvideocapture.h"
#include "qstasm.h"
#include "qeasyplot.h"
#include "qopencvprocessor.h"
#include "videodialog.h"
#include "qvideowriter.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void updateStatus(const QString &str);
    void updateStatus(int value);
    void makeConnections();
    void callFileSelectDialog();
    void callDeviceSelectDialog();
    void callVideoWriteDialog(bool new_session);
    void about();
    void help();
    void addPlot();

private:
    Ui::MainWindow *ui;

    QAction *pt_fileAct;
    QAction *pt_deviceAct;
    QAction *pt_pauseAct;
    QAction *pt_resumeAct;
    QAction *pt_forwardAct;
    QAction *pt_backwardAct;
    QAction *pt_speedupAct;
    QAction *pt_speeddownAct;
    QAction *pt_aboutAct;
    QAction *pt_helpAct;
    QAction *pt_numAct;
    QAction *pt_imageAct;
    QAction *pt_selectionAct;
    QAction *pt_plotAct;
    QAction *pt_writeAct;

    QMenu *pt_sourceMenu;
    QMenu *pt_optionsMenu;
    QMenu *pt_helpMenu;
    QMenu *pt_writeMenu;

    QThread *pt_videoThread;
    QThread *pt_stasmThread;
    QThread *pt_opencvThread;
    QVideoCapture *pt_videocapture;
    QStasm *pt_stasm;
    QOpencvProcessor *pt_opencv;

    std::vector<QEasyPlot*> v_plots;

    VideoDialog m_writeDialog;
    QVideoWriter *pt_videowriter;
    QThread *pt_videowriterThread;

    void createActions();
    void createMenus();
    void createThreads();
protected:
    void closeEvent(QCloseEvent *);
};

#endif // MAINWINDOW_H
