/**
******************************************************************************
* @file    USB_Device/DFU_Standalone/Src/main.c
* @author  MCD Application Team
* @version V1.3.2
* @date    13-November-2015
* @brief   USB device DFU demo main file
******************************************************************************
* @attention
*
* <h2><center>&copy; COPYRIGHT(c) 2015 STMicroelectronics</center></h2>
*
* Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
* You may not use this file except in compliance with the License.
* You may obtain a copy of the License at:
*
*        http://www.st.com/software_license_agreement_liberty_v2
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*
******************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "hw_config.h"
#include "boot_mode.h"
#include "params_hal.h"
#include "ui_hal.h"
#include "core_hal.h"
#include "boot_debug.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define BOOTLOADER_VERSION  1
#define LIGHTTIME           400

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
uint32_t BUTTON_press_time=0;

uint8_t USB_DFU_MODE          = 0;
uint8_t RESERVE_MODE          = 0;
uint8_t FACTORY_RESET_MODE    = 0;
uint8_t NC_MODE               = 0;
uint8_t ALL_RESET_MODE        = 0;
uint8_t START_APP_MODE        = 0;


/**
 * @brief  Main program
 * @param  None
 * @retval None
 */
int main(void)
{
    BOOT_DEBUG("boot start...\r\n");
    HAL_System_Config();
    HAL_UI_RGB_Color(RGB_COLOR_BLACK);

    HAL_PARAMS_Load_Boot_Params();
    HAL_PARAMS_Load_System_Params();

    if(BOOTLOADER_VERSION != HAL_PARAMS_Get_Boot_boot_version())
    {
        BOOT_DEBUG("save boot version...\r\n");
        HAL_PARAMS_Set_Boot_boot_version(BOOTLOADER_VERSION);
        HAL_PARAMS_Save_Params();
    }

    if(!HAL_UI_Mode_BUTTON_GetState(BUTTON1))
    {
#define TIMING_DFU_DOWNLOAD_MODE     1000   //dfu 下载模式
#define TIMING_RESERVE_MODE          3000   //保留模式
#define TIMING_RESERVE1_MODE         7000   //保留1模式
#define TIMING_RESERVE2_MODE         10000  //保留2模式
#define TIMING_FACTORY_RESET_MODE    13000  //恢复出厂程序判断时间 不清空密钥
#define TIMING_NC                    20000  //无操作判断时间
#define TIMING_ALL_RESET_MODE        30000  //完全恢复出厂判断时间 清空密钥
        while (!HAL_UI_Mode_BUTTON_GetState(BUTTON1))
        {
            BUTTON_press_time = HAL_UI_Mode_Button_Pressed();
            if( BUTTON_press_time > TIMING_ALL_RESET_MODE )
            {
                FACTORY_RESET_MODE = 0;
                ALL_RESET_MODE = 1;
                HAL_UI_RGB_Color(RGB_COLOR_YELLOW);
            }
            else if( BUTTON_press_time > TIMING_NC )
            {
                FACTORY_RESET_MODE = 0;
                NC_MODE = 1;
                HAL_UI_RGB_Color(RGB_COLOR_BLACK);
            }
            else if( BUTTON_press_time > TIMING_FACTORY_RESET_MODE )
            {
                RESERVE_MODE = 0;
                FACTORY_RESET_MODE = 1;
                HAL_UI_RGB_Color(RGB_COLOR_CYAN);
            }
            else if( BUTTON_press_time > TIMING_RESERVE2_MODE )
            {
                RESERVE_MODE = 1;
                HAL_UI_RGB_Color(RGB_COLOR_BLUE);
            }
            else if( BUTTON_press_time > TIMING_RESERVE1_MODE )
            {
                RESERVE_MODE = 1;
                HAL_UI_RGB_Color(RGB_COLOR_GREEN);
            }
            else if( BUTTON_press_time > TIMING_RESERVE_MODE )
            {
                USB_DFU_MODE = 0;
                RESERVE_MODE = 1;
                HAL_UI_RGB_Color(RGB_COLOR_RED);
            }
            else if( BUTTON_press_time > TIMING_DFU_DOWNLOAD_MODE )
            {
                USB_DFU_MODE = 1;
                HAL_UI_RGB_Color(RGB_COLOR_MAGENTA);
            }
        }
    }
    else
    {
        switch(HAL_PARAMS_Get_Boot_boot_flag())
        {
            case BOOT_FLAG_NORMAL:          //正常启动
                START_APP_MODE = 1;
                break;
            case BOOT_FLAG_DEFAULT_RESTORE: //默认程序下载
                RESERVE_MODE = 1;
                break;
            case BOOT_FLAG_SERIAL_COM:      //esp8266串口通讯
                RESERVE_MODE = 1;
                break;
            case BOOT_FLAG_FACTORY_RESET:   //恢复出厂
                FACTORY_RESET_MODE = 1;
                break;
            case BOOT_FLAG_OTA_UPDATE:      //在线升级
                RESERVE_MODE = 1;
                break;
            case BOOT_FLAG_ALL_RESET:       //完全恢复出厂  清除密钥
                ALL_RESET_MODE = 1;
                break;
            case BOOT_FLAG_USB_DFU:         //进入DFU下载模式
                USB_DFU_MODE = 1;
                break;
            default:
                break;
        }
    }

    //自动进入DFU下载模式
    if(0x7DEA == HAL_Core_Read_Backup_Register(BKP_DR_01))
    {
        USB_DFU_MODE = 1;
    }
    HAL_Core_Write_Backup_Register(BKP_DR_01, 0xFFFF);

    if(ALL_RESET_MODE)
    {
        BOOT_DEBUG("ALL factroy reset\r\n");
        Enter_Factory_ALL_RESTORE_Mode();
        ALL_RESET_MODE = 0;
    }
    else if(FACTORY_RESET_MODE)
    {
        BOOT_DEBUG("factroy reset\r\n");
        Enter_Factory_RESTORE_Mode();
        FACTORY_RESET_MODE = 0;
    }
    else if(USB_DFU_MODE)
    {
        BOOT_DEBUG("dfu\r\n");
        Enter_DFU_Mode();
    }
    else if(RESERVE_MODE||NC_MODE)
    {
        BOOT_DEBUG("reboot\r\n");
        System_Reset();
    }

    BOOT_DEBUG("start app\r\n");
    HAL_UI_RGB_Color(RGB_COLOR_RED);   // color the same with atom
    delay(LIGHTTIME);
    HAL_UI_RGB_Color(RGB_COLOR_GREEN); // color the same with atom
    delay(LIGHTTIME);
    HAL_UI_RGB_Color(RGB_COLOR_BLUE);  // color the same with atom
    delay(LIGHTTIME);
    HAL_UI_RGB_Color(RGB_COLOR_BLACK); //防止进入应用程序初始化三色灯 导致闪灯

    start_app();
    return 0;
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/