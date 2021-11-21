// Copyright 2021 Haute école d'ingénierie et d'architecture de Fribourg
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

/****************************************************************************
 * @file unitest_transport.c
 * @author Jacques Supcik <jacques.supcik@hefr.ch>
 * 
 * @brief Disco Test transport for the "PlatformIO Unit Testing Engine"
 *
 * @date 2021-08-29
 * @version 0.1.2
 * 
 ***************************************************************************/

#include "unittest_transport.h"

#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/usart.h>

#define UNITTEST_UART_SERIAL_SPEED 115200

void unittest_uart_begin(void)
{
    rcc_periph_clock_enable(RCC_GPIOA);
    rcc_periph_clock_enable(RCC_USART2);
    gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO2 | GPIO3);
    gpio_set_af(GPIOA, GPIO_AF7, GPIO2 | GPIO3);

    usart_set_baudrate(USART2, UNITTEST_UART_SERIAL_SPEED);
    usart_set_databits(USART2, 8);
    usart_set_stopbits(USART2, USART_STOPBITS_1);
    usart_set_mode(USART2, USART_MODE_TX_RX);
    usart_set_parity(USART2, USART_PARITY_NONE);
    usart_set_flow_control(USART2, USART_FLOWCONTROL_NONE);
    usart_enable(USART2);
    usart_wait_send_ready(USART2);
}

void unittest_uart_putchar(char c) { usart_send_blocking(USART2, c); }

void unittest_uart_flush(void) { usart_wait_send_ready(USART2); }

void unittest_uart_end(void)
{
    usart_disable(USART2);
    rcc_periph_clock_disable(RCC_USART2);
}