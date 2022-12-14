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
 * @file DiscoLcd.hpp
 * @author Jacques Supcik <jacques.supcik@hefr.ch> 
 * @author Luca Haab <luca-haab@hefr.ch>
 *
 * @brief Driver for the Disco LCD display
 *
 * @date 2022-12-10
 * @version 0.2.0
 ***************************************************************************/

#ifndef DISCOLCD_HPP_
#define DISCOLCD_HPP_

#include <AdafruitGFX.hpp>
#include <cstdint>
#include "stm32412g_discovery_lcd.h"



class DiscoLcdGFX : public AdafruitGFX {
   public:

    class Pwm {
      public: 
        explicit Pwm(){};
        ~Pwm(){};
        virtual void SetDutyCycle(float duty_cycle) = 0;
        virtual HAL_StatusTypeDef Start()           = 0;
        virtual HAL_StatusTypeDef Stop()            = 0;
    };

    explicit DiscoLcdGFX(int16_t w, int16_t h , Pwm* const pwm);
    explicit DiscoLcdGFX(Pwm* const pwm);
    ~DiscoLcdGFX();

    void drawPixel(int16_t x, int16_t y, uint16_t color) override;
    void drawFastVLine(int16_t x,
                       int16_t y,
                       int16_t h,
                       uint16_t color) override;
    void drawFastHLine(int16_t x,
                       int16_t y,
                       int16_t w,
                       uint16_t color) override;
    void fillRect(
        int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) override;
    void fillScreen(uint16_t color) override;
    void drawRect(
        int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) override;

    void SetBackLightLevel(float level);

   private:
    Pwm* lcdBackLight_;
};

#endif /* DISCOLCD_HPP_ */
