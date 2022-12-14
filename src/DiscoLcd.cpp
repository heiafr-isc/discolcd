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
 * @file DiscoLcd.cpp
 * @author Jacques Supcik <jacques.supcik@hefr.ch> 
 * @author Luca Haab <luca-haab@hefr.ch>
 *
 * @brief Driver for the Disco LCD display
 *
 * @date 2021-11-19
 * @version 0.2.0
 ***************************************************************************/

#include "DiscoLcd.hpp"

#include <AdafruitGFX.hpp>
#include <cstring>

#include "stm32412g_discovery_lcd.h"

DiscoLcdGFX::DiscoLcdGFX(int16_t w, int16_t h, Pwm* const pwm) : 
AdafruitGFX(w, h), lcdBackLight_(pwm) {
    BSP_LCD_InitEx(LCD_ORIENTATION_LANDSCAPE_ROT180);
    BSP_LCD_Clear(LCD_COLOR_BLACK);
    // BSP_LCD_InitEx set PF5 as output, so we have to
    // reconfigure it as PWM after the initialization
    lcdBackLight_->Start();
    lcdBackLight_->SetDutyCycle(0.5);
}

DiscoLcdGFX::DiscoLcdGFX(Pwm* const pwm) : DiscoLcdGFX(240, 240, pwm) {}

DiscoLcdGFX::~DiscoLcdGFX() {
    lcdBackLight_->SetDutyCycle(0);
    lcdBackLight_->Stop();
    BSP_LCD_DeInit();
}

// cppcheck-suppress unusedFunction
void DiscoLcdGFX::drawPixel(int16_t x, int16_t y, uint16_t color) {
    BSP_LCD_DrawPixel(x, y, color);
}

// cppcheck-suppress unusedFunction
void DiscoLcdGFX::drawFastVLine(int16_t x,
                                int16_t y,
                                int16_t h,
                                uint16_t color) {
    BSP_LCD_SetTextColor(color);
    BSP_LCD_DrawVLine(x, y, h);
}

// cppcheck-suppress unusedFunction
void DiscoLcdGFX::drawFastHLine(int16_t x,
                                int16_t y,
                                int16_t w,
                                uint16_t color) {
    BSP_LCD_SetTextColor(color);
    BSP_LCD_DrawHLine(x, y, w);
}

// cppcheck-suppress unusedFunction
void DiscoLcdGFX::fillRect(
    int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    BSP_LCD_SetTextColor(color);
    if (h > 0) {
        BSP_LCD_FillRect(x, y, w, h - 1);
        // Note that we need to substract 1 to get the
        // right height. This is probably a bug
        // in the BSP_LCD_FillRect function.
    }
}

// cppcheck-suppress unusedFunction
void DiscoLcdGFX::fillScreen(uint16_t color) { BSP_LCD_Clear(color); }

// cppcheck-suppress unusedFunction
void DiscoLcdGFX::drawRect(
    int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    BSP_LCD_SetTextColor(color);
    BSP_LCD_DrawRect(x, y, w - 1, h - 1);
    // Note that we need to substract 1 to get the
    // right width and height. This is probably a bug
    // in the BSP_LCD_DrawRect function.
}

// cppcheck-suppress unusedFunction
void DiscoLcdGFX::SetBackLightLevel(float level) {
    lcdBackLight_->SetDutyCycle(level);
}

