

#include <QtGui>
#include "showcaminfo.h"
#include "commondebug.h"


ShowCamInfo::ShowCamInfo(QWidget* parent) : QWidget(parent)
{
    m_nScale = 1.0;
    m_pPixmapSign = new QPixmap();
    m_pLableShowSign = new QLabel(this);
    m_pPixmapLane = new QPixmap();
    m_pLableShowLane = new QLabel(this);
    m_pLabelShowRunningInfo = new QLabel(this);
    QFont font1;
    font1.setFamily(QString::fromUtf8("Verdana"));
    font1.setPointSize(9);
    m_pLabelShowRunningInfo->setFont(font1);
    m_pLabelShowRunningInfo->setAutoFillBackground(true);
    onShowCamRunningInfo("Camera stopped", 1);
}

ShowCamInfo::~ShowCamInfo()
{
    delete m_pPixmapSign;
    delete m_pLableShowLane;
}

void ShowCamInfo::resizeEvent(QResizeEvent*)
{
    m_pLabelShowRunningInfo->setGeometry(0, 720, 480, height() - 720);
}

void ShowCamInfo::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    QPixmap pixmap(":/image/bg.png");
    //painter.drawPixmap(pixmap.width() * 0.5, pixmap.height() * 0.2, pixmap.width() * 0.5, pixmap.height() * 0.8, pixmap);
    //painter.drawPixmap(0, 0, width(), height(), pixmap, pixmap.width() * 0.5, pixmap.height() * 0.2, pixmap.width() * 0.5, pixmap.height() * 0.8);
    painter.drawPixmap(0, 0, width(), height(), pixmap, pixmap.width() * 0.3, pixmap.height() * 0.1, pixmap.width() * 0.5, pixmap.height() * 0.6);
}

//glTexCoord2d(0.3f, 0.1f);   // 左下
//glVertex2d(0, height()); // 左下

//glTexCoord2d(0.8f, 0.1f);   // 右下
//glVertex2d(width(),height()); // 右下

//glTexCoord2d(0.8f, 0.7f);   // 右上
//glVertex2d(width(),0); // 右上

//glTexCoord2d(0.3f, 0.7f);   // 左上
//glVertex2d(0, 0); // 左上

void ShowCamInfo::onShowSignImg(uchar* PictureData, int width, int height)
{
    //COM_DEBUG(5, ("ImgDisplayWidget::onShowScreen, width[%d] height[%d]", width, height));
    QImage* image2 = new QImage(PictureData, width, height, width * 3, QImage::Format_RGB888);
    //COM_DEBUG(5, ("ImgDisplayWidget::onShowScreen, image2[%d,%d]", image2->width(), image2->height()));
    QImage image = image2->scaled(480, 360, Qt::KeepAspectRatio);
    //QImage image = image2->scaled(width, height, Qt::KeepAspectRatio);
    delete image2;
    m_pPixmapSign->convertFromImage(image);
    int pictureX = 0;
    int pictureY = 0;
    m_pLableShowSign->setGeometry(pictureX, pictureY, image.width(), image.height());
    m_pLableShowSign->setPixmap(*m_pPixmapSign);
    COM_DEBUG(5, ("ADC: ShowCamInfo::onShowSignImg, window[%d,%d] image[%d, %d] lable[(%d, %d) (%d, %d)]",
                  this->width(), this->height(),
                  image.width(), image.height(),
                  pictureX, pictureY, m_pLableShowSign->width(), m_pLableShowSign->height()));
}

void ShowCamInfo::onShowLaneImg(uchar* PictureData, int width, int height)
{
    //COM_DEBUG(5, ("ImgDisplayWidget::onShowScreen, width[%d] height[%d]", width, height));
    QImage* image2 = new QImage(PictureData, width, height, width * 3, QImage::Format_RGB888);
    //COM_DEBUG(5, ("ImgDisplayWidget::onShowScreen, image2[%d,%d]", image2->width(), image2->height()));
    QImage image = image2->scaled(480, 360, Qt::KeepAspectRatio);
    //QImage image = image2->scaled(width, height, Qt::KeepAspectRatio);
    delete image2;
    m_pPixmapLane->convertFromImage(image);
    int pictureX = 0;
    int pictureY = 360;
    m_pLableShowLane->setGeometry(pictureX, pictureY, image.width(), image.height());
    m_pLableShowLane->setPixmap(*m_pPixmapLane);
    COM_DEBUG(5, ("ADC: ShowCamInfo::onShowSignImg, window[%d,%d] image[%d, %d] lable[(%d, %d) (%d, %d)]",
                  this->width(), this->height(),
                  image.width(), image.height(),
                  pictureX, pictureY, m_pLableShowLane->width(), m_pLableShowLane->height()));
}

void ShowCamInfo::onShowCamRunningInfo(const QString& info, int level)
{
    QPalette palette;
    palette.setColor(QPalette::WindowText, QColor(255, 255, 255));
    if (0 == level)
        palette.setColor(QPalette::Background, QColor(255, 0, 0, 127));
    else if (1 == level)
        palette.setColor(QPalette::Background, QColor(255, 255, 0, 127));
    else
    {
        //palette.setColor(QPalette::Background, QColor(0, 255, 0, 127));
        palette.setColor(QPalette::Background, QColor(82, 128, 233, 127));
    }
    m_pLabelShowRunningInfo->setPalette(palette);
    m_pLabelShowRunningInfo->setText(info);
}

