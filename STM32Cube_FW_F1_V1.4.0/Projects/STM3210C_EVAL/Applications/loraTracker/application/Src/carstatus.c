
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "utilities.h"
#include "time.h"
#include "carstatus.h"
#include "debug.h"

int  decision_time;
CarStatus_Typedef carStatus = CAR_STATUS_NONE;



static bool IsTimeOverInSpeed(int speed);
static bool IsTimeOverInOutOfService(int outofservice);

void CarStatus_Init()
{
    carStatus = CAR_STATUS_STARTING;
    decision_time = HAL_GetTick();
}

CarStatus_Typedef getCarStatus(int speed, int outofservice)
{
    CarStatus_Typedef currentCarStatus = carStatus;

    DEBUG(ZONE_FUNCTION, ("+getCarStatus: speed = %d, outofservice = %d\r\n", speed,  outofservice));

    switch (currentCarStatus)
    {
    case CAR_STATUS_DRIVING:
        if(IsTimeOverInSpeed(speed) || IsTimeOverInOutOfService(outofservice))
        {
            carStatus = CAR_STATUS_STOPING;   
            currentCarStatus =  carStatus;
        }
        break;
    // case CAR_STATUS_PARKING:
    //     break;
    case CAR_STATUS_STARTING:
        carStatus = CAR_STATUS_DRIVING;
        break;
    case CAR_STATUS_STOPING:
        if(speed > 0)
        {
            carStatus = CAR_STATUS_DRIVING;   
            currentCarStatus =  CAR_STATUS_STARTING;
        }
        break;
    }

    DEBUG(ZONE_FUNCTION, ("-getCarStatus: current = %d, status = %d, sec=%d\r\n", currentCarStatus,  carStatus, decision_time));
    return currentCarStatus;
}
void updateCarStatus(int speed, int outofservice)
{
    IsTimeOverInSpeed(speed);
    IsTimeOverInOutOfService(outofservice);
}

static bool IsTimeOverInSpeed(int speed)
{
    if(speed == 0) 
    {
        int currentTime = HAL_GetTick();

        // If the speed of the vehicle is zero for TIMEOUTSPEED, it is parked. 
        if((currentTime - decision_time) > TIMEOUTSPEED){
            //decision_time = currentTime;
            return true;
        }
    } 
    else 
    {
        decision_time = HAL_GetTick();
    }

    return false;
}

static bool IsTimeOverInOutOfService(int outofservice)
{
    if(outofservice == 0) 
    {
        int currentTime = HAL_GetTick();

        // If there is a shaded area where no gps data is comming in for TIMEOUTOUTOFSERVICE, the vehicle is parked.
        if((currentTime - decision_time) > TIMEOUTOUTOFSERVICE){
            //decision_time = currentTime;
            return true;
        }
    } 
    else 
    {
        decision_time = HAL_GetTick() ;
    }

    return false;
}