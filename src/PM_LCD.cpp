/*
  Yves Gaignard
  
  Pool manager time management
*/

#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE

// Standard library definitions
#include <Arduino.h>
#include <LiquidCrystal_I2C.h>

#include "PM_LCD.h"            // Pool manager LCD management

static const char* TAG = "PM_LCD";

PM_LCD::PM_LCD(uint8_t Device_Addr, uint8_t Cols, uint8_t Rows) {
  this->_initLCD (Device_Addr, Cols, Rows);
}

PM_LCD::~PM_LCD(){
  delete _lcd;
}

void PM_LCD::_initLCD (uint8_t Device_Addr, uint8_t Cols, uint8_t Rows) {
  _lcd = new LiquidCrystal_I2C(Device_Addr, Cols, Rows);
  _deviceAddress= Device_Addr;
  _columnNumber=Cols;
  _rowNumber=Rows;
  _padding = "";
  for (int i=0; i < _columnNumber; i++) {
    _padding+=" ";
  }
  _isInit=true;
}
LiquidCrystal_I2C*  PM_LCD::getLCD() {
  return _lcd;
}
int                 PM_LCD::getColumnNumber() {
  return _columnNumber;
}
int                 PM_LCD::getRowNumber() {
  return _rowNumber;
}

boolean             PM_LCD::getDisplayState() {
  if (_isInit == false) { ESP_LOGE(TAG, "LCD was not initialized. Take care of coredump !!!"); }
  return _displayState;
}
    
void PM_LCD::init() {
  _lcd->init();
}
void PM_LCD::clear() {
  if (_isInit == false) { ESP_LOGE(TAG, "LCD was not initialized. Take care of coredump !!!"); }
  _lcd->clear();
}
void PM_LCD::home() {
  if (_isInit == false) { ESP_LOGE(TAG, "LCD was not initialized. Take care of coredump !!!"); }
  _lcd->home();
}
void PM_LCD::display() {
  if (_isInit == false) { ESP_LOGE(TAG, "LCD was not initialized. Take care of coredump !!!"); }
  _lcd->display();
  _displayState = true;
}
void PM_LCD::noDisplay() {
  if (_isInit == false) { ESP_LOGE(TAG, "LCD was not initialized. Take care of coredump !!!"); }
  _lcd->noDisplay();
  _displayState = false;
}
void PM_LCD::blink() {
  if (_isInit == false) { ESP_LOGE(TAG, "LCD was not initialized. Take care of coredump !!!"); }
  _lcd->blink();
  _blink=true;
}
void PM_LCD::noBlink() {
  if (_isInit == false) { ESP_LOGE(TAG, "LCD was not initialized. Take care of coredump !!!"); }
  _lcd->noBlink();
  _blink=false;
}
void PM_LCD::cursor() {
  if (_isInit == false) { ESP_LOGE(TAG, "LCD was not initialized. Take care of coredump !!!"); }
  _lcd->cursor();
  _cursor=true;
}
void PM_LCD::noCursor() {
  if (_isInit == false) { ESP_LOGE(TAG, "LCD was not initialized. Take care of coredump !!!"); }
  _lcd->noCursor();
  _cursor=false;
}
void PM_LCD::noBacklight() {
  if (_isInit == false) { ESP_LOGE(TAG, "LCD was not initialized. Take care of coredump !!!"); }
  _lcd->noBacklight();
  _backlight=false;
}
void PM_LCD::backlight() {
  if (_isInit == false) { ESP_LOGE(TAG, "LCD was not initialized. Take care of coredump !!!"); }
  _lcd->backlight();
  _backlight=true;
}

void PM_LCD::printScreen       (std::vector<std::string>& screen) {
  if (_isInit == false) { ESP_LOGE(TAG, "LCD was not initialized. Take care of coredump !!!"); }
  this->clear();
  this->display();
  this->backlight();
  this->home();
  this->noBlink();
  this->noCursor();
  std::string subLine;
  for (int row =0; row < _rowNumber; row++) {
    subLine = std::string(screen[row]+_padding).substr(0, _columnNumber);
    _lcd->setCursor(0,row);
    _lcd->print(subLine.c_str());
  }
}
void PM_LCD::printScrollScreen (std::vector<std::string>& screen) {
  if (_isInit == false) { ESP_LOGE(TAG, "LCD was not initialized. Take care of coredump !!!"); }
  
}
void PM_LCD::printScrollScreen (std::vector<std::string>& screen, int displayTime) {
  if (_isInit == false) { ESP_LOGE(TAG, "LCD was not initialized. Take care of coredump !!!"); }
  
}

void PM_LCD::printLine         (uint8_t row, std::string& line) {
  if (_isInit == false) { ESP_LOGE(TAG, "LCD was not initialized. Take care of coredump !!!"); }
  std::string subLine;
  subLine = std::string(line+_padding).substr(0, _columnNumber);
  _lcd->printf(subLine.c_str());
}

void PM_LCD::printScrollLine   (uint8_t row, std::string& line) {
  if (_isInit == false) { ESP_LOGE(TAG, "LCD was not initialized. Take care of coredump !!!"); }
  this->_printScrollLine(row, line, _scrollHorizontalDelay, _scrollDisplayTime);
}

void PM_LCD::printScrollLine   (uint8_t row, std::string& line, int displayTime) {
  if (_isInit == false) { ESP_LOGE(TAG, "LCD was not initialized. Take care of coredump !!!"); }
  this->_printScrollLine(row, line, _scrollHorizontalDelay, displayTime);
}

void PM_LCD::printScrollLine   (uint8_t row, std::string& line, int delayTime, int displayTime) {
  if (_isInit == false) { ESP_LOGE(TAG, "LCD was not initialized. Take care of coredump !!!"); }
  this->_printScrollLine(row, line, delayTime, displayTime);
}

void PM_LCD::_printScrollLine   (uint8_t row, std::string& line, int scrollHorizontalDelay, int scrollDisplayTime) {
  if (_isInit == false) { ESP_LOGE(TAG, "LCD was not initialized. Take care of coredump !!!"); }
  int time_to_display = scrollDisplayTime * 1000;
  std::string message = line + " ";
  int len = message.length();
  message = _padding + message + _padding;
  std::string sub_message;
  time_t start =millis();
  int j = start;
  while (j<start+time_to_display) {
    for (int position = 0; position < len+_columnNumber; position++) {
      _lcd->setCursor(0, row);
      sub_message=message.substr(position, _columnNumber);
      _lcd->print(sub_message.c_str());
      delay(scrollHorizontalDelay);
    }
    j=millis();
  }
}
