// Copyright 2015-2016 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at

//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef _DRIVER_TOUCH_PAD_H_
#define _DRIVER_TOUCH_PAD_H_
#ifdef __cplusplus
extern "C" {
#endif
#include "esp_intr.h"
#include "esp_err.h"
#define TOUCH_PAD_SLEEP_CYCLE_CONFIG   (0x1000)//The Time is 150Khz,the Max value is 0xffff
#define TOUCH_PAD_MEASURE_CYCLE_CONFIG (0xffff)//The Time is 8Mhz,the Max value is 0xffff
typedef enum {
    TOUCH_PAD_NUM0 = 0, /*!< Touch pad channel 0 is GPIO4 */
    TOUCH_PAD_NUM1,    /*!< Touch pad channel 0 is GPIO0 */
    TOUCH_PAD_NUM2,    /*!< Touch pad channel 0 is GPIO2 */
    TOUCH_PAD_NUM3,    /*!< Touch pad channel 0 is GPIO15 */
    TOUCH_PAD_NUM4,    /*!< Touch pad channel 0 is GPIO13 */
    TOUCH_PAD_NUM5,    /*!< Touch pad channel 0 is GPIO12 */
    TOUCH_PAD_NUM6,    /*!< Touch pad channel 0 is GPIO14 */
    TOUCH_PAD_NUM7,    /*!< Touch pad channel 0 is GPIO27*/
    TOUCH_PAD_NUM8,    /*!< Touch pad channel 0 is GPIO33*/
    TOUCH_PAD_NUM9,    /*!< Touch pad channel 0 is GPIO32*/
    TOUCH_PAD_MAX,
} touch_pad_t;
/**
 * @brief       Initialize touch module.
 *
 *This function int touch pad module ,enable touch module
 *
 * @return    None
 *
 */
void touch_pad_init();

/**
 * @brief       Configure touch pad interrupt threshold.
 *
 *
 * @param[in]  touch_num  : config touch num
 *
 * @param[in]  threshold : interrupt threshold  ,When the touch_pad_register less than threshold,
 *                         will trigger the touch interrupt.User can use touch_pad_read function
 *                         to determine the threshold.
 *
 * @return    - ESP_OK Success
 *            - ESP_ERR_INVALID_ARG Touch pad error
 *
 */
esp_err_t touch_pad_config(touch_pad_t touch_num, uint16_t threshold);

/**
 * @brief       get touch pad touch_pad_register counter.
 *
 *User can use this function to determine the the interrupt threshold .When you do not touch  the
 *pad ,read the touch_pad_read number(NumNotTouch) by the touch_pad_register.When you touch the pad ,read the touch_pad_register
 *number(NumTouch) by the touch_pad_read.Normal NumNotTouch>NumTouch,so you can select a interrupt threshold.
 *
 * @param[in]  touch_num    : touch num
 * @param[out]  touch_value : touch output value
 *
 *  @return   - ESP_OK Success
 *            - ESP_ERR_INVALID_ARG Touch pad error
 *
 */
esp_err_t touch_pad_read(touch_pad_t touch_num, uint16_t * touch_value);

/**
 * @brief   register TouchPad interrupt handler, the handler is an ISR.
 *          The handler will be attached to the same CPU core that this function is running on.
 *          @note
 *          Users should know that which CPU is running and then pick a INUM that is not used by system.
 *          We can find the information of INUM and interrupt level in soc.h.
 *
 * @param  touch_intr_num  Touch interrupt number,check the info in soc.h, and please see the core-isa.h for more details
 * @param  fn  Interrupt handler function.
 *
 *         @note
 *         Note that the handler function MUST be defined with attribution of "IRAM_ATTR".
 *
 * @param  arg  Parameter for handler function
 *
 * @return
 *     - ESP_OK Success ;
 *     - ESP_ERR_INVALID_ARG GPIO error
 */
esp_err_t touch_pad_isr_handler_register(uint32_t touch_intr_num, void(*fn)(void*), void *arg);


/**
 * ***************        ATTENTION       ********************/
/**
 *@attention
*Touch button is through the body's capacitive characteristics, 
*there is a charge discharge circuit inside the. When the hands touch, 
*the charge and discharge time will be slow.
*Because of the different hardware, each pad needs to be calibrated at the factory.
*We use touch_pad_read to determine factory parament.
*/
/**
 *----------EXAMPLE TO CONIFGURE GPIO AS OUTPUT ------------ *
 * @code{c}
 *  touch_pad_init();                            
 *  void taskA(void* arg)
 *  {
 *      for(;;){
 *          vtaskDelay(20/portTICK_PERIOD_MS);
 *          ets_printf("tocuch pad value %u\n",touch_pad_read(0));//Take the touched status and untouched status value
 *      } 
 *  }
 * @endcode
 **/
/**
 *----------EXAMPLE TO SET ISR HANDLER ----------------------
 * @code{c}
 * //the first parameter is INUM, you can pick one form interrupt level 1/2 which is not used by the system.
 * touch_pad_isr_handler_register(19,rtc_intr,NULL);    //hook the isr handler for TouchPad interrupt
 * @endcode
 * @note
 *     1. user should arrange the INUMs that used, better not to use a same INUM for different interrupt.
 *     2. do not pick the INUM that already occupied by the system.
 *     3. refer to soc.h to check which INUMs that can be used.
 */
/**
 *----------EXAMPLE TO USE TOUCH_PAD------------ *
 * @code{c}
 *   touch_pad_init();//only init one time
 *   touch_pad_config(0,300);//set the intr threshold,use touch_pad_read to determine this threshold 
 *   touch_pad_isr_handler_register(19,rtc_intr,NULL)
 *   #include "esp_attr.h"
 *   void IRAM_ATTR rtc_intr(void * arg)
 *   {
 *       uint32_t pad_intr = READ_PERI_REG(SARADC_SAR_TOUCH_CTRL2_REG) & 0x3ff;
 *       uint8_t i = 0;
 *       uint32_t rtc_intr = READ_PERI_REG(RTC_CNTL_INT_ST_REG);
 *       WRITE_PERI_REG(RTC_CNTL_INT_CLR_REG, rtc_intr);
 *       SET_PERI_REG_MASK(SARADC_SAR_TOUCH_CTRL2_REG, SARADC_TOUCH_MEAS_EN_CLR);
 *       if (rtc_intr & RTC_CNTL_TOUCH_INT_ST) {
 *           for (i = 0; i < TOUCH_PAD_MAX; ++i) {
 *               if ((pad_intr >> i) & 0x01) {
 *                   ets_printf("touch pad intr %u\n",i);
 *               }
 *           }
 *       }
 *  }
 * @endcode
 **/

#ifdef __cplusplus
}
#endif

#endif/*_DRIVER_TOUCH_PAD_H_*/

