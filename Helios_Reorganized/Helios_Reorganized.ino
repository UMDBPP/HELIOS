/*
 * Code currently intended for Mega with custom shield
 */
#define DEBUG_MODE true //this makes the main code ignore the main setup and loop and instead follow an alternative code sequence

//First line printed identifies what each data column is
#define HEADER_STRING "Header string goes here"
#include "include/processes/processes.h"

/*"Starting:\nYear,Month,Day,Hour,Minute,Second,Millisecond,Latitude_deg,Latitude_min,Latitude_dir,Longitude_deg,Longitude_min,Longitude_dir,
    Velocity,Angle,Altitude,Num_Satellites,In_Pressure,In_Temperature,In_Status,In_Pressure_Raw,In_Temperature_Raw,Out_Pressure,Out_Temperature,Out_Status,Out_Pressure_Raw,
    Out_Temperature_Raw,valveHasOpened,Valve_Closed"
*/



//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#if DEBUG_MODE  //if we are in debug mode, skip turning on everything and just follow this abbreviated code

void setup(){ sBench(); }
void loop(){ lBench(); }

#else //if we are not in debug mode, then run normal sequence

void setup(){ sFlight(); }
void loop(){ lFlight(); }

#endif  //this ends the if/else sequence used while debugging

