#include "rootviewer.h"

QWidget* g_pViewer = NULL;    // define the global root viewer


QWidget* getRootViewer()
{
    // return the root viewer
    return g_pViewer;
}


//RootViewer::RootViewer(QWidget *pParent)
//    :QWidget(pParent)
//{
//}


//RootViewer::~RootViewer()
//{
//}
