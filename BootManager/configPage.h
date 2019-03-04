#ifndef CONFIGPAGE_H
#define CONFIGPAGE_H

#include <QtGui/QtGui>
#include <QtCore/QtCore>
#include <QtWidgets/QtWidgets>

#include "controlthread.h"
#include "showscreen.h"
#include "roadscreen.h"
#include "safeDelete.h"

#define CONFIG_DEF_REGION_COUNTRY 0
#define CONFIG_DEF_REGION_AREA 1

#define CONFIG_REGION_COUNTRY_JPN 0
#define CONFIG_REGION_COUNTRY_CHN 1

#define CONFIG_REGION_AREA_JPN_00 0   //QYD_1
#define CONFIG_REGION_AREA_JPN_01 1   //QYD_2
#define CONFIG_REGION_AREA_JPN_02 2   //SDG   //no map now
#define CONFIG_REGION_AREA_JPN_03 3   //BSG
#define CONFIG_REGION_AREA_JPN_04 4   //3rdJB
#define CONFIG_REGION_AREA_JPN_05 5   //OPG

#define CONFIG_REGION_AREA_CHN_00 0
#define CONFIG_REGION_AREA_CHN_01 1

namespace Ui
{
    class ConfigPage;
}

class ConfigPage : public QWidget
{
    Q_OBJECT
public:
    ConfigPage(QWidget* parent = 0);
    ~ConfigPage();
    void addPlainText(const QString& text);

    void init();
    void getInfo();

public slots:
    void onNaviMapSet1BtnTouched(void);
    void onNaviMapSet2BtnTouched(void);
    void onNaviMapSet3BtnTouched(void);
    void onNaviMapSet4BtnTouched(void);
    void onNaviMapSet5BtnTouched(void);
    void onNaviMapSet6BtnTouched(void);

private:
    Ui::ConfigPage* ui;
    int nRegionCountry;
    int nRegionArea;
    QString strProcessBatArg[2][6];

    void naviMapSet();
    bool naviMapSetFileCheck(void);

};

#endif // PAGES_H
