/************************************************************************/
/*                                                                      */
/*    HTMLReboot.cpp                                                    */
/*                                                                      */
/*    Renders the HTML reboot page                                      */
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
/*    7/24/2013(KeithV): Created                                         */
/************************************************************************/
#include    <HTTPServer.h>

/************************************************************************/
/*    HTML Strings                                                      */
/************************************************************************/
static const char szReboot[] = 
"<head>\r\n\
<title> HTTP Reboot </title>\r\n\
</head>\r\n\
<body>\r\n\
Server Rebooting\r\n\
<br>\r\n\
</body>\r\n";

/************************************************************************/
/*    State machine states                                              */
/************************************************************************/
typedef enum {
    WRITECONTENT,
    DONE
} STATE;

/***    GCMD::ACTION ComposeHTMLRestartPage(CLIENTINFO * pClientInfo)
 *
 *    Parameters:
 *          pClientInfo - the client info representing this connection and web page
 *              
 *    Return Values:
 *          GCMD::ACTION    - GCMD::CONTINUE, just return with no outside action
 *                          - GCMD::READ, non-blocking read of input data into the rgbIn buffer appended to the end of cbRead
 *                          - GCMD::GETLINE, blocking read until a line of input is read or until the rgbIn buffer is full, always the line starts at the beginnig of the rgbIn
 *                          - GCMD::WRITE, loop writing until all cbWrite bytes are written from the pbOut buffer
 *                          - GCMD::DONE, we are done processing and the connection can be closed
 *
 *    Description: 
 *    
 *      Renders the server restart HTML page 
 *    
 * ------------------------------------------------------------ */
GCMD::ACTION ComposeHTMLRebootPage(CLIENTINFO * pClientInfo)
{

   GCMD::ACTION retCMD = GCMD::WRITE;

    switch(pClientInfo->htmlState)
    {
         case HTTPSTART:
            Serial.println("Reboot Request Detected");
            pClientInfo->cbWrite = BuildHTTPOKStr(true, sizeof(szReboot)-1, ".htm", (char *) pClientInfo->rgbOut, sizeof(pClientInfo->rgbOut));
            pClientInfo->pbOut = pClientInfo->rgbOut;
            pClientInfo->htmlState = WRITECONTENT;
            break;

         case WRITECONTENT:
             pClientInfo->pbOut = (const byte *) szReboot;
             pClientInfo->cbWrite = sizeof(szReboot)-1;
             pClientInfo->htmlState = DONE;
             break;

        case DONE:
        default:
            pClientInfo->cbWrite = 0;
            retCMD = GCMD::REBOOT;
            break;
    }

    return(retCMD);
}