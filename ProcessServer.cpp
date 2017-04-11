/************************************************************************/
/*                                                                      */
/*    WebServer.cpp                                                     */
/*                                                                      */
/*    A chipKIT WiFi HTTP Web Server implementation                     */
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
#include    <HTTPServer.h>
#include    "HTTPServerConfig.h"

#define CBDATETIME          32
#define TooMuchTime()       (millis() > cMSecEnd)
#define RestartTimer()      cMSecEnd = millis() + cSecTimeout * 1000
#define SetTimeout(cSec)    {cSecTimeout = cSec;}

#define cSecDefault         60
#define cSecInitRest        30
#define cSecIncRest         60
#define cSecMaxRest         600

static int cSecTimeout = cSecDefault;
static int cMSecEnd = 0;
static int cSecRest = cSecInitRest;

static IPv4         ipMy;
static IPEndPoint   ipEP;
static IPv4         ipRemote;
static char *       szRemoteURL;

typedef enum 
{
    // enumerate these number to make sure we match the host
    // sending these commands
    NONE = 0,
    STARTSTATE,
    STARTWIFISCAN,
    WAITFORSCAN,
    PRINTAPINFO,
    WIFICONNECT,
    WIFICONNECTWITHKEY,
    WIFICONNECTED,
    WIFIKEYGEN,
    DYNAMICIPBEGIN,
    STATICIPBEGIN,
    ENDPASS1,
    INITIALIZE,
    WAITFORTIME,
    GETNETADDRESSES,
    PRINTADDRESSES,
    MAKESTATICIP,
    PRINTWIFICONFIG,
    STARTLISTENING,
    LISTENING,
    NOTLISTENING,
    CHECKING,
    RESTARTNOW,
    RESTARTREST,
    TERMINATE,
    SHUTDOWN,
    RESTFORAWHILE,
    TRYAGAIN,
    DONOTHING,
    REBOOT,
    SOFTMCLR,
    MCLRWAIT,
    WAITFORTIMEOUT,
    ERROR    
} STATECMD;

#if defined(USING_WIFI)
// this is where we will keep our WIFI Key once we calculate it.
DWIFIcK::SECINFO secInfo;
int conID = DWIFIcK::INVALID_CONNECTION_ID;
#endif

// Start with DHCP to get our addresses
// then restart with a static IP
// this is the initial state machine starting point.
STATECMD state = STARTWIFISCAN;           // Scan WiFi First
// STATECMD state = WIFICONNECT;          // No WiFi Scan
STATECMD stateNext = RESTARTREST;
STATECMD stateTimeOut = RESTARTREST;

DNETcK::STATUS status = DNETcK::None;
unsigned int epochTime = 0;

// scan variables
int  cNetworks = 0;
int iNetwork = 0;

//******************************************************************************************
//******************************************************************************************
//*****************************  Supported TcpClients  *************************************
//******************************************************************************************
//******************************************************************************************

// and our server instance
static uint32_t iNextClientToProcess = 0;
static uint32_t cWorkingClients = 0;
TcpServer tcpServer(cMaxSocketsToListen);

static CLIENTINFO rgClient[cMaxSocketsToListen];
static char szTemp[256];            // needs to be long enough to take a WiFi Security key printed out.


/***    void ServerSetup()
 *
 *    Parameters:
 *          None
 *              
 *    Return Values:
 *          None
 *
 *    Description: 
 *    
 *      Initialized the Web Server Network parameters
 *      
 *      
 * ------------------------------------------------------------ */
void ServerSetup(void) 
{
    int i = 0;

    // Set the LED off, for not initialized
    pinMode(PIN_LED_SAFE, OUTPUT);
    SetLED(SLED::NOTREADY);     

    // because I want to control my timing, 
    // I am going to set my default wait time to immediate
    // but this is going to require me to check all the "Is"
    // methods before advancing to the next step
    DNETcK::setDefaultBlockTime(DNETcK::msImmediate); 
    
#if defined(USING_WIFI)
    state = STARTWIFISCAN;          // Scan WiFi First, verify the WiFi connection
#else
    state = DYNAMICIPBEGIN;         // just start with the wired network
#endif

    SetTimeout(cSecDefault);
    stateNext = RESTARTREST;
    stateTimeOut = RESTARTREST;
    RestartTimer();

    cSecRest = cSecInitRest;

    // make sure no clients appear to be in use
    for(i=0; i<cMaxSocketsToListen; i++)
    {
        rgClient[i].fInUse = false;
        rgClient[i].tcpClient.close();
    }
    cWorkingClients = 0;
}

/***    void ProcessServer()
 *
 *    Parameters:
 *          None
 *              
 *    Return Values:
 *          None
 *
 *    Description: 
 *    
 *      This is the main server loop. It:
 *          1. Scans for WiFi connections
 *          2. Connects to a WiFi by SSID
 *          3. Optionally creates a server IP on the detected subnet and dynamically assigns DNS and subnets.
 *          4. Or uses the static IP you assign; then you must supply DNS and subnet
 *          5. Starts listening on the supplied server port.
 *          6. Accepts client connections
 *          7. Schedules the processing on client connections in a round robin yet fashion (cooperative execution).
 *          8. Automatically restart if the network goes down
 *      
 *      This illistrates how to write a state machine like loop
 *      so that the PeriodicTask is called everytime through the loop
 *      so the stack stay alive and responsive.
 *
 *      In the loop we listen for a request, verify it to a limited degree
 *      and then broadcast the Magic Packet to wake the request machine.
 *      
 * ------------------------------------------------------------ */
void ProcessServer(void)
{   
    int i               = 0;
    int cbAvailable     = 0;

  // see if we exceeded our timeout value.
  // then just be done and close the socket 
  // by default, a closed client is never connected
  // so it is safe to call isConnected() even if it is closed 
  if(stateTimeOut != NONE && TooMuchTime())
  {
    Serial.println("Timeout occured");
    state = stateTimeOut;
    stateTimeOut = NONE;
    stateNext = RESTARTREST;
  }

  switch(state)
  {    

#if defined(USING_WIFI)

        case STARTWIFISCAN:
            Serial.println("Start WiFi Scan");
            DWIFIcK::beginScan();
            state = WAITFORSCAN;
            RestartTimer();
            break;

        case WAITFORSCAN:
            iNetwork = 0;
            if(DWIFIcK::isScanDone(&cNetworks, &status))
            {
                Serial.println("Scan Done");
                state = PRINTAPINFO;
                RestartTimer();
            }
            else if(DNETcK::isStatusAnError(status))
            {
                Serial.println("Scan Failed");
                Serial.println("");
                state = WIFICONNECT;
                RestartTimer();
            }
            break;

        case PRINTAPINFO:
            if(iNetwork < cNetworks)
            {
                DWIFIcK::SCANINFO scanInfo;
                int j = 0;

                if(DWIFIcK::getScanInfo(iNetwork, &scanInfo))
                {
                    Serial.println("");
                    Serial.print("Scan info for index: ");
                    Serial.println(iNetwork, DEC);

                    Serial.print("SSID: ");
                    Serial.println(scanInfo.szSsid);

                    Serial.print("Secuity type: ");
                    Serial.println(scanInfo.securityType, DEC);

                    Serial.print("Channel: ");
                    Serial.println(scanInfo.channel, DEC);    

                    Serial.print("Signal Strength: ");
                    Serial.println(scanInfo.signalStrength, DEC);    

                    Serial.print("Count of support bit rates: ");
                    Serial.println(scanInfo.cBasicRates, DEC);    

                    for( j= 0; j< scanInfo.cBasicRates; j++)
                    {
                        Serial.print("\tSupported Rate: ");
                        Serial.print(scanInfo.basicRates[j], DEC); 
                        Serial.println(" bps");
                    }

                    Serial.print("SSID MAC: ");
                    for(j=0; j<sizeof(scanInfo.ssidMAC); j++)
                    {
                        if(scanInfo.ssidMAC[j] < 16)
                        {
                            Serial.print(0, HEX);
                        }
                        Serial.print(scanInfo.ssidMAC[j], HEX);
                    }
                    Serial.println("");

                    Serial.print("Beacon Period: ");
                    Serial.println(scanInfo.beconPeriod, DEC);    

                    Serial.print("dtimPeriod: ");
                    Serial.println(scanInfo.dtimPeriod, DEC);    

                    Serial.print("atimWindow: ");
                    Serial.println(scanInfo.atimWindow, DEC); 
                }
                else
                {
                    Serial.print("Unable to get scan info for iNetwork: ");
                    Serial.println(iNetwork, DEC);
                }

                iNetwork++;
            }
            else
            {
                state = WIFICONNECT;
                Serial.println("");
            }
            RestartTimer();
            break;

        case WIFICONNECT:
            Serial.println("About To Connect");
     
            if((conID = WiFiConnectMacro()) != DWIFIcK::INVALID_CONNECTION_ID)
            {
                Serial.print("Connection Created, ConID = ");
                Serial.println(conID, DEC);
                Serial.println("Wait 30-40 seconds to establish the connection");
                state = WIFICONNECTED;
                RestartTimer();
            }
            else
            {
                Serial.println("Unable to get a connection with passphrase");
                state = ERROR;
                RestartTimer();
            }
            break;

        case WIFICONNECTWITHKEY:                  
            if((conID = DWIFIcK::connect(secInfo.securityType, szSsid, secInfo.key.rgbKey, secInfo.key.cbKey, secInfo.key.index, &status)) != DWIFIcK::INVALID_CONNECTION_ID)
            {
                Serial.print("Key Connection Created, ConID = ");
                Serial.println(conID, DEC);
                Serial.println("Wait 10-20 seconds to establish the connection");
                state = WIFICONNECTED;
                RestartTimer();
            }
            else
            {
                Serial.println("Unable to get a connection with key");
                state = ERROR;
                RestartTimer();
            }
            break;

        case WIFICONNECTED:
            if(DWIFIcK::isConnected(conID, &status))
            {
                Serial.println("Is Connected");
                Serial.print("connection status: ");
                Serial.println(status, DEC);

                state = WIFIKEYGEN;
                cSecRest = cSecInitRest;
                RestartTimer();
            }
            else if(DNETcK::isStatusAnError(status))
            {
                Serial.println("WiFi not connected");
                state = RESTARTREST;
                RestartTimer();
            }
            break;

    case WIFIKEYGEN:
        Serial.println("Getting Key");
        DWIFIcK::getSecurityInfo(conID, &secInfo);
        Serial.print("Key Type: ");
        Serial.println(secInfo.securityType, DEC);
        Serial.print("Key Lenght: ");
        Serial.println(secInfo.key.cbKey, DEC);
        Serial.print("Key value: ");
        GetNumb(secInfo.key.rgbKey, secInfo.key.cbKey, ':', szTemp);
        Serial.println(szTemp);
        state = PRINTWIFICONFIG;
        RestartTimer();
        break;
 
    case PRINTWIFICONFIG:
        {
            DWIFIcK::CONFIGINFO configInfo;

            if(DWIFIcK::getConfigInfo(&configInfo))
            {
                Serial.println("WiFi config information");

                Serial.print("Scan Type: ");
                Serial.println(configInfo.scanType, DEC);

                Serial.print("Beacon Timeout: ");
                Serial.println(configInfo.beaconTimeout, DEC);

                Serial.print("Connect Retry Count: ");
                Serial.println(configInfo.connectRetryCount, DEC);

                Serial.print("Scan Count: ");
                Serial.println(configInfo.scanCount, DEC);

                Serial.print("Minimum Signal Strength: ");
                Serial.println(configInfo.minSignalStrength, DEC);

                Serial.print("Minimum Channel Time: ");
                Serial.println(configInfo.minChannelTime, DEC);

                Serial.print("Maximum Channel Time: ");
                Serial.println(configInfo.maxChannelTime, DEC);

                Serial.print("Probe Delay: ");
                Serial.println(configInfo.probeDelay, DEC);

                Serial.print("Polling Interval: ");
                Serial.println(configInfo.pollingInterval, DEC);

                Serial.println("");

            }
            else
            {
                Serial.println("Unable to get WiFi config data");
            }
        }

        state = DYNAMICIPBEGIN;
        RestartTimer();
        break;

#endif // USING_WIFI

    case DYNAMICIPBEGIN:

        // if I don't have a static IP, then 
        // dynamically connect and calcuate our IP.
        if(ipMyStatic.u32IP == 0)
        {
            Serial.println("Dynamic begin");

            // ultimately I want to to have a static IP address 
            // but first I want to get my network addresses from DHCP
            // so to start, lets use DHCP
            DNETcK::begin();
            state = INITIALIZE;
        }

        // otherwise go directly to the static begin
        else
        {
            state = STATICIPBEGIN;
        }

        RestartTimer();
        break;

    case STATICIPBEGIN: 
        
        Serial.println("Static begin");

        // start again with static IP addresses
        DNETcK::begin(ipMyStatic, ipGateway, subnetMask, ipDns1, ipDns2);    
        state = INITIALIZE;
        RestartTimer();
        break;

    case INITIALIZE:

        // wait for initialization of the internet engine to complete
        if(DNETcK::isInitialized(&status))
            {
                Serial.println("Network Initialized");
                state = GETNETADDRESSES;
                SetLED(SLED::WORKING);
                RestartTimer();
            }
        else if(DNETcK::isStatusAnError(status))
            {
                Serial.println("Not Initialized");
                state = ERROR;
                RestartTimer();
            }
        break;

    case GETNETADDRESSES:
        
        // at this point we know we are initialized and
        // I can get my network address as assigned by DHCP
        // I want to save them so I can restart with them
        // there is no reason for this to fail

        // This is also called during the static IP begin
        // just to get in sync with what the underlying system thinks we have.
        DNETcK::getMyIP(&ipMy);
        DNETcK::getGateway(&ipGateway);
        DNETcK::getSubnetMask(&subnetMask);
        DNETcK::getDns1(&ipDns1);
        DNETcK::getDns2(&ipDns2);

        if(ipMyStatic.u32IP == 0)
        {
            state = MAKESTATICIP;
        }
        else
        {
            stateTimeOut = PRINTADDRESSES;
            state = WAITFORTIME;
        }

        RestartTimer();
        break;
    
    case MAKESTATICIP:

        // build the requested IP for this subnet
        ipMyStatic = ipGateway;
        ipMyStatic.rgbIP[3] = localStaticIP;

        // make sure what we built is in fact on our subnet
        if(localStaticIP != 0 && (ipMyStatic.u32IP & subnetMask.u32IP) == (ipGateway.u32IP & subnetMask.u32IP))
        {
            // if so, restart the IP stack and
            // use our static IP  
            state = ENDPASS1;
        }

        // if not just use our dynamaically assigned IP
        else
        {
            // otherwise just continue with our DHCP assiged IP
            ipMyStatic = ipMy;
            stateTimeOut = PRINTADDRESSES;
            state = WAITFORTIME;
        }
        RestartTimer();
        break;

    case ENDPASS1:

        SetLED(SLED::NOTREADY);

        // this is probably not neccessary but good
        // practice to make sure our instances are closed
        // before shutting down the Internet stack
        for(i=0; i<cMaxSocketsToListen; i++)
        {
            rgClient[i].fInUse = false;
            rgClient[i].tcpClient.close();
        }
        tcpServer.close();
        
        // terminate our internet engine
        DNETcK::end();

        // if we were asked to shut down the WiFi channel as well, then disconnect
        // we should be careful to do this after DNETcK:end().
#if defined(USING_WIFI) && defined(RECONNECTWIFI)
        // disconnect the WiFi, this will free the connection ID as well.
        DWIFIcK::disconnect(conID);
        state = WIFICONNECTWITHKEY;
#else
        state = STATICIPBEGIN;
#endif 

        stateTimeOut = RESTARTREST;
        RestartTimer();
        break;

    case WAITFORTIME:
        epochTime = DNETcK::secondsSinceEpoch(&status); 
        if(status == DNETcK::TimeSinceEpoch)
        {
            GetDayAndTime(epochTime, szTemp);
            Serial.println(szTemp);
            state = PRINTADDRESSES;
            RestartTimer();
        }

        break;

    case PRINTADDRESSES:

        Serial.println("");

        {
            IPv4    ip;
            MAC     mac;

            DNETcK::getMyIP(&ip);
            Serial.print("My ");
            GetIP(ip, szTemp);
            Serial.println(szTemp);

            DNETcK::getGateway(&ip);
            Serial.print("Gateway ");
            GetIP(ip, szTemp);
            Serial.println(szTemp);

            DNETcK::getSubnetMask(&ip);
            Serial.print("Subnet mask: ");
            GetNumb(ip.rgbIP, 4, '.', szTemp);
            Serial.println(szTemp);

            DNETcK::getDns1(&ip);
            Serial.print("Dns1 ");
            GetIP(ip, szTemp);
            Serial.println(szTemp);

            DNETcK::getDns2(&ip);
            Serial.print("Dns2 ");
            GetIP(ip, szTemp);
            Serial.println(szTemp);

            DNETcK::getMyMac(&mac);
            Serial.print("My ");
            GetMAC(mac, szTemp);
            Serial.println(szTemp);

            Serial.println("");
        }

        stateTimeOut = RESTARTREST;
        RestartTimer();
        state = STARTLISTENING;
        break;

    case STARTLISTENING:   
        // we know we are initialized, and our broadcast UdpClient is ready
        // we should just be able to start listening on our TcpIP port
        tcpServer.startListening(listeningPort);
        state = LISTENING;
        RestartTimer();
        break;

    case LISTENING:
        if(tcpServer.isListening(&status))
        {          
            IPv4    ip;

            Serial.print("Listening on ");

            DNETcK::getMyIP(&ip);
            GetIP(ip, szTemp);
            Serial.print(szTemp);
            Serial.print(":");
            Serial.println(listeningPort, DEC);
            Serial.println();

            state = CHECKING;
            SetLED(SLED::READY);
            stateTimeOut = NONE;
        }
        else if(DNETcK::isStatusAnError(status))
        {
            state = NOTLISTENING;
            RestartTimer();
        }
        break;
 
    case NOTLISTENING:

        SetLED(SLED::NOTREADY);
        Serial.println("Not Listening");

        SetTimeout(cSecDefault);
        stateTimeOut = RESTARTREST;

        switch(status)
        {
        case DNETcK::MoreCurrentlyPendingThanAllowed:
            Serial.println("Exceeded the maximum number of connections");
            state = LISTENING;
            break;
        case DNETcK::WaitingConnect:
        case DNETcK::WaitingReConnect:
            Serial.println("Waiting connection");
            state = LISTENING;
            break;
        case DNETcK::NotConnected:
            Serial.println("No longer connected");
            state = RESTARTREST;
            break;
        default:
            Serial.print("Other not-listening status: ");
            Serial.println(status, DEC);
            state = RESTARTREST;
            break;
        }
        Serial.println("");
        RestartTimer();  
        break;

    case CHECKING:   
        if(tcpServer.availableClients() > 0)
        {       
            // find an open client
            // process the next client to be processed
            i = iNextClientToProcess;
            do {
                if(!rgClient[i].fInUse)
                {
                    memset(&rgClient[i].clientState, 0, sizeof(CLIENTINFO) - OFFSETOF(CLIENTINFO, clientState));
                    if(tcpServer.acceptClient(&rgClient[i].tcpClient))
                    {
                        Serial.print("Got a client: 0x");
                        Serial.println((uint32_t) &rgClient[i], HEX);
                        rgClient[i].fInUse = true;
 
                        // set the timer so if something bad happens with the client
                        // we won't hang, also we don't need to check errors on the client
                        // becasue we will just timeout if there is an error
                        RestartTimer();   
                    }
                    else
                    {
                        Serial.println("Failed to get client");
                    }
                    break;
                }
                ++i %=  cMaxSocketsToListen;
            } while(i != iNextClientToProcess);
        }
        else if(!tcpServer.isListening(&status))
        {
            state = NOTLISTENING;
        }
        break;
      
    case RESTARTNOW:
        stateTimeOut = NONE;
        stateNext = TRYAGAIN;
        state = SHUTDOWN;
        break;

    case TERMINATE:
        stateTimeOut = NONE;
        stateNext = DONOTHING;
        state = SHUTDOWN;
        break;

    case REBOOT:
        stateTimeOut = NONE;
        stateNext = MCLRWAIT;
        state = SHUTDOWN;
        break;

    case RESTARTREST:
        stateTimeOut = NONE;
        stateNext = RESTFORAWHILE;
        state = SHUTDOWN;
        break;

    case SHUTDOWN:  // nobody should call this but TEMINATE and RESTARTREST
 
        SetLED(SLED::NOTREADY);

        Serial.println("Shutting down");

        // this is probably not neccessary but good
        // practice to make sure our instances are closed
        // before shutting down the Internet stack
        for(i=0; i<cMaxSocketsToListen; i++)
        {
            rgClient[i].fInUse = false;
            rgClient[i].tcpClient.close();
        }
        tcpServer.close();
        
        // terminate our internet engine
        DNETcK::end();

#if defined(USING_WIFI)
        // disconnect the WiFi, this will free the connection ID as well.
        DWIFIcK::disconnect(conID);
#endif

        // make sure we don't hit our timeout code
        stateTimeOut = NONE;
        state = stateNext;
        stateNext = RESTARTREST;
        break;

    case RESTFORAWHILE:
        {
            static bool fFirstEntry = true;
            static bool fPrintCountDown = true;
            static unsigned int tRestingStart = 0;
            uint32_t tCur = millis();

            if(fFirstEntry)
            {
                fFirstEntry = false;
                fPrintCountDown = true;
                Serial.print("Resting for ");
                Serial.print(cSecRest, DEC);
                Serial.println(" seconds");
                tRestingStart = tCur;
                stateTimeOut = NONE;
            }

            // see if we are done resting
            else if((tCur - tRestingStart) >= (cSecRest * 1000))
            {
                fFirstEntry = true;
                fPrintCountDown = true;

                Serial.println("Done resting");

                cSecRest += cSecIncRest;
                if(cSecRest > cSecMaxRest) cSecRest = cSecMaxRest;

                SetTimeout(cSecDefault);
                state = TRYAGAIN;
             }

            // see if we should print out a countdown time
            else if(((tCur - tRestingStart) % 10000) == 0)
            {
                if(fPrintCountDown)
                {
                    Serial.print(cSecRest - ((tCur - tRestingStart)/1000), DEC);
                    Serial.println(" seconds until restart.");
                    fPrintCountDown = false;
                }
            }

            // so we will print the countdown at the next interval
            else
            {
                fPrintCountDown = true;
            }
        }
        break;

    case TRYAGAIN:
        stateNext = RESTARTREST;
        stateTimeOut = RESTARTREST;
#if defined(USING_WIFI)
        state = WIFICONNECT;
#else
        state = STATICIPBEGIN;
#endif 
        RestartTimer();
        break;

    case DONOTHING:
        stateTimeOut = NONE;
        stateNext = DONOTHING;
        break;

    case WAITFORTIMEOUT:
        break;

    case MCLRWAIT:
        stateTimeOut = SOFTMCLR;
        state = WAITFORTIMEOUT;
        SetTimeout(1);
        RestartTimer();
        break;

    case SOFTMCLR:
        executeSoftReset(RUN_SKETCH_ON_BOOT);
        stateTimeOut = NONE;
        stateNext = DONOTHING;
        break;

    case ERROR:
    default:
        SetLED(SLED::NOTREADY);
        Serial.print("Hard Error status #");
        Serial.print(status, DEC);
        Serial.println(" occurred.");
        stateTimeOut = NONE;
        state = RESTARTREST;
        break;
    }
  
    // process only 1 of the clients in a round robin fashion.
    cWorkingClients = 0;
    i = iNextClientToProcess;
    do {
        if(rgClient[i].fInUse)
        {
            cWorkingClients++;
            SetLED(SLED::WORKING);
            switch(ProcessClient(&rgClient[i]))
            {
                case GCMD::RESTART:
                    state = RESTARTNOW;
                    break;

                case GCMD::TERMINATE:
                    state = TERMINATE;
                    break;

                case GCMD::REBOOT:
                    state = REBOOT;
                    break;

                case GCMD::CONTINUE:
                case GCMD::READ:
                case GCMD::WRITE:  
                case GCMD::DONE:  
                default:
                    break;
            }
            iNextClientToProcess = (i + 1) % cMaxSocketsToListen;
            break;
        }
        ++i %=  cMaxSocketsToListen;
    } while(i != iNextClientToProcess);

    // select our LED state for the next pass
    if(cWorkingClients > 0)
    {
        SetLED(SLED::WORKING);
    }
    else if(tcpServer.isListening(NULL))
    {
        // if we are in the checking state
        // and the count went to zero
        // go back to the listening state to print we are listening
        if(state == CHECKING && GetLEDState() == SLED::WORKING)
        {
            state = LISTENING;
        }
        SetLED(SLED::READY);
    }
    else
    {
        SetLED(SLED::NOTREADY);
    }

    // Keep the Ethernet stack alive
    DNETcK::periodicTasks();
    SetLED(SLED::PROCESS);
}

