/**
  ******************************************************************************
  * @file    filter32.h
  * @author  Wang Hongxi
  * @version V1.0.0
  * @date    2020/3/17
  * @brief   
  ******************************************************************************
  * @attention 
  *
  ******************************************************************************
  */
#ifndef __FILTER32_H
#define __FILTER32_H

#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "main.h"
#include "cmsis_os.h"

#ifdef _CMSIS_OS_H
#define user_malloc pvPortMalloc
#else
#define user_malloc malloc
#endif

#if (__CORTEX_M == (4U))

typedef __packed struct
{
    float Input;        //输入数据
    float Output;       //滤波输出的数据
    float RC;           //滤波参数 RC = 1/omegac
    float Frame_Period; //滤波的时间间隔 单位 s
} First_Order_Filter_t;

typedef __packed struct window_filter
{
    float Input;         //输入数据
    float Output;        //滤波输出的数据
    uint8_t WindowSize;  //窗口大小
    uint8_t WindowNum;   //需要更新的窗口值
    float *WindowBuffer; //窗口数据缓冲区
} Window_Filter_t;

typedef __packed struct
{
    float Input;   //输入数据
    float Output;  //滤波输出的数据
    uint8_t Order; //滤波器阶数
    float *Num;    //Numerator
    float *Den;    //Denominator
    float *xbuf;
    float *ybuf;
} IIR_Filter_t;

void First_Order_Filter_Init(First_Order_Filter_t *first_order_filter, float frame_period, float num);
float First_Order_Filter_Calculate(First_Order_Filter_t *first_order_filter, float input);
void Window_Filter_Init(Window_Filter_t *window_filter, uint8_t windowSize);
float Window_Filter_Calculate(Window_Filter_t *window_filter, float input);
void IIR_Filter_Init(IIR_Filter_t *iir_filter, float *num, float *den, uint8_t order);
float IIR_Filter_Calculate(IIR_Filter_t *iir_filter, float input);
#endif

#endif
