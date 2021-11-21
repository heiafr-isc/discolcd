#include <Disco.h>
#include <FontsGFX/FreeSans12pt7b.h>
#include <FontsGFX/IBMPlexMonoBold60pt7b.h>
#include <libopencm3/cm3/assert.h>

#include <AdafruitGFX.hpp>
#include <DiscoAssert.hpp>
#include <DiscoLcd.hpp>
#include "Pwm.hpp"

#include <unity.h>

static void setup(void)
{
    rcc_clock_setup_pll(&rcc_hse_8mhz_3v3[RCC_CLOCK_3V3_84MHZ]);
}

void Wait2Sec()
{
    for (int i = 0; i < 54000000; i++) {
        asm volatile("nop");
    }
}


void BasicTest()
{
    DiscoBoardSetup();
    DiscoLcdSetup();
    auto backLight = PWM(PWM::PF5);
    backLight.SetDutyCycle(10);

    cm3_assert(DiscoLcdId() == kLcdST7789H2Id);
    auto gfx = DiscoLcdGFX(kLcdScreenWidth, kLcdScreenHeight);

    gfx.setFont(&FreeSans12pt7b);
    gfx.setTextColor(0xF800);
    gfx.setCursor(10, 25);
    gfx.write("Welcome to");
    gfx.setCursor(10, 25 + 25);
    gfx.write("Computer");
    gfx.setCursor(10, 25 + 25 * 2);
    gfx.write("Architecture");

    gfx.setFont(&IBMPlexMono_Bold60pt7b);
    gfx.setTextColor(0xFFFF);
    gfx.setCursor(40, 190);
    gfx.write("42");

    gfx.fillCircle(185, 40, 30, 0xFFE0);
    gfx.fillCircle(185, 40, 20, 0x001F);
}

int main(void)
{
    setup();        // Configure target
    UNITY_BEGIN();  // Mandatory call to initialize test framework
    Wait2Sec();     // Allow platformio to show the first messages
    RUN_TEST(BasicTest);
    UNITY_END();  // Mandatory call to finalize test framework

    while (1) {
        asm volatile("nop");
    }
}