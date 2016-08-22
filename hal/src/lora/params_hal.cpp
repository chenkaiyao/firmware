/**
 ******************************************************************************
  Copyright (c) 2013-2014 IntoRobot Team.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation, either
  version 3 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, see <http://www.gnu.org/licenses/>.
  ******************************************************************************
*/

#include <string.h>
#include <stdio.h>
#include "params_impl.h"
#include "params_hal.h"
#include "eeprom_hal.h"
#include "memory_hal.h"
#include "intorobot_macros.h"
#include "service_debug.h"



#define FLASH_SYSTEM_PARAMS_START_ADDRESS              ((uint32_t)0x08010000)
#define FLASH_SYSTEM_PARAMS_END_ADDRESS                ((uint32_t)0x0801FFFF)

#define EEPROM_BOOT_PARAMS_MAX_SIZE                        (512)    //参数区大小

boot_params_t intorobot_boot_params;         //bootloader参数
system_params_t intorobot_system_params;     //设备参数

//board type
#define INTOROBOT_BOARD_TYPE    "888002"
#define INTOROBOT_BOARD_TYPE1    "887002"


uint16_t HAL_Board_Type(char* dest, uint16_t destLen, uint8_t type)
{
    uint32_t len;

    if (dest!=NULL && destLen>0) {
        len = MIN(strlen(INTOROBOT_BOARD_TYPE1), destLen-1);
        if(0==type) {
            memcpy(dest, INTOROBOT_BOARD_TYPE, len);
        }
        else{
            memcpy(dest, INTOROBOT_BOARD_TYPE1, len);
        }
        return len;
    }
    return 0;
}

uint32_t HAL_Platform_ID(void)
{
    return PLATFORM_ID;
}

/*初始化bootloader参数区*/
void init_boot_params(boot_params_t *pboot_params) {
    memset((uint8_t *)pboot_params, 0, sizeof(boot_params_t));
    pboot_params->header = BOOT_PARAMS_HEADER;
}

/*初始化系统参数区*/
void init_system_params(system_params_t *psystem_params) {
    memset((uint8_t *)psystem_params, 0, sizeof(system_params_t));
    psystem_params->header = SYSTEM_PARAMS_HEADER;
    psystem_params->config_flag = 1;
    psystem_params->zone = 8;
}

/*初始化系统参数区 保留密钥参数*/
void init_fac_system_params(system_params_t *psystem_params) {
    uint8_t  at_mode;                  // 是否已经灌装密钥  0:未灌装 1:已经灌装
    uint8_t  device_id[52]={0};        // 设备序列号
    uint8_t  access_token[52]={0};     // 设备access_token

    at_mode = psystem_params->at_mode;
    memcpy(device_id, psystem_params->device_id, sizeof(psystem_params->device_id));
    memcpy(access_token, psystem_params->access_token, sizeof(psystem_params->access_token));

    init_system_params(psystem_params);

    psystem_params->at_mode = at_mode;
    memcpy(psystem_params->device_id, device_id, sizeof(psystem_params->device_id));
    memcpy(psystem_params->access_token, access_token, sizeof(psystem_params->access_token));
}

void read_boot_params(boot_params_t *pboot_params) {
    uint32_t len = sizeof(boot_params_t);
    uint32_t address = HAL_EEPROM_Length() - EEPROM_BOOT_PARAMS_MAX_SIZE;
    uint8_t *pboot = (uint8_t *)pboot_params;

    memset(pboot, 0, len);
    if(len > EEPROM_BOOT_PARAMS_MAX_SIZE) {
        return;
    }

    for (int num = 0; num<len; num++) {
        pboot[num] = HAL_EEPROM_Read(address+num);
    }
}

/*
 * 保存bootloader参数区
 * */
void save_boot_params(boot_params_t *pboot_params) {
    uint32_t len = sizeof(boot_params_t);
    uint32_t address = HAL_EEPROM_Length() - EEPROM_BOOT_PARAMS_MAX_SIZE;
    uint8_t *pboot = (uint8_t *)pboot_params;

    if(len > EEPROM_BOOT_PARAMS_MAX_SIZE) {
        return;
    }

    for (int num = 0; num<len; num++) {
        HAL_EEPROM_Write(address+num, pboot[num]);
    }
}

/*
 * 加载系统参数区
 * */
void read_system_params(system_params_t *psystem_params) {
    uint32_t len = sizeof(system_params_t);

    memset(psystem_params, 0, sizeof(system_params_t));
    if(len > (FLASH_SYSTEM_PARAMS_END_ADDRESS-FLASH_SYSTEM_PARAMS_START_ADDRESS)) {
        return;
    }
    HAL_FLASH_Interminal_Read(FLASH_SYSTEM_PARAMS_START_ADDRESS, (uint16_t *)psystem_params, len/2);
    if( SYSTEM_PARAMS_HEADER != psystem_params->header ) {
        init_system_params(psystem_params);
        HAL_FLASH_Interminal_Read(FLASH_SYSTEM_PARAMS_START_ADDRESS, (uint16_t *)psystem_params, len/2);
    }
}

/*
 * 保存系统参数区
 * */
void save_system_params(system_params_t *psystem_params) {
    uint32_t len = sizeof(system_params_t);

    if(len > (FLASH_SYSTEM_PARAMS_END_ADDRESS - FLASH_SYSTEM_PARAMS_START_ADDRESS)) {
        return;
    }
    HAL_FLASH_Interminal_Write(FLASH_SYSTEM_PARAMS_START_ADDRESS, (uint16_t *)psystem_params, len/2);
}

/*
 * 恢复默认参数  不保留密钥信息
 * */
void HAL_PARAMS_Init_All_System_Params(void) {
    init_system_params(&intorobot_system_params);
    save_system_params(&intorobot_system_params);
}

/*
 * 恢复默认参数  保留密钥信息
 * */

void HAL_PARAMS_Init_Fac_System_Params(void) {
    init_fac_system_params(&intorobot_system_params);
    save_system_params(&intorobot_system_params);
}
/*
 * 保存系统参数区
 * */

void HAL_PARAMS_Init_Boot_Params(void) {
    init_boot_params(&intorobot_boot_params);
    save_boot_params(&intorobot_boot_params);
}

/*
 * 读取bootloader参数区
 * */
void HAL_PARAMS_Load_System_Params(void) {
    read_system_params(&intorobot_system_params);
    if( SYSTEM_PARAMS_HEADER != intorobot_system_params.header ) {
        HAL_PARAMS_Init_All_System_Params();
    }
}

/*
 * 读取系统参数区
 * */
void HAL_PARAMS_Load_Boot_Params(void) {
    read_boot_params(&intorobot_boot_params);
    if( BOOT_PARAMS_HEADER != intorobot_boot_params.header ) {
        HAL_PARAMS_Init_Boot_Params();
    }
}

/*
 *  保存参数区
 * */
void HAL_PARAMS_Save_Params(void) {
    save_boot_params(&intorobot_boot_params);
    save_system_params(&intorobot_system_params);
}
/*
 * 读取设置启动标志
 * */
uint16_t HAL_PARAMS_Get_Boot_boot_flag(void) {
    return intorobot_boot_params.boot_flag;
}

/*
 * 保存设置启动标志
 * */
int HAL_PARAMS_Set_Boot_boot_flag(uint16_t flag) {
    intorobot_boot_params.boot_flag = flag;
    return 0;
}

/*
 * 读取设置是否恢复默认参数标志
 * */
uint16_t HAL_PARAMS_Get_Boot_initparam_flag(void) {
    return intorobot_boot_params.initparam_flag;
}

/*
 * 保存设置是否恢复默认参数标志
 * */
int HAL_PARAMS_Set_Boot_initparam_flag(uint16_t flag) {
    intorobot_boot_params.initparam_flag = flag;
    return 0;
}

/*
 * 保存设备ID
 * */
uint16_t HAL_PARAMS_Get_System_device_id(char* buffer, uint16_t len) {
    uint16_t templen;

    if (buffer!=NULL && len>0) {
        templen = MIN(strlen(intorobot_system_params.device_id), len-1);
        memset(buffer, 0, len);
        memcpy(buffer, intorobot_system_params.device_id, templen);
        return templen;
    }
    return 0;
}

/*
 * 设置设备ID
 * */
int HAL_PARAMS_Set_System_device_id(const char* buffer) {
    uint16_t templen;

    if (buffer!=NULL) {
        templen = MIN(sizeof(intorobot_system_params.device_id)-1, strlen(buffer));
        memset(intorobot_system_params.device_id, 0, sizeof(intorobot_system_params.device_id));
        memcpy(intorobot_system_params.device_id, buffer, templen);
        return 0;
    }
    return -1;
}

/*
 * 读取设备access token
 * */
uint16_t HAL_PARAMS_Get_System_access_token(char* buffer, uint16_t len) {
    uint16_t templen;

    if (buffer!=NULL && len>0) {
        templen = MIN(strlen(intorobot_system_params.access_token), len-1);
        memset(buffer, 0, len);
        memcpy(buffer, intorobot_system_params.access_token, templen);
        return templen;
    }
    return 0;
}

/*
 * 设置设备access token
 * */
int HAL_PARAMS_Set_System_access_token(const char* buffer) {
    uint16_t templen;

    if (buffer!=NULL) {
        templen = MIN(sizeof(intorobot_system_params.access_token)-1, strlen(buffer));
        memset(intorobot_system_params.access_token, 0, sizeof(intorobot_system_params.access_token));
        memcpy(intorobot_system_params.access_token, buffer, templen);
        return 0;
    }
    return -1;
}
/*
 * 读取at_mode标志
 * */
uint16_t HAL_PARAMS_Get_System_at_mode(void) {
    return intorobot_system_params.at_mode;
}

/*
 * 保存at_mode标志
 * */
int HAL_PARAMS_Set_System_at_mode(uint16_t flag) {
    intorobot_system_params.at_mode = flag;
    return 0;
}

/*
 * 读取时区
 * */
float HAL_PARAMS_Get_System_zone(void) {
    return intorobot_system_params.zone;
}

/*
 * 保存时区
 * */
int HAL_PARAMS_Set_System_zone(float zone) {
    intorobot_system_params.zone = zone;
    return 0;
}

/*
 * 读取服务器域名
 * */
uint16_t HAL_PARAMS_Get_System_sv_domain(char* buffer, uint16_t len) {
    uint16_t templen;

    if (buffer!=NULL && len>0) {
        templen = MIN(strlen(intorobot_system_params.sv_domain), len-1);
        memset(buffer, 0, len);
        memcpy(buffer, intorobot_system_params.sv_domain, templen);
        return templen;
    }
    return 0;
}

/*
 * 设置服务器域名
 * */
int HAL_PARAMS_Set_System_sv_domain(const char* buffer) {
    uint16_t templen;

    if (buffer!=NULL) {
        templen = MIN(sizeof(intorobot_system_params.sv_domain)-1, strlen(buffer));
        memset(intorobot_system_params.sv_domain, 0, sizeof(intorobot_system_params.sv_domain));
        memcpy(intorobot_system_params.sv_domain, buffer, templen);
        return 0;
    }
    return -1;
}

/*
 * 读取服务器端口号
 * */
int HAL_PARAMS_Get_System_sv_port(void) {
    return intorobot_system_params.sv_port;
}

/*
 * 设置服务器端口号
 * */
int HAL_PARAMS_Set_System_sv_port(int port) {
    intorobot_system_params.sv_port = port;
    return 0;
}

/*
 * 读取文件下载域名
 * */
uint16_t HAL_PARAMS_Get_System_dw_domain(char* buffer, uint16_t len) {
    uint16_t templen;

    if (buffer!=NULL && len>0) {
        templen = MIN(strlen(intorobot_system_params.dw_domain), len-1);
        memset(buffer, 0, len);
        memcpy(buffer, intorobot_system_params.dw_domain, templen);
        return templen;
    }
    return 0;
}

/*
 * 设置文件下载域名
 * */
int HAL_PARAMS_Set_System_dw_domain(const char* buffer) {
    uint16_t templen;

    if (buffer!=NULL) {
        templen = MIN(sizeof(intorobot_system_params.dw_domain)-1, strlen(buffer));
        memset(intorobot_system_params.dw_domain, 0, sizeof(intorobot_system_params.dw_domain));
        memcpy(intorobot_system_params.dw_domain, buffer, templen);
        return 0;
    }
    return -1;
}

/*
 * 读取sv_select标志
 * */
uint16_t HAL_PARAMS_Get_System_sv_select(void) {
    return intorobot_system_params.sv_select;
}

/*
 * 保存sv_select标志
 * */
int HAL_PARAMS_Set_System_sv_select(uint16_t flag) {
    intorobot_system_params.sv_select = flag;
    return 0;
}

/*
 * 读取固件库版本号
 * */
uint16_t HAL_PARAMS_Get_System_fwlib_ver(char* buffer, uint16_t len) {
    uint16_t templen;

    if (buffer!=NULL && len>0) {
        templen = MIN(strlen(intorobot_system_params.fwlib_ver), len-1);
        memset(buffer, 0, len);
        memcpy(buffer, intorobot_system_params.fwlib_ver, templen);
        return templen;
    }
    return 0;
}

/*
 * 设置固件库版本号
 * */
int HAL_PARAMS_Set_System_fwlib_ver(const char* buffer) {
    uint16_t templen;

    if (buffer!=NULL) {
        templen = MIN(sizeof(intorobot_system_params.fwlib_ver)-1, strlen(buffer));
        memset(intorobot_system_params.fwlib_ver, 0, sizeof(intorobot_system_params.fwlib_ver));
        memcpy(intorobot_system_params.fwlib_ver, buffer, templen);
        return 0;
    }
    return -1;
}

/*
 * 读取子系统版本号
 * */
uint16_t HAL_PARAMS_Get_System_subsys_ver(char* buffer, uint16_t len) {
    uint16_t templen;

    if (buffer!=NULL && len>0) {
        templen = MIN(strlen(intorobot_system_params.subsys_ver), len-1);
        memset(buffer, 0, len);
        memcpy(buffer, intorobot_system_params.subsys_ver, templen);
        return templen;
    }
    return 0;
}

/*
 * 设置文件下载域名
 * */
int HAL_PARAMS_Set_System_subsys_ver(const char* buffer) {
    uint16_t templen;

    if (buffer!=NULL) {
        templen = MIN(sizeof(intorobot_system_params.subsys_ver)-1, strlen(buffer));
        memset(intorobot_system_params.subsys_ver, 0, sizeof(intorobot_system_params.subsys_ver));
        memcpy(intorobot_system_params.subsys_ver, buffer, templen);
        return 0;
    }
    return -1;
}

/*
 * 读取参数配置标志
 * */
uint16_t HAL_PARAMS_Get_System_config_flag(void) {
   return intorobot_system_params.config_flag;
}

/*
 * 保存参数配置标志
 * */
int HAL_PARAMS_Set_System_config_flag(uint16_t flag) {
    intorobot_system_params.config_flag = flag;
    return 0;
}
