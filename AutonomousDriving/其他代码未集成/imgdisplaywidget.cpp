// imgdisplaywidget.cpp

#include <QtGui>
#include "imgdisplaywidget.h"
#include "commondebug.h"

ImgDisplayWidget::ImgDisplayWidget(QWidget *parent) : QWidget(parent)
{
    m_nScale = 1.0;

    m_pPixmap = new QPixmap();
    m_pLable = new QLabel(this);
}

ImgDisplayWidget::~ImgDisplayWidget()
{
    delete m_pPixmap;
}

void ImgDisplayWidget::onShowScreen(uchar* PictureData, int width, int height)
{
    //COM_DEBUG(5, ("ImgDisplayWidget::onShowScreen, width[%d] height[%d]", width, height));
    QImage* image2 = new QImage(PictureData, width, height, width*3, QImage::Format_RGB888);
    //COM_DEBUG(5, ("ImgDisplayWidget::onShowScreen, image2[%d,%d]", image2->width(), image2->height()));
    QImage image = image2->scaled(this->width(), this->height(), Qt::KeepAspectRatio);
    //QImage image = image2->scaled(width, height, Qt::KeepAspectRatio);
    delete image2;

    m_pPixmap->convertFromImage(image);

    int pictureX = (this->width() - image.width())/2;
    int pictureY = (this->height() - image.height())/2;
    m_pLable->setGeometry(pictureX, pictureY, image.width(), image.height());
    m_pLable->setPixmap(*m_pPixmap);
    COM_DEBUG(5, ("ADC: ImgDisplayWidget::onShowScreen, window[%d,%d] image[%d, %d] lable[(%d, %d) (%d, %d)]",
                  this->width(), this->height(),
                  image.width(), image.height(),
                  pictureX, pictureY, m_pLable->width(), m_pLable->height()));
}

