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
 * @brief main test program for Disco board display
 *
 * @date 2022-12-10
 * @version 0.2.0
 ***************************************************************************/

#include <unity.h>
#include "Fonts/IBMPlexSansMedium12pt8b.h"
#include "Fonts/IBMPlexSansMedium18pt8b.h"
#include "AdafruitGFX.hpp"
#include "DiscoLcd.hpp"
#include "pwm.hpp"
#include "system_clock.h"
#include "f412disco_ado.h"


void setUp(void) {}

void tearDown(void) {}

static void initBoard(void){
    HAL_Init();
    SystemClock_Config();
    DiscoSyscallsInit();
    BSP_LED_Init(LED_GREEN);
    BSP_LED_Init(LED_ORANGE);
    BSP_LED_Init(LED_RED);
    BSP_LED_Init(LED_BLUE);
}

class myPwm : public DiscoLcdGFX::Pwm{
  public: 
    explicit myPwm();
    ~myPwm();
    
    void SetDutyCycle(float duty_cycle) override;
    HAL_StatusTypeDef Start() override;
    HAL_StatusTypeDef Stop() override;
    
  private:
    PWM* pwm_;
};

myPwm::myPwm() : DiscoLcdGFX::Pwm() {pwm_ = new PWM(PWM::kPF5);}
myPwm::~myPwm() {delete pwm_;}
void myPwm::SetDutyCycle(float duty_cycle) {pwm_->SetDutyCycle(duty_cycle);}
HAL_StatusTypeDef myPwm::Start(){return pwm_->Start();}
HAL_StatusTypeDef myPwm::Stop(){return pwm_->Stop();}

void BasicTest()
{
  DiscoLcdGFX::Pwm* aPwm = new myPwm(); 
  auto gfx = DiscoLcdGFX(aPwm);

  gfx.setFont(&IBMPlexSansMedium12pt8b);
  gfx.setTextColor(0xF800);
  gfx.setCursor(10, 25);
  gfx.write("Welcome to");
  gfx.setCursor(10, 25 + 25);
  gfx.write("Computer");
  gfx.setCursor(10, 25 + 25 * 2);
  gfx.write("Architecture");

  gfx.setFont(&IBMPlexSansMedium18pt8b);
  gfx.setTextColor(0xFFFF);
  gfx.setCursor(35, 160);
  gfx.write("ADO ->"); 
  gfx.setCursor(35, 190);
  gfx.write("Real fun ;-)");

  gfx.fillCircle(185, 40, 30, 0xFFE0);
  gfx.fillCircle(185, 40, 20, 0x001F);
}

int main(void)
{
    initBoard();           // Configure target
    UNITY_BEGIN();        // Mandatory call to initialize test framework
    HAL_Delay(2000);      // Allow platformio to show the first messages
    RUN_TEST(BasicTest);
    UNITY_END();          // Mandatory call to finalize test framework

    while (1) {
        asm volatile("nop");
    }
}