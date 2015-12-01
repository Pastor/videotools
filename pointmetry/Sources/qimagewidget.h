/*--------------------------------------------------------------------------------------------
Taranov Alex, 2014                              		                   HEADER FILE
The Develnoter [internet], Marcelo Mottalli, 2014
This class is a descendant of QWidget with QImageWidget::updateImage(...) slot that constructs
QImage instance from cv::Mat image. The QImageWidget should be used as widget for video display  
 * ------------------------------------------------------------------------------------------*/

#ifndef QIMAGEWIDGET_H
#define QIMAGEWIDGET_H
//------------------------------------------------------------------------------------------------------
#ifdef OPENGL_WIDGETS
    #include <QOpenGLWidget>
    #include <QSurfaceFormat>
#else
    #include <QWidget>
#endif
#include <QImage>
#include <QPen>
#include <QPainter>
#include <QPainterPath>
#include <QColor>
#include <QMouseEvent>

#include "opencv2/opencv.hpp"
//------------------------------------------------------------------------------------------------------

class QImageWidget : public WIDGET_CLASS
{
    Q_OBJECT

public:
    explicit QImageWidget(QWidget *parent = 0);
signals:
    void selectionUpdated(const cv::Rect &value);
public slots:
    void updateImage(const cv::Mat &image); // takes cv::Mat image and converts it to the appropriate Qt QImage format
    void updateImage(const cv::Mat &image, float *pointer, uint length);
    void setNumbersVisualization(bool value);

private:
    QImage m_qimage;        // stores current QImage instance
    cv::Mat m_cvMat;        // stores current cv::Mat instance
    QString m_string;       // stores text information that will be drawn on widget in paintEvent
    bool f_select;     // this flag should enable/disable a drawing of the aimrect on a widget
    int x0;                 // mouse coordinate X
    int y0;                 // mouse coordinate Y
    QRect m_aimrect;        // stores current rectangle region on widget
    QRect m_viewRect;       // a rect inside widget's rect with image proportion of dimensions
    QRect m_selectRect;  // a rect that stores user's last selection on m_viewRect
    cv::Rect m_Rect;        // a rect that stores coordinates of selected region on m_cvMat
    float *v_points;
    uint  m_points;

    void updateViewRect();  // calculates m_viewRect by means of m_cvMat dimensions proportion
    void updateSelectRect(); //recalculate coordiantes of m_selectRect after widget rescale

    void drawString(QPainter &painter);
    void drawSelection(QPainter &painter);
    void drawPoints(QPainter &painter);

    bool f_numbers;

protected:
    void paintEvent(QPaintEvent *);
    void mousePressEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent *event);
};

//------------------------------------------------------------------------------------------------------
#endif // QIMAGEWIDGET_H
