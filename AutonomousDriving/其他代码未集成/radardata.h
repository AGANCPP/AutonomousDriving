#ifndef RADARDATA_H
#define RADARDATA_H

typedef struct FRONT_RADAR_INFO_
{
    enum { MAX_OBJ_COUNT = 8 };
    float distance[MAX_OBJ_COUNT];  // Ã×
    float xLocation[MAX_OBJ_COUNT];
    float relSpeed[MAX_OBJ_COUNT];
} FRONT_RADAR_INFO;

typedef struct BACK_RADAR_INFO_
{
    enum { MAX_OBJ_COUNT = 10 };
    unsigned int leftDistance[MAX_OBJ_COUNT];   // ·ÖÃ×
    unsigned int rightDistance[MAX_OBJ_COUNT];
} BACK_RADAR_INFO;

typedef struct IBEO_OBJECT_INFO_
{
    char       flag;
    char       objectID;
    char       classification;
    char       classificationAge;
    float      centerpt_x;
    float      centerpt_y;
    float      boundingbox_x;
    float      boundingbox_y;
    float      boundingCenter_x;
    float      boundingCenter_y;
} IBEO_OBJECT_INFO;

typedef struct IBEO_INFO_
{
    enum { MAX_OBJECTS = 30 };
    IBEO_OBJECT_INFO ibeoObjects[MAX_OBJECTS];
} IBEO_INFO;

#endif // RADARDATA_H
