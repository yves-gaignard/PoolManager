/*
  Copyright 2022 - Yves Gaignard
  
  Functions to scan all I2C devices
*/
#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE

#include <Arduino.h>
#include <Wire.h>               // Library to scan the I2C devices

#include "PM_I2CScan.h"

static const char* TAG = "PM_I2CScan";

// =========================================================================================
// Scan all I2C devices, return an array of device addresses 
// =========================================================================================
int PM_I2CScan_Scan(byte I2CDevices[]) {
  int I2CDeviceNumber=0;
  byte error, address;
  String Log_Msg;

  ESP_LOGI(TAG, "Scanning for I2C Devices…");

  for (address = 1; address < 127; address++ )
  {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0)
    {
      I2CDevices[I2CDeviceNumber]=address;
      I2CDeviceNumber++;
    }
    else if (error == 4)
    {
      Log_Msg = "Unknown error at address 0x";
      if (address < 16)
      {
        Log_Msg+="0";
      }
      Log_Msg+=String (address, HEX);
      ESP_LOGI(TAG, "%s", Log_Msg.c_str());
    }
  }
  if (I2CDeviceNumber == 0)
  {
    ESP_LOGI(TAG, "No I2C devices found");
  }
  else
  {
    Log_Msg=I2CDeviceNumber;
    Log_Msg+=" devices found";
    ESP_LOGI(TAG, "%s", Log_Msg.c_str());
  }
  return I2CDeviceNumber;

}

// =========================================================================================
// Discover all I2C devices and print their addresses to the log  
// =========================================================================================
void PM_I2CScan_Print(int I2CDeviceNumber, byte I2CDevices[]) {
  byte DeviceAddress;
  String Log_Msg;
  for (int i = 0; i < I2CDeviceNumber ; i++ )
  {
    Log_Msg="I2C device found at address 0x";
    DeviceAddress=I2CDevices[i];
    if (DeviceAddress < 16)
    {
      Log_Msg+="0";
    }
    Log_Msg+=String (DeviceAddress, HEX);
    Log_Msg+=" !";
    ESP_LOGI(TAG, "%s", Log_Msg.c_str());
  }

}
