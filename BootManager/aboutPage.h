#ifndef ABOUTPAGE_H
#define ABOUTPAGE_H

#include <QtGui/QtGui>
#include <QtCore/QtCore>
#include <QtWidgets/QtWidgets>

#include "controlthread.h"
#include "showscreen.h"
#include "roadscreen.h"

namespace Ui
{
    class AboutPage;
}

class AboutPage : public QWidget
{
    Q_OBJECT
public:
    AboutPage(QWidget* parent = 0);
    ~AboutPage();
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
    //QLabel *m_CameraCaptureVersionL;
    //QLabel *m_NavigationVersionL;
    //QLabel *m_CameraCanVersionL;
    Ui::AboutPage* ui;
};

#endif // PAGES_H
