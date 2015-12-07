#include "qcommutator.h"

QCommutator::QCommutator(QObject *parent) : QObject(parent)
{
    int idealNum = QThread::idealThreadCount() - 1;
    m_size = idealNum > 1 ? idealNum : 1;
    initialize();
}

QCommutator::QCommutator(QObject *parent, uint threads) : QObject(parent)
{
    m_size = threads;
    initialize();
}

void QCommutator::initialize()
{
    m_pos = 0;
    for(uint i = 0; i < m_size; i++)   {
        QThread *thread = new QThread(this);
        QStasm *stasm = new QStasm();
        stasm->setID(i);
        stasm->moveToThread(thread);
        connect(thread, SIGNAL(finished()), stasm, SLOT(deleteLater()));
        connect(stasm, SIGNAL(landmarksUpdated(cv::Mat,float*,uint)), this, SIGNAL(landmarksUpdated(cv::Mat,float*,uint)));
        connect(stasm, SIGNAL(eyesdistanceUpdated(double)), this, SIGNAL(eyesdistanceUpdated(double)));
        connect(stasm, SIGNAL(facerectUpdated(cv::Rect)), this, SIGNAL(facerectUpdated(cv::Rect)));
        connect(stasm, SIGNAL(doneWork(int)), this, SLOT(updateQueue(int)));
        v_threads.push_back(thread);
        v_stasms.push_back(stasm);
        v_busy.push_back(false);
        thread->start();
    }
    qWarning("Commuatator creates %d threads", m_size);
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
    for(int i = 0; i < v_busy.size(); i++)
        qWarning(v_busy[i] ? "%d busy" : "%d free", i);
    m_pos = searchFreeThread();
    qWarning("Thread %d will be launched", m_pos);
    for(int i = 0; i < v_busy.size(); i++)
        qWarning(v_busy[i] ? "%d busy" : "%d free", i);
    qWarning("\n");
    if(m_pos > 0 && m_pos < m_size) {
        connect(this, SIGNAL(imageUpdated(cv::Mat)), v_stasms[m_pos], SLOT(search_single(cv::Mat)));
        emit imageUpdated(img);
        disconnect(this, SIGNAL(imageUpdated(cv::Mat)), v_stasms[m_pos], SLOT(search_single(cv::Mat)));
    }
    //m_pos = (++m_pos) % m_size;

}

int QCommutator::searchFreeThread()
{
    int res = -1;
    for(int i = 0; i < v_busy.size(); i++)
        if(v_busy[i] == false) {
            v_busy[i] = true;
            res = i;
        }
    return res;
}

void QCommutator::updateQueue(int id)
{
    v_busy[id] = false;
    qWarning("Thread %d done work", id);
    m_frametime = (cv::getTickCount() -  m_time) * 1000.0 / cv::getTickFrequency(); // result is calculated in milliseconds
    m_time = cv::getTickCount();
    emit frametimeUpdated(m_frametime);
}





