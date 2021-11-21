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
 *
 * @brief Driver for the Disco LCD display
 *
 * @date 2021-11-19
 * @version 0.1.1
 ***************************************************************************/

#ifndef DISCOLCD_HPP_
#define DISCOLCD_HPP_

#include <AdafruitGFX.hpp>
#include <cstdint>

constexpr auto kLcdST7789H2Id = 0x85;

constexpr uint16_t kLcdScreenWidth  = 240;
constexpr uint16_t kLcdScreenHeight = 240;

enum class DiscoLcdOrientation {
    kPortrait,         // Portrait orientation
    kLandscape,        // Landscape orientation
    kLandscapeRot180,  // Landscape rotated 180° orientation
};

void DiscoLcdHardReset(void);
void DiscoLcdSetup(DiscoLcdOrientation orientation = DiscoLcdOrientation::kLandscapeRot180);
uint16_t DiscoLcdId(void);

void DiscoLcdSetOrientation(DiscoLcdOrientation orientation);
void DiscoLcdDisplayOn(void);
void DiscoLcdDisplayOff(void);

void DiscoLcdSetCursor(uint16_t xPos, uint16_t yPos);
void DiscoLcdWriteData(uint16_t* data, uint16_t len);
void DiscoLcdWriteSameColor(uint16_t rgbCode, uint16_t len);

void DiscoLcdSetPixel(uint16_t xPos, uint16_t yPos, uint16_t rgbCode);
void DiscoLcdSetHLine(uint16_t xPos, uint16_t yPos, uint16_t len, uint16_t* buffer);

uint16_t DiscoLcdGetPixel(uint16_t xPos, uint16_t yPos);
void DiscoLcdGetHLine(uint16_t xPos, uint16_t yPos, uint16_t len, uint16_t* buffer);

void DiscoLcdClear(uint16_t rgbCode = 0);

void DiscoLcdDrawHLine(uint16_t xPos, uint16_t yPos, uint16_t len, uint16_t rgbCode);

class DiscoLcdGFX : public AdafruitGFX {
   public:
    DiscoLcdGFX(int16_t w, int16_t h) : AdafruitGFX(w, h){
        clearCanvas();
    };
    void drawPixel(int16_t x, int16_t y, uint16_t color);
    void drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color);
    
    void fixCanvasBoundingBox(int16_t x0, int16_t y0, int16_t x1, int16_t y1);
    void clearCanvas(void);
    void saveCanvas(void);
    void restoreCanvas(void);
    void syncCanvas(void);

    void startWrite(void);
    void writePixel(int16_t x, int16_t y, uint16_t color);
    void writeFillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
    void writeFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color);
    void writeFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color);
    void endWrite(void);

   private:
    void fixBoundingBox(int16_t x, int16_t y);
    uint16_t canvas_[kLcdScreenHeight][kLcdScreenWidth];
    uint16_t savedCanvas_[kLcdScreenHeight][kLcdScreenWidth];

    uint16_t minx_, miny_, maxx_, maxy_;
};

#endif /* DISCOLCD_HPP_ */
