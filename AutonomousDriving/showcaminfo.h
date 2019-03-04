#ifndef SHOWCAMINFO_H
#define SHOWCAMINFO_H

#include <QWidget>
#include <QtWidgets/QLabel>
#include <QtGui/QPixmap>

class ShowCamInfo : public QWidget
{
    Q_OBJECT

public:
    ShowCamInfo(QWidget* parent = 0);
    ~ShowCamInfo();

public slots:
    void onShowSignImg(uchar* PictureData, int width, int height);
    void onShowLaneImg(uchar* PictureData, int width, int height);
    void onShowCamRunningInfo(const QString& info, int level);

protected:
    void resizeEvent(QResizeEvent*);
    void paintEvent(QPaintEvent*);

private:
    float m_nScale;
    QPixmap* m_pPixmapSign;
    QLabel* m_pLableShowSign;
    QPixmap* m_pPixmapLane;
    QLabel* m_pLableShowLane;
    // –≈œ¢¿∏
    QLabel* m_pLabelShowRunningInfo;
};

#endif // SHOWCAMINFO_H
