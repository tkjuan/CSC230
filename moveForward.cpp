/************************************************************************/
/*                                                                      */
/*    forward.cpp                                                 */
/*                                                                      */
/*    Renders the HTML terminate page                                   */
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
#include    <HTTPServer.h>

/************************************************************************/
/*    HTML Strings                                                      */
/************************************************************************/


/************************************************************************/
/*    State machine states                                              */
/************************************************************************/
typedef enum {
    WRITECONTENT,
    DONE
} STATE;

/***    GCMD::ACTION ComposeHTMLTerminatePage(CLIENTINFO * pClientInfo)
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
 *      Renders the server terminate HTML page 
 *    
 * ------------------------------------------------------------ */
 const int PIN = 8;
const int PIN2 = 9;
const int PIN3 = 10;
const int PIN4 = 11; 
const int PIN5 = 12;
const int PIN6 = 13;


const int rwPeriod = 10;
int rwTon = 5;

static const char rwNav[] = 
"<head>\r\n\
<title> Robot Movement Page </title>\r\n\
</head>\r\n\
<body>\r\n\
<button id=\'forward' onclick=\"window.location.href=\'forward\'\">forward</button>\r\n\
<button onclick=\"window.location.href=\'left\'\">left</button>\r\n\
<button onclick=\"window.location.href=\'right\'\">right</button>\r\n\
<button onclick=\"window.location.href=\'back\'\">back</button>\r\n\
<button onclick=\"window.location.href=\'full\'\">Full Power</button>\r\n\
<button onclick=\"window.location.href=\'half\'\">Half Power</button>\r\n\
<br>\r\n\
</body>\r\n\
<script>\r\n\
</script>\r\n\
<style>\r\n\
</style>\r\n";

void run(int loops) {
  int rwI = 0;
  while(rwI < loops) {
            digitalWrite(PIN5, HIGH);
            digitalWrite(PIN6, HIGH);  
            delay(rwTon);
            digitalWrite(PIN5, LOW);
            digitalWrite(PIN6, LOW);      
            delay(rwPeriod - rwTon);
            
            rwI++;
          }   
}

GCMD::ACTION moveForward(CLIENTINFO * pClientInfo)
{
GCMD::ACTION retCMD = GCMD::DONE;  
   switch(pClientInfo->htmlState)
    {
         case HTTPSTART:
            pClientInfo->cbWrite = BuildHTTPOKStr(true, sizeof(rwNav)-1, ".htm", (char *) pClientInfo->rgbOut, sizeof(pClientInfo->rgbOut));
            pClientInfo->pbOut = pClientInfo->rgbOut;
            pClientInfo->htmlState = WRITECONTENT;
            retCMD = GCMD::WRITE;
            break;

         case WRITECONTENT:
             pClientInfo->pbOut = (const byte *) rwNav;
             pClientInfo->cbWrite = sizeof(rwNav)-1;
             pClientInfo->htmlState = DONE;
             retCMD = GCMD::WRITE;
             break;

        case DONE:

            // Foward Motion
            digitalWrite(PIN, HIGH);
            digitalWrite(PIN2, LOW);  
            digitalWrite(PIN3, HIGH);
            digitalWrite(PIN4, LOW);
          run(100);
            digitalWrite(PIN, LOW);
            digitalWrite(PIN3, LOW);
        default:
            retCMD = GCMD::DONE;
            break;
    }
  
  return(retCMD);
}

GCMD::ACTION setPowerFull(CLIENTINFO * pClientInfo)
{
  
  GCMD::ACTION retCMD = GCMD::DONE;
  
  switch(pClientInfo->htmlState)
    {
  case HTTPSTART:
            pClientInfo->cbWrite = BuildHTTPOKStr(true, sizeof(rwNav)-1, ".htm", (char *) pClientInfo->rgbOut, sizeof(pClientInfo->rgbOut));
            pClientInfo->pbOut = pClientInfo->rgbOut;
            pClientInfo->htmlState = WRITECONTENT;
            retCMD = GCMD::WRITE;
            break;

         case WRITECONTENT:
             pClientInfo->pbOut = (const byte *) rwNav;
             pClientInfo->cbWrite = sizeof(rwNav)-1;
             pClientInfo->htmlState = DONE;
             retCMD = GCMD::WRITE;
             break;

        case DONE:
             rwTon = 10;
    
        default:
            retCMD = GCMD::DONE;
            break;
    }
      return(retCMD);
}

GCMD::ACTION setPowerHalf(CLIENTINFO * pClientInfo)
{
  GCMD::ACTION retCMD = GCMD::DONE;
  
  switch(pClientInfo->htmlState)
    {
    case HTTPSTART:
            pClientInfo->cbWrite = BuildHTTPOKStr(true, sizeof(rwNav)-1, ".htm", (char *) pClientInfo->rgbOut, sizeof(pClientInfo->rgbOut));
            pClientInfo->pbOut = pClientInfo->rgbOut;
            pClientInfo->htmlState = WRITECONTENT;
            retCMD = GCMD::WRITE;
            break;

         case WRITECONTENT:
             pClientInfo->pbOut = (const byte *) rwNav;
             pClientInfo->cbWrite = sizeof(rwNav)-1;
             pClientInfo->htmlState = DONE;
             retCMD = GCMD::WRITE;
             break;

        case DONE:
             rwTon = 5;
         
        default:
            retCMD = GCMD::DONE;
            break;
    }
      return(retCMD);
}


GCMD::ACTION moveBack(CLIENTINFO * pClientInfo)
{

GCMD::ACTION retCMD = GCMD::DONE;  

   switch(pClientInfo->htmlState)
    {
         case HTTPSTART:
            pClientInfo->cbWrite = BuildHTTPOKStr(true, sizeof(rwNav)-1, ".htm", (char *) pClientInfo->rgbOut, sizeof(pClientInfo->rgbOut));
            pClientInfo->pbOut = pClientInfo->rgbOut;
            pClientInfo->htmlState = WRITECONTENT;
            retCMD = GCMD::WRITE;
            break;

         case WRITECONTENT:
             pClientInfo->pbOut = (const byte *) rwNav;
             pClientInfo->cbWrite = sizeof(rwNav)-1;
             pClientInfo->htmlState = DONE;
             retCMD = GCMD::WRITE;
             break;

        case DONE:
        
            digitalWrite(PIN, LOW);
            digitalWrite(PIN2, HIGH);  
            digitalWrite(PIN3, LOW);
            digitalWrite(PIN4, HIGH);
    
            run(100);
    
            digitalWrite(PIN2, LOW);
            digitalWrite(PIN4, LOW);
        default:
            retCMD = GCMD::DONE;
            break;
    }
  
  return(retCMD);
}


GCMD::ACTION moveLeft(CLIENTINFO * pClientInfo)
{
GCMD::ACTION retCMD = GCMD::DONE;  

   switch(pClientInfo->htmlState)
    {
         case HTTPSTART:
            pClientInfo->cbWrite = BuildHTTPOKStr(true, sizeof(rwNav)-1, ".htm", (char *) pClientInfo->rgbOut, sizeof(pClientInfo->rgbOut));
            pClientInfo->pbOut = pClientInfo->rgbOut;
            pClientInfo->htmlState = WRITECONTENT;
            retCMD = GCMD::WRITE;
            break;

         case WRITECONTENT:
             pClientInfo->pbOut = (const byte *) rwNav;
             pClientInfo->cbWrite = sizeof(rwNav)-1;
             pClientInfo->htmlState = DONE;
             retCMD = GCMD::WRITE;
             break;

        case DONE:
            digitalWrite(PIN, LOW);
            digitalWrite(PIN2, HIGH);
    
            digitalWrite(PIN3, HIGH);
            digitalWrite(PIN4, LOW);
    
            run(15);
    
            digitalWrite(PIN2, LOW);
            digitalWrite(PIN3, LOW);
        default:
            retCMD = GCMD::DONE;
            break;
    }
  
  return(retCMD);
}


GCMD::ACTION moveRight(CLIENTINFO * pClientInfo)
{

  
GCMD::ACTION retCMD = GCMD::DONE;  

   switch(pClientInfo->htmlState)
    {
         case HTTPSTART:
            pClientInfo->cbWrite = BuildHTTPOKStr(true, sizeof(rwNav)-1, ".htm", (char *) pClientInfo->rgbOut, sizeof(pClientInfo->rgbOut));
            pClientInfo->pbOut = pClientInfo->rgbOut;
            pClientInfo->htmlState = WRITECONTENT;
            retCMD = GCMD::WRITE;
            break;

         case WRITECONTENT:
             pClientInfo->pbOut = (const byte *) rwNav;
             pClientInfo->cbWrite = sizeof(rwNav)-1;
             pClientInfo->htmlState = DONE;
             retCMD = GCMD::WRITE;
             break;

        case DONE:
            digitalWrite(PIN, HIGH);
            digitalWrite(PIN2, LOW);
    
            digitalWrite(PIN3, LOW);
            digitalWrite(PIN4, HIGH);

            run(15);
    
            digitalWrite(PIN, LOW);
            digitalWrite(PIN4, LOW);
        default:
            retCMD = GCMD::DONE;
            break;
    }
  
  return(retCMD);
}


