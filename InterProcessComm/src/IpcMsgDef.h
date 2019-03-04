
#ifndef IPC_MESSAGE_DEFINE
#define IPC_MESSAGE_DEFINE

typedef struct _CAM_IMAGE_TYPE
{
    enum { IMAGE_SIZE = 1600 * 1200 * 3 };
    enum
    {
        IMAGE_TYPE_INVALID,
        IMAGE_TYPE_SIGN,
        IMAGE_TYPE_LANE,
        IMAGE_TYPE_ROAD,
    };
    enum
    {
        IMAGE_FORMAT_INVALID,
        IMAGE_FORMAT_RGB888,
    };

    int image_type;
    int cols;
    int rows;
    int step;
    int format;
    size_t imgSize;
    unsigned char data[IMAGE_SIZE];

} CAM_IMAGE_TYPE;

enum
{
    // camera messages
    MSG_CAM_CMD_START_DEBUG,
    MSG_CAM_CMD_END_WORK,
    MSG_CAM_CMD_START_CAPTURE,
    MSG_CAM_CMD_END_CAPTURE,
    MSG_CAM_CMD_SET_LOG_PATH,
    MSG_CAM_CMD_START_LOG,
    MSG_CAM_CMD_END_LOG,
    MSG_CAM_CMD_SET_IMG_PATH,
    MSG_CAM_CMD_START_SAVE_IMG,
    MSG_CAM_CMD_END_SAVE_IMG,
    MSG_CAM_CMD_SET_GRAY_VALUE,
    MSG_CAM_CMD_SET_EXPOSURE_TIME,
    MSG_CAM_CMD_ENABLE_AUTO_EXPOSURE,
    MSG_CAM_CMD_ENABLE_CALIBRATION,
    MSG_CAM_CMD_FULL,
    MSG_VEH_CMD_FIND_WHEEL_CTRL_PORT,
    MSG_VEH_CMD_OPEN_WHEEL_CTRL,
    MSG_VEH_CMD_OPEN_CAN_CTRL,
    MSG_VEH_CMD_CLOSE_WHEEL_CTRL,
    MSG_VEH_CMD_CLOSE_CAN_CTRL,
    MSG_VEH_CMD_START_CAN_RECV,
    MSG_VEH_CMD_END_WORK,
    MSG_VEH_CMD_TURN_WHEEL,
    MSG_VEH_CMD_BRAKE,
};

enum
{
    // camera messages
    MSG_CAM_NTY_PROCESS_START,
    MSG_CAM_NTY_WORK_START,
    MSG_CAM_NTY_IS_FINDING_DEVICE,
    MSG_CAM_NTY_FINDED_DEVICE_NUM,
    MSG_CAM_NTY_RECOG_TYPE,
    MSG_CAM_NTY_GRAY_VALUE,
    MSG_CAM_NTY_EXPOSURE_TIME,
    MSG_CAM_NTY_VERSION,
    // vehicle control messages
    MSG_VEH_NTY_WORK_START,
    MSG_VEH_NTY_ENUM_WHEEL_CTRL_PORT,
};

typedef struct _IPC_MSG_TYPE
{
    enum { MAX_CHAR_SIZE = 256 };
    int msgId;

    union
    {
        char logPath[MAX_CHAR_SIZE];
        char imgPath[MAX_CHAR_SIZE];
        char version[MAX_CHAR_SIZE];
        int cameraNum;
        union
        {
            int recogType;
            int grayValue;
            int exposureTime;
            int enableAutoExposure;
            int enableCalibration;
            int enableFullShow;
        } cam;

        struct
        {
            int direction;
            int speed;
            struct
            {
                unsigned char addr;
                char port[16];
            } port;
            struct
            {
                int all;
                int index;
                char port[16];
            } enum_port;
        } wheel;
        struct
        {
            int strength;
        } brake;
    } msg;
} IPC_MSG_TYPE;

typedef struct _CAM_RES_DATA
{
    int result;
    int errCode;
} IPC_RES_DATA;

#endif 



