/*
  Yves Gaignard
  
  Pool manager time management
*/

/* LwIP SNTP example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#define TAG "PM_Time_Mngt"

#include <Arduino.h>
#include "PM_Pool_Manager.h"
#include "PM_Time_Mngt.h"
#include "PM_Parameters.h"
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_attr.h"
#include "esp_sleep.h"
#include "nvs_flash.h"
#include "esp_sntp.h"

#ifndef INET6_ADDRSTRLEN
#define INET6_ADDRSTRLEN 48
#endif

extern "C" int setenv (const char *__string, const char *__value, int __overwrite);
extern "C" void tzset();

//static void obtain_time(void);
//static void initialize_sntp(void);

#ifdef CONFIG_SNTP_TIME_SYNC_METHOD_CUSTOM
void sntp_sync_time(struct timeval *tv)
{
   settimeofday(tv, NULL);
   LOG_I(TAG, "Time is synchronized from custom code");
   sntp_set_sync_status(SNTP_SYNC_STATUS_COMPLETED);
}
#endif

// Called-back function when the time is setup from NTP
void PM_Time_Mngt_time_sync_notification_cb(struct timeval *tv)
{
  LOG_I(TAG, "Notification of a time synchronization event");

  // If there is no RTC module or if it lost its power, set the time with the NTP time
  if (isRTCLostPower == true || isRTCFound == true) {
    LOG_I(TAG, "Initialize RTC time with NTP server");

    // Get current time. It has been sent by NTP calls.
    time(&now);

    // adjust time of rtc with the time get from NTP server
    DateTime DT_now (now);
    if (DT_now.isValid()) {
      char DT_now_str[20]= "YYYY-MM-DD hh:mm:ss";
      DT_now.toString(DT_now_str);
      LOG_I(TAG, "Adjust the time of RTC with the NTP time: %s", DT_now_str);
      rtc.adjust(DT_now);
    }
    else {
      LOG_E(TAG, "Cannot set time to RTC as DT_now is not valid !!!!" );
    }
  }
  
  /*
  // if there is a RTC module, set the time with RTC time
  if (isRTCFound == true) {
    
    // set time with the RTC time
    DateTime DT_now (rtc.now());
    char DT_now_str[20]= "YYYY-MM-DD hh:mm:ss";
    DT_now.toString(DT_now_str);
    LOG_I(TAG, "Get the time from RTC: %s", DT_now_str);
    
    // set the time
    now = DT_now.unixtime();
    LOG_I(TAG, "rtc.now = %u", now );
    timeval tv = {now, 0}; 
    timezone tz = {0,0} ;
    int ret = settimeofday(&tv, &tz);
    if ( ret != 0 ) {LOG_E(TAG, "Cannot set time from RTC" ); };
    
  }
  */
}

void PM_Time_Mngt_initialize_time(void)
{
    time_t now;
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);
    // Is time set? If not, tm_year will be (1970 - 1900).
    if (timeinfo.tm_year < (2016 - 1900)) {
        LOG_I(TAG, "Time is not set yet. Connecting to WiFi and getting time over NTP.");
        PM_Time_Mngt_obtain_time();
        // update 'now' variable with current time
        time(&now);
    }
#ifdef CONFIG_SNTP_TIME_SYNC_METHOD_SMOOTH
    else {
        // add 500 ms error to the current system time.
        // Only to demonstrate a work of adjusting method!
        {
            LOG_I(TAG, "Add a error for test adjtime");
            struct timeval tv_now;
            gettimeofday(&tv_now, NULL);
            int64_t cpu_time = (int64_t)tv_now.tv_sec * 1000000L + (int64_t)tv_now.tv_usec;
            int64_t error_time = cpu_time + 500 * 1000L;
            struct timeval tv_error = { .tv_sec = error_time / 1000000L, .tv_usec = error_time % 1000000L };
            settimeofday(&tv_error, NULL);
        }

        LOG_I(TAG, "Time was set, now just adjusting it. Use SMOOTH SYNC method.");
        obtain_time();
        // update 'now' variable with current time
        time(&now);
    }
#endif

    char strftime_buf[64];

    // Set timezone to the zone provided in the parameter file to have the real local time
    setenv("TZ", PM_TimeZone, 1);
    tzset();
    localtime_r(&now, &timeinfo);
    strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
    LOG_I(TAG, "The current local date/time is: %s", strftime_buf);

    if (sntp_get_sync_mode() == SNTP_SYNC_MODE_SMOOTH) {
        struct timeval outdelta;
        while (sntp_get_sync_status() == SNTP_SYNC_STATUS_IN_PROGRESS) {
            adjtime(NULL, &outdelta);
            LOG_I(TAG, "Waiting for adjusting time ... outdelta = %jd sec: %li ms: %li us",
                        (intmax_t)outdelta.tv_sec,
                        outdelta.tv_usec/1000,
                        outdelta.tv_usec%1000);
            vTaskDelay(2000 / portTICK_PERIOD_MS);
        }
    }

    //const int deep_sleep_sec = 10;
    //LOG_I(TAG, "Entering deep sleep for %d seconds", deep_sleep_sec);
    //esp_deep_sleep(1000000LL * deep_sleep_sec);
}

void PM_Time_Mngt_obtain_time(void)
{
    //ESP_ERROR_CHECK( nvs_flash_init() );
    //ESP_ERROR_CHECK( esp_netif_init());
    //ESP_ERROR_CHECK( esp_event_loop_create_default() );

    /**
     * NTP server address could be aquired via DHCP,
     * see following menuconfig options:
     * 'LWIP_DHCP_GET_NTP_SRV' - enable STNP over DHCP
     * 'LWIP_SNTP_DEBUG' - enable debugging messages
     *
     * NOTE: This call should be made BEFORE esp aquires IP address from DHCP,
     * otherwise NTP option would be rejected by default.
     */
#ifdef LWIP_DHCP_GET_NTP_SRV
    sntp_servermode_dhcp(1);      // accept NTP offers from DHCP server, if any
#endif

    /* This helper function configures Wi-Fi or Ethernet, as selected in menuconfig.
     * Read "Establishing Wi-Fi or Ethernet Connection" section in
     * examples/protocols/README.md for more information about this function.
     */
    //ESP_ERROR_CHECK(example_connect());

    PM_Time_Mngt_initialize_sntp();

    // wait for time to be set
    time_t now = 0;
    struct tm timeinfo = { 0 };
    int retry = 0;
    const int retry_count = 30;
    while (sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET && ++retry < retry_count) {
        LOG_I(TAG, "Waiting for system time to be set... (%d/%d)", retry, retry_count);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
    time(&now);
    localtime_r(&now, &timeinfo);

    //ESP_ERROR_CHECK( example_disconnect() );
}

void PM_Time_Mngt_initialize_sntp(void)
{
    LOG_I(TAG, "Initializing SNTP");
    sntp_setoperatingmode(SNTP_OPMODE_POLL);

/*
 * If 'NTP over DHCP' is enabled, we set dynamic pool address
 * as a 'secondary' server. It will act as a fallback server in case that address
 * provided via NTP over DHCP is not accessible
 */
#if LWIP_DHCP_GET_NTP_SRV && SNTP_MAX_SERVERS > 1
    sntp_setservername(1, "pool.ntp.org");

#if LWIP_IPV6 && SNTP_MAX_SERVERS > 2          // statically assigned IPv6 address is also possible
    ip_addr_t ip6;
    if (ipaddr_aton("2a01:3f7::1", &ip6)) {    // ipv6 ntp source "ntp.netnod.se"
        sntp_setserver(2, &ip6);
    }
#endif  /* LWIP_IPV6 */

#else   /* LWIP_DHCP_GET_NTP_SRV && (SNTP_MAX_SERVERS > 1) */
    // otherwise, use DNS address from a pool
    //sntp_setservername(0, CONFIG_SNTP_TIME_SERVER);
    char ntp_fr[] = "fr.pool.ntp.org";
    char ntp[] = "pool.ntp.org";
    sntp_setservername(0, ntp_fr);  // set the secondary NTP server (will be used only if SNTP_MAX_SERVERS > 1)
    sntp_setservername(1, ntp);     // set the secondary NTP server (will be used only if SNTP_MAX_SERVERS > 1)
#endif

    sntp_set_time_sync_notification_cb(PM_Time_Mngt_time_sync_notification_cb);
#ifdef CONFIG_SNTP_TIME_SYNC_METHOD_SMOOTH
    sntp_set_sync_mode(SNTP_SYNC_MODE_SMOOTH);
#endif
    sntp_init();

    LOG_I(TAG, "List of configured NTP servers:");

    for (uint8_t i = 0; i < SNTP_MAX_SERVERS; ++i){
        if (sntp_getservername(i)){
            LOG_I(TAG, "server %d: %s", i, sntp_getservername(i));
        } else {
            // we have either IPv4 or IPv6 address, let's print it
            char buff[INET6_ADDRSTRLEN];
            ip_addr_t const *ip = sntp_getserver(i);
            if (ipaddr_ntoa_r(ip, buff, INET6_ADDRSTRLEN) != NULL)
                LOG_I(TAG, "server %d: %s", i, buff);
        }
    }
}

/**
* Convert time_t time to string depending on a format
*/
std::string PM_Time_Mngt_convertTimeToString(time_t time_in, const char* string_format) {
  tm * time_tm;
	char timestamp_string[100];
  #define MAX_LENGTH_TIME_FORMAT 100
  char time_format[MAX_LENGTH_TIME_FORMAT]="%Y-%m-%dT%H:%M:%SZ";
  
  boolean isGmtRequested = false;
  int len=strlen(string_format);
  if (len > MAX_LENGTH_TIME_FORMAT-1) {
    LOG_E(TAG, "Too long datetime format: %d", len);
    return "";
  }
  if (len > 1) {
    strncpy(time_format,&string_format[len-1],len);
    char one_char[2];
    strncpy(one_char,&string_format[len-1],1);
    one_char[1] = '\0';
    LOG_V(TAG, "last char of format = %s", one_char);
    if  ( strcmp(one_char, "Z") == 0 ) { isGmtRequested = true; }
  }

  if (isGmtRequested) {
    LOG_V(TAG, "GMT time requested");
  	time_tm = gmtime(&time_in);
  }
  else {
    LOG_V(TAG, "LOCAL time requested");
  	time_tm = localtime(&time_in);
  }
	strftime(timestamp_string, 100, string_format, time_tm);
  LOG_V(TAG, "%s", timestamp_string);
  return timestamp_string;
}

/**
* Convert seconds in tm * structure
*/
tm PM_Time_Mngt_convertSecondsToTm(const ulong seconds) {
  tm local_time;
  long l = seconds & LONG_MAX;
  gmtime_r(&l, &local_time);
  return local_time;
}
