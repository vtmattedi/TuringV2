#pragma once
#include <Arduino.h>
#include <ArduinoJson.h>


/**
 * @brief Aligns a given message string to a specified size.
 * it is the same as centerFixed but with default mode 0.
 * @param msg The message string to be aligned.
 * @param size The size to which the message should be aligned.
 * @param mode The alignment mode (default is 0). 
 *             -  0: Center align
 *             - >0: Left align
 *             - <0: Right align
 * @return A new string that is aligned according to the specified size and mode.
 */
String align(String msg, int size, int mode = 0);

/**
 * @brief Generates a JSON string with the command information.
 * 
 * @param pretty A boolean flag to specify if the JSON should be pretty printed (default is false).
 * @return A JSON string with the command information.
 */
String generateCommandInfoJson(bool pretty = false);

/**
 * @brief Generates a formatted string with the command information.
 * 
 * @return A formatted string with the command information.
 */
String generateCommandInfo();

extern const char *commandInfo;