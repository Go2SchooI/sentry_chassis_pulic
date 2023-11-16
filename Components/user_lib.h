#ifndef _USER_LIB_H
#define _USER_LIB_H
#include "stdint.h"
#include "main.h"
#include "cmsis_os.h"

enum
{
    CHASSIS_DEBUG = 1,
    GIMBAL_DEBUG,
    INS_DEBUG,
    RC_DEBUG,
    IMU_HEAT_DEBUG,
    SHOOT_DEBUG,
    AIMASSIST_DEBUG,
};

extern uint8_t GlobalDebugMode;

#ifdef _CMSIS_OS_H
#define user_malloc pvPortMalloc
#else
#define user_malloc malloc
#endif

/* boolean type definitions */
#ifndef TRUE
#define TRUE 1 /**< boolean true  */
#endif

#ifndef FALSE
#define FALSE 0 /**< boolean fails */
#endif

/* math relevant */
/* radian coefficient */
#ifndef RADIAN_COEF
#define RADIAN_COEF 57.295779513f
#endif

/* circumference ratio */
#ifndef PI
#define PI 3.14159265354f
#endif

#define VAL_LIMIT(val, min, max) \
    do                           \
    {                            \
        if ((val) <= (min))      \
        {                        \
            (val) = (min);       \
        }                        \
        else if ((val) >= (max)) \
        {                        \
            (val) = (max);       \
        }                        \
    } while (0)

#define ANGLE_LIMIT_360(val, angle)     \
    do                                  \
    {                                   \
        (val) = (angle) - (int)(angle); \
        (val) += (int)(angle) % 360;    \
    } while (0)

#define ANGLE_LIMIT_360_TO_180(val) \
    do                              \
    {                               \
        if ((val) > 180)            \
            (val) -= 360;           \
    } while (0)

#define VAL_MIN(a, b) ((a) < (b) ? (a) : (b))
#define VAL_MAX(a, b) ((a) > (b) ? (a) : (b))

typedef struct
{
    float input;        //????????
    float out;          //???????
    float min_value;    //?????��?
    float max_value;    //???????
    float frame_period; //?????
} ramp_function_source_t;

typedef __packed struct
{
    uint16_t Order;
    uint32_t Count;

    float *x;
    float *y;

    float k;
    float b;

    float StandardDeviation;

    float t[4];
} Ordinary_Least_Squares_t;

//???????
float Sqrt(float x);
//???????????????
float invSqrt(float num);

// ��???????????
void ramp_init(ramp_function_source_t *ramp_source_type, float frame_period, float max, float min);
// ��??????????
float ramp_calc(ramp_function_source_t *ramp_source_type, float input);

//????????
float abs_limit(float num, float Limit);
//?��????��
float sign(float value);
//????????
float float_deadband(float Value, float minValue, float maxValue);
// int26????
int16_t int16_deadband(int16_t Value, int16_t minValue, int16_t maxValue);
//???????
float float_constrain(float Value, float minValue, float maxValue);
//???????
int16_t int16_constrain(int16_t Value, int16_t minValue, int16_t maxValue);
//??????????
float loop_float_constrain(float Input, float minValue, float maxValue);
//??? ????? 180 ~ -180
uint8_t loop_int_constrain(uint8_t Input, int8_t minValue, int8_t maxValue);
float theta_format(float Ang);

int float_rounding(float raw);

//?????????-PI~PI
#define rad_format(Ang) loop_float_constrain((Ang), -PI, PI)

void OLS_Init(Ordinary_Least_Squares_t *OLS, uint16_t order);
void OLS_Update(Ordinary_Least_Squares_t *OLS, float deltax, float y);
float OLS_Derivative(Ordinary_Least_Squares_t *OLS, float deltax, float y);
float OLS_Smooth(Ordinary_Least_Squares_t *OLS, float deltax, float y);
float Get_OLS_Derivative(Ordinary_Least_Squares_t *OLS);
float Get_OLS_Smooth(Ordinary_Least_Squares_t *OLS);
float Data_mapping(float intput, float intput_min, float intput_max, float map_min, float map_max);
#endif
