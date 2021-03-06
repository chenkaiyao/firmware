#include "hw_config.h"
#include "hw_timer.h"
#include "esp8266_config.h"


uint8_t esp8266_gpioToFn[16] = {0x34, 0x18, 0x38, 0x14, 0x3C, 0x40, 0x1C, 0x20, 0x24, 0x28, 0x2C, 0x30, 0x04, 0x08, 0x0C, 0x10};
extern uint8_t _bss_start, _bss_end;

static void zero_bss(void) {
    uint32_t *addr;

    for (addr = &_bss_start; addr < &_bss_end; addr++) {
        *addr = 0;
    }
}

/**
 * @brief  System Clock Configuration
 * @param  96M
 * @retval None
 */
void SystemClock_Config(void)
{
    if(rom_i2c_readReg(103,4,1) == 8) { // 8: 40MHz, 136: 26MHz
        // set 80MHz PLL CPU (Q = 26MHz)
        rom_i2c_writeReg(103,4,1,136);
        rom_i2c_writeReg(103,4,2,145);
        ets_delay_us(150);
    }
}
/**
 * @brief  This function handles SysTick Handler.
 * @param  None
 * @retval None
 */
void SysTick_Handler(void)
{
    HAL_UI_SysTick_Handler();
}

/**
 * @brief  System Clock Configuration
 * @param  96M
 * @retval None
 */
void HwTimer_config(void)
{
    //开启1ms滴答定时器
    hw_timer_init(FRC1_SOURCE, 1);
    hw_timer_set_func(SysTick_Handler);
    hw_timer_arm(1000);
}

/**
 * @brief  Configures Main system clocks & power.
 * @param  None
 * @retval None
 */
void Set_System(void)
{
    zero_bss();
    SystemClock_Config();
    ets_wdt_disable();   //必须禁止看门口，因为从应用软重启进入boot，看门狗是打开的，烧写的时候将导致重启。
    HwTimer_config();
    HAL_UI_Initial();
}

void delay(uint32_t ms)
{
    ets_delay_us(ms*1000);
}

void System_Reset(void)
{
    _ResetVector();
}

