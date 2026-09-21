#ifndef _headfile_h
#define _headfile_h

#define CONFIG_MSPM0G350X

#if defined(__ti_version__) || defined(__TI_COMPILER_VERSION__)
#define SYSCONFIG_WEAK __attribute__((weak))
#elif defined(__IAR_SYSTEMS_ICC__)
#define SYSCONFIG_WEAK __weak
#elif defined(__GNUC__)
#define SYSCONFIG_WEAK __attribute__((weak))
#endif

// 系统相关
#include <ti/devices/msp/msp.h>
#include <ti/driverlib/driverlib.h>
#include <ti/driverlib/m0p/dl_core.h>
#include <ti/devices/msp/m0p/mspm0g350x.h>
#include <stdio.h>
#include "ti_msp_dl_config.h"

#include "ml_delay.h"
#include "ml_system.h"

// 驱动相关
#include "ml_gpio.h"
#include "ml_tim.h"
#include "ml_uart.h"
#include "ml_pwm.h"
#include "ml_oled.h"
#include "ml_i2c.h"
#include "ml_mpu6050.h"
#include "ml_exti.h"
#include "ml_motor.h"
#include "encoder.h"
#include "seriacl.h"
#include "track.h"
#include "buzzer_light.h"
#include "xunji.h"
#include "_pid.h"
#include "key.h"

// 应用相关
#include "pid.h"
#include "IMU.h"
#include "string.h"

#endif