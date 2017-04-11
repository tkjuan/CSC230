/************************************************************************/
/*                                                                      */
/*    HTTPServerConfig.h                                                */
/*                                                                      */
/*    The network and WiFi configuration file required                  */
/*    to specify the network parameters to the network libraries        */
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
/*    7/19/2013(KeithV): Created                                        */
/************************************************************************/
#if !defined(_WEBSERVERCONFIG_H)
#define	_WEBSERVERCONFIG_H

//************************************************************************
//************************************************************************
//******************  SET THESE VALUES FOR YOUR NETWORK  *****************
//************************************************************************
//************************************************************************

// You have a choice of either calculating a static IP based on LocalStaticIP (next variable)
// Or setting the whole IP yourself. If you want DHCP to supply and automatically set
// your network parameters, set this to {0,0,0,0}. However, you then MUST set localStaticIP.
// If you fully specify your static IP, then you do NOT need to set localStaticIP
// but you MUST then set the remaining Gateway, subnet, and DNS values below.

//static IPv4 ipMyStatic = {192,168,1,225};    // a place to calculate our static IP 
static IPv4 ipMyStatic = {192,168,1,11};    // a place to calculate our static IP 

// This will be ignored if ipMyStatic is NOT set to {0,0,0,0}
// If ipMyStatic == {0,0,0,0} AND localStaticIP == 0, then the full IP returned by DHCP will be used
// Otherwise this is the last octet in the IP address... so if DHCP comes up with a subnet of 192.168.1.x
// this will set your IP address to 192.168.1.(localStaticIP) for example if 
// localStaticIP == 190; your final IP address would be 192.168.1.190 assuming the subnet address of 192.168.1.x
static byte localStaticIP = 0;   // this will be the gateway IP with the last octet of the IP being 195

// Set the port to listen on; this is a required item
static unsigned short listeningPort = 80;      // 80 is the default for an HTTP server

// how many sockets/connections will we be able to handle at once? Also required
#define cMaxSocketsToListen 6

// You ONLY MUST set these if you specifically assigned ipMyStatic to a static
// IP address other than {0,0,0,0}; otherwise DHCP will overwrite these.
static IPv4 ipGateway   = {192,168,1,1};
static IPv4 subnetMask  = {255,255,255,0};
static IPv4 ipDns1      = {8,8,8,8};        // public Google DNS server
static IPv4 ipDns2      = {8,8,4,4};        // public Google DNS server

//************************************************************************
//************************************************************************
//*****************  SET THESE VALUES FOR YOUR WIFI AP  ******************
//************* only have to set these if you are using WiFi *************
//************************************************************************
//************************************************************************
#ifdef USING_WIFI

// Specify the SSID of your AP
//const char * szSsid = "a"; // Router
const char * szSsid = "cw-2.4";

// select ONLY 1 for the security you want, or none for no security
// then updated the appropriate section below for your key or passphrase

#define USE_WPA2_PASSPHRASE
//#define USE_WPA2_KEY
//#define USE_WEP40
//#define USE_WEP104
//#define USE_WF_CONFIG_H     // See documentaton for WF_Config.x override

// modify the security key to what you have.
#if defined(USE_WPA2_PASSPHRASE)

//    const char * szPassPhrase = "basketball";
const char * szPassPhrase = "trudyblack2";
    #define WiFiConnectMacro() DWIFIcK::connect(szSsid, szPassPhrase, &status)

#elif defined(USE_WPA2_KEY)

    DWIFIcK::WPA2KEY key = { 0x27, 0x2C, 0x89, 0xCC, 0xE9, 0x56, 0x31, 0x1E, 
                            0x3B, 0xAD, 0x79, 0xF7, 0x1D, 0xC4, 0xB9, 0x05, 
                            0x7A, 0x34, 0x4C, 0x3E, 0xB5, 0xFA, 0x38, 0xC2, 
                            0x0F, 0x0A, 0xB0, 0x90, 0xDC, 0x62, 0xAD, 0x58 };
    #define WiFiConnectMacro() DWIFIcK::connect(szSsid, key, &status)

#elif defined(USE_WEP40)

    const int iWEPKey = 0;
    DWIFIcK::WEP40KEY keySet = {    0xBE, 0xC9, 0x58, 0x06, 0x97,     // Key 0
                                    0x00, 0x00, 0x00, 0x00, 0x00,     // Key 1
                                    0x00, 0x00, 0x00, 0x00, 0x00,     // Key 2
                                    0x00, 0x00, 0x00, 0x00, 0x00 };   // Key 3
    #define WiFiConnectMacro() DWIFIcK::connect(szSsid, keySet, iWEPKey, &status)

#elif defined(USE_WEP104)

    const int iWEPKey = 0;
    DWIFIcK::WEP104KEY keySet = {   0x3E, 0xCD, 0x30, 0xB2, 0x55, 0x2D, 0x3C, 0x50, 0x52, 0x71, 0xE8, 0x83, 0x91,   // Key 0
                                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   // Key 1
                                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   // Key 2
                                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }; // Key 3
    #define WiFiConnectMacro() DWIFIcK::connect(szSsid, keySet, iWEPKey, &status)

#elif defined(USE_WF_CONFIG_H)

    #define WiFiConnectMacro() DWIFIcK::connect(0, &status)

#else   // no security - OPEN

    #define WiFiConnectMacro() DWIFIcK::connect(szSsid, &status)

#endif
   
// this should be uncommented if you want to restart 
// the WiFi connection when DNETcK is restarted.
// This is generally uninteresting unless you have a reason
#define RECONNECTWIFI

#endif // USING_WIFI

//************************************************************************
//************************************************************************
//***************************** END OF CONFIGURATION *********************
//************************************************************************
//************************************************************************

#endif // _WEBSERVERCONFIG_H
