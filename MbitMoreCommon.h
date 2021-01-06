#ifndef MBIT_MORE_COMMON_H
#define MBIT_MORE_COMMON_H

#define MM_CH_BUFFER_SIZE_MAX 20
#define MM_CH_BUFFER_SIZE_NOTIFY 20
#define MM_CH_BUFFER_SIZE_SENSORS 7
#define MM_CH_BUFFER_SIZE_DIRECTION 18
#define MM_CH_BUFFER_SIZE_ANALOG_IN 2
#define MM_CH_BUFFER_SIZE_SHARED_DATA 5

enum MbitMoreCommand // 3 bits (0x00..0x07)
{
  CMD_CONFIG = 0x00,
  CMD_PIN = 0x01,
  CMD_DISPLAY = 0x02,
  CMD_SHARED_DATA = 0x03,
};

enum MbitMorePinCommand
{
  SET_OUTPUT = 0x01,
  SET_PWM = 0x02,
  SET_SERVO = 0x03,
  SET_PULL = 0x04,
  SET_EVENT = 0x05,
  SET_TOUCH = 0x06,
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

enum MbitMorePinMode
{
  MbitMorePullNone = 0,
  MbitMorePullUp = 1,
  MbitMorePullDown = 2,
};

enum MbitMoreDataFormat
{
  PIN_EVENT = 0x10,
  ACTION_EVENT = 0x11,
  SHARED_DATA = 0x13
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

#endif // MBIT_MORE_COMMON_H
