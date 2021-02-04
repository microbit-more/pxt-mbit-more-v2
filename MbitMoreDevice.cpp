
#include "pxt.h"

#include "MicroBit.h"
#include "MicroBitConfig.h"

/**
 * Class definition for the Scratch MicroBit More Service.
 * Provides a BLE service to remotely controll Micro:bit from Scratch3.
 */
#include "MbitMoreDevice.h"

/**
 * @brief Compute average value for the int array.
 *
 * @param data Array to compute average.
 * @param dataSize Length of the array.
 * @return average value.
 */
int average(int *data, int dataSize) {
  int sum = 0;
  int i;
  for (i = 0; i < dataSize - 1; i++) {
    sum += data[i];
  }
  return sum / dataSize;
}

/**
 * @brief Compute median value for the array.
 *
 * @param data Array to compute median.
 * @param dataSize Length of the array.
 * @return Median value.
 */
int median(int *data, int dataSize) {
  int temp;
  int i, j;
  // the following two loops sort the array x in ascending order
  for (i = 0; i < dataSize - 1; i++) {
    for (j = i + 1; j < dataSize; j++) {
      if (data[j] < data[i]) {
        // swap elements
        temp = data[i];
        data[i] = data[j];
        data[j] = temp;
      }
    }
  }
  return data[dataSize / 2];
}

/**
 * @brief Copy ManagedString to char array with max size.
 * 
 * @param dst char array as destination
 * @param mstr string as source
 * @param maxLength max size to copy
 */
void copyManagedString(char *dst, ManagedString mstr, size_t maxLength) {
  memcpy(dst, mstr.toCharArray(), ((size_t)mstr.length() < maxLength ? mstr.length() : maxLength));
}

/**
 * Position of data format in a value holder.
 */
#define MBIT_MORE_DATA_FORMAT_INDEX 19

#define MBIT_MORE_BUTTON_PIN_A 5
#define MBIT_MORE_BUTTON_PIN_B 11

/**
 * Constructor.
 * Create a representation of the device for Microbit More service.
 * @param _uBit The instance of a MicroBit runtime.
 */
MbitMoreDevice::MbitMoreDevice(MicroBit &_uBit) : uBit(_uBit) {
  // Reset compass
#if MICROBIT_CODAL
  // On microbit-v2, re-calibration destract compass heading.
#else // NOT MICROBIT_CODAL
  if (uBit.buttonA.isPressed()) {
    uBit.compass.clearCalibration();
  }
#endif // NOT MICROBIT_CODAL

  // Compass must be calibrated before starting bluetooth service.
  if (!uBit.compass.isCalibrated()) {
    uBit.compass.calibrate();
  }

  uBit.messageBus.listen(MICROBIT_ID_BUTTON_A, MICROBIT_EVT_ANY, this,
                         &MbitMoreDevice::onButtonChanged,
                         MESSAGE_BUS_LISTENER_IMMEDIATE);
  uBit.messageBus.listen(MICROBIT_ID_BUTTON_B, MICROBIT_EVT_ANY, this,
                         &MbitMoreDevice::onButtonChanged,
                         MESSAGE_BUS_LISTENER_IMMEDIATE);
  uBit.messageBus.listen(MICROBIT_ID_GESTURE, MICROBIT_EVT_ANY, this,
                         &MbitMoreDevice::onGestureChanged,
                         MESSAGE_BUS_LISTENER_IMMEDIATE);

  uBit.messageBus.listen(
      MICROBIT_ID_BLE,
      MICROBIT_BLE_EVT_CONNECTED,
      this,
      &MbitMoreDevice::onBLEConnected);
  uBit.messageBus.listen(
      MICROBIT_ID_BLE,
      MICROBIT_BLE_EVT_DISCONNECTED,
      this,
      &MbitMoreDevice::onBLEDisconnected);
}

MbitMoreDevice::~MbitMoreDevice() {
  uBit.messageBus.ignore(MICROBIT_ID_BUTTON_A, MICROBIT_EVT_ANY, this,
                         &MbitMoreDevice::onButtonChanged);
  uBit.messageBus.ignore(MICROBIT_ID_BUTTON_B, MICROBIT_EVT_ANY, this,
                         &MbitMoreDevice::onButtonChanged);
  uBit.messageBus.ignore(MICROBIT_ID_GESTURE, MICROBIT_EVT_ANY, this,
                         &MbitMoreDevice::onGestureChanged);
  uBit.messageBus.ignore(MICROBIT_ID_ANY, MICROBIT_EVT_ANY, this,
                         &MbitMoreDevice::onPinEvent);
  delete basicService;
}

/**
 * @brief Set pin configuration for initial.
 *
 */
void MbitMoreDevice::initializeConfig() {
  // P0,P1,P2 are pull-up as standerd extension.
  for (size_t i = 0; i < (sizeof(initialPullUp) / sizeof(initialPullUp[0]));
       i++) {
#if MICROBIT_CODAL
    setPullMode(initialPullUp[i], PullMode::Up);
#else // NOT MICROBIT_CODAL
    setPullMode(initialPullUp[i], PinMode::PullUp);
#endif // NOT MICROBIT_CODAL
  }
  uBit.display.stopAnimation(); // To stop display friendly name.
  uBit.display.print("M");
}

void MbitMoreDevice::onBLEConnected(MicroBitEvent _e) {
  initializeConfig();
}

void MbitMoreDevice::onBLEDisconnected(MicroBitEvent _e) {
  uBit.reset(); // reset to off microphone and its LED.
}

/**
 * @brief Call when a command was received.
 *
 * @param data
 * @param length
 */
void MbitMoreDevice::onCommandReceived(uint8_t *data, size_t length) {
  const int command = (data[0] >> 5);
  if (command == MbitMoreCommand::CMD_DISPLAY) {
    const int displayCommand = data[0] & 0b11111;
    if (displayCommand == MbitMoreDisplayCommand::TEXT) {
      char text[length - 1] = {0};
      memcpy(text, &(data[2]), length - 2);
      displayText(text, (data[1] * 10));
    } else if (displayCommand == MbitMoreDisplayCommand::PIXELS_0) {
      setPixelsShadowLine(0, &data[1]);
      setPixelsShadowLine(1, &data[6]);
      setPixelsShadowLine(2, &data[11]);
    } else if (displayCommand == MbitMoreDisplayCommand::PIXELS_1) {
      setPixelsShadowLine(3, &data[1]);
      setPixelsShadowLine(4, &data[6]);
      displayShadowPixels();
    }
  } else if (command == MbitMoreCommand::CMD_PIN) {
    const int pinCommand = data[0] & 0b11111;
    if (pinCommand == MbitMorePinCommand::SET_PULL) {
#if MICROBIT_CODAL
      switch (data[2]) {
      case MbitMorePullMode::None:
        setPullMode(data[1], PullMode::None);
        break;
      case MbitMorePullMode::Up:
        setPullMode(data[1], PullMode::Up);
        break;
      case MbitMorePullMode::Down:
        setPullMode(data[1], PullMode::Down);
        break;

      default:
        break;
      }
#else // NOT MICROBIT_CODAL
      switch (data[2]) {
      case MbitMorePullMode::None:
        setPullMode(data[1], PinMode::PullNone);
        break;
      case MbitMorePullMode::Up:
        setPullMode(data[1], PinMode::PullUp);
        break;
      case MbitMorePullMode::Down:
        setPullMode(data[1], PinMode::PullDown);
        break;

      default:
        break;
      }
#endif // NOT MICROBIT_CODAL
    } else if (pinCommand == MbitMorePinCommand::SET_TOUCH) {
      setPinModeTouch(data[1]);
    } else if (pinCommand == MbitMorePinCommand::SET_OUTPUT) {
      setDigitalValue(data[1], data[2]);
    } else if (pinCommand == MbitMorePinCommand::SET_PWM) {
      // value is read as uint16_t little-endian.
      uint16_t value;
      memcpy(&value, &(data[2]), 2);
      setAnalogValue(data[1], value);
    } else if (pinCommand == MbitMorePinCommand::SET_SERVO) {
      int pinIndex = (int)data[1];
      // angle is read as uint16_t little-endian.
      uint16_t angle;
      memcpy(&angle, &(data[2]), 2);
      // range is read as uint16_t little-endian.
      uint16_t range;
      memcpy(&range, &(data[5]), 2);
      // center is read as uint16_t little-endian.
      uint16_t center;
      memcpy(&center, &(data[7]), 2);
      if (range == 0) {
        uBit.io.pin[pinIndex].setServoValue(angle);
      } else if (center == 0) {
        uBit.io.pin[pinIndex].setServoValue(angle, range);
      } else {
        uBit.io.pin[pinIndex].setServoValue(angle, range, center);
      }
    } else if (pinCommand == MbitMorePinCommand::SET_EVENT) {
      listenPinEventOn((int)data[1], (int)data[2]);
    }
#if MICROBIT_CODAL
  } else if (command == MbitMoreCommand::CMD_MESSAGE) {
    int messageID = findWaitingMessageID((char *)(&data[1]));
    if (messageID != MBIT_MORE_WAITING_MESSAGE_NOT_FOUND) {
      receivedMessages[messageID].type = (MbitMoreMessageType)(data[0] & 0b11111);
      int contentStart = 1 + MBIT_MORE_MESSAGE_LABEL_SIZE;
      memset(receivedMessages[messageID].content, 0, MBIT_MORE_MESSAGE_CONTENT_SIZE);
      memcpy(receivedMessages[messageID].content, &data[contentStart], length - contentStart);
      MicroBitEvent evt(MBIT_MORE_MESSAGE, messageID);
    }
  } else if (command == MbitMoreCommand::CMD_CONFIG) {
    const int config = data[0] & 0b11111;
    if (config == MbitMoreConfig::MIC) {
      micInUse = ((data[1] == 1) ? true : false);
    }
#endif // MICROBIT_CODAL
  }
}

/**
 * @brief Set the pattern on the line of the shadow pixels.
 *
 * @param line Index of the lines to set.
 * @param pattern Array of brightness(0..255) according columns.
 */
void MbitMoreDevice::setPixelsShadowLine(int line, uint8_t *pattern) {
  for (size_t col = 0; col < 5; col++) {
    shadowPixcels[line][col] = pattern[col];
  }
}

/**
 * @brief Display the shadow pixels on the LED.
 *
 */
void MbitMoreDevice::displayShadowPixels() {
  uBit.display.stopAnimation();
  for (size_t y = 0; y < 5; y++) {
    for (size_t x = 0; x < 5; x++) {
      uBit.display.image.setPixelValue(x, y, shadowPixcels[y][x]);
    }
  }
}

/**
 * Make it listen events of the event type on the pin.
 * Remove listener if the event type is MICROBIT_PIN_EVENT_NONE.
 */
void MbitMoreDevice::listenPinEventOn(int pinIndex, int eventType) {
  int componentID = pinIndex + 100; // conventional scheme to convert from pin
                                    // index to componentID in v1 and v2.

  if (eventType == MbitMorePinEventType::NONE) {
    uBit.messageBus.ignore(componentID, MICROBIT_EVT_ANY, this,
                           &MbitMoreDevice::onPinEvent);
    uBit.io.pin[pinIndex].eventOn(MICROBIT_PIN_EVENT_NONE);
  } else {
    uBit.messageBus.listen(componentID, MICROBIT_EVT_ANY, this,
                           &MbitMoreDevice::onPinEvent,
                           MESSAGE_BUS_LISTENER_DROP_IF_BUSY);
    if (eventType == MbitMorePinEventType::ON_EDGE) {
      uBit.io.pin[pinIndex].eventOn(MICROBIT_PIN_EVENT_ON_EDGE);
    } else if (eventType == MbitMorePinEventType::ON_PULSE) {
      uBit.io.pin[pinIndex].eventOn(MICROBIT_PIN_EVENT_ON_PULSE);
    } else if (eventType == MbitMorePinEventType::ON_TOUCH) {
      uBit.io.pin[pinIndex].eventOn(MICROBIT_PIN_EVENT_ON_TOUCH);
    }
  }
}

/**
 * Callback. Invoked when a pin event sent.
 */
void MbitMoreDevice::onPinEvent(MicroBitEvent evt) {
  uint8_t *data = moreService->pinEventChBuffer;

  // pinIndex is sent as uint8_t.
  data[0] = evt.source - 100; // conventional scheme to convert from componentID
                              // to pin index in v1 and v2.

  // event ID is sent as uint8_t.
  switch (evt.value) {
  case MICROBIT_PIN_EVT_RISE:
    data[1] = MbitMorePinEvent::RISE;
    break;
  case MICROBIT_PIN_EVT_FALL:
    data[1] = MbitMorePinEvent::FALL;
    break;
  case MICROBIT_PIN_EVT_PULSE_HI:
    data[1] = MbitMorePinEvent::PULSE_HIGH;
    break;
  case MICROBIT_PIN_EVT_PULSE_LO:
    data[1] = MbitMorePinEvent::PULSE_LOW;
    break;

  default:
    // send event whatever for dev.
    data[1] = evt.value;
    break;
  }

  // event timestamp is sent as uint32_t little-endian
  // coerced from uint64_t value.
  uint32_t timestamp = (uint32_t)evt.timestamp;
  memcpy(&(data[2]), &timestamp, 4);
  data[MBIT_MORE_DATA_FORMAT_INDEX] = MbitMoreDataFormat::PIN_EVENT;
  moreService->notifyPinEvent();
}

/**
 * Button update callback
 */
void MbitMoreDevice::onButtonChanged(MicroBitEvent evt) {
  uint8_t *data = moreService->actionEventChBuffer;
  data[0] = MbitMoreActionEvent::BUTTON;
  // Component ID that generated the event.
  switch (evt.source) {
  case MICROBIT_ID_BUTTON_A:
    data[1] = MBIT_MORE_BUTTON_PIN_A;
    break;

  case MICROBIT_ID_BUTTON_B:
    data[1] = MBIT_MORE_BUTTON_PIN_B;
    break;

  default:
    data[1] = evt.source;
    break;
  }
  // Event ID.
  switch (evt.value) {
  case MICROBIT_BUTTON_EVT_DOWN:
    data[2] = MbitMoreButtonEvent::DOWN;
    break;

  case MICROBIT_BUTTON_EVT_UP:
    data[2] = MbitMoreButtonEvent::UP;
    break;

  case MICROBIT_BUTTON_EVT_CLICK:
    data[2] = MbitMoreButtonEvent::CLICK;
    break;

  case MICROBIT_BUTTON_EVT_LONG_CLICK:
    data[2] = MbitMoreButtonEvent::LONG_CLICK;
    break;

  case MICROBIT_BUTTON_EVT_HOLD:
    data[2] = MbitMoreButtonEvent::HOLD;
    break;

  case MICROBIT_BUTTON_EVT_DOUBLE_CLICK:
    data[2] = MbitMoreButtonEvent::DOUBLE_CLICK;
    break;

  default:
    data[2] = evt.value;
    break;
  }
  // Timestamp of the event.
  memcpy(&(data[3]), &evt.timestamp, 4);
  data[MBIT_MORE_DATA_FORMAT_INDEX] = MbitMoreDataFormat::ACTION_EVENT;
  moreService->notifyActionEvent();
}

void MbitMoreDevice::onGestureChanged(MicroBitEvent evt) {
  uint8_t *data = moreService->actionEventChBuffer;
  data[0] = MbitMoreActionEvent::GESTURE;
  switch (evt.value) {
  case MICROBIT_ACCELEROMETER_EVT_TILT_UP:
    data[1] = MbitMoreGestureEvent::TILT_UP;
    break;

  case MICROBIT_ACCELEROMETER_EVT_TILT_DOWN:
    data[1] = MbitMoreGestureEvent::TILT_DOWN;
    break;

  case MICROBIT_ACCELEROMETER_EVT_TILT_LEFT:
    data[1] = MbitMoreGestureEvent::TILT_LEFT;
    break;

  case MICROBIT_ACCELEROMETER_EVT_TILT_RIGHT:
    data[1] = MbitMoreGestureEvent::TILT_RIGHT;
    break;

  case MICROBIT_ACCELEROMETER_EVT_FACE_UP:
    data[1] = MbitMoreGestureEvent::FACE_UP;
    break;

  case MICROBIT_ACCELEROMETER_EVT_FACE_DOWN:
    data[1] = MbitMoreGestureEvent::FACE_DOWN;
    break;

  case MICROBIT_ACCELEROMETER_EVT_FREEFALL:
    data[1] = MbitMoreGestureEvent::FREEFALL;
    break;

  case MICROBIT_ACCELEROMETER_EVT_3G:
    data[1] = MbitMoreGestureEvent::G3;
    break;

  case MICROBIT_ACCELEROMETER_EVT_6G:
    data[1] = MbitMoreGestureEvent::G6;
    break;

  case MICROBIT_ACCELEROMETER_EVT_8G:
    data[1] = MbitMoreGestureEvent::G8;
    break;

  case MICROBIT_ACCELEROMETER_EVT_SHAKE:
    data[1] = MbitMoreGestureEvent::SHAKE;
    break;

  default:
    data[1] = evt.value;
    break;
  }
  // Timestamp of the event.
  memcpy(&(data[2]), &evt.timestamp, 4);
  data[MBIT_MORE_DATA_FORMAT_INDEX] = MbitMoreDataFormat::ACTION_EVENT;
  moreService->notifyActionEvent();
}

/**
 * Normalize angle in upside down.
 */
int MbitMoreDevice::normalizeCompassHeading(int heading) {
  if (uBit.accelerometer.getZ() > 0) {
    if (heading <= 180) {
      heading = 180 - heading;
    } else {
      heading = 360 - (heading - 180);
    }
  }
  return heading;
}

/**
 * Convert roll/pitch radians to Scratch extension value (-1000 to 1000).
 */
int MbitMoreDevice::convertToTilt(float radians) {
  float degrees = (360.0f * radians) / (2.0f * PI);
  float tilt = degrees * 1.0f / 90.0f;
  if (degrees > 0) {
    if (tilt > 1.0f)
      tilt = 2.0f - tilt;
  } else {
    if (tilt < -1.0f)
      tilt = -2.0f - tilt;
  }
  return (int)(tilt * 1000.0f);
}

#if MICROBIT_CODAL
void MbitMoreDevice::setPullMode(int pinIndex, PullMode pull) {
#else // NOT MICROBIT_CODAL
void MbitMoreDevice::setPullMode(int pinIndex, PinMode pull) {
#endif // NOT MICROBIT_CODAL
  uBit.io.pin[pinIndex].getDigitalValue(pull);
  pullMode[pinIndex] = pull;
}

void MbitMoreDevice::setDigitalValue(int pinIndex, int value) {
  uBit.io.pin[pinIndex].setDigitalValue(value);
}

void MbitMoreDevice::setAnalogValue(int pinIndex, int value) {
#if MICROBIT_CODAL
  // stable level is 0 .. 1022 in micro:bit v2,
  int validValue = value > 1022 ? 1022 : value;
#else // NOT MICROBIT_CODAL
  // stable level is 0 .. 1021 in micro:bit v1.5,
  int validValue = value > 1021 ? 1021 : value;
#endif // NOT MICROBIT_CODAL
  uBit.io.pin[pinIndex].setAnalogValue(validValue);
}

void MbitMoreDevice::setServoValue(int pinIndex, int angle, int range,
                                   int center) {
  uBit.io.pin[pinIndex].setServoValue(angle, range, center);
}

void MbitMoreDevice::setPinModeTouch(int pinIndex) {
  uBit.io.pin[pinIndex].isTouched(); // Configure to touch mode then the
                                     // return value is not used.
}

/**
 * @brief Display text on LED.
 *
 * @param text Contents to display with null termination.
 * @param delay The time to delay between characters, in milliseconds.
 */
void MbitMoreDevice::displayText(char *text, int delay) {
  ManagedString mstr(text);
  uBit.display.stopAnimation();
  // Interval=120 is corresponding with the standard extension.
  uBit.display.scrollAsync(mstr, delay);
}

/**
 * @brief Get data of the sensors.
 *
 * @param data Buffer for BLE characteristics.
 */
void MbitMoreDevice::updateSensors(uint8_t *data) {
  uint32_t digitalLevels = 0;
  for (size_t i = 0; i < sizeof(gpioPin) / sizeof(gpioPin[0]); i++) {
    if (uBit.io.pin[gpioPin[i]].isDigital()) {
      if (uBit.io.pin[gpioPin[i]].isInput()) {
        digitalLevels =
            digitalLevels |
            (uBit.io.pin[gpioPin[i]].getDigitalValue() << gpioPin[i]);
      }
    }
  }
  // DAL can not read the buttons state as digital input. (CODAL can do that)
#if MICROBIT_CODAL
  digitalLevels =
      digitalLevels | (uBit.io.pin[MBIT_MORE_BUTTON_PIN_A].getDigitalValue()
                       << MBIT_MORE_BUTTON_PIN_A);
  digitalLevels =
      digitalLevels | (uBit.io.pin[MBIT_MORE_BUTTON_PIN_B].getDigitalValue()
                       << MBIT_MORE_BUTTON_PIN_B);
#else // NOT MICROBIT_CODAL
  digitalLevels =
      digitalLevels | (!uBit.buttonA.isPressed() << MBIT_MORE_BUTTON_PIN_A);
  digitalLevels =
      digitalLevels | (!uBit.buttonB.isPressed() << MBIT_MORE_BUTTON_PIN_B);
#endif // NOT MICROBIT_CODAL
  memcpy(data, (uint8_t *)&digitalLevels, 4);
  data[4] = sampleLigthLevel();
  data[5] = (uint8_t)(uBit.thermometer.getTemperature() + 128);
#if MICROBIT_CODAL
  // data[6] = uBit.microphone.soundLevel(); // This is not implemented yet.
  data[6] = soundLevel;
#endif // MICROBIT_CODAL
}

/**
 * @brief Update data of direction.
 *
 * @param data Buffer for BLE characteristics.
 */
void MbitMoreDevice::updateDirection(uint8_t *data) {
  // Accelerometer
  int16_t rot;
  // Pitch (radians / 1000) is sent as int16_t little-endian [0..1].
  rot = (int16_t)(uBit.accelerometer.getPitchRadians() * 1000);
  memcpy(&(data[0]), &rot, 2);
  // Roll (radians / 1000) is sent as int16_t little-endian [2..3].
  rot = (int16_t)(uBit.accelerometer.getRollRadians() * 1000);
  memcpy(&(data[2]), &rot, 2);

  int16_t acc;
  // Acceleration X [milli-g] is sent as int16_t little-endian [4..5].
  acc = (int16_t)-uBit.accelerometer.getX(); // Face side is positive in Z-axis.
  memcpy(&(data[4]), &acc, 2);
  // Acceleration Y [milli-g] is sent as int16_t little-endian [6..7].
  acc = (int16_t)uBit.accelerometer.getY();
  memcpy(&(data[6]), &acc, 2);
  // Acceleration Z [milli-g] is sent as int16_t little-endian [8..9].
  acc = (int16_t)-uBit.accelerometer.getZ(); // Face side is positive in Z-axis.
  memcpy(&(data[8]), &acc, 2);

  // Magnetometer
  // Compass Heading is sent as uint16_t little-endian [10..11]
  uint16_t heading = (uint16_t)normalizeCompassHeading(uBit.compass.heading());
  memcpy(&(data[10]), &heading, 2);

  int16_t force;
  // Magnetic force X (micro-teslas) is sent as int16_t little-endian[12..13].
  force = (int16_t)(uBit.compass.getX() / 1000);
  memcpy(&(data[12]), &force, 2);
  // Magnetic force Y (micro-teslas) is sent as int16_t little-endian[14..15].
  force = (int16_t)(uBit.compass.getY() / 1000);
  memcpy(&(data[14]), &force, 2);
  // Magnetic force Z (micro-teslas) is sent as int16_t little-endian[16..17].
  force = (int16_t)(uBit.compass.getZ() / 1000);
  memcpy(&(data[16]), &force, 2);
}

/**
 * @brief Get data of analog input of the pin.
 *
 * @param data Buffer for BLE characteristics.
 * @param pinIndex Index of the pin [0, 1, 2].
 */
void MbitMoreDevice::updateAnalogIn(uint8_t *data, size_t pinIndex) {
  if (uBit.io.pin[pinIndex].isInput()) {
#if MICROBIT_CODAL
    uBit.io.pin[pinIndex].setPull(PullMode::None);
    // value = (uint16_t)uBit.io.pin[pinIndex].getAnalogValue();
#else // NOT MICROBIT_CODAL
    uBit.io.pin[pinIndex].setPull(PinMode::PullNone);
#endif // NOT MICROBIT_CODAL

    // filter
    for (size_t i = 0; i < ANALOG_IN_SAMPLES_SIZE; i++) {
      analogInSamples[pinIndex][i] = uBit.io.pin[pinIndex].getAnalogValue();
    }
    uint16_t value = median(analogInSamples[pinIndex], ANALOG_IN_SAMPLES_SIZE);

    // analog value (0 to 1023) is sent as uint16_t little-endian.
    memcpy(&(data[0]), &value, 2);
    setPullMode(pinIndex, pullMode[pinIndex]);
  }
}

/**
 * @brief Sample current light level and return filtered value.
 *
 * @return int Filtered light level.
 */
int MbitMoreDevice::sampleLigthLevel() {
  lightLevelSamplesLast++;
  if (lightLevelSamplesLast == LIGHT_LEVEL_SAMPLES_SIZE) {
    lightLevelSamplesLast = 0;
  }
  lightLevelSamples[lightLevelSamplesLast] = uBit.display.readLightLevel();
  return average(lightLevelSamples, LIGHT_LEVEL_SAMPLES_SIZE);
}

#if MICROBIT_CODAL
/**
 * @brief Return message ID for the label
 * 
 * @param messageLabelChar message label
 * @return int 
 */
int MbitMoreDevice::findWaitingMessageID(const char *messageLabelChar) {
  for (int i = 0; i < MBIT_MORE_WAITING_MESSAGE_LENGTH; i++) {
    if (receivedMessages[i].label[0] == 0)
      continue;
    if (strncmp(receivedMessages[i].label, messageLabelChar, MBIT_MORE_MESSAGE_LABEL_SIZE)) {
      return i;
    }
  }
  return MBIT_MORE_WAITING_MESSAGE_NOT_FOUND;
}

/**
 * @brief Register message label retrun message ID.
 *
 * @param messageLabel
 * @return int ID for the message label
 */
int MbitMoreDevice::registerWaitingMessage(ManagedString messageLabel) {
  int id = findWaitingMessageID(messageLabel.toCharArray());
  if (id == MBIT_MORE_WAITING_MESSAGE_NOT_FOUND) {
    // find blank message ID and resister it
    for (int i = 0; i < MBIT_MORE_WAITING_MESSAGE_LENGTH; i++) {
      if (receivedMessages[i].label[0] == 0) {
        id = i;
        copyManagedString(
            (char *)(receivedMessages[id].label),
            messageLabel,
            MBIT_MORE_MESSAGE_LABEL_SIZE);
        break;
      }
    }
  }
  return id;
}

/**
* @brief Get type of content for the message ID
*
* @param messageID
* @return message type
*/
MbitMoreMessageType MbitMoreDevice::messageType(int messageID) {
  return receivedMessages[messageID].type;
}

/**
 * @brief Return content of the message as number
 *
 * @param messageID
 * @return content of the message
 */
float MbitMoreDevice::messageContentAsNumber(int messageID) {
  float content;
  memcpy(&content, receivedMessages[messageID].content, 4);
  return content;
}

/**
 * @brief Return content of the message as string
 *
 * @param messageID
 * @return content of the message
 */
ManagedString MbitMoreDevice::messageContentAsText(int messageID) {
  return ManagedString((char *)(receivedMessages[messageID].content));
}

/**
 * @brief Send a labeled message with content in float.
 * 
 * @param messageLabel 
 * @param messageContent 
 */
void MbitMoreDevice::sendMessageWithNumber(ManagedString messageLabel, float messageContent) {
  uint8_t *data = moreService->messageChBuffer;
  memset(data, 0, MM_CH_BUFFER_SIZE_NOTIFY);
  copyManagedString((char *)(&data[0]), messageLabel, MBIT_MORE_MESSAGE_LABEL_SIZE);
  memcpy(&data[MBIT_MORE_MESSAGE_LABEL_SIZE], &messageContent, 4);
  data[MBIT_MORE_DATA_FORMAT_INDEX] = MbitMoreDataFormat::MESSAGE_NUMBER;
  moreService->notifyMessage();
}

/**
 * @brief Send a labeled message with content in string.
 * 
 * @param messageLabel 
 * @param messageContent 
 */
void MbitMoreDevice::sendMessageWithText(ManagedString messageLabel, ManagedString messageContent) {
  uint8_t *data = moreService->messageChBuffer;
  memset(data, 0, MM_CH_BUFFER_SIZE_NOTIFY);
  copyManagedString(
      (char *)(&data[0]),
      messageLabel,
      MBIT_MORE_MESSAGE_LABEL_SIZE);
  copyManagedString(
      (char *)(&data[MBIT_MORE_MESSAGE_LABEL_SIZE]),
      messageContent,
      MBIT_MORE_MESSAGE_CONTENT_SIZE);
  data[MBIT_MORE_DATA_FORMAT_INDEX] = MbitMoreDataFormat::MESSAGE_TEXT;
  moreService->notifyMessage();
}

/**
 * @brief Whether the on-board microphon is in use.
 * 
 * @return true  when use
 * @return false when not use
 */
bool MbitMoreDevice::isMicInUse() {
  return micInUse;
}

/**
   * @brief Set sound loudness level.
   * 
   * @param level
   */
void MbitMoreDevice::setSoundLevel(float level) {
  soundLevel = level;
}
#endif // MICROBIT_CODAL

void MbitMoreDevice::displayFriendlyName() {
  ManagedString version(" -M 0.6.0- ");
  uBit.display.scrollAsync(ManagedString(microbit_friendly_name()) + version,
                           120);
}
