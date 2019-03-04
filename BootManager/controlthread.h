#ifndef CONTROLTHREAD_H
#define CONTROLTHREAD_H
#include <windows.h>
#include <QtCore/QtCore>

class ControlThreadSign: public QThread
{
    Q_OBJECT
public:
    ControlThreadSign();
    ~ControlThreadSign();
    void getData();

signals:
    void notifyShowLaveOneScreen(uchar* PictureData, int width, int height);
    void notifyShowLaveTwoScreen(uchar* PictureData, int width, int height);

protected:
    void run();
};

class ControlThreadLane: public QThread
{
    Q_OBJECT
public:
    ControlThreadLane();
    ~ControlThreadLane();
    void getData();

signals:
    void notifyShowLaveOneScreen(uchar* PictureData, int width, int height);
    void notifyShowLaveTwoScreen(uchar* PictureData, int width, int height);

protected:
    void run();
};

class ControlThreadRecvCamNotify: public QThread
{
    Q_OBJECT
public:
    ControlThreadRecvCamNotify();
    ~ControlThreadRecvCamNotify();
    void getData();

signals:
    void notifyShowLaveOneScreen(uchar* PictureData, int width, int height);
    void notifyShowLaveTwoScreen(uchar* PictureData, int width, int height);
    void sigDisplayDeviceNumber(const QString&);
    void sigDisplayRecogType(const QString&);
    void sigDisplayGrayValue(int);
    //void sigDisplayExposureTime(const QString &);
    void sigDisplayExposureTime(int);
    void sigDisplayCamVersion(const QString&);

    void sigProcessStartSuccess();
protected:
    void run();
};

class ControlThreadCamera: public QThread
{
    Q_OBJECT
public:
    ControlThreadCamera();
    ~ControlThreadCamera();
    void getData();

    void sendProcessStartCamSig();
    void sendProcessEndCamSig();
public slots:
    void processStartCam();
    void processEndCam();

signals:
    //    void notifyProcessStartCam(uchar ucResult);
    //    void notifyProcessEndCam(uchar ucResult);


    void sigProcessStartFailure();
    void sigProcessEndSuccess();
    void sigProcessEndFailure();
protected:
    void run();

private:
    HANDLE m_hEventStart;
    HANDLE m_hEventEnd;
};


//class ControlIPCComm: public QThread
//{
//    Q_OBJECT
//public:
//    ControlThreadLane();
//    ~ControlThreadLane();
//    void getData();

//signals:
//    void notifyShowLaveOneScreen(uchar* PictureData, int width, int height);
//    void notifyShowLaveTwoScreen(uchar* PictureData, int width, int height);

//protected:
//    void run();
//};

#endif // CONTROLTHREAD_H
