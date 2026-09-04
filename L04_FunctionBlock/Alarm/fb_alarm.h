/***************************************************************
Description : 
	This is a user alarm program application header.

	author: CH.
	
	modify :
			0. setup first version, ch@2026/05/23
			1. change ..........

	version: V0001
	

***************************************************************/
#ifndef SSM_STD_FB_APP_USER_ALARM_CODE_H_
#define SSM_STD_FB_APP_USER_ALARM_CODE_H_

#include "../../ssm_std_define.h"



//------------------------------------------------------------------------------------//
// C++ compatibility  // DO NOT DELETE
#ifdef __cplusplus
extern "C" {
#endif
//------------------------------------------------------------------------------------//
//export your program for others' use.

/*----------------------------------------------------------
 Clear Bit
----------------------------------------------------------*/
#define ALARM_CLEAR_RESET     (1U << 0)    /* Clear Alarm Hold */
#define ALARM_CLEAR_PEAK      (1U << 1)    /* Clear Peak Record */

/*----------------------------------------------------------
 Function Option Bit
----------------------------------------------------------*/
#define ALARM_OPT_STANDBY     (1U << 0)
#define ALARM_OPT_INVERT      (1U << 1)
#define ALARM_OPT_HOLD        (1U << 2)
#define ALARM_OPT_PEAK        (1U << 3)

/*----------------------------------------------------------
 Alarm Mode
----------------------------------------------------------*/
typedef enum
{
    ALARM_MODE_DISABLE = 0,
    ALARM_MODE_REL_HL,
    ALARM_MODE_REL_H,
    ALARM_MODE_REL_L,
    ALARM_MODE_ABS_HL,
    ALARM_MODE_ABS_H,
    ALARM_MODE_ABS_L,
    ALARM_MODE_HYS_H,
    ALARM_MODE_HYS_L

}APP_ALARM_MODE_E;

typedef struct
{
    bool alarmLatch;
    bool standbyReady;
    int16_t peakH;
    int16_t peakL;

}APP_ALARM_FB_T;

typedef struct
{
    int16_t AlarmOutput;
    int16_t PeakH;
    int16_t PeakL;

}APP_ALARM_FB_OUT_T;




MY_API uint16_t app_fb_user_alarm(APP_ALARM_FB_T *fb, int16_t PV, int16_t SV, int16_t Mode,
								  int16_t AlarmH, int16_t AlarmL, int16_t *Clear, uint8_t FunctionOption,
								  APP_ALARM_FB_OUT_T *out);


//------------------------------------------------------------------------------------//
// C++ compatibility
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------------//
#endif  // SSM_STD_FB_APP_USER_ALARM_CODE_H




