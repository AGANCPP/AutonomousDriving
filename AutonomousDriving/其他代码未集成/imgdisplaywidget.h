#ifndef IMGDISPLAYWIDGET_H
#define IMGDISPLAYWIDGET_H

#include <QWidget>

class QPixmap;
class QLabel;
class ImgDisplayWidget : public QWidget
{
    Q_OBJECT

public:
    ImgDisplayWidget(QWidget *parent = 0);
    ~ImgDisplayWidget();

public slots:
    void onShowScreen(uchar* PictureData, int width, int height);

private:
    float m_nScale;
    QPixmap* m_pPixmap;
    QLabel* m_pLable;
};

#endif // IMGDISPLAYWIDGET_H
