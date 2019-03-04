
#include "showscreen.h"
#include "safeDelete.h"

ShowScreen::ShowScreen(QWidget* parent) :
    QWidget(parent)
{
    this->setWindowFlags(Qt::FramelessWindowHint);
    showImageLabel = new QLabel(this);
    showImageLabel->setAlignment(Qt::AlignCenter);
    showImageLabel->show();
}

ShowScreen::~ShowScreen()
{
    SAFE_DELETE(showImageLabel);
}

void ShowScreen::onShowScreen(uchar* PictureData, int width, int height)
{
    //QImage* image = new QImage(PictureData, width, height, width*3, QImage::Format_RGB888);
    //if (image->isNull()) {
    //    qDebug("ShowScreen::onShowScreen(%d, %d), QImage is null", width, height);
    //    return;
    //}
    //*image = image->scaled((int)(image->width()*m_nScale), (int)(image->height()*m_nScale), Qt::IgnoreAspectRatio);
    //showImageLabel->resize(image->width(), image->height());
    //int m_pictureX;
    //int m_pictureY;
    //m_pictureX = (this->width() - width)/2;
    //m_pictureY = (this->height() - height)/2;
    //showImageLabel->setGeometry(m_pictureX,m_pictureY,width,height);
    //showImageLabel->setPixmap(QPixmap::fromImage(*image));
    //SAFE_DELETE(image);
    QImage image = QImage(PictureData, width, height, width * 3, QImage::Format_RGB888);
    if (image.isNull())
    {
        qWarning("ShowScreen::onShowScreen(%d, %d), QImage is null", width, height);
        return;
    }
    image = image.scaled((int)(image.width() * m_nScale), (int)(image.height() * m_nScale), Qt::IgnoreAspectRatio);
    showImageLabel->resize(image.width(), image.height());
    int m_pictureX;
    int m_pictureY;
    m_pictureX = (this->width() - width) / 2;
    m_pictureY = (this->height() - height) / 2;
    showImageLabel->setGeometry(m_pictureX, m_pictureY, width, height);
    showImageLabel->setPixmap(QPixmap::fromImage(image));
}

void ShowScreen::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        dragPosition = event->globalPos() - frameGeometry().topLeft();
        event->accept();
    }
}
void ShowScreen::mouseMoveEvent(QMouseEvent* event)
{
    if (event->buttons() & Qt::LeftButton)
    {
        move(event->globalPos() - dragPosition);
        event->accept();
    }
}

void ShowScreen::setImageScale(float scale)
{
    m_nScale = scale;
}
