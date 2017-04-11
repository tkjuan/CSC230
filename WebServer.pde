/************************************************************************/
/*                                                                      */
/*    WebServer                                                         */
/*                                                                      */
/*    A Example chipKIT HTTP Server implementation                      */
/*    This sketch is designed to work with web browser clients          */
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
/*                                                                      */
/*    7/15/2013(KeithV): Created                                        */
/************************************************************************/
/************************************************************************/
/*                                                                      */
/*  This HTTP Server Example contains sample content in the content     */
/*  subdirectory under the sketch directory. The files in the content   */
/*  directory should be copied to the root of your SD card. However,    */
/*  you can view the content by opening HomePage.htm page               */
/*  with your browser right out of the content subdirectory before      */
/*  loading it on the SD card. The sample pages contain                 */
/*  instructions on how to set up this example and therefore may be     */
/*  valuable to read before proceeding.                                 */
/*                                                                      */
/************************************************************************/
/************************************************************************/
/*                       Supported hardware:                            */
/*                                                                      */
/*  Most / All Uno32 Shield form factor boards with a WiFiShield        */
/*          For example the Uno32, uC32, WF32                           */
/*  Max32 with a WiFiShield                                             */
/*  MX7cK with a pmodSD on JPF                                          */
/*                                                                      */
/*  NOTE: you can NOT stack a NetworkShield and a WiFiShield on a Max32 */
/************************************************************************/

//************************************************************************
//************************************************************************
//********  SET THESE LIBRARIES FOR YOUR HARDWARE CONFIGURATION **********
//************************************************************************
//************************************************************************

/************************************************************************/
/*                                                                      */
/*    Network Hardware libraries                                        */
/*    INCLUDE ONLY ONE                                                  */
/*                                                                      */
/************************************************************************/
// You MUST select 1 and ONLY 1 of the following hardware libraries
// they are here so that MPIDE will put the lib path on the compiler include path.
#include    <WiFiShieldOrPmodWiFi_G.h>	// MRF24WG WiFi transceiver -- also use for the WF32
//#include    <WiFiShieldOrPmodWiFi.h>	// MRF24WB WiFi transceiver
//#include    <NetworkShield.h>         // Network Shield / Internal MAC -- also use for the MX7cK
//#include    <PmodNIC.h>               // PmodNIC (obsolete but works), PmodNIC100 is the replacement.
//#include    <PmodNIC100.h>            // PmodNIC100

/************************************************************************/
/*    Network libraries                                                 */
/************************************************************************/
// The base network library
// this is a required library
// Do not comment out this library
#include    <DNETcK.h>

//  -----  COMMENT THIS OUT IF YOU ARE NOT USING WIFI  -----
#include    <DWIFIcK.h>

//************************************************************************
//************************************************************************
//**************** END OF LIBRARY CONFIGURATION **************************
//************************************************************************
//************************************************************************

/************************************************************************/
/*                                                                      */
/*           YOU MUST.....                                              */
/*                                                                      */
/*    You MUST put HTTPServerConfig.h in your sketch director           */
/*    And you MUST configure it with your network parameters            */
/*                                                                      */
/*    You also MUST load your content onto your SD card and             */
/*    the file HomePage.htm MUST exist at the root of the SD            */
/*    file structure. Of course you must insert your SD card            */
/*    into the SD reader on the chipKIT board                           */
/*                                                                      */
/*    Go do this now....                                                */
/*                                                                      */
/************************************************************************/

/************************************************************************/
/*    Other libraries; Required libraries                               */
/************************************************************************/
// You must have an SD card reader somewhere
// as the HTTP server uses the SD card to hold the HTML pages.
// this is a required library
#include    <SD.h>

// and this is the HTTPServer library code.
// this is a required library
#include    <HTTPServer.h>



/**************************************************************************************
/*    Manual    Edits
/**************************************************************************************/
#include <moveForward.h>
static const char forwardStr[]       = "GET /forward ";
static const char leftStr[]       = "GET /left ";
static const char rightStr[]       = "GET /right ";
static const char backStr[]       = "GET /back ";
static const char halfStr[]       = "GET /half ";
static const char fullStr[]       = "GET /full ";


const int PIN = 8;
const int PIN2 = 9;
const int PIN3 = 10;
const int PIN4 = 11; 
const int PIN5 = 12;
const int PIN6 = 13;




/**************************************************************************************


/************************************************************************/
/*    HTTP URL Matching Strings                                         */
/************************************************************************/
// These are the HTTP URL match strings for the dynamically created
// HTML rendering functions.
// Make these static const so they get put in flash
static const char szHTMLRestart[]       = "GET /Restart ";
static const char szHTMLTerminate[]     = "GET /Terminate ";
static const char szHTMLReboot[]        = "GET /Reboot ";
static const char szHTMLFavicon[]       = "GET /favicon.ico ";
static const char szHTMLSample[]        = "GET /Sample ";

// here is our sample/example dynamically created HTML page
GCMD::ACTION ComposeHTMLSamplePage(CLIENTINFO * pClientInfo);

/***    void setup(void)
 *
 *    Parameters:
 *          None
 *              
 *    Return Values:
 *          None
 *
 *    Description: 
 *    
 *      Arduino Master Initialization routine
 *      
 *      
 * ------------------------------------------------------------ */
 
const int LED = 13;

void setup(void) 
{   
    // Must do a Serial.begin because the HTTP Server
    // has diagnostic prints in it.
    Serial.begin(9600);
    Serial.println("WebServer v2.0");
    Serial.println("Copyright 2013, Digilent Inc.");
    Serial.println("Written by Keith Vogel");
    Serial.println();

    // add rendering functions for dynamically created web pages
    // max of 10 AddHTMLPage() allowed 

    // This is adding our sample dynamically HTML page
    // It will get invoked when http://<IP>/Sample is specified on the browser
    AddHTMLPage(szHTMLSample,      ComposeHTMLSamplePage);

    // comment this out if you do not want to support
    // restarting the network stack from a browser
    AddHTMLPage(szHTMLRestart,      ComposeHTMLRestartPage);

    // comment this out if you do not want to support
    // terminating the server from a browser
    AddHTMLPage(szHTMLTerminate,    ComposeHTMLTerminatePage);

    // comment this out if you do not want to support
    // rebooting (effectively hitting MCLR) the server from a browser
    AddHTMLPage(szHTMLReboot,       ComposeHTMLRebootPage);



    AddHTMLPage(forwardStr,    moveForward);
    AddHTMLPage(leftStr,       moveLeft);
    AddHTMLPage(rightStr,      moveRight);
    AddHTMLPage(backStr,       moveBack);
    AddHTMLPage(halfStr,       setPowerHalf);
    AddHTMLPage(fullStr,       setPowerFull);
    
    pinMode(PIN, OUTPUT);
    pinMode(PIN2, OUTPUT);
    pinMode(PIN3, OUTPUT);
    pinMode(PIN4, OUTPUT);
    pinMode(PIN5, OUTPUT);
    pinMode(PIN6, OUTPUT);
    digitalWrite(PIN, LOW);
    digitalWrite(PIN2, LOW);
    digitalWrite(PIN3, LOW);
    digitalWrite(PIN4, LOW);
    digitalWrite(PIN5, LOW);
    digitalWrite(PIN6, LOW);
    
    // This example supports favorite ICONs, 
    // those are those icon's next to the URL in the address line 
    // on the browser once the page is displayed.
    // To support those icons, have at the root of the SD file direcotory
    // an ICON (.ico) file with your ICON in it. The file MUST be named
    // favicon.ico. If you do not have an icon, then uncomment the following
    // line so the server will tell the browser with an HTTP file not found
    // error that we don't have a favoite ICON.
    // AddHTMLPage(szHTMLFavicon,      ComposeHTTP404Error);

    // Make reading files from the SD card the default compose function
    SetDefaultHTMLPage(ComposeHTMLSDPage);

    // Initialize the SD card
    SDSetup();

    // Initialize the HTTP server
    ServerSetup();
}

/***    void loop(void) 
 *
 *    Parameters:
 *          None
 *              
 *    Return Values:
 *          None
 *
 *    Description: 
 *    
 *      Arduino Master Loop routine
 *      
 *      
 * ------------------------------------------------------------ */

void loop(void) 
{
//    ProcessServer();   
    digitalWrite(PIN, LOW);
  digitalWrite(PIN2, HIGH);  
  digitalWrite(PIN3, LOW);
  digitalWrite(PIN4, HIGH);
  run(100);
}
