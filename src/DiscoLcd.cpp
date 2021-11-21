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
 *
 * @brief Driver for the Disco LCD display
 *
 * @date 2021-11-19
 * @version 0.1.1
 ***************************************************************************/

#include "DiscoLcd.hpp"

#include <libopencm3/cm3/assert.h>
#include <libopencm3/cm3/common.h>
#include <libopencm3/stm32/fsmc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/timer.h>

#include <AdafruitGFX.hpp>

constexpr auto kLcdRccResetPort = RCC_GPIOD;
constexpr auto kLcdResetPort    = GPIOD;
constexpr auto kLcdResetPin     = GPIO11;

constexpr auto kLcdRccTearingEffectPort = RCC_GPIOG;
constexpr auto kLcdTearingEffectPort    = GPIOG;
constexpr auto kLcdTearingEffectPin     = GPIO4;

constexpr auto kLcdSwReset          = 0x01;
constexpr auto kLcdId               = 0x04;
constexpr auto kLcdSleepIn          = 0x10;
constexpr auto kLcdSleepOut         = 0x11;
constexpr auto kLcdPartialDisplay   = 0x12;
constexpr auto kLcdDisplayInversion = 0x21;
constexpr auto kLcdDisplayOn        = 0x29;
constexpr auto kLcdWriteRam         = 0x2C;
constexpr auto kLcdReadRam          = 0x2E;
constexpr auto kLcdCaSet            = 0x2A;
constexpr auto kLcdRaSet            = 0x2B;
constexpr auto kLcdVscrDef          = 0x33; /* Vertical Scroll Definition */
constexpr auto kLcdVsCsAd           = 0x37; /* Vertical Scroll Start Address of RAM */
constexpr auto kLcdTearingEffect    = 0x35;
constexpr auto kLcdNormalDisplay    = 0x36;
constexpr auto kLcdIdleModeOff      = 0x38;
constexpr auto kLcdIdleModeOn       = 0x39;
constexpr auto kLcdColorMode        = 0x3A;
constexpr auto kLcdPorchCtrl        = 0xB2;
constexpr auto kLcdGateCtrl         = 0xB7;
constexpr auto kLcdVComSet          = 0xBB;
constexpr auto kLcdDisplayOff       = 0xBD;
constexpr auto kLcdLcmCtrl          = 0xC0;
constexpr auto kLcdVdvVrhEn         = 0xC2;
constexpr auto kLcdVdvSet           = 0xC4;
constexpr auto kLcdVcomHOffsetSet   = 0xC5;
constexpr auto kLcdFrCtrl           = 0xC6;
constexpr auto kLcdPowerCtrl        = 0xD0;
constexpr auto kLcdPvGammaCtrl      = 0xE0;
constexpr auto kLcdNvGammaCtrl      = 0xE1;

constexpr uint32_t delayFactor = 14000;  // for system running @84Mhz

struct lcd_controller {
    volatile uint16_t reg;
    volatile uint16_t ram;
};

static volatile struct lcd_controller* lcd_controller = (struct lcd_controller*)FSMC_BANK1_BASE;

static void DelayMilliseconds(uint32_t ms)
{
    for (uint32_t i = 0; i < ms * delayFactor; i++) {
        asm volatile("nop");
    }
}

static void GpioSetup(void)
{
    rcc_periph_clock_enable(kLcdRccResetPort);
    rcc_periph_clock_enable(kLcdRccTearingEffectPort);

    gpio_set_output_options(kLcdResetPort, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, kLcdResetPin);
    gpio_set(kLcdResetPort, kLcdResetPin);
    gpio_mode_setup(kLcdResetPort, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, kLcdResetPin);
    gpio_mode_setup(kLcdTearingEffectPort, GPIO_MODE_INPUT, GPIO_PUPD_NONE, kLcdTearingEffectPin);
}

static void FsmcSetup(void)
{
    rcc_periph_clock_enable(RCC_FSMC);
    rcc_periph_clock_enable(RCC_GPIOD);
    rcc_periph_clock_enable(RCC_GPIOE);
    rcc_periph_clock_enable(RCC_GPIOF);

    // D4  -> FSMC_NOE
    // D5  -> FSMC_NWE
    // D7  -> FSMC_NE1

    // D14 -> FSMC_D0
    // D15 -> FSMC_D1
    // D0  -> FSMC_D2
    // D1  -> FSMC_D3

    // D8  -> FSMC_D13
    // D9  -> FSMC_D14
    // D10 -> FSMC_D15

    auto pins = GPIO0 | GPIO1 | GPIO4 | GPIO5 | GPIO7 | GPIO8 | GPIO9 | GPIO10 | GPIO14 | GPIO15;

    gpio_set_output_options(GPIOD, GPIO_OTYPE_PP, GPIO_OSPEED_100MHZ, pins);
    gpio_mode_setup(GPIOD, GPIO_MODE_AF, GPIO_PUPD_PULLUP, pins);
    gpio_set_af(GPIOD, GPIO_AF12, pins);

    // E7  -> FSMC_D4
    // E8  -> FSMC_D5
    // E9  -> FSMC_D6
    // E10 -> FSMC_D7
    // E11 -> FSMC_D8
    // E12 -> FSMC_D9
    // E13 -> FSMC_D10
    // E14 -> FSMC_D11
    // E15 -> FSMC_D12

    pins = GPIO7 | GPIO8 | GPIO9 | GPIO10 | GPIO11 | GPIO12 | GPIO13 | GPIO14 | GPIO15;
    gpio_set_output_options(GPIOE, GPIO_OTYPE_PP, GPIO_OSPEED_100MHZ, pins);
    gpio_mode_setup(GPIOE, GPIO_MODE_AF, GPIO_PUPD_PULLUP, pins);
    gpio_set_af(GPIOE, GPIO_AF12, pins);

    // F0  -> FSMC_A0

    pins = GPIO0;
    gpio_set_output_options(GPIOF, GPIO_OTYPE_PP, GPIO_OSPEED_100MHZ, pins);
    gpio_mode_setup(GPIOF, GPIO_MODE_AF, GPIO_PUPD_PULLUP, pins);
    gpio_set_af(GPIOF, GPIO_AF12, pins);

    // Values from here :
    // https://github.com/STMicroelectronics/STM32CubeF4/blob/4aba24d78fef03d797a82b258f37dbc84728bbb5/Drivers/BSP/STM32412G-Discovery/stm32412g_discovery.c#L812

    // 16 bit memory data bus width
    // Write operations enabled
    // Write FIFO disabled
    FSMC_BCR1 = FSMC_BCR_MWID | FSMC_BCR_WREN | BIT21;

    FSMC_BTR1 = FSMC_BTR_ADDSETx(9) | FSMC_BTR_ADDHLDx(1) | FSMC_BTR_DATASTx(36) |
                FSMC_BTR_BUSTURNx(1) | FSMC_BTR_CLKDIVx(2) | FSMC_BTR_DATLATx(2) |
                FSMC_BTR_ACCMODx(FSMC_BTx_ACCMOD_A);

    FSMC_BWTR1 = FSMC_BTR_ADDSETx(1) | FSMC_BTR_ADDHLDx(1) | FSMC_BTR_DATASTx(7) |
                 FSMC_BTR_BUSTURNx(0) | FSMC_BTR_CLKDIVx(2) | FSMC_BTR_DATLATx(2) |
                 FSMC_BTR_ACCMODx(FSMC_BTx_ACCMOD_A);

    FSMC_BCR1 |= FSMC_BCR_MBKEN;
}

static inline void FsmcWriteReg(uint8_t reg)
{
    lcd_controller->reg = reg;
    asm volatile("dsb" ::: "memory");
}

static inline uint16_t FsmcReadData(void) { return lcd_controller->ram; }

static void LcdFsmcWriteData(uint16_t data)
{
    lcd_controller->ram = data;
    asm volatile("dsb" ::: "memory");
}

static inline uint8_t ReadRegister(uint8_t Command)
{
    FsmcWriteReg(Command);    // Send the command
    FsmcReadData();           // Read dummy data
    return (FsmcReadData());  // Read register value
}

void WriteRegister(uint8_t Command, uint8_t* Parameters = nullptr, uint8_t NbParameters = 0)
{
    FsmcWriteReg(Command);  // Send the command
    // Send command's parameters if any
    for (int i = 0; i < NbParameters; i++) {
        LcdFsmcWriteData(Parameters[i]);
    }
}

// ---- public methods ----

void DiscoLcdHardReset(void)
{
    gpio_clear(kLcdResetPort, kLcdResetPin);
    DelayMilliseconds(5);  // Reset signal asserted during 5ms
    gpio_set(kLcdResetPort, kLcdResetPin);
    DelayMilliseconds(10);  // Reset signal released during 10ms
    gpio_clear(kLcdResetPort, kLcdResetPin);
    DelayMilliseconds(20);  // Reset signal asserted during 20ms
    gpio_set(kLcdResetPort, kLcdResetPin);
    DelayMilliseconds(10);  // Reset signal released during 10ms
}

void DiscoLcdDisplayOn(void)
{
    WriteRegister(kLcdDisplayOn);
    WriteRegister(kLcdSleepOut);
}

void DiscoLcdDisplayOff(void)
{
    uint8_t parameter[1];
    parameter[0] = 0xFE;
    WriteRegister(kLcdDisplayOff, parameter, 1);
    WriteRegister(kLcdSleepIn);
    DelayMilliseconds(10);
}

void DiscoLcdSetCursor(uint16_t xPos, uint16_t yPos)
{
    uint8_t parameter[4];
    // CASET: Column Addresses Set
    parameter[0] = 0x00;
    parameter[1] = 0x00 + xPos;
    parameter[2] = 0x00;
    parameter[3] = 0xEF + xPos;
    WriteRegister(kLcdCaSet, parameter, 4);
    // RASET: Row Addresses Set */
    parameter[0] = 0x00;
    parameter[1] = 0x00 + yPos;
    parameter[2] = 0x00;
    parameter[3] = 0xEF + yPos;
    WriteRegister(kLcdRaSet, parameter, 4);
}

void DiscoLcdWriteData(uint16_t* data, uint16_t len)
{
    WriteRegister(kLcdWriteRam);  // Prepare to write to LCD RAM
    for (int i = 0; i < len; i++) {
        LcdFsmcWriteData(data[i]);
    }
}

void DiscoLcdWriteSameColor(uint16_t rgbCode, uint16_t len)
{
    WriteRegister(kLcdWriteRam);  // Prepare to write to LCD RAM
    for (int i = 0; i < len; i++) {
        LcdFsmcWriteData(rgbCode);
    }
}

void DiscoLcdSetPixel(uint16_t xPos, uint16_t yPos, uint16_t rgbCode)
{
    DiscoLcdSetCursor(xPos, yPos);
    WriteRegister(kLcdWriteRam);  // Prepare to write to LCD RAM
    LcdFsmcWriteData(rgbCode);
}

void DiscoLcdDrawHLine(uint16_t xPos, uint16_t yPos, uint16_t len, uint16_t rgbCode)
{
    DiscoLcdSetCursor(xPos, yPos);
    WriteRegister(kLcdWriteRam);  // Prepare to write to LCD RAM
    for (int i = 0; i < len; i++) {
        LcdFsmcWriteData(rgbCode);
    }
}

uint16_t DiscoLcdGetPixel(uint16_t xPos, uint16_t yPos)
{
    DiscoLcdSetCursor(xPos, yPos);
    WriteRegister(kLcdReadRam);  // RAM read data command

    FsmcReadData();  // Read dummy data
    uint16_t part0 = FsmcReadData();
    uint16_t part1 = FsmcReadData();
    uint16_t red   = (part0 & 0xFC00) >> 8;
    uint16_t green = (part0 & 0x00FC) >> 0;
    uint16_t blue  = (part1 & 0xFC00) >> 8;
    return (red & 0xF8) << 8 | (green & 0xFC) << 3 | (blue & 0xF8) >> 3;
}

void DiscoLcdGetHLine(uint16_t xPos, uint16_t yPos, uint16_t len, uint16_t* buffer)
{
    DiscoLcdSetCursor(xPos, yPos);
    WriteRegister(kLcdReadRam);  // RAM read data command

    FsmcReadData();  // Read dummy data
    for (int i = 0; i < len; i++) {
        uint16_t part0 = FsmcReadData();
        uint16_t part1 = FsmcReadData();
        uint16_t red   = (part0 & 0xFC00) >> 8;
        uint16_t green = (part0 & 0x00FC) >> 0;
        uint16_t blue  = (part1 & 0xFC00) >> 8;
        buffer[i]      = (red & 0xF8) << 8 | (green & 0xFC) << 3 | (blue & 0xF8) >> 3;
    }
}

void LcdSetOrientation(DiscoLcdOrientation orientation)
{
    uint8_t parameter[6];

    if (orientation == DiscoLcdOrientation::kLandscape) {
        parameter[0] = 0x00;
    } else if (orientation == DiscoLcdOrientation::kLandscapeRot180) {
        //----- Vertical Scrolling Definition -----
        // TFA describes the Top Fixed Area
        parameter[0] = 0x00;
        parameter[1] = 0x00;
        // VSA describes the height of the Vertical Scrolling Area
        parameter[2] = 0x01;
        parameter[3] = 0xF0;
        // BFA describes the Bottom Fixed Area
        parameter[4] = 0x00;
        parameter[5] = 0x00;
        WriteRegister(kLcdVscrDef, parameter, 6);

        //---- Vertical Scroll Start Address of RAM ----
        // GRAM row nbr (320) - Display row nbr (240) = 80 = 0x50
        parameter[0] = 0x00;
        parameter[1] = 0x50;
        WriteRegister(kLcdVsCsAd, parameter, 2);

        parameter[0] = 0xC0;
    } else {  // LcdOrientation::kPortrait
        parameter[0] = 0x60;
    }
    WriteRegister(kLcdNormalDisplay, parameter, 1);
}

void DiscoLcdClear(uint16_t rgbCode)
{
    DiscoLcdSetCursor(0, 0);
    WriteRegister(kLcdWriteRam);  // Prepare to write to LCD RAM
    for (int i = 0; i < kLcdScreenWidth * kLcdScreenHeight; i++) {
        LcdFsmcWriteData(rgbCode);
    }
}

void DiscoLcdSetup(DiscoLcdOrientation orientation)
{
    uint8_t parameter[14];
    GpioSetup();
    FsmcSetup();
    DiscoLcdHardReset();

    WriteRegister(kLcdSleepIn);
    DelayMilliseconds(10);
    WriteRegister(kLcdSwReset);
    DelayMilliseconds(200);
    WriteRegister(kLcdSleepOut);
    DelayMilliseconds(120);

    parameter[0] = 0x00;
    WriteRegister(kLcdNormalDisplay, parameter, 1);

    parameter[0] = 0x05;  // Color mode 16bits/pixel
    WriteRegister(kLcdColorMode, parameter, 1);

    WriteRegister(kLcdDisplayInversion);

    // Set Column address CASET
    parameter[0] = 0x00;
    parameter[1] = 0x00;
    parameter[2] = 0x00;
    parameter[3] = 0xEF;
    WriteRegister(kLcdCaSet, parameter, 4);

    // Set Row address RASET
    parameter[0] = 0x00;
    parameter[1] = 0x00;
    parameter[2] = 0x00;
    parameter[3] = 0xEF;
    WriteRegister(kLcdRaSet, parameter, 4);

    // ---- ST7789H2 Frame rate setting ----

    // PORCH control setting
    parameter[0] = 0x0C;
    parameter[1] = 0x0C;
    parameter[2] = 0x00;
    parameter[3] = 0x33;
    parameter[4] = 0x33;
    WriteRegister(kLcdPorchCtrl, parameter, 5);

    // GATE control setting
    parameter[0] = 0x35;
    WriteRegister(kLcdGateCtrl, parameter, 1);

    // ---- ST7789H2 Power setting ----
    // VCOM setting
    parameter[0] = 0x1F;
    WriteRegister(kLcdVComSet, parameter, 1);

    // LCM Control setting
    parameter[0] = 0x2C;
    WriteRegister(kLcdLcmCtrl, parameter, 1);

    // VDV and VRH Command Enable
    parameter[0] = 0x01;
    parameter[1] = 0xC3;
    WriteRegister(kLcdVdvVrhEn, parameter, 2);

    // VDV Set
    parameter[0] = 0x20;
    WriteRegister(kLcdVdvSet, parameter, 1);

    // Frame Rate Control in normal mode
    parameter[0] = 0x0F;
    WriteRegister(kLcdFrCtrl, parameter, 1);

    // Power Control
    parameter[0] = 0xA4;
    parameter[1] = 0xA1;
    WriteRegister(kLcdPowerCtrl, parameter, 1);

    //---- ST7789H2 Gamma setting ----
    // Positive Voltage Gamma Control
    parameter[0]  = 0xD0;
    parameter[1]  = 0x08;
    parameter[2]  = 0x11;
    parameter[3]  = 0x08;
    parameter[4]  = 0x0C;
    parameter[5]  = 0x15;
    parameter[6]  = 0x39;
    parameter[7]  = 0x33;
    parameter[8]  = 0x50;
    parameter[9]  = 0x36;
    parameter[10] = 0x13;
    parameter[11] = 0x14;
    parameter[12] = 0x29;
    parameter[13] = 0x2D;
    WriteRegister(kLcdPvGammaCtrl, parameter, 14);

    // Negative Voltage Gamma Control
    parameter[0]  = 0xD0;
    parameter[1]  = 0x08;
    parameter[2]  = 0x10;
    parameter[3]  = 0x08;
    parameter[4]  = 0x06;
    parameter[5]  = 0x06;
    parameter[6]  = 0x39;
    parameter[7]  = 0x44;
    parameter[8]  = 0x51;
    parameter[9]  = 0x0B;
    parameter[10] = 0x16;
    parameter[11] = 0x14;
    parameter[12] = 0x2F;
    parameter[13] = 0x31;
    WriteRegister(kLcdNvGammaCtrl, parameter, 14);

    LcdSetOrientation(orientation);
    DiscoLcdClear();
    DiscoLcdDisplayOn();

    // Tearing Effect Line On: Option (00h:VSYNC Interface OFF, 01h:VSYNC Interface ON)
    parameter[0] = 0x00;
    WriteRegister(kLcdTearingEffect, parameter, 1);
}

uint16_t DiscoLcdId(void) { return ReadRegister(kLcdId); }

void DiscoLcdGFX::drawPixel(int16_t x, int16_t y, uint16_t color) { DiscoLcdSetPixel(x, y, color); }
void DiscoLcdGFX::drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color)
{
    DiscoLcdDrawHLine(x, y, w, color);
}
