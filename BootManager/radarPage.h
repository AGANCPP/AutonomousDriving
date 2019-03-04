#ifndef RADARPAGE_H
#define RADARPAGE_H

#include <QtGui/QtGui>
#include <QtCore/QtCore>
#include <QtWidgets/QtWidgets>

#include "controlthread.h"
#include "showscreen.h"
#include "roadscreen.h"

class RadarPage : public QWidget
{
    Q_OBJECT
public:
    RadarPage(QWidget* parent = 0);
    ~RadarPage();
    void addPlainText(const QString& text);

    void init();
    void getInfo();

public slots:
    void onNaviOpenBtnTouched(void);
    void onNaviCloseBtnTouched(void);
    void onLaveOpenBtnTouched(void);
    void onLaveCloseBtnTouched(void);
    void onLogOpenBtnTouched(void);
    void onLogCloseBtnTouched(void);

private:
    QTextEdit* messageShower;
    QPushButton* NaviOpenBtn;
    QPushButton* NaviCloseBtn;
    QPushButton* LaveOpenBtn;
    QPushButton* LaveCloseBtn;
    QPushButton* logOpenBtn;
    QPushButton* logCloseBtn;
    QLineEdit* leftDistanceEditer;
    QLineEdit* rightDistanceEditer;
    QLineEdit* distance_XEditer;
    QLineEdit* distance_YEditer;
    ControlThreadSign* controlThreadSign;
    ControlThreadLane* controlThreadLane;

    //ShowScreen* showNaviScreen;
    ShowScreen* showLaveOneScreen;
    ShowScreen* showLaveTwoScreen;
    RoadScreen* showRoadScreen;

    QProcess* m_pProcessCameraCan;
    QProcess* m_pProcessNavi;

    QProcess* m_pProcessVechileControl;

    int m_nNaviWidth;
    int m_nNaviHeight;
    int m_nLaneWidth;
    int m_nLaneHeight;
    int m_nSignWidth;
    int m_nSignHeight;
    int m_nRoadWidth;
    int m_nRoadHeight;

    int m_nScreenWidth;
    int m_nScreenHeight;

    int g_nActScreenW;
    int g_nActScreenH;

    float m_nScaleW;
    float m_nScaleH;
};

#endif // PAGES_H
