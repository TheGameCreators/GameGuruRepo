#ifndef __MESSAGES_H__
#define __MESSAGES_H__

////////////////////////////////////////
// Eventlog categories
//
// Categories always have to be the first entries in a message file!
//
//
//  Values are 32 bit values laid out as follows:
//
//   3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
//   1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
//  +---+-+-+-----------------------+-------------------------------+
//  |Sev|C|R|     Facility          |               Code            |
//  +---+-+-+-----------------------+-------------------------------+
//
//  where
//
//      Sev - is the severity code
//
//          00 - Success
//          01 - Informational
//          10 - Warning
//          11 - Error
//
//      C - is the Customer code flag
//
//      R - is a reserved bit
//
//      Facility - is the facility code
//
//      Code - is the facility's status code
//
//
// Define the facility codes
//


//
// Define the severity codes
//


//
// MessageId: CATEGORY_ONE
//
// MessageText:
//
// Logo Compliance Error
//
#define CATEGORY_ONE                     0x00000001L

////////////////////////////////////////
// Events
//
//
// MessageId: EVENT_ERROR_NOREMOTESESSION
//
// MessageText:
//
// This application is not designed to run over fast user switching or as a remote session. An attempt may have been made to run this application from a Remote Desktop or via Fast User Switching. You can only run this application locally.
//
#define EVENT_ERROR_NOREMOTESESSION      0xC0000002L

////////////////////////////////////////
// Additional messages
//
//
// MessageId: IDS_HELLOWORLD
//
// MessageText:
//
// Hello World!
//
#define IDS_HELLOWORLD                   0xC00003E8L

//
// MessageId: IDS_HELLOTGC
//
// MessageText:
//
// Hello TGC!
//
#define IDS_HELLOTGC                     0xC00003E9L


#endif  //__MESSAGES_H__
