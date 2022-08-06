/*
 Copyright (c) 2014-present PlatformIO <contact@platformio.org>
 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at
    http://www.apache.org/licenses/LICENSE-2.0
 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
**/

#include <PM_LCD.h>
#include <unity.h>

PM_LCD lcd(0x27, 20, 4);

void setUp(void) {
    // set stuff up here
}

void tearDown(void) {
    // clean stuff up here
}

void test_LCD_init(void) {
    lcd.init();
}
/*
void test_calculator_addition(void) {
    TEST_ASSERT_EQUAL(32, calc.add(25, 7));
}

void test_calculator_subtraction(void) {
    TEST_ASSERT_EQUAL(20, calc.sub(23, 3));
}

void test_calculator_multiplication(void) {
    TEST_ASSERT_EQUAL(50, calc.mul(25, 2));
}

void test_calculator_division(void) {
    TEST_ASSERT_EQUAL(32, calc.div(96, 3));
}

void test_expensive_operation(void) {
    TEST_IGNORE();
}

*/
void RUN_UNITY_TESTS() {
    UNITY_BEGIN();
    RUN_TEST(test_LCD_init);
    // RUN_TEST(test_calculator_subtraction);
    // RUN_TEST(test_calculator_multiplication);
    // RUN_TEST(test_calculator_division);
    // RUN_TEST(test_expensive_operation);
    UNITY_END();
}


#ifdef ARDUINO

#include <Arduino.h>
void setup() {
    // NOTE!!! Wait for >2 secs
    // if board doesn't support software reset via Serial.DTR/RTS
    delay(2000);

    RUN_UNITY_TESTS();
}

void loop() {
    digitalWrite(13, HIGH);
    delay(100);
    digitalWrite(13, LOW);
    delay(500);
}

#else

int main(int argc, char **argv) {
    RUN_UNITY_TESTS();
    return 0;
}

#endif