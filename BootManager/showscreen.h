#ifndef SHOWSCREEN_H
#define SHOWSCREEN_H

#include <QtGui/QtGui>
#include <QtWidgets/QtWidgets>

class ShowScreen : public QWidget
{
    Q_OBJECT
public:
    explicit ShowScreen(QWidget* parent = 0);
    void setImageScale(float scale);
    ~ShowScreen();
signals:

public slots:
    void onShowScreen(uchar* PictureData, int width, int height);

private:
    QLabel* showImageLabel;
    float m_nScale;

protected:
    void mousePressEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    QPoint dragPosition;
};

#endif // SHOWSCREEN_H
