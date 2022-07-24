/*
  Copyright 2022 - Yves Gaignard
  
  Structures and constant definitions of the project
*/
#ifndef PM_Structures_h
#define PM_Structures_h

#include <Arduino.h>

struct ProjectStruct {
  String Name;
  String Version;
};


struct WifiNetwork {
  String ssid;
  String password;
};
struct WifiConfig {
  IPAddress local_IP;
  IPAddress gateway;
  IPAddress subnet;
  IPAddress primaryDNS;   //optional
  IPAddress secondaryDNS; //optional
};

#endif