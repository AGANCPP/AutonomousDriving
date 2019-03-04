#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_BootManager.h"

class BootManager : public QMainWindow
{
    Q_OBJECT

public:
    BootManager(QWidget *parent = Q_NULLPTR);

private:
    Ui::BootManagerClass ui;
};
