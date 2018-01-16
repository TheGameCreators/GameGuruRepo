;#ifndef __MESSAGES_H__
;#define __MESSAGES_H__
;

LanguageNames =
    (
        English = 0x0409:Messages_ENU
    )

;////////////////////////////////////////
;// Eventlog categories
;//
;// Categories always have to be the first entries in a message file!
;//

MessageId       = 1
SymbolicName    = CATEGORY_ONE
Severity        = Success
Language        = English
Logo Compliance Error
.

;////////////////////////////////////////
;// Events
;//

MessageId       = +1
SymbolicName    = EVENT_ERROR_NOREMOTESESSION
Severity        = Error
Language        = English
This application is not designed to run over fast user switching or as a remote session. An attempt may have been made to run this application from a Remote Desktop or via Fast User Switching. You can only run this application locally.
.

;////////////////////////////////////////
;// Additional messages
;//

MessageId       = 1000
SymbolicName    = IDS_HELLOWORLD
Language        = English
Hello World!
.

MessageId       = +1
SymbolicName    = IDS_HELLOTGC
Language        = English
Hello TGC!
.

;
;#endif  //__MESSAGES_H__
;