/****************************************************************************/
/// @file    app-commands-subscriptions-constants.h
/// @author
/// Constant values to be shared between the iCS and the Applications.
/****************************************************************************/
// iTETRIS, see http://www.ict-itetris.eu
// Copyright © 2008 iTetris Project Consortium - All rights reserved
/****************************************************************************/

#ifndef APP_COMM_SUBS_CONSTANTS_H
#define APP_COMM_SUBS_CONSTANTS_H

// ===========================================================================
// included modules
// ===========================================================================
#ifdef _MSC_VER
#include <windows_config.h>
#else
#include <config.h>
#endif

// ****************************************
// COMMANDS & SUBSCRIPTIONS
// ****************************************

// command: close connection
#define CMD_APP_CLOSE 0xFF

// subscription: Return cars in a zone
#define SUB_RETURNS_CARS_IN_ZONE 0x02

// command: Send a message to a station.
#define CMD_SEND_MESSAGE_TO_STATION 0x03

// subscription: Request received messages status
#define SUB_REQ_RECEIVED_MESSAGES_STATUS 0x05

// command: Notify Application to execute
#define CMD_NOTIFY_APP_EXECUTE 0x06

// command: End of Application Execution
#define CMD_NOTIFY_APP_END_EXECUTE 0x07

// command End of Subscription
#define CMD_END_SUBSCRIPTION 0x08

// command Checks Applications readiness
#define CMD_APP_READY 0x09

// command Ask for subscriptions
#define CMD_ASK_FOR_SUBSCRIPTION 0x0A

// command End of subscription requests
#define CMD_END_SUBSCRIPTION_REQUEST 0x0B

// command Drop the subscription
#define CMD_DROP_SUBSCRIPTION 0x0C

// command Renew the subscription
#define CMD_RENEW_SUBSCRIPTION 0x0D

// command Sends returns cars in zone subscription data
#define CMD_CARS_IN_ZONE 0x0E

// subscription Set area to send CAM
#define SUB_SET_CAM_AREA 0x11

// notify app of arrived messages
#define CMD_NOTIFY_APP_MESSAGE_STATUS 0x14

// subscription start travel time estimation
#define SUB_TRAVEL_TIME_ESTIMATION_START 0x12

// subscription end travel time estimation
#define SUB_TRAVEL_TIME_ESTIMATION_END 0x13

// subscription to monitor the travel time estimation
#define SUB_TRAVEL_TIME_ESTIMATION 0x15

// subscription to monitor the travel time estimation
#define CMD_TRAVEL_TIME_ESTIMATION 0x20

// subscription get received cam info
#define SUB_RECEIVED_CAM_INFO 0x18

// command Sends returns received cam info data
#define CMD_RECEIVED_CAM_INFO 0x19

// ****************************************
// OUTPUT OF APPLICATIONS
// Where the result of the application should be applied
// ****************************************

// Set maximum speed for a vehicle
#define OUTPUT_SET_VEHICLE_MAX_SPEED 0x04

// Calculate the travel time
#define OUTPUT_TRAVEL_TIME_ESTIMATION 0x21

// Traffic jam detection
#define OUTPUT_TRAFFIC_JAM_DETECTION 0x23

// Application without result
#define OUTPUT_VOID 0x22

// reply: The application executed its algorithm //15
#define APP_RESULT_ON 0x0F

// reply: The application DID NOT execute //16
#define APP_RESULT_OFF 0x10

// ****************************************
// RESULT TYPES
// ****************************************

// result type: Ok
#define APP_RTYPE_OK    0x01
// result type: not implemented
#define APP_RTYPE_NOTIMPLEMENTED  0x02
// result type: error
#define APP_RTYPE_ERR   0xFF

// ****************************************
// MESSAGE STATUS
// ****************************************

// result : to schedule in ns-3
#define APP_RESULT_TO_SCHEDULE    0x15
// result : to be applied in traffic simulator
#define APP_RESULT_TO_APPLY  0x16
// result : discard it (do not apply)
#define APP_RESULT_TO_DISCARD   0x17

#endif
