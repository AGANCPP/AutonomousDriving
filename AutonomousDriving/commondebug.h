#ifndef COMMONDEBUG_H
#define COMMONDEBUG_H

#include <QtGui>

#define COM_DEBUG(level, arg)			\
    if(level <= 5) {					\
        qDebug arg ;	                \
    }

#define COM_WARN qDebug
#define COM_ERR  qDebug

#endif // COMMONDEBUG_H
