/*
  Copyright 2022 - Yves Gaignard
  
  Functions to manage a LCD device
*/
#ifndef PM_TFT_h
#define PM_TFT_h

#include <Arduino.h>
#include <TFT_eSPI.h>
#include <PNGdec.h>                // Include the PNG decoder library

// Parameters of the TFT screen
#define SCREEN_WIDTH    480
#define SCREEN_HEIGHT   320
#define MAX_IMAGE_WDITH 480

#define TOUCH_ZONE_HEIGHT     35
#define TOUCH_ZONE_Y          285
#define TOUCH_MEASURES_WIDTH  99
#define TOUCH_MEASURES_X      0
#define TOUCH_SWITCHES_WIDTH  116
#define TOUCH_SWITCHES_X      99
#define TOUCH_CALIBS_WIDTH    132
#define TOUCH_CALIBS_X        215
#define TOUCH_SETTINGS_WIDTH  133
#define TOUCH_SETTINGS_X      347

// name of the files on SPIFFS
#define CALIBRATION_FILE    "/calibrationData"
#define HOME_MEASURES       "/Home_Measures.png"
#define HOME_MEASURES_2     "/Home_Measures2.png"
#define HOME_SETTINGS       "/Home_Settings.png"
#define HOME_SETTINGS_ERROR "/Home_Settings_Err.png"
#define HOME_SWITCHES       "/Home_Switches.png"
#define HOME_CALIBS         "/Home_Calibs.png"
#define HOME_ERRORS         "/Home_Errors.png"
#define ONS_SMALL           "/ONs_small.png"
#define ONS                 "/ONs.png"
#define OFFS_SMALL          "/OFFs_small.png"
#define OFFS                "/OFFs.png"
#define ICON_WIFI_ON        "/wifi_on.png"
#define ICON_WIFI_OFF       "/wifi_off.png"
#define SETTINGS_ERR        "/Settings_Err_Red.png"
#define TOUCH_ZONE          "/Touch_zone.png"
#define TOUCH_MEASURES      "/Touch_Measures_zone_99x35.png"
#define TOUCH_SWITCHES      "/Touch_Switches_zone_116x35.png"
#define TOUCH_CALIBS        "/Touch_Calibs_zone_132x35.png"
#define TOUCH_SETTINGS      "/Touch_Settings_zone_133x35.png"

// Color table in RGB565 (see: http://www.rinkydinkelectronics.com/calc_rgb565.php)
#define C_BLACK         0x0000  
#define C_TURQUOISE     0x1CB8   // RGB(28,151,194)
#define C_WHITE         0xFFFF

class PM_TFT {
  private:
    TFT_eSPI* _pTFT;
    int16_t   _TFT_Wide;
    int16_t   _TFT_Height;
    u_int8_t  _TFT_Led_Pin;

  public:
    // Constructors
    PM_TFT(int16_t TFT_Wide, int16_t TFT_Height, u_int8_t TFT_Led_pin);
    ~PM_TFT();

    void Init(void);
    void Loop(void);
    void Clear();
    void NoBacklight();
    void Backlight();

    void PrintError(std::string ErrorNumber, std::string ErrorMessage);
    void PrintMeasuresScreen();
    void PrintSwitchesScreen();
    void PrintCalibsScreen();
    void PrintSettingsScreen();
    void PrintScreenHeader();
    int16_t ImageDraw(int16_t _xpos, int16_t _ypos, const std::string& filename);
    void CalibrationScreen();

    static void *  pngOpen(const char *filename, int32_t *size);
    static void    pngClose(void *handle);
    static int32_t pngRead(PNGFILE *page, uint8_t *buffer, int32_t length) ;
    static int32_t pngSeek(PNGFILE *page, int32_t position) ;
    static void    pngDraw(PNGDRAW *pDraw);
};

#endif