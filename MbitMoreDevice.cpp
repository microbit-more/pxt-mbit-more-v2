
#include "pxt.h"

#include "MicroBit.h"
#include "MicroBitConfig.h"

#if MICROBIT_CODAL
// microphone sound level
#include "LevelDetector.h"
#include "LevelDetectorSPL.h"

#define MICROPHONE_MIN 52.0f
#define MICROPHONE_MAX 120.0f

namespace pxt {
  codal::LevelDetectorSPL *getMicrophoneLevel();
} // namespace pxt

int getMicLevel() {
  auto level = pxt::getMicrophoneLevel();
  if (NULL == level)
    return 0;
  const int micValue = level->getValue();
  const int scaled = max(MICROPHONE_MIN, min(micValue, MICROPHONE_MAX)) - MICROPHONE_MIN;
  return min(0xff, scaled * 0xff / (MICROPHONE_MAX - MICROPHONE_MIN));
}
#endif // MICROBIT_CODAL

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

#include "MbitMoreDevice.h"

/**
 * Constructor.
 * Create a representation of the device for Microbit More service.
 * @param _uBit The instance of a MicroBit runtime.
 */
MbitMoreDevice::MbitMoreDevice(MicroBit &_uBit) : uBit(_uBit) {
  // Reset compass
#if MICROBIT_CODAL
  // On microbit-v2, re-calibration destruct compass heading.
#else // NOT MICROBIT_CODAL
  if (uBit.buttonA.isPressed()) {
    uBit.compass.clearCalibration();
  }
#endif // NOT MICROBIT_CODAL

  // Compass must be calibrated before starting bluetooth service.
  if (!uBit.compass.isCalibrated()) {
    uBit.compass.calibrate();
  }

  // to detect 8G gesture event
  uBit.accelerometer.setRange(8);

  displayVersion();

  uBit.messageBus.listen(
      MICROBIT_ID_BUTTON_A, MICROBIT_EVT_ANY,
      this,
      &MbitMoreDevice::onButtonChanged,
      MESSAGE_BUS_LISTENER_QUEUE_IF_BUSY);
  uBit.messageBus.listen(
      MICROBIT_ID_BUTTON_B,
      MICROBIT_EVT_ANY,
      this,
      &MbitMoreDevice::onButtonChanged,
      MESSAGE_BUS_LISTENER_QUEUE_IF_BUSY);

#if MICROBIT_CODAL
  uBit.messageBus.listen(
      MICROBIT_ID_LOGO,
      MICROBIT_EVT_ANY,
      this,
      &MbitMoreDevice::onButtonChanged,
      MESSAGE_BUS_LISTENER_QUEUE_IF_BUSY);
#endif // MICROBIT_CODAL

  uBit.messageBus.listen(
      MICROBIT_ID_GESTURE,
      MICROBIT_EVT_ANY,
      this,
      &MbitMoreDevice::onGestureChanged,
      MESSAGE_BUS_LISTENER_QUEUE_IF_BUSY);

  uBit.messageBus.listen(
      MICROBIT_ID_BLE,
      MICROBIT_BLE_EVT_CONNECTED,
      this,
      &MbitMoreDevice::onBLEConnected,
      MESSAGE_BUS_LISTENER_QUEUE_IF_BUSY);
  uBit.messageBus.listen(
      MICROBIT_ID_BLE,
      MICROBIT_BLE_EVT_DISCONNECTED,
      this,
      &MbitMoreDevice::onBLEDisconnected,
      MESSAGE_BUS_LISTENER_QUEUE_IF_BUSY);
#if MBIT_MORE_USE_SERIAL
  serialService = new MbitMoreSerial(*this);
#endif // MBIT_MORE_USE_SERIAL
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
  // P0,P1,P2 are pull-up as standard extension.
  for (size_t i = 0; i < (sizeof(initialPullUp) / sizeof(initialPullUp[0])); i++) {
    setPullMode(initialPullUp[i], MbitMorePullMode::Up);
    uBit.io.pin[initialPullUp[i]].getDigitalValue(); // set the pin to input-mode
  }
}

/**
 * @brief Update version data on the characteristic.
 * 
 */
void MbitMoreDevice::updateVersionData() {
  uint8_t *data = moreService->commandChBuffer;
#if MICROBIT_CODAL
  data[0] = MbitMoreHardwareVersion::MICROBIT_V2;
#else // NOT MICROBIT_CODAL
  data[0] = MbitMoreHardwareVersion::MICROBIT_V1;
#endif // NOT MICROBIT_CODAL
  data[1] = MbitMoreProtocol::MBIT_MORE_V2;
}

/**
 * @brief Invoked when BLE connected.
 * 
 * @param _e event which has connection data
 */
void MbitMoreDevice::onBLEConnected(MicroBitEvent _e) {
#if MICROBIT_CODAL
  fiber_sleep(100); // to change pull-mode in micro:bit v2
#endif // MICROBIT_CODAL
  initializeConfig();
  uBit.display.stopAnimation(); // To stop display friendly name.
  uBit.display.print("M");
}

/**
 * @brief Invoked when BLE disconnected.
 * 
 * @param _e event which has disconnection data
 */
void MbitMoreDevice::onBLEDisconnected(MicroBitEvent _e) {
  uBit.reset(); // reset to off microphone and its LED.
}

void MbitMoreDevice::onSerialConnected() {
  uBit.ble->stopAdvertising();
  initializeConfig();
  uBit.display.stopAnimation(); // To stop display friendly name.
  uBit.display.print("M");
  serialConnected = true;
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
    int pinIndex = (int)data[1];
    if (pinCommand == MbitMorePinCommand::SET_PULL) {
      uBit.io.pin[pinIndex].getDigitalValue(); // set the pin to input mode
      setPullMode(pinIndex, (MbitMorePullMode)data[2]);
    } else if (pinCommand == MbitMorePinCommand::SET_OUTPUT) {
#if MICROBIT_CODAL
      // workaround to set d-out from touch-mode in microbit-codal-v2
      if (touchMode[pinIndex]) {
        uBit.io.pin[pinIndex].setAnalogValue(0);
      }
#endif // MICROBIT_CODAL
      setDigitalValue(pinIndex, data[2]);
    } else if (pinCommand == MbitMorePinCommand::SET_PWM) {
      // value is read as uint16_t little-endian.
      uint16_t value;
      memcpy(&value, &(data[2]), 2);
      setAnalogValue(pinIndex, value);
    } else if (pinCommand == MbitMorePinCommand::SET_SERVO) {
      // angle is read as uint16_t little-endian.
      uint16_t angle;
      memcpy(&angle, &(data[2]), 2);
      // range is read as uint16_t little-endian.
      uint16_t range;
      memcpy(&range, &(data[4]), 2);
      // center is read as uint16_t little-endian.
      uint16_t center;
      memcpy(&center, &(data[6]), 2);
      if (range == 0) {
        uBit.io.pin[pinIndex].setServoValue(angle);
      } else if (center == 0) {
        uBit.io.pin[pinIndex].setServoValue(angle, range);
      } else {
        uBit.io.pin[pinIndex].setServoValue(angle, range, center);
      }
    } else if (pinCommand == MbitMorePinCommand::SET_EVENT) {
      listenPinEventOn(pinIndex, (int)data[2]);
    }
    touchMode[pinIndex] = false;
  } else if (command == MbitMoreCommand::CMD_AUDIO) {
    int audioCommand = data[0] & 0b11111;
    if (audioCommand == MbitMoreAudioCommand::PLAY_TONE) {
      uint32_t period;
      memcpy(&period, &(data[1]), 4);
      playTone(period, data[5]);
    } else if (audioCommand == MbitMoreAudioCommand::STOP_TONE) {
      stopTone();
    }
#if MICROBIT_CODAL
  } else if (command == MbitMoreCommand::CMD_DATA) {
    MbitMoreDataContentType dataType = (MbitMoreDataContentType)(data[0] & 0b11111);
    int index = findWaitingDataLabelIndex((char *)(&data[1]), dataType);
    if (index != MBIT_MORE_WAITING_DATA_LABEL_NOT_FOUND) {
      int contentStart = 1 + MBIT_MORE_DATA_LABEL_SIZE;
      memset(receivedData[index].content, 0, MBIT_MORE_DATA_CONTENT_SIZE);
      memcpy(receivedData[index].content, &data[contentStart], length - contentStart);
      MicroBitEvent evt(MBIT_MORE_DATA_RECEIVED, index + 1);
    }
#endif // MICROBIT_CODAL
  } else if (command == MbitMoreCommand::CMD_CONFIG) {
    const int config = data[0] & 0b11111;
    if (config == MbitMoreConfig::MIC) {
#if MICROBIT_CODAL
      micInUse = ((data[1] == 1) ? true : false);
#endif // MICROBIT_CODAL
    } else if (config == MbitMoreConfig::TOUCH) {
      int pinIndex = data[1];
      if (pinIndex > 2)
        return;
      int componentID = pinIndex + 100;
      if (data[2] == 1) {
        uBit.messageBus.listen(
            componentID,
            MICROBIT_EVT_ANY,
            this,
            &MbitMoreDevice::onButtonChanged,
            MESSAGE_BUS_LISTENER_QUEUE_IF_BUSY);
#if MICROBIT_CODAL
        uBit.io.pin[pinIndex].isTouched(codal::TouchMode::Capacitative);
#else // NOT MICROBIT_CODAL
        uBit.io.pin[pinIndex].isTouched();
#endif // NOT MICROBIT_CODAL
        touchMode[pinIndex] = true;
      } else {
        uBit.messageBus.ignore(
            componentID,
            MICROBIT_EVT_ANY,
            this,
            &MbitMoreDevice::onButtonChanged);
      }
    }
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
 * @brief Display text on LED.
 *
 * @param text Contents to display with null termination.
 * @param delay The time to delay between characters, in milliseconds.
 */
void MbitMoreDevice::displayText(char *text, int delay) {
  ManagedString mstr(text);
  if (mstr.length() < 1) {
    return;
  }
  uBit.display.stopAnimation();
  if (delay <= 0) {
    uBit.display.printCharAsync(mstr.charAt(0), delay);
    return;
  }
  uBit.display.scrollAsync(mstr, delay);
}

/**
 * @brief Update GPIO and sensors state.
 *
 * @param data Buffer for BLE characteristics.
 */
void MbitMoreDevice::updateState(uint8_t *data) {
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
  if (touchMode[0]) {
    digitalLevels = digitalLevels | (uBit.io.pin[0].isTouched() << MbitMoreButtonStateIndex::P0);
  }
  if (touchMode[1]) {
    digitalLevels = digitalLevels | (uBit.io.pin[1].isTouched() << MbitMoreButtonStateIndex::P1);
  }
  if (touchMode[2]) {
    digitalLevels = digitalLevels | (uBit.io.pin[2].isTouched() << MbitMoreButtonStateIndex::P2);
  }
  digitalLevels = digitalLevels | (uBit.buttonA.isPressed() << MbitMoreButtonStateIndex::A);
  digitalLevels = digitalLevels | (uBit.buttonB.isPressed() << MbitMoreButtonStateIndex::B);
#if MICROBIT_CODAL
  digitalLevels = digitalLevels | (uBit.logo.isPressed() << MbitMoreButtonStateIndex::LOGO);
#endif // MICROBIT_CODAL
  memcpy(data, (uint8_t *)&digitalLevels, 4);
  data[4] = sampleLightLevel();
  data[5] = (uint8_t)(uBit.thermometer.getTemperature() + 128);
#if MICROBIT_CODAL
  if (micInUse) {
    data[6] = getMicLevel();
  }
#endif // MICROBIT_CODAL
}

/**
 * @brief Update data of motion.
 *
 * @param data Buffer for BLE characteristics.
 */
void MbitMoreDevice::updateMotion(uint8_t *data) {
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
int MbitMoreDevice::sampleLightLevel() {
  lightLevelSamplesLast++;
  if (lightLevelSamplesLast == LIGHT_LEVEL_SAMPLES_SIZE) {
    lightLevelSamplesLast = 0;
  }
  lightLevelSamples[lightLevelSamplesLast] = uBit.display.readLightLevel();
  return average(lightLevelSamples, LIGHT_LEVEL_SAMPLES_SIZE);
}

/**
 * @brief Set PMW signal to the pin for play tone.
 * 
 * @param period  PWM period (1000000 / frequency)[us]
 * @param volume laudness of the sound [0..255]
 */
void MbitMoreDevice::playTone(int period, int volume) {
#if MICROBIT_CODAL
  MicroBitPin speakerPin = uBit.io.speaker;
#else // NOT MICROBIT_CODAL
  MicroBitPin speakerPin = uBit.io.pin[0];
#endif // NOT MICROBIT_CODAL
  if (period <= 0 || volume == 0) {
    speakerPin.setAnalogValue(0);
  } else {
    int v = 1 << (volume >> 5); // [2..14]
    speakerPin.setAnalogValue(v);
    speakerPin.setAnalogPeriodUs(period);
  }
}

/**
 * @brief Stop playing tone.
 * 
 */
void MbitMoreDevice::stopTone() {
#if MICROBIT_CODAL
  MicroBitPin speakerPin = uBit.io.speaker;
#else // NOT MICROBIT_CODAL
  MicroBitPin speakerPin = uBit.io.pin[0];
#endif // NOT MICROBIT_CODAL
  speakerPin.setAnalogValue(0);
}

#if MICROBIT_CODAL
/**
 * @brief Return index for the label
 * 
 * @param dataLabel label to find
 * @param dataType type of the data
 * @return int index of the label
 */
int MbitMoreDevice::findWaitingDataLabelIndex(const char *dataLabel, MbitMoreDataContentType dataType) {
  for (int i = 0; i < MBIT_MORE_WAITING_DATA_LABELS_LENGTH; i++) {
    if (receivedData[i].label[0] == 0)
      continue;
    if (receivedData[i].type == dataType) {
      if (0 == strncmp(receivedData[i].label,
                       dataLabel,
                       MBIT_MORE_DATA_LABEL_SIZE)) {
        return i;
      }
    }
  }
  return MBIT_MORE_WAITING_DATA_LABEL_NOT_FOUND;
}

/**
 * @brief Register data label and retrun ID for the label.
 *
 * @param dataLabel label to register
 * @param dataType type of the data
 * @return int ID for the label
 */
int MbitMoreDevice::registerWaitingDataLabel(ManagedString dataLabel, MbitMoreDataContentType dataType) {
  int index = findWaitingDataLabelIndex(dataLabel.toCharArray(), dataType);
  if (index == MBIT_MORE_WAITING_DATA_LABEL_NOT_FOUND) {
    // find blank index and resister it
    for (int i = 0; i < MBIT_MORE_WAITING_DATA_LABELS_LENGTH; i++) {
      if (receivedData[i].label[0] == 0) {
        index = i;
        receivedData[index].type = dataType;
        strncpy(
            receivedData[index].label,
            dataLabel.toCharArray(),
            MBIT_MORE_DATA_LABEL_SIZE);
        return index + 1; // It is used for event value and must not be 0 (0 to accept any events).
      }
    }
  }
  return 0;
}

/**
 * @brief Get type of content for the labeled data
 *
 * @param labelID ID of the label in received data
 * @return content type
 */
MbitMoreDataContentType MbitMoreDevice::dataType(int labelID) {
  return receivedData[labelID - 1].type;
}

/**
 * @brief Return content of the data as number
 *
 * @param labelID ID of the label in received data
 * @return content of the data
 */
float MbitMoreDevice::dataContentAsNumber(int labelID) {
  float content;
  memcpy(&content, receivedData[labelID - 1].content, 4);
  return content;
}

/**
 * @brief Return content of the data as text
 *
 * @param labelID ID of the label in received data
 * @return content of the data
 */
ManagedString MbitMoreDevice::dataContentAsText(int labelID) {
  return ManagedString((char *)(receivedData[labelID - 1].content));
}

/**
 * @brief Send number with label.
 * 
 * @param dataLabel 
 * @param dataContent 
 */
void MbitMoreDevice::sendNumberWithLabel(ManagedString dataLabel, float dataContent) {
  uint8_t *data = moreService->dataChBuffer;
  memset(data, 0, MM_CH_BUFFER_SIZE_NOTIFY);
  copyManagedString((char *)(&data[0]), dataLabel, MBIT_MORE_DATA_LABEL_SIZE);
  memcpy(&data[MBIT_MORE_DATA_LABEL_SIZE], &dataContent, 4);
  data[MBIT_MORE_DATA_FORMAT_INDEX] = MbitMoreDataFormat::DATA_NUMBER;
#if MBIT_MORE_USE_SERIAL
  if (serialConnected) {
    serialService->notifyOnSerial(0x0130, data, MM_CH_BUFFER_SIZE_NOTIFY);
    return;
  }
#endif // MBIT_MORE_USE_SERIAL
  moreService->notifyData();
}

/**
 * @brief Send text with label.
 * 
 * @param dataLabel 
 * @param dataContent 
 */
void MbitMoreDevice::sendTextWithLabel(ManagedString dataLabel, ManagedString dataContent) {
  uint8_t *data = moreService->dataChBuffer;
  memset(data, 0, MM_CH_BUFFER_SIZE_NOTIFY);
  copyManagedString(
      (char *)(&data[0]),
      dataLabel,
      MBIT_MORE_DATA_LABEL_SIZE);
  copyManagedString(
      (char *)(&data[MBIT_MORE_DATA_LABEL_SIZE]),
      dataContent,
      MBIT_MORE_DATA_CONTENT_SIZE);
  data[MBIT_MORE_DATA_FORMAT_INDEX] = MbitMoreDataFormat::DATA_TEXT;
#if MBIT_MORE_USE_SERIAL
  if (serialConnected) {
    serialService->notifyOnSerial(0x0130, data, MM_CH_BUFFER_SIZE_NOTIFY);
    return;
  }
#endif // MBIT_MORE_USE_SERIAL
  moreService->notifyData();
}

#endif // MICROBIT_CODAL

/**
 * @brief Listen pin events on the pin.
 * Make it listen events of the event type on the pin.
 * Remove listener if the event type is MICROBIT_PIN_EVENT_NONE.
 * 
 * @param pinIndex index in edge pins
 * @param eventType type of events
 */
void MbitMoreDevice::listenPinEventOn(int pinIndex, int eventType) {
  if (!isGpio(pinIndex)) {
    return;
  }
  // conventional scheme to convert from pin index to componentID in v1 and v2.
  int componentID = pinIndex + 100;
  uBit.messageBus.ignore(
      componentID,
      MICROBIT_PIN_EVT_RISE,
      this,
      &MbitMoreDevice::onPinEvent);
  uBit.messageBus.ignore(
      componentID,
      MICROBIT_PIN_EVT_FALL,
      this,
      &MbitMoreDevice::onPinEvent);
  uBit.messageBus.ignore(
      componentID,
      MICROBIT_PIN_EVT_PULSE_HI,
      this,
      &MbitMoreDevice::onPinEvent);
  uBit.messageBus.ignore(
      componentID,
      MICROBIT_PIN_EVT_PULSE_LO,
      this,
      &MbitMoreDevice::onPinEvent);

  if (eventType == MbitMorePinEventType::ON_EDGE) {
    uBit.messageBus.listen(
        componentID,
        MICROBIT_PIN_EVT_RISE,
        this,
        &MbitMoreDevice::onPinEvent,
        MESSAGE_BUS_LISTENER_QUEUE_IF_BUSY);
    uBit.messageBus.listen(
        componentID,
        MICROBIT_PIN_EVT_FALL,
        this,
        &MbitMoreDevice::onPinEvent,
        MESSAGE_BUS_LISTENER_QUEUE_IF_BUSY);
    uBit.io.pin[pinIndex].eventOn(MICROBIT_PIN_EVENT_ON_EDGE);
  } else if (eventType == MbitMorePinEventType::ON_PULSE) {
    uBit.messageBus.listen(
        componentID,
        MICROBIT_PIN_EVT_PULSE_HI,
        this,
        &MbitMoreDevice::onPinEvent,
        MESSAGE_BUS_LISTENER_QUEUE_IF_BUSY);
    uBit.messageBus.listen(
        componentID,
        MICROBIT_PIN_EVT_PULSE_LO,
        this,
        &MbitMoreDevice::onPinEvent,
        MESSAGE_BUS_LISTENER_QUEUE_IF_BUSY);
#if MICROBIT_CODAL
    // ?? Freeze BLE when onEvent(PULSE) first time. ??
    uBit.io.pin[pinIndex].eventOn(MICROBIT_PIN_EVENT_NONE); // workaround to prevent to freeze BLE
    uBit.io.pin[pinIndex].eventOn(MICROBIT_PIN_EVENT_ON_PULSE);
    // ?? Pull-mode is released and will not be reset in this thread. ??
    setPullMode(pinIndex, pullMode[pinIndex]); // does not work?
#else // NOT MICROBIT_CODAL
    uBit.io.pin[pinIndex].eventOn(MICROBIT_PIN_EVENT_ON_PULSE);
#endif // NOT MICROBIT_CODAL
  } else if (eventType == MbitMorePinEventType::NONE) {
    uBit.io.pin[pinIndex].eventOn(MICROBIT_PIN_EVENT_NONE);
#if MICROBIT_CODAL
    // ?? Pull-mode is released and will not be reset in this thread. ??
    setPullMode(pinIndex, pullMode[pinIndex]); // does not work?
#endif // MICROBIT_CODAL
  }
}

/**
 * Callback. Invoked when a pin event sent.
 */
void MbitMoreDevice::onPinEvent(MicroBitEvent evt) {
  uint8_t *data = moreService->pinEventChBuffer;

  // pinIndex is sent as uint8_t.
  // conventional scheme to convert from componentID to pin index in v1 and v2.
  data[0] = evt.source - 100;
  // event ID is sent as uint8_t.
  data[1] = (uint8_t)evt.value;

  // event timestamp is sent as uint32_t little-endian
  // downcast from uint64_t value.
  uint32_t timestamp = (uint32_t)evt.timestamp;
  memcpy(&(data[2]), &timestamp, 4);
  data[MBIT_MORE_DATA_FORMAT_INDEX] = MbitMoreDataFormat::PIN_EVENT;
#if MBIT_MORE_USE_SERIAL
  if (serialConnected) {
    serialService->notifyOnSerial(0x0110, data, MM_CH_BUFFER_SIZE_NOTIFY);
    return;
  }
#endif // MBIT_MORE_USE_SERIAL
  moreService->notifyPinEvent();
}

/**
 * @brief Invoked when button state changed.
 * 
 * @param evt event which has button states
 */
void MbitMoreDevice::onButtonChanged(MicroBitEvent evt) {
  uint8_t *data = moreService->actionEventChBuffer;
  data[0] = MbitMoreActionEvent::BUTTON;
  // source is a component ID that generated the event as uint16_t little-endian.
  // MICROBIT_ID_BUTTON_A, MICROBIT_ID_IO_P0, MICROBIT_ID_LOGO, etc.
  memcpy(&(data[1]), &evt.source, 2);
  // Event ID send as uint16_t little-endian.
  // MICROBIT_BUTTON_EVT_DOWN, MICROBIT_BUTTON_EVT_CLICK, etc.
  data[3] = (uint8_t)evt.value;
  // Timestamp of the event send as uint32_t little-endian.
  // downcast from uint64_t value.
  uint32_t timestamp = (uint32_t)evt.timestamp;
  memcpy(&(data[4]), &timestamp, 4);
  data[MBIT_MORE_DATA_FORMAT_INDEX] = MbitMoreDataFormat::ACTION_EVENT;
#if MBIT_MORE_USE_SERIAL
  if (serialConnected) {
    serialService->notifyOnSerial(0x0111, data, MM_CH_BUFFER_SIZE_NOTIFY);
    return;
  }
#endif // MBIT_MORE_USE_SERIAL
  moreService->notifyActionEvent();
}

/**
 * @brief Invoked when gesture state changed.
 * 
 * @param evt event which has gesture states.
 */
void MbitMoreDevice::onGestureChanged(MicroBitEvent evt) {
  uint8_t *data = moreService->actionEventChBuffer;
  data[0] = MbitMoreActionEvent::GESTURE;
  // Event ID send as uint8_t.
  // MICROBIT_ACCELEROMETER_EVT_TILT_UP, MICROBIT_ACCELEROMETER_EVT_FACE_UP, etc.
  data[1] = (uint8_t)evt.value;
  // Timestamp of the event send as uint32_t little-endian.
  // downcast from uint64_t value.
  uint32_t timestamp = (uint32_t)evt.timestamp;
  memcpy(&(data[2]), &timestamp, 4);
  data[MBIT_MORE_DATA_FORMAT_INDEX] = MbitMoreDataFormat::ACTION_EVENT;
#if MBIT_MORE_USE_SERIAL
  if (serialConnected) {
    serialService->notifyOnSerial(0x0111, data, MM_CH_BUFFER_SIZE_NOTIFY);
    return;
  }
#endif // MBIT_MORE_USE_SERIAL
  moreService->notifyActionEvent();
}

/**
 * @brief Normalize angle when upside down.
 * 
 * @param heading value of the compass heading
 * @return normalizes angle relative to north [degree]
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
 * @brief Set pull-mode.
 * 
 * @param pinIndex index to set
 * @param pull pull-mode to set
 */
void MbitMoreDevice::setPullMode(int pinIndex, MbitMorePullMode pull) {
  pullMode[pinIndex] = pull;
#if MICROBIT_CODAL
  switch (pull) {
  case MbitMorePullMode::None:
    uBit.io.pin[pinIndex].setPull(PullMode::None);
    break;
  case MbitMorePullMode::Up:
    uBit.io.pin[pinIndex].setPull(PullMode::Up);
    break;
  case MbitMorePullMode::Down:
    uBit.io.pin[pinIndex].setPull(PullMode::Down);
    break;

  default:
    break;
  }
#else // NOT MICROBIT_CODAL
  switch (pull) {
  case MbitMorePullMode::None:
    uBit.io.pin[pinIndex].setPull(PinMode::PullNone);
    break;
  case MbitMorePullMode::Up:
    uBit.io.pin[pinIndex].setPull(PinMode::PullUp);
    break;
  case MbitMorePullMode::Down:
    uBit.io.pin[pinIndex].setPull(PinMode::PullDown);
    break;

  default:
    break;
  }
#endif // NOT MICROBIT_CODAL
}

/**
 * @brief Set the value on the pin as digital output.
 * 
 * @param pinIndex index in edge pins
 * @param value digital value [0 | 1]
 */
void MbitMoreDevice::setDigitalValue(int pinIndex, int value) {
  uBit.io.pin[pinIndex].setDigitalValue(value);
}

/**
 * @brief Set the value on the pin as analog output (PWM).
 * 
 * @param pinIndex index in edge pins
 * @param value analog value (0..1024)
 */
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

/**
 * @brief Set the value on the pin as servo driver.
 * 
 * @param pinIndex index in edge pins
 * @param angle the level to set on the output pin, in the range 0 - 180.
 * @param range which gives the span of possible values the i.e. the lower and upper bounds (center +/- range/2). Defaults to DEVICE_PIN_DEFAULT_SERVO_RANGE.
 * @param center the center point from which to calculate the lower and upper bounds. Defaults to DEVICE_PIN_DEFAULT_SERVO_CENTER
 */
void MbitMoreDevice::setServoValue(int pinIndex, int angle, int range,
                                   int center) {
  uBit.io.pin[pinIndex].setServoValue(angle, range, center);
}

/**
 * @brief Display friendly name of the micro:bit.
 * 
 */
void MbitMoreDevice::displayFriendlyName() {
  if (serialConnected)
    return;
  uBit.display.scrollAsync(ManagedString(microbit_friendly_name()), 120);
}

/**
 * @brief Display software version of Microbit More.
 * 
 */
void MbitMoreDevice::displayVersion() {
  uBit.display.scrollAsync(ManagedString(" -M 0.2.5- "), 120);
}

/**
 * @brief Whether the pin is a GPIO of not.
 * 
 * @param pinIndex index in edge pins
 * @return true the pin is a GPIO
 * @return false the pin is not a GPIO
 */
bool MbitMoreDevice::isGpio(int pinIndex) {
  for (size_t i = 0; i < (sizeof(gpioPin) / sizeof(gpioPin[0])); i++) {
    if (pinIndex == gpioPin[i])
      return true;
  }
  return false;
}