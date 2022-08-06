/*
  Copyright 2022 - Yves Gaignard
  
  Functions to discover all OneWire devices
*/
#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE

#include <Arduino.h>
#include <OneWire.h>               // Library to scan the OneWire devices

#include "PM_OneWire.h"
#include "PM_Utils.h"

static const char* TAG = "PM_OneWire";


// Scan all OneWire devices, return the number of devices found
int PM_OneWire_Scan(OneWire & one_wire, std::vector<std::vector<byte>>& addrArray ) {
  byte addr[8];
  addrArray.clear();

  ESP_LOGI(TAG, "Scanning for OneWire Devices…");
 
  while (1) {
    if (!one_wire.search(addr)) {

#if ARDUHAL_LOG_LEVEL >= ARDUHAL_LOG_LEVEL_DEBUG
      ESP_LOGD(TAG, "%d Devices found", addrArray.size());
      std::string Message;
      for (int i = 0; i < addrArray.size(); i++) {
        Message=PM_OneWireDeviceAddressVectorToString(addrArray[i]);
        ESP_LOGD(TAG, "Device [%d] = %s", i, Message.c_str());
      }
#endif
      one_wire.reset_search();
      delay(250);
      return addrArray.size();
    }

    std::vector<byte> byteArray;
    for (int i = 0; i < 8; i++) {
      byteArray.push_back(addr[i]);
    }

    addrArray.push_back(byteArray);
  }
  
}

// =========================================================================================
// Discover all devices connected to OneWire and print their serial number to the log  
// =========================================================================================
void PM_OneWire_Scan_And_Print(OneWire & one_wire) {

  std::vector<std::vector<byte>> addrArray;
  std::vector<byte> AddrVector;
  std::string Message;

  PM_OneWire_Scan(one_wire, addrArray );

  int DeviceNumber = addrArray.size();
  if ( DeviceNumber <= 0 ) {
    ESP_LOGI(TAG, "No OneWire devices found !!!!!");
  } 
  else {
    ESP_LOGI(TAG, "%d OneWire devices have been founf", DeviceNumber);
    Message = "";
    for (int i = 0; i < DeviceNumber; i++) {
      Message += "Dev[" + PM_itoa(i) + "]="; 
      Message += PM_OneWireDeviceAddressVectorToString(addrArray[i]); 
      ESP_LOGI(TAG, "%s", Message.c_str());
    }
  }
}