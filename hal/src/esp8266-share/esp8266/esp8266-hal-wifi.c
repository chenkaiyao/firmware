/*
 ESP8266WiFiGeneric.cpp - WiFi library for esp8266

 Copyright (c) 2014 Ivan Grokhotkov. All rights reserved.
 This file is part of the esp8266 core for Arduino environment.

 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 2.1 of the License, or (at your option) any later version.

 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Lesser General Public License for more details.

 You should have received a copy of the GNU Lesser General Public
 License along with this library; if not, write to the Free Software
 Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

 Reworked on 28 Dec 2015 by Markus Sattler

 */

#include "esp8266-hal-wifi.h"
#include "c_types.h"
#include "ets_sys.h"
#include "os_type.h"
#include "osapi.h"
#include "mem.h"
#include "user_interface.h"
#include "smartconfig.h"
#include "lwip/err.h"
#include "lwip/dns.h"
#include "tick_hal.h"
#include "net_hal.h"
#include "core_hal_esp8266.h"
#include "molmc_log.h"

const static char *TAG = "hal-wifi";

static volatile uint32_t esp8266_wifi_timeout_start;
static volatile uint32_t esp8266_wifi_timeout_duration;

inline void ARM_WIFI_TIMEOUT(uint32_t dur) {
    esp8266_wifi_timeout_start = HAL_Tick_Get_Milli_Seconds();
    esp8266_wifi_timeout_duration = dur;
    MOLMC_LOGD(TAG, "HAL WIFI WD Set %d",(dur));
}
inline bool IS_WIFI_TIMEOUT(void) {
    return esp8266_wifi_timeout_duration && ((HAL_Tick_Get_Milli_Seconds()-esp8266_wifi_timeout_start)>esp8266_wifi_timeout_duration);
}

inline void CLR_WIFI_TIMEOUT(void) {
    esp8266_wifi_timeout_duration = 0;
    MOLMC_LOGD(TAG, "HAL WIFI WD Cleared, was %d", esp8266_wifi_timeout_duration);
}

static void _eventCallback(System_Event_t * evt)
{
    switch (evt->event) {
        case EVENT_STAMODE_CONNECTED:
            MOLMC_LOGD(TAG, "EVENT_STAMODE_CONNECTED");
            break;
        case EVENT_STAMODE_DISCONNECTED:
            MOLMC_LOGD(TAG, "EVENT_STAMODE_DISCONNECTED");
            HAL_NET_notify_disconnected();
            break;
        case EVENT_STAMODE_AUTHMODE_CHANGE:
            MOLMC_LOGD(TAG, "EVENT_STAMODE_AUTHMODE_CHANGE");
            break;
        case EVENT_STAMODE_GOT_IP:
            MOLMC_LOGD(TAG, "EVENT_STAMODE_GOT_IP");
            HAL_NET_notify_dhcp(true);
            HAL_NET_notify_connected();
            break;
        case EVENT_SOFTAPMODE_STACONNECTED:
        case EVENT_SOFTAPMODE_STADISCONNECTED:
            break;
        default:
            break;
    }
}

/**
 * set new mode
 * @param m WiFiMode_t
 */
bool esp8266_wifiInit(void)
{
    MOLMC_LOGD(TAG, "esp8266_wifiInit");
    wifi_set_event_handler_cb(_eventCallback);
    return true;
}

/**
 * set new mode
 * @param m WiFiMode_t
 */
bool esp8266_setMode(WiFiMode_t m)
{
    if(wifi_get_opmode() == (uint8) m) {
        return true;
    }

    bool ret = false;

    ETS_UART_INTR_DISABLE();
    ret = wifi_set_opmode(m);
    ETS_UART_INTR_ENABLE();

    return ret;
}

/**
 * get WiFi mode
 * @return WiFiMode
 */
WiFiMode_t esp8266_getMode(void)
{
    return (WiFiMode_t) wifi_get_opmode();
}

/**
 * control STA mode
 * @param enable bool
 * @return ok
 */
bool esp8266_enableSTA(bool enable)
{

    WiFiMode_t currentMode = esp8266_getMode();
    bool isEnabled = ((currentMode & WIFI_STA) != 0);

    if(isEnabled != enable) {
        if(enable) {
            return esp8266_setMode((WiFiMode_t)(currentMode | WIFI_STA));
        } else {
            return esp8266_setMode((WiFiMode_t)(currentMode & (~WIFI_STA)));
        }
    } else {
        return true;
    }
}

/**
 * control AP mode
 * @param enable bool
 * @return ok
 */
bool esp8266_enableAP(bool enable)
{

    WiFiMode_t currentMode = esp8266_getMode();
    bool isEnabled = ((currentMode & WIFI_AP) != 0);

    if(isEnabled != enable) {
        if(enable) {
            return esp8266_setMode((WiFiMode_t)(currentMode | WIFI_AP));
        } else {
            return esp8266_setMode((WiFiMode_t)(currentMode & (~WIFI_AP)));
        }
    } else {
        return true;
    }
}

/**
 * Get the station interface MAC address.
 * @param mac   pointer to uint8_t array with length WL_MAC_ADDR_LENGTH
 * @return      pointer to uint8_t *
 */
uint8_t* esp8266_getMacAddress(uint8_t* mac)
{
    wifi_get_macaddr(STATION_IF, mac);
    return mac;
}

bool esp8266_setDHCP(char enable)
{
    if(true == enable) {
        wifi_station_dhcpc_start();
    } else {
        wifi_station_dhcpc_stop();
    }
    return true;
}

/**
 * Setting the ESP8266 station to connect to the AP (which is recorded)
 * automatically or not when powered on. Enable auto-connect by default.
 * @param autoConnect bool
 * @return if saved
 */
bool esp8266_setAutoConnect(bool autoConnect)
{
    bool ret;
    ETS_UART_INTR_DISABLE();
    ret = wifi_station_set_auto_connect(autoConnect);
    ETS_UART_INTR_ENABLE();
    return ret;
}

/**
 * Checks if ESP8266 station mode will connect to AP
 * automatically or not when it is powered on.
 * @return auto connect
 */
bool esp8266_getAutoConnect(void)
{
    return (wifi_station_get_auto_connect() != 0);
}

/**
 * Set whether reconnect or not when the ESP8266 station is disconnected from AP.
 * @param autoReconnect
 * @return
 */
bool esp8266_setAutoReconnect(bool autoReconnect)
{
    return wifi_station_set_reconnect_policy(autoReconnect);
}

/**
 * Return the current network RSSI.
 * @return  RSSI value
 */
int32_t esp8266_getRSSI(void)
{
    return wifi_station_get_rssi();
}

int esp8266_connect(void)
{
    ETS_UART_INTR_DISABLE();
    wifi_station_connect();
    ETS_UART_INTR_ENABLE();
    return 0;
}

int esp8266_disconnect(void)
{
    ETS_UART_INTR_DISABLE();
    wifi_station_disconnect();
    ETS_UART_INTR_ENABLE();
    return 0;
}

static bool _smartConfigStarted = false;
static bool _smartConfigDone = false;

/**
 * _smartConfigCallback
 * @param st
 * @param result
 */
void smartConfigCallback(uint32_t st, void* result)
{
    sc_status status = (sc_status) st;

    MOLMC_LOGD(TAG, "beginSmartConfig status = %d", status);
    if(status == SC_STATUS_LINK) {
        struct station_config* sta_conf = (struct station_config*)(result);
        MOLMC_LOGD(TAG, "ssid     = %s", sta_conf->ssid);
        MOLMC_LOGD(TAG, "password = %s", sta_conf->password);
        wifi_station_set_config(sta_conf);
        wifi_station_disconnect();
        wifi_station_connect();
    } else if(status == SC_STATUS_LINK_OVER) {
        _smartConfigDone = true;
    }
}

/**
 * Start SmartConfig
 */
bool esp8266_beginSmartConfig(void)
{
    if(_smartConfigStarted) {
        return false;
    }

    MOLMC_LOGD(TAG, "esp8266_beginSmartConfig");
    if(!esp8266_enableSTA(true)) {
        // enable STA failed
        return false;
    }

    if(smartconfig_start((sc_callback_t)(&smartConfigCallback), 0)) {
        _smartConfigStarted = true;
        _smartConfigDone = false;
        return true;
    }
    return false;
}

/**
 *  Stop SmartConfig
 */
bool esp8266_stopSmartConfig(void)
{
    if(!_smartConfigStarted) {
        return true;
    }

    MOLMC_LOGD(TAG, "esp8266_stopSmartConfig");
    if(smartconfig_stop()) {
        _smartConfigStarted = false;
        return true;
    }
    return false;
}

/**
 * Query SmartConfig status, to decide when stop config
 * @return smartConfig Done
 */
bool esp8266_smartConfigDone(void)
{
    if(!_smartConfigStarted) {
        return false;
    }

    return _smartConfigDone;
}

volatile uint8_t _dns_founded=0;

void wifi_dns_found_callback(const char *name, ip_addr_t *ipaddr, void *callback_arg)
{
    if(ipaddr) {
        (*(uint32_t*)(callback_arg)) = ipaddr->addr;
    }
    _dns_founded = 1;
}

int esp8266_gethostbyname(const char* hostname, uint16_t hostnameLen, uint32_t *ip_addr)
{
    ip_addr_t addr;

    if(!strcmp(hostname, "255.255.255.255")) {
        *ip_addr = IPADDR_NONE;
        return 0;
    }

    addr.addr = ipaddr_addr(hostname);
    if (addr.addr != IPADDR_NONE) {
        *ip_addr = addr.addr;
        return 0;
    }

    *ip_addr = 0;
    err_t err = dns_gethostbyname(hostname, &addr, &wifi_dns_found_callback, ip_addr);
    if(err == ERR_OK) {
        *ip_addr = addr.addr;
    } else if(err == ERR_INPROGRESS) {
        ARM_WIFI_TIMEOUT(2000);
        _dns_founded =0;
        while (!_dns_founded) {
            optimistic_yield(100);
            if(IS_WIFI_TIMEOUT()) {
                CLR_WIFI_TIMEOUT();
                break;
            }
        }
        // will return here when dns_found_callback fires
        if(*ip_addr != 0) {
            err = ERR_OK;
        }
    }
    return (err == ERR_OK) ? 0 : 1;
}

