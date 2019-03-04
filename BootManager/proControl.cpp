#include "proControl.h"
#include "controlPage.h"
#include "radarPage.h"
#include "configPage.h"
#include "aboutPage.h"
#include "safeDelete.h"
#include "systemlog.h"


CProControl::CProControl(QWidget *parent)
    :QWidget(parent)
{
    this->setWindowFlags(Qt::FramelessWindowHint);
    this->setAttribute(Qt::WA_DeleteOnClose,true);
    contentsWidget = new QListWidget;
    contentsWidget->setViewMode(QListView::IconMode);
    contentsWidget->setIconSize(QSize(60, 70));
    contentsWidget->setMovement(QListView::Static);
    contentsWidget->setMinimumWidth(80);
    contentsWidget->setMaximumWidth(80);
    contentsWidget->setMinimumHeight(480);
    contentsWidget->setMaximumHeight(480);
    contentsWidget->setSpacing(10);

    contentsWidget->setStyleSheet("QListWidget{background:#23456C}");

    //Read config file for debug version
    QFileInfo fConfigIniFileInfo(CONFIG_INI_FILE);
    m_ucDebugType = 0;
    if(fConfigIniFileInfo.exists())
    {
        QSettings iniSettings(QString(CONFIG_INI_FILE), QSettings::IniFormat);
        m_ucDebugType = iniSettings.value("Debug/DebugType").toInt();
    }

    pagesWidget = new QStackedWidget;
    pagesWidget->addWidget(new ControlPage);
    pagesWidget->addWidget(new ConfigPage);
    if(0 != m_ucDebugType)
    {
        pagesWidget->addWidget(new RadarPage);
    }
    pagesWidget->addWidget(new AboutPage);

    QFont font1;
    font1.setFamily(QString::fromUtf8("Verdana"));
    font1.setPointSize(12);

    QPushButton *closeButton = new QPushButton(tr("Close"));
    closeButton->setStyleSheet("QPushButton{color:white;background:#23456C}");
    closeButton->setMinimumHeight(30);
    closeButton->setFont(font1);
    connect(closeButton, SIGNAL(clicked()), this, SLOT(closeAll()));
    createIcons();
    contentsWidget->setCurrentRow(0);

    QHBoxLayout *horizontalLayout = new QHBoxLayout;
    horizontalLayout->addWidget(contentsWidget);
    horizontalLayout->addWidget(pagesWidget, 1);

    QHBoxLayout *buttonsLayout = new QHBoxLayout;
    buttonsLayout->addStretch(1);
    buttonsLayout->addWidget(closeButton);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(horizontalLayout);
    mainLayout->addStretch(1);
    mainLayout->addSpacing(5);
    mainLayout->addLayout(buttonsLayout);
    setLayout(mainLayout);

    this->setWindowTitle(tr("BootManager"));
    QPalette palette2;
    QBrush brush2(QColor(19, 39, 63, 255));
    palette2.setBrush(QPalette::All, QPalette::Window, brush2);
    this->setPalette(palette2);
    //this->setStyleSheet("QWidget{background:#23456C}");
//    setStyleSheet("QWidget{color:white;background:#13273F}");

}

CProControl::~CProControl()
{
    qDebug("~CProControl");
    SAFE_DELETE(contentsWidget);
    SAFE_DELETE(pagesWidget);

}

void CProControl::createIcons()
{
    QListWidgetItem *configButton = new QListWidgetItem(contentsWidget);
    configButton->setIcon(QIcon("./image/icon_control.png"));
    configButton->setText(tr("Control"));
    configButton->setTextColor(Qt::white);
    configButton->setTextAlignment(Qt::AlignHCenter);
    configButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

    QListWidgetItem *updateButton = new QListWidgetItem(contentsWidget);
    updateButton->setIcon(QIcon("./image/icon_update.png"));
    updateButton->setText(tr("Config"));
    updateButton->setTextColor(Qt::white);
    updateButton->setTextAlignment(Qt::AlignHCenter);
    updateButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);


    if(0 != m_ucDebugType)
    {
        QListWidgetItem *updateButton = new QListWidgetItem(contentsWidget);
        updateButton->setIcon(QIcon("./image/icon_update.png"));
        updateButton->setText(tr("Update"));
        updateButton->setTextColor(Qt::white);
        updateButton->setTextAlignment(Qt::AlignHCenter);
        updateButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    }

    QListWidgetItem *queryButton = new QListWidgetItem(contentsWidget);
    queryButton->setIcon(QIcon("./image/icon_query.png"));
    queryButton->setText(tr("About"));
    queryButton->setTextColor(Qt::white);
    queryButton->setTextAlignment(Qt::AlignHCenter);
    queryButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

    contentsWidget->setStyleSheet("QListWidget{background:#23456C}");
    connect(contentsWidget,
            SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)),
            this, SLOT(changePage(QListWidgetItem*,QListWidgetItem*)));
}

void CProControl::changePage(QListWidgetItem *current, QListWidgetItem *previous)
{
    if (!current)
        current = previous;

    pagesWidget->setCurrentIndex(contentsWidget->row(current));
}

void CProControl::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        dragPosition = event->globalPos() - frameGeometry().topLeft();
        event->accept();
    }
}
void CProControl::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton)
    {
        move(event->globalPos() - dragPosition);
        event->accept();
    }
}


void CProControl::closeAll()
{
    if(NAVI_OPENED == navi_status)
    {
        emit notifyCloseNavi();
    }
    //emit notifyCloseAll();
    emit notifyCloseCam();
    close();
}

