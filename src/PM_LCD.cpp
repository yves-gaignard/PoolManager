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
  return _displayState;
}
    
void PM_LCD::init() {
  ESP_LOGD(TAG, "before init()");
  _lcd->init();
  ESP_LOGD(TAG, "after  init()");
}
void PM_LCD::clear() {
  ESP_LOGD(TAG, "before clear()");
  _lcd->clear();
  ESP_LOGD(TAG, "after  clear()");
}
void PM_LCD::home() {
  _lcd->home();
}
void PM_LCD::display() {
  _lcd->display();
  _displayState = true;
}
void PM_LCD::noDisplay() {
  _lcd->noDisplay();
  _displayState = false;
}
void PM_LCD::blink() {
  _lcd->blink();
  _blink=true;
}
void PM_LCD::noBlink() {
  _lcd->noBlink();
  _blink=false;
}
void PM_LCD::cursor() {
  _lcd->cursor();
  _cursor=true;
}
void PM_LCD::noCursor() {
  _lcd->noCursor();
  _cursor=false;
}
void PM_LCD::noBacklight() {
  _lcd->noBacklight();
  _backlight=false;
}
void PM_LCD::backlight() {
  _lcd->backlight();
  _backlight=true;
}

void PM_LCD::printScreen       (std::vector<std::string>& screen) {
  ESP_LOGV(TAG, "before printScreen->clear()");
  this->clear();
  ESP_LOGV(TAG, "before printScreen->display()");
  this->display();
  ESP_LOGV(TAG, "before printScreen->backlight()");
  this->backlight();
  ESP_LOGV(TAG, "before printScreen->home()");
  this->home();
  ESP_LOGV(TAG, "before printScreen->noBlink()");
  this->noBlink();
  ESP_LOGV(TAG, "before printScreen->noCursor()");
  this->noCursor();
  std::string subLine;
  for (int row =0; row < _rowNumber; row++) {
    subLine = std::string(screen[row]+_padding).substr(0, _columnNumber);
    ESP_LOGV(TAG, "before setcursor(0,%d)",row);
    _lcd->setCursor(0,row);
    ESP_LOGV(TAG, "before printf(%s)",subLine.c_str());
    _lcd->print(subLine.c_str());
  }
  ESP_LOGV(TAG, "end of printScreen");
}
void PM_LCD::printScrollScreen (std::vector<std::string>& screen) {
  
}
void PM_LCD::printScrollScreen (std::vector<std::string>& screen, int displayTime) {
  
}

void PM_LCD::printLine         (uint8_t row, std::string& line) {
  std::string subLine;
  subLine = std::string(line+_padding).substr(0, _columnNumber);
  _lcd->printf(subLine.c_str());
}

void PM_LCD::printScrollLine   (uint8_t row, std::string& line) {
  this->_printScrollLine(row, line, _scrollHorizontalDelay, _scrollDisplayTime);
}

void PM_LCD::printScrollLine   (uint8_t row, std::string& line, int displayTime) {
  this->_printScrollLine(row, line, _scrollHorizontalDelay, displayTime);
}

void PM_LCD::printScrollLine   (uint8_t row, std::string& line, int delayTime, int displayTime) {
  this->_printScrollLine(row, line, delayTime, displayTime);
}

void PM_LCD::_printScrollLine   (uint8_t row, std::string& line, int scrollHorizontalDelay, int scrollDisplayTime) {
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
