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
#include    <robotControl.h>
/************************************************************************/
/*    HTML Strings                                                      */
/************************************************************************/
// Basic HTTP OK response.  Additional header lines are there to ensure browser doesn't cache AJAX requests.  
// POST requests shouldn't be cached in the browser by default
static const char szHTTPOK[] = "HTTP/1.1 200 OK\r\nCache-Control: no-store, no-cache, must-revalidate, post-check=0, pre-check=0\r\nPragma: no-cache\r\n Expires: Thu, 19 Nov 1981 08:52:00 GMT\r\n";

/************************************************************************/
/*    State machine states                                              */
/************************************************************************/
typedef enum {
    WRITECONTENT,
    DONE
} STATE;
/*
/************************************************************************/
/*    Robot Global Values                                               */
/************************************************************************/
const int PIN = 8;
const int PIN2 = 9;
const int PIN3 = 10;
const int PIN4 = 11; 
const int PIN5 = 12;
const int PIN6 = 7;
const int rwPeriod = 10;
int rwTon = 5;
/************************************************************************/
/*    Function Calls                                                    */
/************************************************************************/


/*  ComposeHTTP200
 *    This function sends a basic blank HTTP OK message
 *    This is needed for the POST requests as the WebServer
 *    State machine and AJAX request requires a response
 *    to be sent
 */

GCMD::ACTION ComposeHTTP200(CLIENTINFO * pClientInfo)
{     
    GCMD::ACTION retCMD = GCMD::CONTINUE;
    switch(pClientInfo->htmlState)
    {
         case HTTPSTART:
            pClientInfo->cbWrite = BuildHTTPOKStr(true, sizeof(szHTTPOK)-1, ".htm", (char *) pClientInfo->rgbOut, sizeof(pClientInfo->rgbOut));
            pClientInfo->pbOut = pClientInfo->rgbOut;
            pClientInfo->htmlState = WRITECONTENT;
            retCMD = GCMD::WRITE;
            break;

         case WRITECONTENT:
             pClientInfo->pbOut = (const byte *) szHTTPOK;
             pClientInfo->cbWrite = sizeof(szHTTPOK)-1;
             pClientInfo->htmlState = DONE;
             retCMD = GCMD::WRITE;
             break;

        case DONE:
        default:
            retCMD = GCMD::DONE;
            break;
    }
  
  return(retCMD);
}




static const char rwNav[] = "<head><title> Robot Movement Page </title></head><body><div id='navButtons'><div><button id='forward' onclick=\"window.location.href=\'forward\'\">forward</button></div><div><button id='left' onclick=\"window.location.href=\'left\'\">left</button><button id='right' onclick=\"window.location.href=\'right\'\">right</button></div><div><button id='back' onclick=\"window.location.href=\'back\'\">back</button></div><div><button id='half' onclick=\"window.location.href=\'half\'\">half</button><button id='full' onclick=\"window.location.href=\'full\'\">full</button></div></div>";


/*  run(int Loops)
 *  This function runs the motors for a set number of periods
 *  The period length is defined by rwPeriod
 *  The Power level is determined by rwTon
 * configured by software PWM  (rwTon / rwPeriod)
 */

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

/* Run()
 *  Turns on Enable Pins
 */
void run() {  
  digitalWrite(PIN5, HIGH);
  digitalWrite(PIN6, HIGH);
}


GCMD::ACTION moveForward(CLIENTINFO * pClientInfo){
  
   GCMD::ACTION retCMD = GCMD::DONE;  
   
   switch(pClientInfo->htmlState) {
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

    // Set the Directional Pins
    case DONE:
      // Foward Motion
      digitalWrite(PIN, HIGH);
      digitalWrite(PIN2, LOW);  
      digitalWrite(PIN3, HIGH);
      digitalWrite(PIN4, LOW);
      run(100);
      // Set Directional Pins to Zero
      digitalWrite(PIN, LOW);
      digitalWrite(PIN3, LOW);
    default:
      retCMD = GCMD::DONE;
      break;
  }
  return(retCMD);
}

GCMD::ACTION setPowerFull(CLIENTINFO * pClientInfo) {
  
  GCMD::ACTION retCMD = GCMD::DONE;

  
  switch(pClientInfo->htmlState) {
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

GCMD::ACTION setPowerHalf(CLIENTINFO * pClientInfo) {
  
  GCMD::ACTION retCMD = GCMD::DONE;
  
  switch(pClientInfo->htmlState) {
    case HTTPSTART:
      pClientInfo->cbWrite = BuildHTTPOKStr(true, sizeof(rwNav)-1, ".htm", (char *) pClientInfo->rgbOut, sizeof(pClientInfo->rgbOut));
      pClientInfo->pbOut = pClientInfo->rgbOut;
      pClientInfo->htmlState = WRITECONTENT;
      retCMD = GCMD::WRITE;
      break;
            
    // Send the page to the client
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


GCMD::ACTION moveBack(CLIENTINFO * pClientInfo) {

  GCMD::ACTION retCMD = GCMD::DONE;  

  switch(pClientInfo->htmlState) {
    // Write the page to the buffer
    case HTTPSTART:
      pClientInfo->cbWrite = BuildHTTPOKStr(true, sizeof(rwNav)-1, ".htm", (char *) pClientInfo->rgbOut, sizeof(pClientInfo->rgbOut));
      pClientInfo->pbOut = pClientInfo->rgbOut;
      pClientInfo->htmlState = WRITECONTENT;
      retCMD = GCMD::WRITE;
      break;
      
    // Send the page to the client
    case WRITECONTENT:
      pClientInfo->pbOut = (const byte *) rwNav;
      pClientInfo->cbWrite = sizeof(rwNav)-1;
      pClientInfo->htmlState = DONE;
      retCMD = GCMD::WRITE;
      break;

    // Set the Directional Pins
    case DONE:    
      digitalWrite(PIN, LOW);
      digitalWrite(PIN2, HIGH);  
      digitalWrite(PIN3, LOW);
      digitalWrite(PIN4, HIGH);
      run(100);
      // Set Directional Pins to Zero
      digitalWrite(PIN2, LOW);
      digitalWrite(PIN4, LOW);
    default:
      retCMD = GCMD::DONE;
      break;
  }
  return(retCMD);
}


GCMD::ACTION moveLeft(CLIENTINFO * pClientInfo) {
  
  GCMD::ACTION retCMD = GCMD::DONE;  
  
  switch(pClientInfo->htmlState) {
    // Write the page to the buffer
    case HTTPSTART:
      pClientInfo->cbWrite = BuildHTTPOKStr(true, sizeof(rwNav)-1, ".htm", (char *) pClientInfo->rgbOut, sizeof(pClientInfo->rgbOut));
      pClientInfo->pbOut = pClientInfo->rgbOut;
      pClientInfo->htmlState = WRITECONTENT;
      retCMD = GCMD::WRITE;
      break;

    // Send the page to the client
    case WRITECONTENT:
      pClientInfo->pbOut = (const byte *) rwNav;
      pClientInfo->cbWrite = sizeof(rwNav)-1;
      pClientInfo->htmlState = DONE;
      retCMD = GCMD::WRITE;
      break;
       
    // Set the Directional Pins
    case DONE:
      digitalWrite(PIN, LOW);
      digitalWrite(PIN2, HIGH);
      digitalWrite(PIN3, HIGH);
      digitalWrite(PIN4, LOW);
      run(15);
      // Set Directional Pins to Zero
      digitalWrite(PIN2, LOW);
      digitalWrite(PIN3, LOW);
    default:
      retCMD = GCMD::DONE;
      break;
  }
  return(retCMD);
}


GCMD::ACTION moveRight(CLIENTINFO * pClientInfo) {
 
  GCMD::ACTION retCMD = GCMD::DONE;  

  switch(pClientInfo->htmlState) {
    // Write the page to the buffer
    case HTTPSTART:
      pClientInfo->cbWrite = BuildHTTPOKStr(true, sizeof(rwNav)-1, ".htm", (char *) pClientInfo->rgbOut, sizeof(pClientInfo->rgbOut));
      pClientInfo->pbOut = pClientInfo->rgbOut;
      pClientInfo->htmlState = WRITECONTENT;
      retCMD = GCMD::WRITE;
      break;
      
    // Send the page to the client
    case WRITECONTENT:
      pClientInfo->pbOut = (const byte *) rwNav;
      pClientInfo->cbWrite = sizeof(rwNav)-1;
      pClientInfo->htmlState = DONE;
      retCMD = GCMD::WRITE;
      break;
      
    // Set the Directional Pins
    case DONE:
      digitalWrite(PIN, HIGH);
      digitalWrite(PIN2, LOW);
      digitalWrite(PIN3, LOW);
      digitalWrite(PIN4, HIGH);
      run(15);
      // Set Directional Pins to Zero
      digitalWrite(PIN, LOW);
      digitalWrite(PIN4, LOW);
      
    default:
      retCMD = GCMD::DONE;
      break;
  }
  return(retCMD);
}


/*
 * POST Request
 */
 typedef enum {
    CONTLEN,
    ENDHDR,
    DATA,
    GETPAGE,
    FINISH
} STATEPost;

static CLIENTINFO * pClientMutex    = NULL;
static uint32_t cbContentLenght = 0;
static const char szContentLength[] = "Content-Length: ";
static uint32_t iIn = 0;
static uint32_t cParsed = 0;

GCMD::ACTION moveCommand(CLIENTINFO * pClientInfo) {

  GCMD::ACTION retCMD = GCMD::CONTINUE;

  // a word of caution... DO NOT cast htmlState to your enum type!
  // the compiler will silently remove the HTTPSTART case as
  // that state is not part of your enum. Keep the switch on typed
  // aginst the generic uint32_t.
  switch(pClientInfo->htmlState) {
  
    // Every Compose function will start at the magic HTTPSTART state
    // we MUST support this state.
    case HTTPSTART:
      // serialize so we only do this page once at a time
      // this protects the szPageBuffer
      if(pClientMutex != NULL) {
        break;
      }
      pClientMutex = pClientInfo;
      pClientInfo->htmlState = CONTLEN;
      retCMD = GCMD::GETLINE;
      break;
      
      case CONTLEN:
        // if we hit the end of the header then there was no content length
        // and we don't know how to handle that, so exit with an error
        // File not found is probably the wrong error, but it does get out out
        // Fortunately all major browsers put in the content lenght, so this
        // will almost never fail.
        if(strlen((char *) pClientInfo->rgbIn) == 0) {   // cbRead may be longer than just the line, so do a strlen()
          pClientMutex = NULL;
          return(JumpToComposeHTMLPage(pClientInfo, ComposeHTTP404Error));
        }
  
        // found the content lengths
        else if(memcmp((byte *) szContentLength, pClientInfo->rgbIn, sizeof(szContentLength)-1) == 0) {
          cbContentLenght = atoi((char *) &pClientInfo->rgbIn[sizeof(szContentLength)-1]);
          pClientInfo->htmlState = ENDHDR;
        }
        retCMD = GCMD::GETLINE;
        break;

      case ENDHDR:      
        // the header is ended with a double \r\n\r\n, so I will get
        // a zero length line. Just keep reading lines until we get to the blank line
        if(strlen((char *) pClientInfo->rgbIn) == 0) {   // cbRead may be longer than just the line, so do a strlen()
          uint32_t i = 0;
  
          // go to beyond the \0
          for(i = 0; i < pClientInfo->cbRead && pClientInfo->rgbIn[i] == '\0'; i++);
  
          // move the buffer to the front
          pClientInfo->cbRead -= i;
          if(pClientInfo->cbRead > 0) {
            memcpy(pClientInfo->rgbIn, &pClientInfo->rgbIn[i], pClientInfo->cbRead);
          }
  
          pClientInfo->htmlState = DATA;
          cParsed = 0;
          iIn = 0;
        }
        else {
          retCMD = GCMD::GETLINE;
        }
        break;

      case DATA:
        // cParsed -> Number of characters parsed                  
        char postData[5];
             
        if(cParsed < 1) {
          char * pUnderscore = NULL;
  
          // because we are doing string searches, we need to make sure there is a null terminator on the string
          // remember, we have 4 extra overflow byes at the end of the read buffer we can write into, so even if
          // the rgbIn buffer is completely full, we can write one past the end with the zero terminator.
          pClientInfo->rgbIn[pClientInfo->cbRead] = '\0';
  
          // there are other things in the form post besides the pin values, specifically
          // there is the submit button entry, so we need to skip about anything that is not a pin state
          // search for an underscore, that is the start of the pin number
  
          // The underscore is the begining of the data we want to find in the header
          if((pUnderscore = strstr((char *) &pClientInfo->rgbIn[iIn], "_")) != NULL) {
            // move up to the underscore, the start of the pin number
            iIn += ((byte *) pUnderscore - &pClientInfo->rgbIn[iIn]);
          }
          
          if((pClientInfo->cbRead - iIn) < 4) { // 4 = size of POST data to capture
            memcpy(pClientInfo->rgbIn, &pClientInfo->rgbIn[iIn], (pClientInfo->cbRead - iIn));
            pClientInfo->cbRead -= iIn;
            iIn = 0;
            retCMD = GCMD::READ;
          }
          else {
            postData[0] = (pClientInfo->rgbIn[iIn + 1]);
            postData[1] = (pClientInfo->rgbIn[iIn + 2]);
            postData[2] = (pClientInfo->rgbIn[iIn + 3]);
            postData[3] = (pClientInfo->rgbIn[iIn + 4]);
            postData[4] = '\0';
            processPostData(postData);
            cParsed++;
          }
        }
        else {
            pClientInfo->htmlState = GETPAGE;
        }
      break;

      case GETPAGE:
        pClientMutex = NULL;
        return(JumpToComposeHTMLPage(pClientInfo, ComposeHTTP200));
        break;

      case HTTPDISCONNECT:
          if(pClientMutex == pClientInfo) {
            pClientMutex = NULL;
          }
      // fall thru Done  
      // the done state is were we say we are done, and that
      // the connection can be closed
      case FINISH:
      default:
        // by returning DONE, we will close the connection
        // and be done with this page
        retCMD = GCMD::DONE;
        pClientMutex = NULL;
        break;
    }
  // Return the command we want to do
  // like WRITE, or DONE
  return(retCMD);
}

/* 0 - Stop
 * 1 - Forward
 * 2 - Back
 * 3 - Left
 * 4 - Right
*/

/* processPostData(char* postData)
 *  Takes a string of ints converts to int and 
 *  performs motion based on code
 *  4 digits used to allow for additional functionality
 */

void processPostData(char* postData) {
  Serial0.print("PostData: ");
  Serial0.println(postData);
  int moveCode = atoi(postData);
  
  if (moveCode == 0000) {
      // Stop Motion
      Serial0.println("Stop Motion");
      digitalWrite(PIN, LOW);  
      digitalWrite(PIN2, LOW);
      digitalWrite(PIN3, LOW);
      digitalWrite(PIN4, LOW);
      digitalWrite(PIN5, LOW);
      digitalWrite(PIN6, LOW);   
  }
  
  else if (moveCode == 1000) {
      // Foward Motion
    Serial0.println("Forward Motion");
      digitalWrite(PIN, HIGH);
      digitalWrite(PIN2, LOW);  
      digitalWrite(PIN3, HIGH);
      digitalWrite(PIN4, LOW);
      run();
  }
  else if (moveCode == 2000) {
    // Backward Motion
    Serial0.println("Back Motion");
    digitalWrite(PIN, LOW);
    digitalWrite(PIN2, HIGH);  
    digitalWrite(PIN3, LOW);
    digitalWrite(PIN4, HIGH);
    run();
  }
  else if (moveCode == 3000) {
    // Left Motion
    Serial0.println("Left Motion");
    digitalWrite(PIN, LOW);
    digitalWrite(PIN2, HIGH);
    digitalWrite(PIN3, HIGH);
    digitalWrite(PIN4, LOW);
    run();
  }
  else if (moveCode == 4000) {
    // Right Motion
    Serial0.println("Right Motion");
    digitalWrite(PIN, HIGH);
    digitalWrite(PIN2, LOW);
    digitalWrite(PIN3, LOW);
    digitalWrite(PIN4, HIGH);
    run();
  }
}
