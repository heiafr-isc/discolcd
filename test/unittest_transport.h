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
 * @file unitest_transport.h
 * @author Jacques Supcik <jacques.supcik@hefr.ch>
 *
 * @brief Disco Test transport for the "PlatformIO Unit Testing Engine"
 *
 * @date 2021-08-29
 * @version 0.1.2
 ***************************************************************************/

#ifndef UNITTEST_TRANSPORT_H_
#define UNITTEST_TRANSPORT_H_

#ifdef __cplusplus
extern "C" {
#endif

extern void unittest_uart_begin(void);
extern void unittest_uart_putchar(char c);
extern void unittest_uart_flush(void);
extern void unittest_uart_end(void);

#ifdef __cplusplus
}
#endif

#endif /* UNITTEST_TRANSPORT_H_ */
