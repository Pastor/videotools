#include "qcommutator.h"

QCommutator::QCommutator(QObject *parent) : QObject(parent)
{
    m_pos = 0;
    int idealNum = QThread::idealThreadCount() - 1;
    m_size = idealNum > 1 ? idealNum : 1;
    qWarning("Commuatator create %d treads", size);

    for(uint i = 0; i < m_size; i++)   {
        QThread *thread = new QThread(this);
        QStasm *stasm = new QStasm();
        stasm.moveToThread(thread);
        connect(stasm, SIGNAL(finished()), thread, SLOT(deleteLater()));
        connect(stasm, SIGNAL(landmarksUpdated(cv::Mat,float*,uint)), this, SIGNAL(landmarksUpdated(cv::Mat,float*,uint)));
        connect(stasm, SIGNAL(eyesdistanceUpdated(double)), this, SIGNAL(eyesdistanceUpdated(double)));
        connect(stasm, SIGNAL(facerectUpdated(cv::Rect)), this, SIGNAL(facerectUpdated(cv::Rect)));
        v_threads.push_back(thread);
        v_stasms.push_back(stasm);
    }
}

QCommutator::~QCommutator()
{
    for(uint i = 0; i < v_stasms.size(); i++) {
        v_threads[i]->exit();
        v_threads[i]->wait();
    }
}

void QCommutator::search_single(const cv::Mat &img)
{
    connect(this, SIGNAL(imageUpdated(cv::Mat)), v_stasms[m_pos], SLOT(search_single(cv::Mat)));
    emit imageUpdated(img);
    disconnect(this, SIGNAL(imageUpdated(cv::Mat)), v_stasms[m_pos], SLOT(search_single(cv::Mat)));
    m_pos = (++m_pos) % m_size;
}





