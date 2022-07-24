/*
  Copyright 2022 - Yves Gaignard
  
  Structures and constant definitions of the project
*/
#ifndef PM_Constants_h
#define PM_Constants_h

#include <Arduino.h>
#include <PM_Structures.h>

// ================================================================================================================================================
// project definition
const ProjectStruct Project {"Pool Manager", "1.0.0"};
// ================================================================================================================================================


// list of known Wifi networks
const WifiNetwork WifiMM       {"YGDASUS", "alexaufootalicealamusique"};
const WifiNetwork WifiLaConche {"TP-Link_D50E", "17084826" };

// standard configuration of the wifi config: local_IP,            gateway_IP,             subnet,               primaryDNS,      secondaryDNS
const WifiConfig  WifiConf     {IPAddress(192,168,1,99), IPAddress(192,168,1,1), IPAddress(255,255,0,0), IPAddress(1,1,1,1), IPAddress(8,8,8,8)};

// Web server listening port
const int         WebServerPort = 80; 

// welcome phrase send by the Web Server at the initialization
const String      WelcomePhrase (String("Project:" + Project.Name + " Version:"+ Project.Version + "\n\nHello ! I am the ESP32 OTA web server serving:" + Project.Name + " project."));

#endif