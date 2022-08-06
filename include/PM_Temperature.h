/*
  Copyright 2022 - Yves Gaignard
  
  Functions to manage time in Pool Manager project
*/
#ifndef PM_Temperature_h
#define PM_Temperature_h

#include <Arduino.h>
#include <DallasTemperature.h>

class PM_Temperature {

  private: 
    DallasTemperature            _sensors;
    DallasTemperature::request_t _request;
    boolean                      _isInit=false;
    int                          _sensorNumber=0;
    std::vector<std::string>     _deviceAddressArray;
    std::vector<std::string>     _deviceNameArray;

  public:
    // Constructor
    PM_Temperature();

    // Destructor
    ~PM_Temperature();

    // operator ==
    //friend bool operator== (const DeviceAddr& a, const DeviceAddr& b);

    // initialization of the object (MANDATORY)
    void init(DallasTemperature& sensors);

    // return the number of devices found
    int getDeviceCount();

    // return a string containing the hexadecimal address of a device
    std::string getDeviceAddress(int deviceIndex);

    // Add a device in the array of the devices through its name (you can choose it but it must be unique) and its address
    int addDevice(std::string deviceName, std::string deviceAddress);

    // Get the device name by index
    std::string getDeviceNameByIndex(int idx);

    // Get the handle of the DallasTemperature object
    DallasTemperature& getSensors();

    // sends command for all devices on the bus to perform a temperature conversion
    void requestTemperatures();

    // Request temperature (float precision) for the sensor through its index in the array
    float getPreciseTempCByIndex(int idx);

    // Request temperature (int precision) for the sensor through its index in the array
    int getTempCByIndex(int idx);

    // Request temperature (float precision) for the sensor through its address
    float getPreciseTempCByAddress(std::string deviceAddress);

    // Request temperature (int precision) for the sensor through its address
    int getTempCByAddress(std::string deviceAddress);

    // Request temperature (float precision) for the sensor through its name
    float getPreciseTempCByName(std::string deviceName);

    // Request temperature (int precision) for the sensor through its name
    int getTempCByName(std::string deviceName);

    // function to convert a device address to a string
    static std::string deviceAddressToString(DeviceAddress deviceAddress);

    // function to convert a string to a device address
    template <typename T> 
    static T stringToDeviceAddress(std::string deviceAddressStr);
   
};

#endif