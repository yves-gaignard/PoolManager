/*
  Copyright 2022 - Yves Gaignard
  
  Functions to scan all I2C devices
*/
#ifndef PM_I2CScan_h
#define PM_I2CScan_h

// Scan all I2C devices, return an array of device addresses 
int PM_I2CScan_Scan(byte I2CDevices[]);

// Print all found I2C device addresses in the log  
void PM_I2CScan_Print(int I2CDeviceNumber, byte I2CDevices[]);

#endif