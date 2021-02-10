#ifndef MBIT_MORE_COMMON_H
#define MBIT_MORE_COMMON_H

#define MBIT_MORE_MESSAGE 8000

/**
 * Data type of message content.
 */
enum MbitMoreMessageType
{
  //% block="number"
  MM_MSG_NUMBER = 1,
  //% block="text"
  MM_MSG_TEXT = 2,
};

#define MM_CH_BUFFER_SIZE_MAX 20
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
  CMD_MESSAGE = 0x04,
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
  PIN_EVENT = 0x10,
  ACTION_EVENT = 0x11,
  MESSAGE_NUMBER = 0x13,
  MESSAGE_TEXT = 0x14
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
