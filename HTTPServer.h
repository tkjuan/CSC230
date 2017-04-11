/************************************************************************/
/*                                                                      */
/*    WebServer.h                                                       */
/*                                                                      */
/*    A chipKIT WiFi Server implementation                              */
/*    Designed to look for and parse /GET commands and automatically    */
/*    handle multiple open TCP connections                              */
/*    This sketch is designed to work with web browsers                 */
/*                                                                      */
/************************************************************************/
/*    Author:     Keith Vogel                                           */
/*    Copyright 2013, Digilent Inc.                                     */
/************************************************************************/
/*
  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/
/************************************************************************/
/*  Revision History:                                                   */
/*    2/1/2013(KeithV): Created                                         */
/************************************************************************/
#if !defined(_WEBSERVER_H)
#define	_WEBSERVER_H

#include	<WProgram.h>
#include	<inttypes.h>
#include    <SD.h>

#include    <NetworkProfile.x>
#include    <DNETcK.h>
#if defined(DWIFIcK_WiFi_Hardware)
#include    <DWIFIcK.h>
#endif // DNETcK_LAN_Hardware

#include    "BoardDefs.h"

#define OFFSETOF(t,m)        ((uint32_t) (&(((t *) 0)->m)))

#define CNTHTTPCMD          10      // The Max number of unique HTML pages we vector off of. This must be at least 1. 
#define secClientTO         3       // time in seconds to wait for a response before aborting a client connection.
#define CBCLILENTINPUTBUFF  256     // The max size of the TCP read buffer, this typically only has to be as large as the URL up to the HTTP tag in the GET line
#define CBCLILENTOUTPUTBUFF 256     // This is scratch output buffer space to generate out going HTML strings it, it is optional if not needed can be set as small as 4;

// these are predefine HTML state machine states, HTTPINIT "must" be implemented, the others can be processed under case default if not needed.
#define HTTPSTART           10000   // This is a predefine state for the rendering HTML page to initialize the state machine 
#define HTTPDISCONNECT      20001   // This state is called after the TCP connection is closed, for whatever reason even timeouts, so the HTML state machine to clean up
#define HTTPTIMEOUT         20002   // If a read timeout occured, your state machine will be called with this timeout value

#define SDREADTIMEOUT 1000          // We have waited too long to read from the SD card in ms

// LED State Machine
// we put all of the global state variables in namespaces so the namespace is completely open in each HTML render file
namespace SLED {

    typedef enum
    {
        PROCESS = 0,    // just do what you were doing
        NOTREADY,       // LED is OFF
        READY,          // LED blinks
        WORKING         // LED is ON
    } STATE;
}

// This are global commands used by the WebServer and ProcessClient state machines.
// again it there own namespace to not consume the names.
namespace GCMD {

    // These are global states that are used by both the HTTP Server and the Process Client code.
    typedef enum
    {
        // These are used mostly by process client to instruct from the
        // HTML page what acition it would like to perform next.
        CONTINUE = 0,   // just call the HTML code again on the next loop
        READ,           // Read more input from the TCP connection
        GETLINE,        // Read the next line of input.
        WRITE,          // iteratively write data to the TCP connection until the buffer has been completely written
        DONE,           // The HTML page is finished and the TCP connection can be closed. 
        ERROR,          // An error occured in the underlying code

        // These are specific to the Server and not used other than a command 
        // passed up to the server code to instruct the server to do something.
        RESTART,        // Restart the network stack
        TERMINATE,      // Terminate the server; spin
        REBOOT,         // Reboot the processor; do a soft MCLR
    } ACTION;
}

typedef GCMD::ACTION (* FNRENDERHTML) (struct CLIENTINFO_T * pClientInfo);

// information that is needed to process the client and ultimately call and render and HTML page
typedef struct CLIENTINFO_T
{
    // If this entry is in use
    bool            fInUse;                         // used only by the HTTP Server code to indicate that this is an active TCP client connection

    // TCP Client state machine varables
    TcpClient       tcpClient;                      // the socket
    uint32_t        clientState;                    // a state machine variable for process client to use
    uint32_t        nextClientState;                // a delayed state variable for process client to use (state specific)
    uint32_t        tStartClient;                   // a timer value used for timeout
    uint32_t        cbRead;                         // number of valid bytes in rgbIn, 
    byte            rgbIn[CBCLILENTINPUTBUFF];      // The input buffer, this is where the /GET and URL will be
    byte            rgbOverflow[4];                 // some overflow space to put characters in while parsing; typically not used.

    // HTML processing variables
    uint32_t        htmlState;                      // a state variable for the HTML web page state machine to use; each page is different
    uint32_t        cbWrite;                        // number of bytes to write out when GCMD::WRITE is returned
    uint32_t        cbWritten;                      // a variable for process client to use to know how many bytes have been written
    byte            rgbOut[CBCLILENTOUTPUTBUFF];    // a per client working scratch output buffer space that can be used if the HTML page/data is being created dynamically. 
    const byte *    pbOut;                          // The actual pointer to the output buffer for ProcessClient to write to the TCP connection. 
                                                    // Typically this is either set to a static string in flash, or to a dynmically created string stored in rgbOut[] or another static buffer in RAM
                                                    // This is usally assigned in the HTML rendering code. If GCMD::WRITE is returned, this pointer must not be NULL.

    // pointer to the HTML page rendering function
    FNRENDERHTML    ComposeHTMLPage;
} CLIENTINFO;

// server/client functions
void ServerSetup(void);
void ProcessServer(void);
GCMD::ACTION ProcessClient(CLIENTINFO * pClientInfo);
GCMD::ACTION JumpToComposeHTMLPage(CLIENTINFO * pClientInfo, FNRENDERHTML FnJumpComposeHTMLPage);

// rendering functions
bool AddHTMLPage(const char * szMatchStr, FNRENDERHTML FnComposeHTMLPage);
void SetDefaultHTMLPage(FNRENDERHTML FnDefaultHTMLPage);

// predefined rendering pages
GCMD::ACTION ComposeHTTP404Error(CLIENTINFO * pClientInfo);
GCMD::ACTION ComposeHTMLRestartPage(CLIENTINFO * pClientInfo);
GCMD::ACTION ComposeHTMLTerminatePage(CLIENTINFO * pClientInfo);
GCMD::ACTION ComposeHTMLRebootPage(CLIENTINFO * pClientInfo);
GCMD::ACTION ComposeHTMLSDPage(CLIENTINFO * pClientInfo);

void SDSetup(void);
uint32_t SDRead(File& fileSD, uint8_t * pbRead, uint32_t cbRead);

// SD variables for external use
extern File fileSD;
extern bool fSDfs;
extern uint32_t sdLockCur;
extern uint32_t sdLock;

// SD Helper functions
#define SDUNLOCKED 0
#define isCardReaderAvailable() (fSDfs == true && sdLockCur == SDUNLOCKED)
#define isMySD(_sdLockMy) (sdLockCur != SDUNLOCKED && sdLockCur == (_sdLockMy))
static inline uint32_t __attribute__((always_inline)) lockSD(void)
{
    if(sdLockCur == SDUNLOCKED) 
    {
        sdLockCur = sdLock;
        return(sdLockCur);
    }
    return(SDUNLOCKED);
}
static inline uint32_t __attribute__((always_inline)) unlockSD(uint32_t sdUnlock)
{
    if(isMySD(sdUnlock)) 
    {
        sdLockCur = SDUNLOCKED;
        sdLock++;
        if(sdLock == SDUNLOCKED) sdLock++;
    }
    return(SDUNLOCKED);
}

// HTTP helper functions
const char * GetContentTypeFromExt(const char * szExt);
uint32_t BuildHTTPOKStr(bool fNoCache, uint32_t cbContentLen, const char * szFile, char * szHTTPOKStr, uint32_t cbHTTPOK);

// Generic Helper functions
void SetLED(SLED::STATE state);
SLED::STATE GetLEDState(void);
int GetDayAndTime(unsigned int epochTimeT, char * szDateTime);
int GetNumb(byte * rgb, int cb, char chDelim, char * sz);
int GetIP(IPv4& ip, char * sz);
int GetMAC(MAC& mac, char * sz);

#endif //	_WEBSERVER_H
