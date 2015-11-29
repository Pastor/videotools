/*--------------------------------------------------------------------------------------------
Taranov Alex, 2014                              		               CPP SOURCE FILE
The Develnoter [internet], Marcelo Mottalli, 2014
This class is a descendant of QWidget with QImageWidget::updateImage(...) slot that constructs
QImage instance from cv::Mat image. The QImageWidget should be used as widget for video display  
 * ------------------------------------------------------------------------------------------*/

#include "qimagewidget.h"

//-----------------------------------------------------------------------------------
QImageWidget::QImageWidget(QWidget *parent): WIDGET_CLASS(parent)
{
    #ifdef OPENGL_WIDGETS
        qWarning("QOpenGLWidget default samples: %d", this->format().samples());
        this->setAutoFillBackground(true);
        QSurfaceFormat format;
        format.setSamples(3);   //antialiasing becomes better for high values, but performance becomes too slow (for the instance, 3 is good enought for Pentium IV)
        this->setFormat(format);
        qWarning("QOpenGLWidget manual samples: %d", this->format().samples());
    #endif
    f_select = true;
}
//-----------------------------------------------------------------------------------
void QImageWidget::updateImage(const cv::Mat& image)
{
    m_string = QString::number(image.cols) + "x" + QString::number(image.rows);

    switch ( image.type() )
    {
        case CV_8UC1:
            cv::cvtColor(image, m_cvMat, CV_GRAY2RGB);
            break;
        case CV_8UC3:
            cv::cvtColor(image, m_cvMat, CV_BGR2RGB);
            break;
    }
    assert(m_cvMat.isContinuous()); // QImage needs the data to be stored continuously in memory
    m_qimage = QImage(m_cvMat.data, m_cvMat.cols, m_cvMat.rows, m_cvMat.cols * 3, QImage::Format_RGB888);  // Assign OpenCV's image buffer to the QImage
    update();
}
//------------------------------------------------------------------------------------
void QImageWidget::paintEvent(QPaintEvent* )
{
    updateViewRect();
    QPainter painter( this );
    painter.drawImage(m_viewRect, m_qimage);
    painter.setRenderHint(QPainter::Antialiasing);
    //drawString(painter, m_viewRect);
    drawSelection(painter);
}
//------------------------------------------------------------------------------------
void QImageWidget::mousePressEvent(QMouseEvent *event)
{
    x0 = event->x();
    y0 = event->y();
    m_aimrect.setX( x0 );
    m_aimrect.setY( y0 );

}
//--------------------------------------------------------------------------------------
void QImageWidget::mouseMoveEvent(QMouseEvent *event)
{
    if( event->x() > x0)
        m_aimrect.setWidth(event->x() - x0);
    else {
        m_aimrect.setX( event->x() );
        m_aimrect.setWidth( x0 - event->x() );
    }
    if( event->y() > y0)
        m_aimrect.setHeight(event->y() - y0);
    else {
        m_aimrect.setY( event->y() );
        m_aimrect.setHeight( y0 - event->y() );
    }
    //Crop
    if( !m_viewRect.isNull() ) {
        m_selectRect = m_viewRect.intersected( m_aimrect );
        m_Rect.x = ( (qreal)(m_selectRect.x() - m_viewRect.x())/m_viewRect.width() ) * m_cvMat.cols;
        m_Rect.y = ( (qreal)(m_selectRect.y() - m_viewRect.y())/m_viewRect.height() ) * m_cvMat.rows;
        m_Rect.width = ( (qreal)m_selectRect.width()/m_viewRect.width() ) * m_cvMat.cols;
        m_Rect.height = ( (qreal)m_selectRect.height()/m_viewRect.height() ) * m_cvMat.rows;
    }
    emit selectionUpdated( m_Rect );
}
//------------------------------------------------------------------------------------
void QImageWidget::drawString(QPainter &painter, const QRect &input_rect)
{
    QPainterPath path;

    QFont font("Calibry", (qreal)input_rect.height()/26, QFont::Bold);
    QPen pen(Qt::NoBrush, 1.0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    pen.setColor(QColor(0,0,0));
    painter.setPen( pen );
    painter.setBrush(Qt::green);

    path.addText(input_rect.x(), input_rect.y() + font.pointSize(), font, m_string);
    painter.drawPath(path);
    painter.setBrush(Qt::NoBrush);
}
//--------------------------------------------------------------------------------------
void QImageWidget::updateViewRect()
{
    int cols = m_cvMat.cols;
    int rows = m_cvMat.rows;
    if( (cols > 0) && (rows > 0) )
    {
        m_viewRect = rect();
        int width = rect().width();
        int height = rect().height();
        if( ((qreal)cols/rows) > ((qreal)width/height) )
        {
            m_viewRect.setHeight( width * (qreal)rows/cols );
            m_viewRect.moveTop( (height - m_viewRect.height())/2.0 );
        }
        else
        {
            m_viewRect.setWidth( height * (qreal)cols/rows );
            m_viewRect.moveLeft( (width - m_viewRect.width())/2.0 );
        }
    }
    updateSelectRect();
}
//-------------------------------------------------------------------------------------
void QImageWidget::updateSelectRect()
{
    if((m_cvMat.rows > 0) && (m_cvMat.cols > 0))
    {
        m_selectRect.setX( m_viewRect.x() + (m_viewRect.width() * (qreal)m_Rect.x/m_cvMat.cols) );
        m_selectRect.setY( m_viewRect.y() + (m_viewRect.height() * (qreal)m_Rect.y/m_cvMat.rows) );
        m_selectRect.setWidth( (m_viewRect.width() * (qreal)m_Rect.width/m_cvMat.cols) );
        m_selectRect.setHeight( (m_viewRect.height() * (qreal)m_Rect.height/m_cvMat.rows) );
    }
}
//--------------------------------------------------------------------------------------
void QImageWidget::drawSelection(QPainter &painter)
{
    if(!m_selectRect.isNull())
    {
        QPen pen(Qt::NoBrush, 1.0, Qt::DashLine, Qt::RoundCap, Qt::RoundJoin);
        pen.setColor(QColor(0,255,0));
        painter.setPen( pen );
        painter.drawRect(m_selectRect);
    }
}
//--------------------------------------------------------------------------------------


