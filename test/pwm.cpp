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
 * @file pwm.cpp
 * @author Jacques Supcik <jacques.supcik@hefr.ch>
 * @author Luca Haab <luca-haab@hefr.ch>
 *
 * @brief PWM driver for the Disco board
 *
 * @date 2022-12-10
 * @version 0.2.0
 ***************************************************************************/

#include "pwm.hpp"

constexpr uint32_t kPwmFreq    = 50000UL;
constexpr uint32_t kResolution = 1000UL;
constexpr static struct {
    GPIO_TypeDef* port;
    uint32_t pin;
    uint32_t channel;
    uint8_t alternate;
} channelConfig[] = {
    [PWM::kPF3]  = {GPIOF, GPIO_PIN_3, TIM_CHANNEL_1, GPIO_AF2_TIM5},
    [PWM::kPF5]  = {GPIOF, GPIO_PIN_5, TIM_CHANNEL_3, GPIO_AF2_TIM5},
    [PWM::kPF10] = {GPIOF, GPIO_PIN_10, TIM_CHANNEL_4, GPIO_AF2_TIM5},
};

TIM_HandleTypeDef PWM::htim5 = {};

PWM::PWM(Pin pin) : pin_(pin) {
    InitPwm();
    __HAL_RCC_GPIOF_CLK_ENABLE();
    GPIO_InitTypeDef gpio_init_structure = {};
    gpio_init_structure.Mode             = GPIO_MODE_AF_PP;
    gpio_init_structure.Pull             = GPIO_PULLUP;
    gpio_init_structure.Speed            = GPIO_SPEED_FREQ_HIGH;
    gpio_init_structure.Alternate        = channelConfig[pin].alternate;
    gpio_init_structure.Pin              = channelConfig[pin].pin;
    HAL_GPIO_Init(GPIOF, &gpio_init_structure);

    TIM_OC_InitTypeDef sConfigOC = {};
    sConfigOC.OCMode             = TIM_OCMODE_PWM1;
    sConfigOC.Pulse              = 0;
    HAL_TIM_PWM_ConfigChannel(&htim5, &sConfigOC, channelConfig[pin].channel);
}

PWM::~PWM() { HAL_GPIO_DeInit(GPIOF, channelConfig[pin_].pin); }

void PWM::SetDutyCycle(float duty_cycle) {
    uint32_t pulse = duty_cycle * kResolution;
    __HAL_TIM_SET_COMPARE(&htim5, channelConfig[pin_].channel, pulse);
}

HAL_StatusTypeDef PWM::Start() {
    return HAL_TIM_PWM_Start(&htim5, channelConfig[pin_].channel);
}

HAL_StatusTypeDef PWM::Stop() {
    return HAL_TIM_PWM_Stop(&htim5, channelConfig[pin_].channel);
}

HAL_StatusTypeDef PWM::InitPwm() {
    static bool is_initialized = false;
    if (is_initialized) return HAL_OK;

    __HAL_RCC_TIM5_CLK_ENABLE();

    RCC_ClkInitTypeDef clkconfig;
    uint32_t latency;
    HAL_RCC_GetClockConfig(&clkconfig, &latency);

    uint32_t APB1Freq      = HAL_RCC_GetPCLK1Freq();
    uint32_t APB1Prescaler = clkconfig.APB1CLKDivider;
    uint32_t timclock;
    if (APB1Prescaler == RCC_HCLK_DIV1) {
        timclock = APB1Freq;
    } else {
        timclock = 2UL * APB1Freq;
    }

    uint32_t prescalerValue =
        (uint32_t)((timclock / kPwmFreq / kResolution) - 1U);
    uint32_t p = kResolution - 1U;

    htim5.Instance           = TIM5;
    htim5.Init.Period        = p;
    htim5.Init.Prescaler     = prescalerValue;
    htim5.Init.ClockDivision = 0;
    htim5.Init.CounterMode   = TIM_COUNTERMODE_UP;

    HAL_StatusTypeDef status = HAL_TIM_PWM_Init(&htim5);

    is_initialized = true;
    return status;
}
