/*
  Yves Gaignard
  
  Main procedure of the Pool Manager project
*/

#define TAG "PM_TFT" 

#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>              // Hardware-specific TFT library https://github.com/Bodmer/TFT_eSPI
#include <FS.h>
#include <LittleFS.h>
#include <PNGdec.h>                // Include the PNG decoder library
#include <ESPPerfectTime.h>        // Library for time maangement

#include "PM_TFT.h"                // Pool manager TFT management
#include "PM_Log.h"                // Pool manager log management
#include "PM_Utils.h"
#include "PM_Parameters.h"

//#define FORCE_CALIBRATE 1

#define FileSys LittleFS

PNG png;
fs::File pngfile;
TFT_eSPI* _pGlobalTFT=nullptr;

int16_t xpos = 0;
int16_t ypos = 0;

//====================================================================================
//                            TFT Constructors / Destructors
//====================================================================================
PM_TFT::PM_TFT(int16_t TFT_Wide, int16_t TFT_Height, u_int8_t TFT_Led_pin) {
  _TFT_Wide = TFT_Wide;
  _TFT_Height = TFT_Height;
  _pTFT = new TFT_eSPI(TFT_Wide, TFT_Height);
  _TFT_Led_Pin = TFT_Led_pin;
  _lastTouchTime = 0;
  Clear();
  Backlight();
}

PM_TFT::~PM_TFT(void) {
  delete _pTFT;
  _pTFT=nullptr;
  _TFT_Wide = 0;
  _TFT_Height = 0;
  _TFT_Led_Pin = 0;
  NoBacklight();
}

//====================================================================================
//                            TFT Init
//====================================================================================
void PM_TFT::Init(void) {
  // Initialise FS
  if (!FileSys.begin()) {
    LOG_E(TAG, "LittleFS initialisation failed!");
    return;
  }

  // Scan LittleFS files
  fs::File root = LittleFS.open("/", "r");
  while (fs::File file = root.openNextFile()) {
    std::string strname = file.name();
    strname = "/" + strname;
    LOG_D(TAG, "File: %s", strname.c_str());
  }
  root.close();

  // Initialise the TFT
  _pTFT->begin();
  pinMode(_TFT_Led_Pin, OUTPUT);
  digitalWrite(_TFT_Led_Pin,HIGH);
  _lastTouchTime = millis();
  _pTFT->fillScreen(TFT_BLACK);
  _pTFT->setRotation(3);

  uint16_t calibrationData[5];
  calibrationData[0] = 430;
  calibrationData[1] = 3450;
  calibrationData[2] = 400;
  calibrationData[3] = 3100;
  calibrationData[4] = 3;
  _pTFT->setTouch(calibrationData);
  LOG_I(TAG, "TFT calibrationData used from code:");
  for (int i=0; i<5; i++) { LOG_I(TAG, "calibrationData[%d] : %d\n", i, calibrationData[i]); }

#ifdef FORCE_CALIBRATE
  CalibrationScreen(); // if needed
#endif
  // Print the Pool Manager Home screen with measure
  PrintMeasuresScreen();
  _lastScreenRefresh = millis();
  LOG_I(TAG, "TFT Initialisation done.");
}

//====================================================================================
//                            Clear the screen
//====================================================================================
void PM_TFT::Clear(void) {
  _pTFT->fillScreen(TFT_BLACK);
}

//====================================================================================
//                       Stop backlight of screen
//====================================================================================
void PM_TFT::NoBacklight(void) {
  digitalWrite(_TFT_Led_Pin,LOW);
  _Backlight = FALSE;
}

//====================================================================================
//                       Activate backlight of screen
//====================================================================================
void PM_TFT::Backlight(void) {
  digitalWrite(_TFT_Led_Pin,HIGH);
  _Backlight = TRUE;
}

//====================================================================================
//                            Error Screen
//====================================================================================
void PM_TFT::PrintError(std::string ErrorNumber, std::string ErrorMessage) {
  // Display the error page
  ImageDraw(0, 0, (std::string)HOME_ERRORS);    // (480x320)
  std::string message = "Error number: " + ErrorNumber;
  _pTFT->drawString(message.c_str(),  10,  10, 4); 
  message = "Error: " + ErrorMessage;
  _pTFT->drawString(message.c_str(),  10,  25, 4); 
}

//====================================================================================
//                                    Loop
//====================================================================================
void PM_TFT::Loop(void)
{
  uint16_t x, y;
  unsigned long currentTime = millis();

  if ( currentTime > (_lastTouchTime + TFT_NO_TOUCH_TIMEOUT*1000)) { NoBacklight(); }  // shutdown the TFT light if inactivity > TFT_NO_TOUCH_TIMEOUT

  LOG_V(TAG, "TFT - In Loop");
  if (_pTFT->getTouch(&x, &y)) {
    LOG_D(TAG, "TFT - Touch in: %d , %d", x, y);

    if (! _Backlight) { Backlight(); } // Activate the TFT light if needed
    _lastTouchTime = millis();

    // Touch zone 
    if ( x>TOUCH_MEASURES_X && x <= (TOUCH_MEASURES_X+TOUCH_MEASURES_WIDTH) && y > TOUCH_ZONE_Y && y <= (TOUCH_ZONE_Y+TOUCH_ZONE_HEIGHT)) {
      LOG_D(TAG, "TFT - In Measures zone");
      PrintMeasuresScreen();
    }
    else if (x>TOUCH_SWITCHES_X && x <= (TOUCH_SWITCHES_X+TOUCH_SWITCHES_WIDTH) && y > TOUCH_ZONE_Y && y <= (TOUCH_ZONE_Y+TOUCH_ZONE_HEIGHT))
    {
      LOG_D(TAG, "TFT - In Switches zone");
      PrintSwitchesScreen();
    }
    else if (x>TOUCH_CALIBS_X && x <= (TOUCH_CALIBS_X+TOUCH_CALIBS_WIDTH) && y > TOUCH_ZONE_Y && y <= (TOUCH_ZONE_Y+TOUCH_ZONE_HEIGHT))
    {
      LOG_D(TAG, "TFT - In Calibs zone");
      PrintCalibsScreen();
    }
    else if ( x>TOUCH_SETTINGS_X && x <= (TOUCH_SETTINGS_X+TOUCH_SETTINGS_WIDTH) && y > TOUCH_ZONE_Y && y <= (TOUCH_ZONE_Y+TOUCH_ZONE_HEIGHT)) {
      LOG_D(TAG, "TFT - In Settings zone");
      PrintSettingsScreen();
    }
  }

  if (_Backlight && (currentTime > (_lastScreenRefresh + TFT_SCREEN_REFRESH*1000)) ) {
    if      (_currentScreen == SCR_MEASURES) { PrintMeasuresScreen(); }
    else if (_currentScreen == SCR_CALIBS  ) { PrintCalibsScreen(); }
    else if (_currentScreen == SCR_SWITCHES) { PrintSwitchesScreen(); }
    else if (_currentScreen == SCR_SETTINGS) { PrintSettingsScreen(); }
    _lastScreenRefresh = millis();
  }
  
}

//====================================================================================
//                               Measures Screen
//====================================================================================
void PM_TFT::PrintMeasuresScreen() {
  tm * time_tm;
  char timestamp_str[20];
  std::string pngName;

  // Select font color
  _pTFT->setTextColor(C_TURQUOISE, C_BLACK);

  // redraw completely the screen only if needed
  if ( _currentScreen != SCR_MEASURES ) {
    // Display the empty page
    ImageDraw(0, 0, (std::string)HOME_MEASURES);    // (480x320)
  
    // Coordinate are relatives to TOP-LEFT
    _pTFT->setTextDatum(TL_DATUM);

    // Display the pH
    _pTFT->drawString("pH"   ,  15,  50, 4); 
    _pTFT->drawString("(7.2)",  15, 115, 4);  // minimum valid pH Value
    _pTFT->drawString("(7.8)", 180,  50, 4);  // minimum valid pH Value

    // Display the ORP
    _pTFT->drawString("ORP"  , 250,  50, 4); 
    _pTFT->drawString("mV"   , 410,  92, 4);  // ORP units
    _pTFT->drawString("(550)", 250, 115, 4);  // minimum valid ORP Value
    _pTFT->drawString("(750)", 420,  50, 4);  // minimum valid ORP Value

    // Display the temperatures and the filtration pressure
    _pTFT->drawString("Water"    ,  10, 156, 4);
    _pTFT->drawString("o"        , 152, 150, 2);
    _pTFT->drawString("Air In"   ,  10, 189, 4); 
    _pTFT->drawString("o"        , 152, 183, 2);
    _pTFT->drawString("Air Out"  ,  10, 222, 4); 
    _pTFT->drawString("o"        , 152, 219, 2);
    _pTFT->drawString("Press.  " ,  10, 255, 4); 
    _pTFT->drawString("b"        , 152, 260, 2);

    // Display Pump state
    _pTFT->drawString("Pump / Uptime / Tank", 220, 156, 4);
    _pTFT->drawString("Filtr."   , 175, 189, 4);
    _pTFT->drawString("/"        , 390, 189, 4);
    _pTFT->drawString("pH-"      , 175, 222, 4); 
    _pTFT->drawString("Chl"      , 175, 255, 4); 
  }

  // Display header of the page
  PrintScreenHeader();

  // Coordinate are relatives to TOP-RIGHT
  _pTFT->setTextDatum(TR_DATUM);

  // Display the pH
  _pTFT->drawFloat (pm_measures.pHValue, 1,  160,  75, 6);  // current pH Value

  // Display the ORP
  _pTFT->drawNumber(pm_measures.OrpValue,    405,  75, 6);  // current ORP Value

  // Coordinate are relatives to TOP-RIGHT
  _pTFT->setTextDatum(TR_DATUM);

  // Display the temperatures
  _pTFT->drawFloat (pm_measures.WaterTemp, 1   , 150, 156, 4);  // current water temperature
  _pTFT->drawFloat (pm_measures.InAirTemp, 1   , 150, 189, 4);  // current Indoor Air temperature
  _pTFT->drawFloat (pm_measures.OutAirTemp,1   , 150, 222, 4);  // current Outdoor Air temperature
  _pTFT->drawFloat (pm_measures.Pressure , 1   , 150, 255, 4);  // current pressure

  // Display Pump state
  if (pm_measures.FilterPumpState) { pngName = (std::string)ONS_SMALL; }
  else                             { pngName = (std::string)OFFS_SMALL; }
  ImageDraw(250, 190, pngName);
  if (pm_measures.pHMinusPumpState) { pngName = (std::string)ONS_SMALL; }
  else                              { pngName = (std::string)OFFS_SMALL; }
  ImageDraw(250, 223, pngName); 
  if (pm_measures.ChlorinePumpState) { pngName = (std::string)ONS_SMALL; }
  else                               { pngName = (std::string)OFFS_SMALL; }
  ImageDraw(250, 256, pngName); 

  // Display Filtration time
  time_tm = gmtime(&pm_measures.DayFiltrationUptime);
	strftime(timestamp_str, sizeof(timestamp_str), PM_HourMinFormat, time_tm);
  _pTFT->drawString(timestamp_str, 380, 189, 4);

  time_tm = gmtime(&pm_measures.DayFiltrationTarget);
	strftime(timestamp_str, sizeof(timestamp_str), PM_HourFormat, time_tm);
  _pTFT->drawString(timestamp_str, 450, 189, 4);
  
  // Display pH pump uptime
  time_tm = gmtime(&pm_measures.pHPumpUptime);
	strftime(timestamp_str, sizeof(timestamp_str), PM_HourMinFormat, time_tm);
  _pTFT->drawString(timestamp_str, 380, 222, 4);

  // Display Chlorine pump uptime
  time_tm = gmtime(&pm_measures.OrpPumpUptime);
	strftime(timestamp_str, sizeof(timestamp_str), PM_HourMinFormat, time_tm);
  _pTFT->drawString(timestamp_str, 380, 255, 4);

  // Display Tank Fill & usage time
  std::string pHMinusTankFill = PM_itoa((int) (pm_measures.pHMinusTankFill + 0.5 - (pm_measures.pHMinusTankFill<0)));
  pHMinusTankFill+=" %";
  _pTFT->drawString(pHMinusTankFill.c_str()   , 470, 222, 4);

  std::string ChlorineTankFill = PM_itoa((int) (pm_measures.ChlorineTankFill + 0.5 - (pm_measures.ChlorineTankFill<0)));
  ChlorineTankFill+=" %"; 
  _pTFT->drawString(ChlorineTankFill.c_str()  , 470, 255, 4);

  // Change the current screen with this one
  _currentScreen = SCR_MEASURES;
}

//====================================================================================
//                               Switches Screen
//====================================================================================
void PM_TFT::PrintSwitchesScreen() {

  // Select font color
  _pTFT->setTextColor(C_TURQUOISE, C_BLACK);
  
  // redraw completely the screen only if needed
  if ( _currentScreen != SCR_SWITCHES ) {  
    // Display the empty page
    ImageDraw(0, 0, (std::string)HOME_SWITCHES);    // (480x320)
  }
  
  // Display header of the page
  PrintScreenHeader();

  // Change the current screen with this one
  _currentScreen = SCR_SWITCHES;
}

//====================================================================================
//                               Calibs Screen
//====================================================================================
void PM_TFT::PrintCalibsScreen() {

  // Select font color
  _pTFT->setTextColor(C_TURQUOISE, C_BLACK);
  
  // redraw completely the screen only if needed
  if ( _currentScreen != SCR_CALIBS ) {  
    // Display the empty page
    ImageDraw(0, 0, (std::string)HOME_CALIBS);    // (480x320)
  }  


  // Display header of the page
  PrintScreenHeader();

  // Change the current screen with this one
  _currentScreen = SCR_CALIBS;
}

//====================================================================================
//                               Settings Screen
//====================================================================================
void PM_TFT::PrintSettingsScreen() {

  // Select font color
  _pTFT->setTextColor(C_TURQUOISE, C_BLACK);

  // redraw completely the screen only if needed
  if ( _currentScreen != SCR_SETTINGS ) {  
    // Display the empty page
    ImageDraw(0, 0, (std::string)HOME_SETTINGS);    // (480x320)
  }
  
  // Display header of the page
  PrintScreenHeader();

  // Change the current screen with this one
  _currentScreen = SCR_SETTINGS;
}

//====================================================================================
//                               Screen Header
//====================================================================================
void PM_TFT::PrintScreenHeader() { 
  // Select font color
  _pTFT->setTextColor(C_TURQUOISE, C_BLACK);
  // Coordinate are relatives to TOP-RIGHT
  _pTFT->setTextDatum(TR_DATUM);

  // Display date and time
  tm * time_tm;
  char timestamp_str[20];
  suseconds_t usec;
  now = pftime::time(nullptr); // get current time
  time_tm = pftime::localtime(&now, &usec);  // Change in localtime
  strftime(timestamp_str, sizeof(timestamp_str), "%Y-%m-%d  %H:%M", time_tm);
  _pTFT->drawString(timestamp_str,  470,  10, 4);

  // Display Wifi state
  if (IsWifiConnected) {
    ImageDraw(210, 4, (std::string)ICON_WIFI_ON); 
  } 
  else {
    ImageDraw(210, 4, (std::string)ICON_WIFI_OFF); 
  } 
}

//====================================================================================
//                            Draw any PNG image on screen
//====================================================================================
int16_t PM_TFT::ImageDraw(const int16_t x_pos, const int16_t y_pos, const char* filename) {
  std::string Str_filename = filename;
  return ImageDraw(x_pos, y_pos, Str_filename);
}

int16_t PM_TFT::ImageDraw(const int16_t x_pos, const int16_t y_pos, const std::string& filename) {
  int16_t rc = 0;
  xpos=x_pos;
  ypos=y_pos;
  fs::File file = LittleFS.open(filename.c_str());
  if (!file.isDirectory() && string_endsWith(filename, ".png")) {
    // Pass support callback function names to library
    _pGlobalTFT = _pTFT;
    rc = png.open(filename.c_str(), pngOpen, pngClose, pngRead, pngSeek, pngDraw);
    if (rc == PNG_SUCCESS) {
      _pTFT->startWrite();
      LOG_V(TAG, "TFT - image specs: (%d x %d), %d bpp, pixel type: %d", png.getWidth(), png.getHeight(), png.getBpp(), png.getPixelType());
      uint32_t dt = millis();
      if (png.getWidth() > MAX_IMAGE_WDITH) {
        LOG_E(TAG, "TFT - Image too wide for allocated line buffer size!");
      }
      else {
        rc = png.decode(NULL, 0);
        png.close();
      }
      _pTFT->endWrite();
    }
  }
  else {
    LOG_E(TAG, "TFT - File: %s is a directory or finish not by .png", filename.c_str());
    rc=1;
  }
  return rc;
}

// Here are the callback functions that the decPNG library
// will use to open files, fetch data and close the file.

void * PM_TFT::pngOpen(const char *filename, int32_t *size) {
  LOG_V(TAG, "TFT - Attempting to open %s", filename);
  pngfile = FileSys.open(filename, "r");
  *size = pngfile.size();
  return &pngfile;
}

void PM_TFT::pngClose(void *handle) {
  fs::File pngfile = *((fs::File*)handle);
  if (pngfile) pngfile.close();
}

int32_t PM_TFT::pngRead(PNGFILE *page, uint8_t *buffer, int32_t length) {
  if (!pngfile) return 0;
  page = page; // Avoid warning
  return pngfile.read(buffer, length);
}

int32_t PM_TFT::pngSeek(PNGFILE *page, int32_t position) {
  if (!pngfile) return 0;
  page = page; // Avoid warning
  return pngfile.seek(position);
}

void PM_TFT::pngDraw(PNGDRAW *pDraw) {
  uint16_t lineBuffer[MAX_IMAGE_WDITH];
  png.getLineAsRGB565(pDraw, lineBuffer, PNG_RGB565_BIG_ENDIAN, 0xffffffff);
  _pGlobalTFT->pushImage(xpos, ypos + pDraw->y, pDraw->iWidth, 1, lineBuffer);
}

//====================================================================================
//                            Calibration screen for the touch
//====================================================================================
void PM_TFT::CalibrationScreen() {
  uint16_t calibrationData[5];
  uint8_t calDataOK = 0;

  _pTFT->setCursor(20, 0, 2);
  _pTFT->setTextColor(TFT_BLACK, TFT_WHITE);  
  _pTFT->setTextSize(1);
  _pTFT->println("calibration run");

  // check if calibration file exists
  if (LittleFS.exists(CALIBRATION_FILE)) {
    fs::File f = LittleFS.open(CALIBRATION_FILE, "r");
    if (f) {
      if (f.readBytes((char *)calibrationData, 14) == 14)
        calDataOK = 1;
      f.close();
    }
    LOG_I(TAG, "TFT - calibrationData existing in file: %s", CALIBRATION_FILE);
    for (int i=0; i<5; i++) {
      LOG_I(TAG, "TFT - calibrationData[%d] : %d", i, calibrationData[i]);
    }
  }

  if (calDataOK) {
    // calibration data valid
    _pTFT->setTouch(calibrationData);
    LOG_I(TAG, "TFT - calibrationData used from in file: %s", CALIBRATION_FILE);
    for (int i=0; i<5; i++) { 
      LOG_I(TAG, "TFT - calibrationData[%d] : %d", i, calibrationData[i]);
    }
  } else {
    // data not valid. recalibrate
    _pTFT->fillScreen((0xFFFF));
    _pTFT->calibrateTouch(calibrationData, TFT_WHITE, TFT_RED, 5);
    // store data
    fs::File f = LittleFS.open(CALIBRATION_FILE, "w");
    if (f) {
      f.write((const unsigned char *)calibrationData, 14);
      f.close();
    }
    _pTFT->fillScreen((0xFFFF));
    LOG_I(TAG, "TFT - calibrationData recalculated");
    for (int i=0; i<5; i++) { 
      LOG_I(TAG, "TFT - calibrationData[%d] : %d", i, calibrationData[i]);
    }
  }
}
