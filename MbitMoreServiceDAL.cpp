#include "pxt.h"

#if !MICROBIT_CODAL

/**
  * Class definition for the Scratch MicroBit More Service.
  * Provides a BLE service to remotely controll Micro:bit from Scratch3.
  */
#include "MbitMoreServiceDAL.h"

int gpio[] = {0, 1, 2, 8, 13, 14, 15, 16};
int analogIn[] = {0, 1, 2};
int digitalIn[] = {0, 1, 2}; // PullUp at connected to be same behaviour as the standard extension.

/**
 * Get voltage of the power supply [mV].
 */
int getPowerVoltage(void)
{
  // configuration
  NRF_ADC->ENABLE = ADC_ENABLE_ENABLE_Enabled;
  NRF_ADC->CONFIG = (ADC_CONFIG_RES_10bit << ADC_CONFIG_RES_Pos) |
                    (ADC_CONFIG_INPSEL_SupplyOneThirdPrescaling << ADC_CONFIG_INPSEL_Pos) |
                    (ADC_CONFIG_REFSEL_VBG << ADC_CONFIG_REFSEL_Pos) |
                    (ADC_CONFIG_PSEL_Disabled << ADC_CONFIG_PSEL_Pos) |
                    (ADC_CONFIG_EXTREFSEL_None << ADC_CONFIG_EXTREFSEL_Pos);

  // read analog value from power supply
  NRF_ADC->CONFIG &= ~ADC_CONFIG_PSEL_Msk;
  NRF_ADC->CONFIG |= ADC_CONFIG_PSEL_Disabled << ADC_CONFIG_PSEL_Pos;
  NRF_ADC->TASKS_START = 1;
  while (((NRF_ADC->BUSY & ADC_BUSY_BUSY_Msk) >> ADC_BUSY_BUSY_Pos) == ADC_BUSY_BUSY_Busy)
  {
  };
  return (int)(((float)NRF_ADC->RESULT * 3600.0f) / 1023.0f);
}

/**
  * Constructor.
  * Create a representation of the Microbit More BLE Service
  * @param _uBit The instance of a MicroBit runtime.
  */
MbitMoreServiceDAL::MbitMoreServiceDAL()
    : uBit(pxt::uBit)
{
  // Calibrate the compass before start bluetooth service.
  if (!uBit.compass.isCalibrated())
  {
    uBit.compass.calibrate();
  }

  // Create the data structures that represent each of our characteristics in Soft Device.
  GattCharacteristic txCharacteristic(
      MBIT_MORE_BASIC_TX,
      (uint8_t *)&txData,
      0,
      sizeof(txData),
      GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_READ | GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY);

  GattCharacteristic rxCharacteristic(
      MBIT_MORE_BASIC_RX,
      (uint8_t *)&rxBuffer,
      0,
      sizeof(rxBuffer),
      GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_WRITE | GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_WRITE_WITHOUT_RESPONSE);

  // Set default security requirements
  txCharacteristic.requireSecurity(SecurityManager::MICROBIT_BLE_SECURITY_LEVEL);
  rxCharacteristic.requireSecurity(SecurityManager::MICROBIT_BLE_SECURITY_LEVEL);

  GattCharacteristic *characteristics[] = {&txCharacteristic, &rxCharacteristic};
  GattService service(
      MBIT_MORE_BASIC_SERVICE, characteristics,
      sizeof(characteristics) / sizeof(GattCharacteristic *));

  uBit.ble->addService(service);

  txCharacteristicHandle = txCharacteristic.getValueHandle();
  rxCharacteristicHandle = rxCharacteristic.getValueHandle();

  uBit.ble->gattServer().write(
      txCharacteristicHandle,
      (uint8_t *)&txData,
      sizeof(txData));

  // Advertise this service.
  const uint16_t uuid16_list[] = {MBIT_MORE_BASIC_SERVICE};
  
  uBit.ble->gap().accumulateAdvertisingPayload(GapAdvertisingData::INCOMPLETE_LIST_16BIT_SERVICE_IDS, (uint8_t *)uuid16_list, sizeof(uuid16_list));

  // Mbit More Service
  // Create the data structures that represent each of our characteristics in Soft Device.

  eventChar = new GattCharacteristic(
      MBIT_MORE_EVENT,
      (uint8_t *)&eventBuffer,
      0,
      sizeof(eventBuffer),
      GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_READ | GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY);
  ioChar = new GattCharacteristic(
      MBIT_MORE_IO,
      (uint8_t *)&ioBuffer,
      0,
      sizeof(ioBuffer),
      GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_READ);
  analogInChar = new GattCharacteristic(
      MBIT_MORE_ANALOG_IN,
      (uint8_t *)&analogInBuffer,
      0,
      sizeof(analogInBuffer),
      GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_READ);
  analogInChar->setReadAuthorizationCallback(this, &MbitMoreServiceDAL::onReadAnalogIn);
  sensorsChar = new GattCharacteristic(
      MBIT_MORE_SENSORS,
      (uint8_t *)&sensorsBuffer,
      0,
      sizeof(sensorsBuffer),
      GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_READ);
  sharedDataChar = new GattCharacteristic(
      MBIT_MORE_SHARED_DATA,
      (uint8_t *)&sharedBuffer,
      0,
      sizeof(sharedBuffer),
      GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_READ | GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY | GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_WRITE | GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_WRITE_WITHOUT_RESPONSE);

  // Set default security requirements
  analogInChar->requireSecurity(SecurityManager::MICROBIT_BLE_SECURITY_LEVEL);
  eventChar->requireSecurity(SecurityManager::MICROBIT_BLE_SECURITY_LEVEL);
  ioChar->requireSecurity(SecurityManager::MICROBIT_BLE_SECURITY_LEVEL);
  sensorsChar->requireSecurity(SecurityManager::MICROBIT_BLE_SECURITY_LEVEL);
  sharedDataChar->requireSecurity(SecurityManager::MICROBIT_BLE_SECURITY_LEVEL);

  GattCharacteristic *mbitMoreChars[] = {
      eventChar,
      ioChar,
      analogInChar,
      sensorsChar,
      sharedDataChar};
  GattService mbitMoreService(
      MBIT_MORE_SERVICE, mbitMoreChars,
      sizeof(mbitMoreChars) / sizeof(GattCharacteristic *));

  uBit.ble->addService(mbitMoreService);

  eventCharHandle = eventChar->getValueHandle();
  ioCharHandle = ioChar->getValueHandle();
  sensorsCharHandle = sensorsChar->getValueHandle();
  sharedDataCharHandle = sharedDataChar->getValueHandle();

  // Initialize buffers.
  sharedBuffer[DATA_FORMAT_INDEX] = MbitMoreDataFormat::SHARED_DATA;
  eventBuffer[DATA_FORMAT_INDEX] = MbitMoreDataFormat::EVENT;

  // Advertise this service.
  const uint8_t *mbitMoreServices[] = {MBIT_MORE_SERVICE};
  uBit.ble->gap().accumulateAdvertisingPayload(GapAdvertisingData::COMPLETE_LIST_128BIT_SERVICE_IDS, (uint8_t *)mbitMoreServices, sizeof(mbitMoreServices));

  // Setup callbacks for events.
  uBit.ble->onDataWritten(this, &MbitMoreServiceDAL::onDataWritten);

  uBit.messageBus.listen(MICROBIT_ID_BLE, MICROBIT_BLE_EVT_CONNECTED, this, &MbitMoreServiceDAL::onBLEConnected, MESSAGE_BUS_LISTENER_IMMEDIATE);
  uBit.messageBus.listen(MICROBIT_ID_BLE, MICROBIT_BLE_EVT_DISCONNECTED, this, &MbitMoreServiceDAL::onBLEDisconnected, MESSAGE_BUS_LISTENER_IMMEDIATE);

  uBit.messageBus.listen(MICROBIT_ID_BUTTON_A, MICROBIT_EVT_ANY, this, &MbitMoreServiceDAL::onButtonChanged, MESSAGE_BUS_LISTENER_IMMEDIATE);
  uBit.messageBus.listen(MICROBIT_ID_BUTTON_B, MICROBIT_EVT_ANY, this, &MbitMoreServiceDAL::onButtonChanged, MESSAGE_BUS_LISTENER_IMMEDIATE);
  uBit.messageBus.listen(MICROBIT_ID_GESTURE, MICROBIT_EVT_ANY, this, &MbitMoreServiceDAL::onGestureChanged, MESSAGE_BUS_LISTENER_IMMEDIATE);
}

void MbitMoreServiceDAL::initConfiguration()
{
  // Initialize pin configuration.
  for (size_t i = 0; i < sizeof(digitalIn) / sizeof(digitalIn[0]); i++)
  {
    setPullMode(digitalIn[i], PinMode::PullUp);
  }

  // Initialize microbit more protocol.
  mbitMoreProtocol = 0;
}

/**
  * Callback. Invoked when AnalogIn is read via BLE.
  */
void MbitMoreServiceDAL::onReadAnalogIn(GattReadAuthCallbackParams *authParams)
{
  updateAnalogValues();
  // analog value (0 to 1023) is sent as uint16_t little-endian.
  memcpy(&(analogInBuffer[0]), &(analogValues[0]), 2);
  memcpy(&(analogInBuffer[2]), &(analogValues[1]), 2);
  memcpy(&(analogInBuffer[4]), &(analogValues[2]), 2);
  // voltage of power supply [mV] is sent as uint16_t little-endian.
  memcpy(&(analogInBuffer[6]), &powerVoltage, 2);
  authParams->data = (uint8_t *)&analogInBuffer;
  authParams->offset = 0;
  authParams->len = sizeof(analogInBuffer) / sizeof(analogInBuffer[0]);
  authParams->authorizationReply = AUTH_CALLBACK_REPLY_SUCCESS;
}

/**
  * Callback. Invoked when any of our attributes are written via BLE.
  */
void MbitMoreServiceDAL::onDataWritten(const GattWriteCallbackParams *params)
{
  uint8_t *data = (uint8_t *)params->data;

  if (params->handle == rxCharacteristicHandle && params->len > 0)
  {
    if (data[0] == ScratchBLECommand::CMD_DISPLAY_TEXT)
    {
      char text[params->len];
      memcpy(text, &(data[1]), (params->len) - 1);
      text[(params->len) - 1] = '\0';
      ManagedString mstr(text);
      uBit.display.stopAnimation();        // Do not wait the end of current animation as same as the standard extension.
      uBit.display.scrollAsync(mstr, 120); // Interval is corresponding with the standard extension.
    }
    else if (data[0] == ScratchBLECommand::CMD_DISPLAY_LED)
    {
      uBit.display.stopAnimation();
      for (int y = 1; y < params->len; y++)
      {
        for (int x = 0; x < 5; x++)
        {
          uBit.display.image.setPixelValue(x, y - 1, (data[y] & (0x01 << x)) ? 255 : 0);
        }
      }
    }
    else if (data[0] == ScratchBLECommand::CMD_PIN)
    {
      if (data[1] == MbitMorePinCommand::SET_PULL)
      {
        switch (data[3])
        {
        case MbitMorePullNone:
          setPullMode(data[2], PinMode::PullNone);
          break;
        case MbitMorePullUp:
          setPullMode(data[2], PinMode::PullUp);
          break;
        case MbitMorePullDown:
          setPullMode(data[2], PinMode::PullDown);
          break;

        default:
          break;
        }
      }
      else if (data[1] == MbitMorePinCommand::SET_TOUCH)
      {
        setPinModeTouch(data[2]);
      }
      else if (data[1] == MbitMorePinCommand::SET_OUTPUT)
      {
        setDigitalValue(data[2], data[3]);
      }
      else if (data[1] == MbitMorePinCommand::SET_PWM)
      {
        // value is read as uint16_t little-endian.
        int value;
        memcpy(&value, &(data[3]), 2);
        setAnalogValue(data[2], value);
      }
      else if (data[1] == MbitMorePinCommand::SET_SERVO)
      {
        int pinIndex = (int)data[2];
        // angle is read as uint16_t little-endian.
        uint16_t angle;
        memcpy(&angle, &(data[3]), 2);
        // range is read as uint16_t little-endian.
        uint16_t range;
        memcpy(&range, &(data[5]), 2);
        // center is read as uint16_t little-endian.
        uint16_t center;
        memcpy(&center, &(data[7]), 2);
        if (range == 0)
        {
          uBit.io.pin[pinIndex].setServoValue(angle);
        }
        else if (center == 0)
        {
          uBit.io.pin[pinIndex].setServoValue(angle, range);
        }
        else
        {
          uBit.io.pin[pinIndex].setServoValue(angle, range, center);
        }
      }
      else if (data[1] == MbitMorePinCommand::SET_EVENT)
      {
        listenPinEventOn((int)data[2], (int)data[3]);
      }
    }
    else if (data[0] == ScratchBLECommand::CMD_SHARED_DATA)
    {
      // value is read as int16_t little-endian.
      int16_t value;
      memcpy(&value, &(data[2]), 2);
      sharedData[data[1]] = value;
    }
    else if (data[0] == ScratchBLECommand::CMD_PROTOCOL)
    {
      mbitMoreProtocol = data[1];
    }
    else if (data[0] == ScratchBLECommand::CMD_LIGHT_SENSING)
    {
      setLightSensingDuration(data[1]);
    }
  }
}

/**
 * Make it listen events of the event type on the pin.
 * Remove listener if the event type is MICROBIT_PIN_EVENT_NONE.
 */
void MbitMoreServiceDAL::listenPinEventOn(int pinIndex, int eventType)
{
  int componentID;  // ID of the MicroBit Component that generated the event.
  switch (pinIndex) // Index of pin to set event.
  {
  case 0:
    componentID = MICROBIT_ID_IO_P0;
    break;
  case 1:
    componentID = MICROBIT_ID_IO_P1;
    break;
  case 2:
    componentID = MICROBIT_ID_IO_P2;
    break;
  case 8:
    componentID = MICROBIT_ID_IO_P8;
    break;
  case 13:
    componentID = MICROBIT_ID_IO_P13;
    break;
  case 14:
    componentID = MICROBIT_ID_IO_P14;
    break;
  case 15:
    componentID = MICROBIT_ID_IO_P15;
    break;
  case 16:
    componentID = MICROBIT_ID_IO_P16;
    break;

  default:
    return;
  }
  if (eventType == MbitMorePinEventType::NONE)
  {
    uBit.messageBus.ignore(componentID, MICROBIT_EVT_ANY, this, &MbitMoreServiceDAL::onPinEvent);
    uBit.io.pin[pinIndex].eventOn(MICROBIT_PIN_EVENT_NONE);
  }
  else
  {
    uBit.messageBus.listen(componentID, MICROBIT_EVT_ANY, this, &MbitMoreServiceDAL::onPinEvent, MESSAGE_BUS_LISTENER_DROP_IF_BUSY);
    if (eventType == MbitMorePinEventType::ON_EDGE)
    {
      uBit.io.pin[pinIndex].eventOn(MICROBIT_PIN_EVENT_ON_EDGE);
    }
    else if (eventType == MbitMorePinEventType::ON_PULSE)
    {
      uBit.io.pin[pinIndex].eventOn(MICROBIT_PIN_EVENT_ON_PULSE);
    }
    else if (eventType == MbitMorePinEventType::ON_TOUCH)
    {
      uBit.io.pin[pinIndex].eventOn(MICROBIT_PIN_EVENT_ON_TOUCH);
    }
  }
}

/**
 * Callback. Invoked when a pin event sent.
 */
void MbitMoreServiceDAL::onPinEvent(MicroBitEvent evt)
{
  uint8_t pinIndex;
  switch (evt.source) // ID of the MicroBit Component that generated the event. (uint16_t)
  {
  case MICROBIT_ID_IO_P0:
    eventBuffer[0] = 0;
    break;
  case MICROBIT_ID_IO_P1:
    eventBuffer[0] = 1;
    break;
  case MICROBIT_ID_IO_P2:
    eventBuffer[0] = 2;
    break;
  case MICROBIT_ID_IO_P8:
    eventBuffer[0] = 8;
    break;
  case MICROBIT_ID_IO_P13:
    eventBuffer[0] = 13;
    break;
  case MICROBIT_ID_IO_P14:
    eventBuffer[0] = 14;
    break;
  case MICROBIT_ID_IO_P15:
    eventBuffer[0] = 15;
    break;
  case MICROBIT_ID_IO_P16:
    eventBuffer[0] = 16;
    break;

  default:
    break;
  }

  // event ID is sent as uint16_t little-endian.
  // #define MICROBIT_PIN_EVT_RISE               2
  // #define MICROBIT_PIN_EVT_FALL               3
  // #define MICROBIT_PIN_EVT_PULSE_HI           4
  // #define MICROBIT_PIN_EVT_PULSE_LO           5
  memcpy(&(eventBuffer[1]), &(evt.value), 2);

  // event timestamp is sent as uint32_t little-endian coerced from uint64_t value.
  uint32_t timestamp = (uint32_t)evt.timestamp;
  memcpy(&(eventBuffer[3]), &timestamp, 4);

  uBit.ble->gattServer().notify(
      eventCharHandle,
      (uint8_t *)&eventBuffer,
      sizeof(eventBuffer) / sizeof(eventBuffer[0]));
}

/**
  * Button update callback
  */
void MbitMoreServiceDAL::onButtonChanged(MicroBitEvent e)
{
  int state;
  if (e.value == MICROBIT_BUTTON_EVT_UP)
  {
    state = 0;
  }
  if (e.value == MICROBIT_BUTTON_EVT_DOWN)
  {
    state = 1;
  }
  if (e.value == MICROBIT_BUTTON_EVT_HOLD)
  {
    state = 5;
  }
  // if (e.value == MICROBIT_BUTTON_EVT_CLICK)
  // {
  //   state = 3;
  // }
  // if (e.value == MICROBIT_BUTTON_EVT_LONG_CLICK)
  // {
  //   state = 4;
  // }
  // if (e.value == MICROBIT_BUTTON_EVT_DOUBLE_CLICK)
  // {
  //   state = 6;
  // }
  if (e.source == MICROBIT_ID_BUTTON_A)
  {
    buttonAState = state;
  }
  if (e.source == MICROBIT_ID_BUTTON_B)
  {
    buttonBState = state;
  }
}

void MbitMoreServiceDAL::onGestureChanged(MicroBitEvent e)
{
  if (e.value == MICROBIT_ACCELEROMETER_EVT_SHAKE)
  {
    gesture = gesture | 1;
  }
  if (e.value == MICROBIT_ACCELEROMETER_EVT_FREEFALL)
  {
    gesture = gesture | 1 << 1;
  }
}

/**
 * Normalize angle in upside down.
 */
int MbitMoreServiceDAL::normalizeCompassHeading(int heading)
{
  if (uBit.accelerometer.getZ() > 0)
  {
    if (heading <= 180)
    {
      heading = 180 - heading;
    }
    else
    {
      heading = 360 - (heading - 180);
    }
  }
  return heading;
}

/**
 * Convert roll/pitch radians to Scratch extension value (-1000 to 1000).
 */
int MbitMoreServiceDAL::convertToTilt(float radians)
{
  float degrees = (360.0f * radians) / (2.0f * PI);
  float tilt = degrees * 1.0f / 90.0f;
  if (degrees > 0)
  {
    if (tilt > 1.0f)
      tilt = 2.0f - tilt;
  }
  else
  {
    if (tilt < -1.0f)
      tilt = -2.0f - tilt;
  }
  return (int)(tilt * 1000.0f);
}

void MbitMoreServiceDAL::updateGesture()
{
  int old[] = {lastAcc[0], lastAcc[1], lastAcc[2]};
  lastAcc[0] = uBit.accelerometer.getX();
  lastAcc[1] = uBit.accelerometer.getY();
  lastAcc[2] = uBit.accelerometer.getZ();
  if ((gesture >> 2) & 1)
  {
    gesture = gesture ^ (1 << 2);
    return;
  }
  int threshold = 50;
  if ((abs(lastAcc[0] - old[0]) > threshold) || (abs(lastAcc[1] - old[1]) > threshold) || (abs(lastAcc[2] - old[2]) > threshold))
  {
    // Moved
    gesture = gesture | (1 << 2);
  }
}

void MbitMoreServiceDAL::resetGesture()
{
  gesture = gesture & (1 << 2); // Save moved state to detect continuous movement.
}

void MbitMoreServiceDAL::updateDigitalValues()
{
  digitalValues = 0;
  for (size_t i = 0; i < sizeof(gpio) / sizeof(gpio[0]); i++)
  {
    if (uBit.io.pin[gpio[i]].isDigital())
    {
      if (uBit.io.pin[gpio[i]].isInput())
      {
        digitalValues =
            digitalValues | (uBit.io.pin[gpio[i]].getDigitalValue() << gpio[i]);
      }
    }
  }
}

void MbitMoreServiceDAL::updatePowerVoltage()
{
  powerVoltage = getPowerVoltage();
}

void MbitMoreServiceDAL::updateAnalogValues()
{
  for (size_t i = 0; i < sizeof(analogIn) / sizeof(analogIn[0]); i++)
  {
    updatePowerVoltage();
    int samplingCount;
    int prevValue;
    int value;
    if (uBit.io.pin[analogIn[i]].isInput())
    {
      uBit.io.pin[analogIn[i]].setPull(PinMode::PullNone);
      // for accuracy, read more than 2 times to get same values continuously
      do
      {
        prevValue = value;
        value = (uint16_t)uBit.io.pin[analogIn[i]].getAnalogValue();
        samplingCount++;
      } while (prevValue != value || samplingCount < 4);
      analogValues[i] = value;
      setPullMode(analogIn[i], pullMode[analogIn[i]]);
    }
  }

  //// It will cause flickering LED.
  // uBit.display.disable();
  // analogValues[3] = (uint16_t)uBit.io.P3.getAnalogValue();
  // analogValues[4] = (uint16_t)uBit.io.P4.getAnalogValue();
  // analogValues[5] = (uint16_t)uBit.io.P10.getAnalogValue();
  // uBit.display.enable();
}

void MbitMoreServiceDAL::updateLightSensor()
{
  if (lightSensingDuration <= 0)
  {
    uBit.display.setDisplayMode(DisplayMode::DISPLAY_MODE_BLACK_AND_WHITE);
    return;
  }
  lightLevel = uBit.display.readLightLevel();
  if (lightSensingDuration < 255) // over 255 means no-limited.
  {
    lightSensingDuration--;
  }
}

void MbitMoreServiceDAL::updateAccelerometer()
{
  acceleration[0] = -uBit.accelerometer.getX(); // Face side is positive in Z-axis.
  acceleration[1] = uBit.accelerometer.getY();
  acceleration[2] = -uBit.accelerometer.getZ(); // Face side is positive in Z-axis.
  rotation[0] = uBit.accelerometer.getPitchRadians();
  rotation[1] = uBit.accelerometer.getRollRadians();
}

void MbitMoreServiceDAL::updateMagnetometer()
{
  compassHeading = uBit.compass.heading();
  magneticForce[0] = uBit.compass.getX();
  magneticForce[1] = uBit.compass.getY();
  magneticForce[2] = uBit.compass.getZ();
}

void MbitMoreServiceDAL::setPullMode(int pinIndex, PinMode pull)
{
  uBit.io.pin[pinIndex].getDigitalValue(pull);
  pullMode[pinIndex] = pull;
}

void MbitMoreServiceDAL::setDigitalValue(int pinIndex, int value)
{
  uBit.io.pin[pinIndex].setDigitalValue(value);
}

void MbitMoreServiceDAL::setAnalogValue(int pinIndex, int value)
{
  uBit.io.pin[pinIndex].setAnalogValue(value);
}

void MbitMoreServiceDAL::setServoValue(int pinIndex, int angle, int range, int center)
{
  uBit.io.pin[pinIndex].setServoValue(angle, range, center);
}

void MbitMoreServiceDAL::setPinModeTouch(int pinIndex)
{
  uBit.io.pin[pinIndex].isTouched(); // Configure to touch mode then the return value is not used.
}

void MbitMoreServiceDAL::setLightSensingDuration(int duration)
{
  lightSensingDuration = duration;
}

void MbitMoreServiceDAL::composeDefaultData(uint8_t *buff)
{
  // Tilt value is sent as int16_t big-endian.
  int16_t tiltX = (int16_t)convertToTilt(rotation[1]);
  buff[0] = (tiltX >> 8) & 0xFF;
  buff[1] = tiltX & 0xFF;
  int16_t tiltY = (int16_t)convertToTilt(rotation[0]);
  buff[2] = (tiltY >> 8) & 0xFF;
  buff[3] = tiltY & 0xFF;
  buff[4] = (uint8_t)buttonAState;
  buff[5] = (uint8_t)buttonBState;
  buff[6] = (uint8_t)(((digitalValues >> 0) & 1) ^ 1);
  buff[7] = (uint8_t)(((digitalValues >> 1) & 1) ^ 1);
  buff[8] = (uint8_t)(((digitalValues >> 2) & 1) ^ 1);
  buff[9] = (uint8_t)gesture;
}

/**
  * Notify shared data to Scratch3
  */
void MbitMoreServiceDAL::notifySharedData()
{
  for (size_t i = 0; i < sizeof(sharedData) / sizeof(sharedData[0]); i++)
  {
    memcpy(&(sharedBuffer[(i * 2)]), &sharedData[i], 2);
  }
  uBit.ble->gattServer().notify(
      sharedDataCharHandle,
      (uint8_t *)&sharedBuffer,
      sizeof(sharedBuffer) / sizeof(sharedBuffer[0]));
}

/**
 * Notify default micro:bit data to Scratch.
 */
void MbitMoreServiceDAL::notifyDefaultData()
{
  composeDefaultData(txData);
  uBit.ble->gattServer().notify(
      txCharacteristicHandle,
      (uint8_t *)&txData,
      sizeof(txData) / sizeof(txData[0]));
}

/**
  * Notify data to Scratch3
  */
void MbitMoreServiceDAL::notify()
{
  if (uBit.ble->gap().getState().connected)
  {
    updateGesture();
    notifyDefaultData();
    resetGesture();
  }
  else
  {
    displayFriendlyName();
  }
}

/**
 * Set value to shared data.
 * shared data (0, 1, 2, 3)
 */
void MbitMoreServiceDAL::setSharedData(int index, int value)
{
  // value (-32768 to 32767) is sent as int16_t little-endian.
  int16_t data = (int16_t)value;
  sharedData[index] = data;
  notifySharedData();
}

/**
 * Get value of a shared data.
 * shared data (0, 1, 2, 3)
 */
int MbitMoreServiceDAL::getSharedData(int index)
{
  return (int)(sharedData[index]);
}

void MbitMoreServiceDAL::onBLEConnected(MicroBitEvent _e)
{
  uBit.display.stopAnimation(); // To stop display friendly name.
  initConfiguration();
  uBit.display.scrollAsync("v.0.5.0");
  lightSensingDuration = 255; // Continuous light sensing for GUI v0.4.2
}

void MbitMoreServiceDAL::onBLEDisconnected(MicroBitEvent _e)
{
  lightSensingDuration = 0;
}

/**
 * Update sensors.
 */
void MbitMoreServiceDAL::update()
{
  updateDigitalValues();
  writeIo();
  updateLightSensor();
  updateAccelerometer();
  updateMagnetometer();
  writeSensors();
}

/**
  * Write IO characteristics.
  */
void MbitMoreServiceDAL::writeIo()
{
  memcpy(&(ioBuffer[0]), &digitalValues, 4);

  uBit.ble->gattServer().write(
      ioCharHandle,
      (uint8_t *)&ioBuffer,
      sizeof(ioBuffer) / sizeof(ioBuffer[0]));
}

/**
  * Write shared data characteristics.
  */
void MbitMoreServiceDAL::writeSharedData()
{
  for (size_t i = 0; i < sizeof(sharedData) / sizeof(sharedData[0]); i++)
  {
    memcpy(&(sharedBuffer[(i * 2)]), &sharedData[i], 2);
  }

  uBit.ble->gattServer().write(
      sharedDataCharHandle,
      (uint8_t *)&sharedBuffer,
      sizeof(sharedBuffer) / sizeof(sharedBuffer[0]));
}

/**
  * Write data of all sensors to the characteristic.
  */
void MbitMoreServiceDAL::writeSensors()
{
  // Accelerometer
  int16_t acc;
  // Acceleration X [milli-g] is sent as int16_t little-endian.
  acc = (int16_t)acceleration[0];
  memcpy(&(sensorsBuffer[0]), &acc, 2);
  // Acceleration Y [milli-g] is sent as int16_t little-endian.
  acc = (int16_t)acceleration[1];
  memcpy(&(sensorsBuffer[2]), &acc, 2);
  // Acceleration Z [milli-g] is sent as int16_t little-endian.
  acc = (int16_t)acceleration[2];
  memcpy(&(sensorsBuffer[4]), &acc, 2);

  int16_t rot;
  // Pitch (radians / 1000) is sent as int16_t little-endian [6..7].
  rot = (int16_t)(rotation[0] * 1000);
  memcpy(&(sensorsBuffer[6]), &rot, 2);
  // Roll (radians / 1000) is sent as int16_t little-endian [8..9].
  rot = (int16_t)(rotation[1] * 1000);
  memcpy(&(sensorsBuffer[8]), &rot, 2);

  // Magnetometer
  uint16_t heading = (uint16_t)normalizeCompassHeading(compassHeading);
  memcpy(&(sensorsBuffer[10]), &heading, 2);

  int16_t force;
  // Magnetic force X (micro-teslas) is sent as uint16_t little-endian [2..3].
  force = (int16_t)(magneticForce[0] / 1000);
  memcpy(&(sensorsBuffer[12]), &force, 2);
  // Magnetic force Y (micro-teslas) is sent as uint16_t little-endian [4..5].
  force = (int16_t)(magneticForce[1] / 1000);
  memcpy(&(sensorsBuffer[14]), &force, 2);
  // Magnetic force Z (micro-teslas) is sent as uint16_t little-endian [6..7].
  force = (int16_t)(magneticForce[2] / 1000);
  memcpy(&(sensorsBuffer[16]), &force, 2);

  // Light sensor
  sensorsBuffer[18] = (uint8_t)lightLevel;

  // Temperature
  sensorsBuffer[19] = (uint8_t)(uBit.thermometer.getTemperature() + 128);

  uBit.ble->gattServer().write(
      sensorsCharHandle,
      (uint8_t *)&sensorsBuffer,
      sizeof(sensorsBuffer) / sizeof(sensorsBuffer[0]));
}

void MbitMoreServiceDAL::displayFriendlyName()
{
  ManagedString suffix(" MORE! ");
  uBit.display.scrollAsync(uBit.getName() + suffix, 120);
}

const uint16_t MBIT_MORE_BASIC_SERVICE = 0xf005;
const uint8_t MBIT_MORE_BASIC_TX[] = {0x52, 0x61, 0xda, 0x01, 0xfa, 0x7e, 0x42, 0xab, 0x85, 0x0b, 0x7c, 0x80, 0x22, 0x00, 0x97, 0xcc};
const uint8_t MBIT_MORE_BASIC_RX[] = {0x52, 0x61, 0xda, 0x02, 0xfa, 0x7e, 0x42, 0xab, 0x85, 0x0b, 0x7c, 0x80, 0x22, 0x00, 0x97, 0xcc};

const uint8_t MBIT_MORE_SERVICE[] = {0xa6, 0x2d, 0x57, 0x4e, 0x1b, 0x34, 0x40, 0x92, 0x8d, 0xee, 0x41, 0x51, 0xf6, 0x3b, 0x28, 0x65};
const uint8_t MBIT_MORE_EVENT[] = {0xa6, 0x2d, 0x00, 0x01, 0x1b, 0x34, 0x40, 0x92, 0x8d, 0xee, 0x41, 0x51, 0xf6, 0x3b, 0x28, 0x65};
const uint8_t MBIT_MORE_IO[] = {0xa6, 0x2d, 0x00, 0x02, 0x1b, 0x34, 0x40, 0x92, 0x8d, 0xee, 0x41, 0x51, 0xf6, 0x3b, 0x28, 0x65};
const uint8_t MBIT_MORE_ANALOG_IN[] = {0xa6, 0x2d, 0x00, 0x03, 0x1b, 0x34, 0x40, 0x92, 0x8d, 0xee, 0x41, 0x51, 0xf6, 0x3b, 0x28, 0x65};
const uint8_t MBIT_MORE_SENSORS[] = {0xa6, 0x2d, 0x00, 0x04, 0x1b, 0x34, 0x40, 0x92, 0x8d, 0xee, 0x41, 0x51, 0xf6, 0x3b, 0x28, 0x65};
const uint8_t MBIT_MORE_SHARED_DATA[] = {0xa6, 0x2d, 0x00, 0x10, 0x1b, 0x34, 0x40, 0x92, 0x8d, 0xee, 0x41, 0x51, 0xf6, 0x3b, 0x28, 0x65};

#endif // !MICROBIT_CODAL
