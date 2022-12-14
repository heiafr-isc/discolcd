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
 * @file pwm.hpp
 * @author Jacques Supcik <jacques.supcik@hefr.ch>
 * @author Luca Haab <luca-haab@hefr.ch>
 *
 * @brief PWM driver for the Disco board
 *
 * @date 2022-12-10
 * @version 0.2.0
 ***************************************************************************/

#ifndef PWM_HPP_
#define PWM_HPP_

#include "stm32f4xx_hal.h"
#include <stdint.h>

class PWM {
   public:
    enum Pin { kPF3, kPF5, kPF10 };
    
    explicit PWM(Pin pin);
    ~PWM();
    
    void SetDutyCycle(float duty_cycle);
    HAL_StatusTypeDef Start();
    HAL_StatusTypeDef Stop();

   private:
    static TIM_HandleTypeDef htim5;
    static HAL_StatusTypeDef InitPwm();
    Pin pin_;
};    

#endif /* PWM_HPP_ */
