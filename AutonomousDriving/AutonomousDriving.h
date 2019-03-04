#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_AutonomousDriving.h"

class AutonomousDriving : public QMainWindow
{
    Q_OBJECT

public:
    AutonomousDriving(QWidget *parent = Q_NULLPTR);

private:
    Ui::AutonomousDrivingClass ui;
};
