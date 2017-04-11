/************************************************************************/
/*                                                                      */
/*    HTMLSample.cpp                                                    */
/*                                                                      */
/*    Renders a simple dynamically generated HTML page                  */
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
// Required header
#include    <HTTPServer.h>

/************************************************************************/
/*    HTML Strings                                                      */
/************************************************************************/
// the sample HTML page as a string constant
static const char szSample[] = 
"<head>\r\n\
<title> HTTP Sample </title>\r\n\
</head>\r\n\
<body>\r\n\
This is a simple HTML sample page that was dynamically rendered.\r\n\
<br>\r\n\
</body>\r\n";

/************************************************************************/
/*    State machine states                                              */
/************************************************************************/

/*
    There are 3 magic predefined states:

        HTTPSTART: 
            This is a required state to have.
            This is always the starting state and the first state
            in your compose function.

        HTTPDISCONNECT:
            Not a required state to have, but this state will be called whenever
            your conneciton is closed. This will always be called even if you
            return GCMD::DONE requesting the connection to be closed. This is
            nice because you can do clean up steps here, because no matter how
            the connection was closed, you get a chance to clean up.

        HTTPTIMEOUT:
            This is not a required state to have.
            This is called if the connection timed out, that is, the connection was
            made, your compose function got called but some pending data did not come in.
            For example if you returned a GCMD::GETLINE yet a line never came in.
*/

// these are local to this file, and we can duplicate the names
// in other files as the compiler will not see these in other files.
// These are your local state machine states.
typedef enum {
    WRITECONTENT,
    DONE
} STATE;

/***    GCMD::ACTION ComposeHTMLSamplePage(CLIENTINFO * pClientInfo)
 *
 *    Parameters:
 *          pClientInfo - the client info representing this connection and web page
 *              
 *    Return Values:
 *          GCMD::ACTION    - GCMD::CONTINUE, just return with no outside action
 *                          - GCMD::READ, non-blocking read of input data into the rgbIn buffer appended to the end of rgbIn[] which has a predefined size of 256 bytes
 *                              when we return to this compose function cbRead will have the number of bytes read, and likely could be zero.
 *                          - GCMD::GETLINE, blocking read until a line of input is read or until the rgbIn buffer is full, always the line starts at the beginnig of the rgbIn
 *                              cbRead has the number of bytes read
 *                          - GCMD::WRITE, loop writing until all cbWrite bytes are written from the pbOut buffer
 *                              pbOut can point to any valid buffer that will remain unchanged until execution returns to this function. We could get a TIMOUT
 *                              if we can't write the data. cbWritten will have the number of bytes actually written. As part of each connection there is a 
 *                              scratch buffer of 256 provide at rgbOut; it is optional to point pbOut to rgbOut. PbOut can point anywhere and that is what will be written
 *                              cbWrite must be set to the number of bytes to write.
 *                          - GCMD::DONE, we are done processing and the connection can be closed
 *
 *    Description: 
 *    
 *      This composes the sample HTML page 
 *    
 * ------------------------------------------------------------ */
GCMD::ACTION ComposeHTMLSamplePage(CLIENTINFO * pClientInfo)
{
    GCMD::ACTION retCMD = GCMD::DONE;

    // a word of caution... DO NOT cast htmlState to your enum type!
    // the compiler will silently remove the HTTPSTART case as 
    // that state is not part of your enum. Keep the switch on typed
    // aginst the generic uint32_t.
    switch(pClientInfo->htmlState)
    {

        // Every Compose function will start at the magic HTTPSTART state
        // we MUST support this state.
        case HTTPSTART:
            Serial.println("Sample Page Detected");

            // here we make an HTTP header directive for an HTML (.htm) MIME type; our action for this state will be to write out the HTTP header
            pClientInfo->cbWrite = BuildHTTPOKStr(false, sizeof(szSample)-1, ".htm", (char *) pClientInfo->rgbOut, sizeof(pClientInfo->rgbOut));
            pClientInfo->pbOut = pClientInfo->rgbOut;

            // say we want to write cbWrite bytes from pbOut
            retCMD = GCMD::WRITE;

            // after writing our HTTP directive, return to this compose functions
            // and execute the WRITECONTENT state
            pClientInfo->htmlState = WRITECONTENT;

            break;

        // Now we write our simple HTML page out
        // do this by setting pbOut to the string or bytes we
        // want to write out on the connection, and set the size in cbWrite
         case WRITECONTENT:

            // set our write pointer and size
            pClientInfo->pbOut = (const byte *) szSample;

            // do not want to include the null terminator in the size
            // we could use strlen, but here I can have the compiler calculate 
            // the size as a constant; less the null terminator
            pClientInfo->cbWrite = sizeof(szSample)-1;

            // say we want to write cbWrite bytes from pbOut
            retCMD = GCMD::WRITE;

            // after writing the body of our HTML page, return to this compose functions
            // and execute the DONE state
            pClientInfo->htmlState = DONE;             
            break;
    
        // the done state is were we say we are done, and that
        // the connection can be closed
        case DONE:
        default:

            // by returning DONE, we will close the connection
            // and be done with this page
            retCMD = GCMD::DONE;
            break;
    }

    // Return the command we want to do
    // like WRITE, or DONE
    return(retCMD);
}