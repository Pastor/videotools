#include "videodialog.h"
#include "ui_videodialog.h"

#include <QFileDialog>

VideoDialog::VideoDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::VideoDialog)
{
    ui->setupUi(this);    
}

VideoDialog::~VideoDialog()
{
    delete ui;
}

void VideoDialog::on_acceptB_clicked()
{
    this->accept();
}

void VideoDialog::on_cancelB_clicked()
{
    this->reject();
}

void VideoDialog::on_filenameB_clicked()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"),
                                                    "/record.avi",
                                                    tr("Video (*.avi *.mp4)"));
    ui->filenameE->setText(fileName);
}

std::string VideoDialog::getFileName() const
{
    return ui->filenameE->text().toStdString();
}

int VideoDialog::getCodec() const
{
    switch(ui->codecCB->currentIndex()) {
        case 0:
            return CV_FOURCC('M','P','4','2');
        case 1:
            return CV_FOURCC('M','J','P','G');
    }
}
