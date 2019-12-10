/******************************************************************************

 * u-blox Cellular IoT AT Commands Example
 * Leonardo Bispo
 * Dec, 2019
 * https://github.com/ldab/ublox_sara_nina

 * Distributed as-is; no warranty is given.

******************************************************************************/

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "boards.h"
#include "bsp.h"

#include "app_timer.h"
#include "nrf_drv_clock.h"
#include "nrf_delay.h"
#include "nrf_drv_power.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include "iotublox.h"

//https://devzone.nordicsemi.com/nordic/short-range-guides/b/software-development-kit/posts/application-timer-tutorial
#if NRF_PWR_MGMT_CONFIG_USE_SCHEDULER
#include "app_scheduler.h"
#define APP_SCHED_MAX_EVENT_SIZE    0   /**< Maximum size of scheduler events. */
#define APP_SCHED_QUEUE_SIZE        4   /**< Maximum number of events in the scheduler queue. */
#endif // NRF_PWR_MGMT_CONFIG_USE_SCHEDULER

static void sleep_handler(void)
{
    __WFE();
    __SEV();
    __WFE();
}

void URC_handler(void)
{
  _URC = false;
  NRF_LOG_INFO("URC: %s", _buff);
  NRF_LOG_FLUSH();

  uart_clear();
}

/**
 * @brief Function for main application entry.
 */
int main(void)
{
    ret_code_t err_code = NRF_LOG_INIT(app_timer_cnt_get);
    APP_ERROR_CHECK(err_code);

    NRF_LOG_DEFAULT_BACKENDS_INIT();

    bsp_board_init(BSP_INIT_LEDS);

    nrf_gpio_cfg_input(ARDUINO_6_PIN, NRF_GPIO_PIN_PULLUP);   // SW3, user button
    nrf_gpio_cfg_input(ARDUINO_A2_PIN, NRF_GPIO_PIN_PULLUP);  // V_INT input
    nrf_gpio_cfg_input(BUTTON_2, NRF_GPIO_PIN_PULLUP);        // EVK button
    nrf_gpio_cfg_output(ARDUINO_A1_PIN);                      // PWR_ON Pin, active High
    
    // Function starting the internal low-frequency clock LFCLK XTAL oscillator.
    // (When SoftDevice is enabled the LFCLK is always running and this is not needed).
    ret_code_t ret = nrf_drv_clock_init();
    APP_ERROR_CHECK(ret);
    nrf_drv_clock_lfclk_request(NULL);

    ret = app_timer_init();
    APP_ERROR_CHECK(ret);

#if NRF_PWR_MGMT_CONFIG_USE_SCHEDULER
    APP_SCHED_INIT(APP_SCHED_MAX_EVENT_SIZE, APP_SCHED_QUEUE_SIZE);
#endif // NRF_PWR_MGMT_CONFIG_USE_SCHEDULER

    //ret = nrf_pwr_mgmt_init();
    APP_ERROR_CHECK(ret);

    uart_init();

    static uint8_t welcome[] = "App started\r\n";

    NRF_LOG_INFO("%s", welcome);
    NRF_LOG_FLUSH();

    //uart_write(welcome);

    iotublox_init(199, "8,7", 524420, 524420);
    iotublox_powerSave(false, false, NULL, NULL);
    iotublox_connect("lpwa.telia.iot");

    while (true)
    {
    #if NRF_PWR_MGMT_CONFIG_USE_SCHEDULER
        app_sched_execute();
    #endif // NRF_PWR_MGMT_CONFIG_USE_SCHEDULER
        if(_URC == true)  URC_handler();

        if( !nrf_gpio_pin_read(BUTTON_2) )
        {
          iot_connSocket("untrol.blynk.cc", 443);

          char GET[] = "GET /ENaXVkhOawt3DJm80zrgIiCIkeGxecGF/get/V1 HTTP/1.1\n\rHost: untrol.blynk.cc\n\rConnection: close\n\r\n\r";
          iot_writeSSL(GET, strlen(GET));
          iot_readSocket();

          //uart_write(socket.content);
          
          NRF_LOG_INFO("Respons -> %s", socket.content);
          NRF_LOG_FLUSH();

          nrf_delay_ms(1000);
        }

        NRF_LOG_FLUSH();
    }
}


/** @} */
