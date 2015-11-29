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

private:
    Ui::MainWindow *ui;

    QAction *pt_aboutAct;
    QAction *pt_helpAct;
    QAction *pt_fileAct;
    QAction *pt_pauseAct;
    QAction *pt_resumeAct;
    QAction *pt_forwardAct;
    QAction *pt_backwardAct;
    QAction *pt_speedupAct;
    QAction *pt_speeddownAct;

    QMenu *pt_sourceMenu;
    QMenu *pt_helpMenu;

    QThread *pt_videoThread;
    QVideoCapture *pt_videocapture;

    void createActions();
    void createMenus();
    void createThreads();
};

#endif // MAINWINDOW_H
