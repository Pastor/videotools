#ifndef VIDEODIALOG_H
#define VIDEODIALOG_H

#include <QDialog>

namespace Ui {
class VideoDialog;
}

class VideoDialog : public QDialog
{
    Q_OBJECT

public:
    explicit VideoDialog(QWidget *parent = 0);
    ~VideoDialog();

    std::string getFileName() const;

private slots:
    void on_acceptB_clicked();

    void on_cancelB_clicked();

    void on_filenameB_clicked();

private:
    Ui::VideoDialog *ui;
};

#endif // VIDEODIALOG_H
