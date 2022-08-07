/*
  Copyright 2022 - Yves Gaignard
  
  Functions to manage a LCD device
*/
#ifndef PM_LCD_h
#define PM_LCD_h

#include <Arduino.h>
#include <LiquidCrystal_I2C.h>

class PM_LCD {
  private:
    LiquidCrystal_I2C*       _lcd;                         // LiquidCrystal_I2C handle 
    uint8_t                  _deviceAddress;               // Adress of the LCD device
    uint8_t                  _columnNumber;                // Column Number of the device
    uint8_t                  _rowNumber;                   // Row Number of the device
    boolean                  _isInit      = false;         // State of the LCD to store if the init function has been done 
    boolean                  _displayState= true;          // the LCD is on in case of true and is off in case of false
    boolean                  _backlight   = true;          // backlight state
    boolean                  _blink       = false;         // blink state 
    boolean                  _cursor      = false;         // cursor state
    boolean                  _scrollState = false;         // screen scroll state
    int                      _scrollHorizontalDelay = 250; // time in milliseconds between one column shift of the scroll 
    int                      _scrollVerticalDelay   = 250; // time in milliseconds between one row shift of the scroll
    int                      _scrollDisplayTime     = 10;  // number of seconds to display the row or the screen
    std::string              _padding;                     // string containing a blank line
    std::vector<std::string> _screen;                      // vector of strings. each string represent a line. The number of strings can be greater than the row number.
                                                           // if the scrollState == false, string vector index > max row will ignore 

    void _initLCD (uint8_t Device_Addr, uint8_t Cols, uint8_t Rows);

    void _printScrollLine   (uint8_t row, std::string& line, int scrollHorizontalDelay, int scrollDisplayTime);


  public:
    // Constructors
    PM_LCD(uint8_t Device_Addr, uint8_t Cols, uint8_t Rows);
    ~PM_LCD();
    
    LiquidCrystal_I2C*  getLCD();
    int                 getColumnNumber();
    int                 getRowNumber();

    boolean             getDisplayState();

    void init();
    void clear();
    void home();
    void display();
    void noDisplay();
    void blink();
    void noBlink();
    void cursor();
    void noCursor();
    void noBacklight();
    void backlight();

    void printScreen       (std::vector<std::string>& screen);
    void printScrollScreen (std::vector<std::string>& screen);
    void printScrollScreen (std::vector<std::string>& screen, int displayTime);

    void printLine         (uint8_t row, std::string& line);
    void printScrollLine   (uint8_t row, std::string& line);
    void printScrollLine   (uint8_t row, std::string& line, int displayTime);
    void printScrollLine   (uint8_t row, std::string& line, int delayTime, int displayTime);
};

#endif