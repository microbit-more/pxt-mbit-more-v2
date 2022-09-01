#ifndef MBIT_MORE_COMMON_H
#define MBIT_MORE_COMMON_H

#include "pxt.h"

#if MICROBIT_CODAL
#define MBIT_MORE_USE_SERIAL 1 // 1 for use USB serial
#else // MICROBIT_CODAL
// v1 has not enough memory space
#define MBIT_MORE_USE_SERIAL 0 // 1 for use USB serial
#endif // MICROBIT_CODAL

#define MBIT_MORE_DATA_RECEIVED 8000

/**
 * Data type of content.
 */
enum MbitMoreDataContentType
{
  //% block="number"
  MM_DATA_NUMBER = 1,
  //% block="text"
  MM_DATA_TEXT = 2,
};

#define MM_CH_BUFFER_SIZE_COMMAND 20
#define MM_CH_BUFFER_SIZE_NOTIFY 20
#define MM_CH_BUFFER_SIZE_STATE 7
#define MM_CH_BUFFER_SIZE_MOTION 18
#define MM_CH_BUFFER_SIZE_ANALOG_IN 2

enum MbitMoreCommand // 3 bits (0x00..0x07)
{
  CMD_CONFIG = 0x00,
  CMD_PIN = 0x01,
  CMD_DISPLAY = 0x02,
  CMD_AUDIO = 0x03,
  CMD_DATA = 0x04,
};

enum MbitMorePinCommand
{
  SET_OUTPUT = 0x01,
  SET_PWM = 0x02,
  SET_SERVO = 0x03,
  SET_PULL = 0x04,
  SET_EVENT = 0x05,
};

enum MbitMoreDisplayCommand
{
  CLEAR = 0x00,
  TEXT = 0x01,
  PIXELS_0 = 0x02,
  PIXELS_1 = 0x03,
};

/**
 * @brief Enum for write mode of display pixels.
 */
enum MbitMoreDisplayWriteMode
{
  LAYER = 0,
  OVER_WRITE = 1
};

enum MbitMorePullMode
{
  None = 0,
  Down = 1,
  Up = 2,
};

enum MbitMoreDataFormat
{
  CONFIG = 0x10, // not used at this version
  PIN_EVENT = 0x11,
  ACTION_EVENT = 0x12,
  DATA_NUMBER = 0x13,
  DATA_TEXT = 0x14
};

enum MbitMoreActionEvent
{
  BUTTON = 0x01,
  GESTURE = 0x02
};

enum MbitMoreButtonEvent
{
  DOWN = 1,
  UP = 2,
  CLICK = 3,
  LONG_CLICK = 4,
  HOLD = 5,
  DOUBLE_CLICK = 6
};

enum MbitMoreGestureEvent
{
  TILT_UP = 1,
  TILT_DOWN = 2,
  TILT_LEFT = 3,
  TILT_RIGHT = 4,
  FACE_UP = 5,
  FACE_DOWN = 6,
  FREEFALL = 7,
  G3 = 8,
  G6 = 9,
  G8 = 10,
  SHAKE = 11
};

enum MbitMorePinEventType
{
  NONE = 0,
  ON_EDGE = 1,
  ON_PULSE = 2,
  ON_TOUCH = 3
};

enum MbitMorePinEvent
{
  RISE = 2,
  FALL = 3,
  PULSE_HIGH = 4,
  PULSE_LOW = 5
};

/**
 * @brief Enum for sub-command about configurations.
 * 
 */
enum MbitMoreConfig
{
  MIC = 0x01, // microphone
  TOUCH = 0x02
};

/**
 * @brief Enum for sub-commands about audio.
 * 
 */
enum MbitMoreAudioCommand
{
  STOP_TONE = 0x00,
  PLAY_TONE = 0x01,
};

#endif // MBIT_MORE_COMMON_H
