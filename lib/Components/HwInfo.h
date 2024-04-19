#pragma once

#define DEVICE_NAME "Turing"

//MUX and SIPO HW
#define SIPO_CLK 4
#define SIPO_LATCH 17
#define SIPO_DATA 16

#define MUX_S0 17
#define MUX_S1 16
#define MUX_S2 18
#define MUX_S3 19
#define MUX_ENABLE 25
#define MUX_INPUT 33
#define MUX_ON LOW
#define MUX_OFF HIGH

//ULTRASONIC
#define US_TRIG 27
#define US_ECHO 14

//LEDS
#define WS2812_PIN 23
#define WS2811_PIN 26

#define ONBOARD_RGB_PIN 2
#define ONBOARD_RGB_SIZE 3

#define SIPO_SIZE 8
// SIPO Outputs
#define LED_0 0
#define LED_1 1
#define LED_2 2
#define MOIST_EN0 3
#define MOIST_EN1 4

#define RELAY_PIN 13

#define MUX_SIZE 16
// MUX Ports
#define MOIST_1 0
#define MOIST_2 1
#define RAIN_PIN 5
#define DHT_PIN 9
#define DS18b20_PIN 10
#define WL_BOTTOM 13
#define WL_MIDDLE 12
#define WL_EXTRA 11
#define LDR_PIN 14

#define VBAT_PIN 35

#define PUMP_LOGIC_INVERTED

#ifdef PUMP_LOGIC_INVERTED
#define PUMP_ON false
#define PUMP_OFF true
#define SIPO_START_BYTE 0b00000000
#else
#define SIPO_START_BYTE 0b00000000
#define PUMP_ON true
#define PUMP_OFF false
#endif

