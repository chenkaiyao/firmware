#include "hw_config.h"

UART_HandleTypeDef UartHandleDebug;

void debug_print(char *p)
{
    while(*p)
    {
        HAL_UART_Transmit(&UartHandleDebug, (uint8_t *)p++, 1, 30);
    }
}

void usart_debug_initial(uint32_t baud)
{
    HAL_UART_DeInit(&UartHandleDebug);

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_USART2_CLK_ENABLE();

    GPIO_InitTypeDef  GPIO_InitStruct;
    /* UART TX GPIO pin configuration  */
    GPIO_InitStruct.Pin       = GPIO_PIN_2 | GPIO_PIN_3;
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull      = GPIO_NOPULL;
    GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;//GPIO_SPEED_FAST;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    UartHandleDebug.Instance          = USART2;
    UartHandleDebug.Init.BaudRate     = baud;
    UartHandleDebug.Init.WordLength   = UART_WORDLENGTH_8B;
    UartHandleDebug.Init.StopBits     = UART_STOPBITS_1;
    UartHandleDebug.Init.Parity       = UART_PARITY_NONE;
    UartHandleDebug.Init.HwFlowCtl    = UART_HWCONTROL_NONE;
    UartHandleDebug.Init.Mode         = UART_MODE_TX_RX;
    UartHandleDebug.Init.OverSampling = UART_OVERSAMPLING_16;
    HAL_UART_Init(&UartHandleDebug);
}

void HAL_System_Config(void)
{
    Set_System();
    HAL_RTC_Initial();
    HAL_UI_Initial();
    usart_debug_initial(115200);
}

system_tick_t millis(void)
{
    return HAL_GetTick();
}

void delay(uint32_t ms)
{
    unsigned long startTime = millis();
    while( millis() - startTime < ms)
    {
    }
}

void System_Reset(void)
{
    NVIC_SystemReset();
}

