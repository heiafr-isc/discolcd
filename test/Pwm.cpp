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
 * @file Pwm.cpp
 * @author Jacques Supcik <jacques.supcik@hefr.ch>
 *
 * @brief PWM driver for the Disco board
 *
 * @date 2021-10-16
 * @version 0.1.0
 ***************************************************************************/

#include "Pwm.hpp"

#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/timer.h>
#include <stdint.h>

constexpr static struct {
    enum rcc_periph_clken gpioRcc;
    uint32_t port;
    uint16_t pin;
    enum rcc_periph_clken timerRcc;
    uint32_t timer;
    enum tim_oc_id channel;
} pinsConfig[] = {
    // clang-format off
        [PWM::PA0]  = {.gpioRcc  = RCC_GPIOA, .port  = GPIOA, .pin     = GPIO0,
                       .timerRcc = RCC_TIM5,  .timer = TIM5,  .channel = TIM_OC1},
        [PWM::PA1]  = {.gpioRcc  = RCC_GPIOA, .port  = GPIOA, .pin     = GPIO1,
                       .timerRcc = RCC_TIM5,  .timer = TIM5,  .channel = TIM_OC2},
        [PWM::PA2]  = {.gpioRcc  = RCC_GPIOA, .port  = GPIOA, .pin     = GPIO2,
                       .timerRcc = RCC_TIM5,  .timer = TIM5,  .channel = TIM_OC3},
        [PWM::PA3]  = {.gpioRcc  = RCC_GPIOA, .port  = GPIOA, .pin     = GPIO3,
                       .timerRcc = RCC_TIM5,  .timer = TIM5,  .channel = TIM_OC4},
        [PWM::PA6]  = {.gpioRcc  = RCC_GPIOA, .port  = GPIOA, .pin     = GPIO6,
                       .timerRcc = RCC_TIM3,  .timer = TIM3,  .channel = TIM_OC1},
        [PWM::PA7]  = {.gpioRcc  = RCC_GPIOA, .port  = GPIOA, .pin     = GPIO7,
                       .timerRcc = RCC_TIM3,  .timer = TIM3,  .channel = TIM_OC2},
        [PWM::PB0]  = {.gpioRcc  = RCC_GPIOB, .port  = GPIOB, .pin     = GPIO0,
                       .timerRcc = RCC_TIM3,  .timer = TIM3,  .channel = TIM_OC3},
        [PWM::PB1]  = {.gpioRcc  = RCC_GPIOB, .port  = GPIOB, .pin     = GPIO1,
                       .timerRcc = RCC_TIM3,  .timer = TIM3,  .channel = TIM_OC4},
        [PWM::PB4]  = {.gpioRcc  = RCC_GPIOB, .port  = GPIOB, .pin     = GPIO4,
                       .timerRcc = RCC_TIM3,  .timer = TIM3,  .channel = TIM_OC1},
        [PWM::PB5]  = {.gpioRcc  = RCC_GPIOB, .port  = GPIOB, .pin     = GPIO5,
                       .timerRcc = RCC_TIM3,  .timer = TIM3,  .channel = TIM_OC2},
        [PWM::PB6]  = {.gpioRcc  = RCC_GPIOB, .port  = GPIOB, .pin     = GPIO6,
                       .timerRcc = RCC_TIM4,  .timer = TIM4,  .channel = TIM_OC1},
        [PWM::PB7]  = {.gpioRcc  = RCC_GPIOB, .port  = GPIOB, .pin     = GPIO7,
                       .timerRcc = RCC_TIM4,  .timer = TIM4,  .channel = TIM_OC2},
        [PWM::PB8]  = {.gpioRcc  = RCC_GPIOB, .port  = GPIOB, .pin     = GPIO8,
                       .timerRcc = RCC_TIM4,  .timer = TIM4,  .channel = TIM_OC3},
        [PWM::PB9]  = {.gpioRcc  = RCC_GPIOB, .port  = GPIOB, .pin     = GPIO9,
                       .timerRcc = RCC_TIM4,  .timer = TIM4,  .channel = TIM_OC4},
        [PWM::PC6]  = {.gpioRcc  = RCC_GPIOC, .port  = GPIOC, .pin     = GPIO6,
                       .timerRcc = RCC_TIM3,  .timer = TIM3,  .channel = TIM_OC1},
        [PWM::PC7]  = {.gpioRcc  = RCC_GPIOC, .port  = GPIOC, .pin     = GPIO7,
                       .timerRcc = RCC_TIM3,  .timer = TIM3,  .channel = TIM_OC2},
        [PWM::PC8]  = {.gpioRcc  = RCC_GPIOC, .port  = GPIOC, .pin     = GPIO8,
                       .timerRcc = RCC_TIM3,  .timer = TIM3,  .channel = TIM_OC3},
        [PWM::PC9]  = {.gpioRcc  = RCC_GPIOC, .port  = GPIOC, .pin     = GPIO9,
                       .timerRcc = RCC_TIM3,  .timer = TIM3,  .channel = TIM_OC4},
        [PWM::PD12] = {.gpioRcc  = RCC_GPIOD, .port  = GPIOD, .pin     = GPIO12,
                       .timerRcc = RCC_TIM4,  .timer = TIM4,  .channel = TIM_OC1},
        [PWM::PD13] = {.gpioRcc  = RCC_GPIOD, .port  = GPIOD, .pin     = GPIO13,
                       .timerRcc = RCC_TIM4,  .timer = TIM4,  .channel = TIM_OC2},
        [PWM::PD14] = {.gpioRcc  = RCC_GPIOD, .port  = GPIOD, .pin     = GPIO14,
                       .timerRcc = RCC_TIM4,  .timer = TIM4,  .channel = TIM_OC3},
        [PWM::PD15] = {.gpioRcc  = RCC_GPIOD, .port  = GPIOD, .pin     = GPIO15,
                       .timerRcc = RCC_TIM4,  .timer = TIM4,  .channel = TIM_OC4},
        [PWM::PF3]  = {.gpioRcc  = RCC_GPIOF, .port  = GPIOF, .pin     = GPIO3,
                       .timerRcc = RCC_TIM5,  .timer = TIM5,  .channel = TIM_OC1},
        [PWM::PF4]  = {.gpioRcc  = RCC_GPIOF, .port  = GPIOF, .pin     = GPIO4,
                       .timerRcc = RCC_TIM5,  .timer = TIM5,  .channel = TIM_OC2},
        [PWM::PF5]  = {.gpioRcc  = RCC_GPIOF, .port  = GPIOF, .pin     = GPIO5,
                       .timerRcc = RCC_TIM5,  .timer = TIM5,  .channel = TIM_OC3},
        [PWM::PF10] = {.gpioRcc  = RCC_GPIOF, .port  = GPIOF, .pin     = GPIO10,
                       .timerRcc = RCC_TIM5,  .timer = TIM5,  .channel = TIM_OC4},
    // clang-format on
};

PWM::PWM(Pin pin, int frequency, int dutyCycle)
    : pin_(pin), frequency_(frequency), dutyCycle_(dutyCycle)
{
    // Configure GPIO
    rcc_periph_clock_enable(pinsConfig[pin].gpioRcc);
    gpio_mode_setup(pinsConfig[pin].port, GPIO_MODE_AF, GPIO_PUPD_NONE, pinsConfig[pin].pin);
    gpio_set_af(pinsConfig[pin].port, GPIO_AF2, pinsConfig[pin].pin);

    // Configure timer
    rcc_periph_clock_enable(pinsConfig[pin].timerRcc);
    timer_set_mode(pinsConfig[pin].timer, TIM_CR1_CKD_CK_INT, TIM_CR1_CMS_EDGE, TIM_CR1_DIR_UP);
    SetFrequency(frequency);
    timer_set_oc_mode(pinsConfig[pin].timer, pinsConfig[pin].channel, TIM_OCM_PWM2);
    SetDutyCycle(dutyCycle);
    timer_enable_oc_output(pinsConfig[pin].timer, pinsConfig[pin].channel);
    timer_enable_counter(pinsConfig[pin].timer);
};

void PWM::SetDutyCycle(int dutyCycle)
{
    dutyCycle_ = dutyCycle;
    timer_set_oc_value(
        pinsConfig[pin_].timer, pinsConfig[pin_].channel, Period() * (100 - dutyCycle_) / 100);
}
void PWM::SetFrequency(int frequency)
{
    frequency_ = frequency;
    timer_set_counter(pinsConfig[pin_].timer, 0);
    timer_set_period(pinsConfig[pin_].timer, rcc_apb2_frequency / frequency);
}

int PWM::DutyCycle() const { return dutyCycle_; }

int PWM::Frequency() const { return frequency_; }

int PWM::Period() const { return rcc_apb2_frequency / frequency_; }
