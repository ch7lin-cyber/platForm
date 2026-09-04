
/***************************************************************
Description : 
	This is a user alarm program application.


------------------------------------------------------------------------------------------------------------------------------------------
Change notice:

Date-> 2026/05/13
[ADD] 1. The first version sets up. 

[MODIFY] 1. The first version sets up. 

[DELETE] 1. The first version sets up. 

**************************************************************************************/


#include "../../ssm_std_FB_lib.h"


//------------------------------------------------------------------------------------//
// C++ compatibility // DO NOT DELETE
#ifdef __cplusplus
extern "C" {
#endif
//------------------------------------------------------------------------------------//

/*-----------------------------------------------------------------------------------------------------------------------------------------
FB Description :
        This is the user alarm main function

Interface Parameters:
[-in-]
APP_ALARM_FB_T *fb
int16_t PV
int16_t SV
int16_t Mode
int16_t AlarmH
int16_t AlarmL
int16_t *Clear
uint8_t FunctionOption

[-out-]
APP_ALARM_FB_OUT_T *out			  // return result

----------------------------------------------------------------------------------------*/
MY_API uint16_t app_fb_user_alarm( APP_ALARM_FB_T *fb, int16_t PV, int16_t SV, int16_t Mode,
									int16_t AlarmH, int16_t AlarmL, int16_t *Clear, uint8_t FunctionOption,
									APP_ALARM_FB_OUT_T *out)

{
    bool alarm = false;

    bool standby = (FunctionOption & ALARM_OPT_STANDBY) != 0;
    bool invert = (FunctionOption & ALARM_OPT_INVERT) != 0;
    bool hold = (FunctionOption & ALARM_OPT_HOLD) != 0;
    bool peak = (FunctionOption & ALARM_OPT_PEAK) != 0;

    /****************************************************
     Clear
    ****************************************************/

    if ((*Clear) & ALARM_CLEAR_RESET)
    {
        fb->alarmLatch = false;

        (*Clear) &= ~ALARM_CLEAR_RESET;
    }

    if ((*Clear) & ALARM_CLEAR_PEAK)
    {
        fb->peakH = PV;
        fb->peakL = PV;

        (*Clear) &= ~ALARM_CLEAR_PEAK;
    }

    /****************************************************
     Peak Record
    ****************************************************/

    if (peak)
    {
        if (fb->peakH < PV)
            fb->peakH = PV;

        if (fb->peakL > PV)
            fb->peakL = PV;
    }

    /****************************************************
     Standby
    ****************************************************/

    if (standby)
    {
        if (!fb->standbyReady)
        {
            if ((PV >= (SV - 10)) &&
                (PV <= (SV + 10)))
            {
                fb->standbyReady = true;
            }
        }
    }
    else
    {
        fb->standbyReady = true;
    }

    if (!fb->standbyReady)
    {
        Mode = ALARM_MODE_DISABLE;
    }

    /****************************************************
     Alarm Detect
    ****************************************************/

    switch (Mode)
    {
        case ALARM_MODE_DISABLE:

            alarm = false;

            break;

        case ALARM_MODE_REL_HL:

            alarm =
                (PV > (SV + AlarmH)) ||
                (PV < (SV - AlarmL));

            break;

        case ALARM_MODE_REL_H:

            alarm =
                (PV > (SV + AlarmH));

            break;

        case ALARM_MODE_REL_L:

            alarm =
                (PV < (SV - AlarmL));

            break;

        case ALARM_MODE_ABS_HL:

            alarm =
                (PV > AlarmH) ||
                (PV < AlarmL);

            break;

        case ALARM_MODE_ABS_H:

            alarm =
                (PV > AlarmH);

            break;

        case ALARM_MODE_ABS_L:

            alarm =
                (PV < AlarmL);

            break;

        case ALARM_MODE_HYS_H:

            if (fb->alarmLatch)
            {
                if (PV < (SV + AlarmL))
                {
                    fb->alarmLatch = false;
                }
            }
            else
            {
                if (PV > (SV + AlarmH))
                {
                    fb->alarmLatch = true;
                }
            }

            alarm = fb->alarmLatch;

            break;

        case ALARM_MODE_HYS_L:

            if (fb->alarmLatch)
            {
                if (PV > (SV - AlarmL))
                {
                    fb->alarmLatch = false;
                }
            }
            else
            {
                if (PV < (SV - AlarmH))
                {
                    fb->alarmLatch = true;
                }
            }

            alarm = fb->alarmLatch;

            break;

        default:

            alarm = false;

            break;
    }

    /****************************************************
     Hold
    ****************************************************/

    if (hold == 1)
    {
        if (alarm){
            fb->alarmLatch = true;
        }

        alarm = fb->alarmLatch;
    }

    /****************************************************
     Invert
    ****************************************************/

    if (invert) {
        alarm = !alarm;
    }

    /****************************************************
     Output
    ****************************************************/

    out->AlarmOutput = alarm ? 1 : 0;

    out->PeakH = fb->peakH;
    out->PeakL = fb->peakL;

	return  1 ;
}




//------------------------------------------------------------------------------------//
// C++ compatibility
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------------//


