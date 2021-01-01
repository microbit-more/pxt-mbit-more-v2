#include "pxt.h"

#if !MICROBIT_CODAL

#include "MicroBitConfig.h"

#include "MicroBitButton.h"

#include "MbitMoreServiceDAL.h"

/**
 * Position of data format in a value holder.
 */
#define DATA_FORMAT_INDEX 19
#define MBIT_MORE_DATA_FORMAT_BUTTON_EVENT 0x11

/**
 * @brief Service ID of Microbit More.
 *
 */
const uint8_t MBIT_MORE_SERVICE[] = {0xa6, 0x2d, 0x57, 0x4e, 0x1b, 0x34,
                                     0x40, 0x92, 0x8d, 0xee, 0x41, 0x51,
                                     0xf6, 0x3b, 0x28, 0x65};

/**
 * @brief Characteristics in Microbit More Service.
 *
 */
const uint8_t MBIT_MORE_CH_COMMAND[] = {0xa6, 0x2d, 0x01, 0x00, 0x1b, 0x34,
                                        0x40, 0x92, 0x8d, 0xee, 0x41, 0x51,
                                        0xf6, 0x3b, 0x28, 0x65};
const uint8_t MBIT_MORE_CH_DIGITAL_IN[] = {0xa6, 0x2d, 0x01, 0x01, 0x1b, 0x34,
                                           0x40, 0x92, 0x8d, 0xee, 0x41, 0x51,
                                           0xf6, 0x3b, 0x28, 0x65};
const uint8_t MBIT_MORE_CH_LIGHT_LEVEL[] = {0xa6, 0x2d, 0x01, 0x02, 0x1b, 0x34,
                                            0x40, 0x92, 0x8d, 0xee, 0x41, 0x51,
                                            0xf6, 0x3b, 0x28, 0x65};
const uint8_t MBIT_MORE_CH_ACCELERATION[] = {0xa6, 0x2d, 0x01, 0x03, 0x1b, 0x34,
                                             0x40, 0x92, 0x8d, 0xee, 0x41, 0x51,
                                             0xf6, 0x3b, 0x28, 0x65};
const uint8_t MBIT_MORE_CH_MAGNET[] = {0xa6, 0x2d, 0x01, 0x04, 0x1b, 0x34,
                                       0x40, 0x92, 0x8d, 0xee, 0x41, 0x51,
                                       0xf6, 0x3b, 0x28, 0x65};
const uint8_t MBIT_MORE_CH_TEMPERATURE[] = {0xa6, 0x2d, 0x01, 0x05, 0x1b, 0x34,
                                            0x40, 0x92, 0x8d, 0xee, 0x41, 0x51,
                                            0xf6, 0x3b, 0x28, 0x65};
const uint8_t MBIT_MORE_CH_MICROPHONE[] = {0xa6, 0x2d, 0x01, 0x06, 0x1b, 0x34,
                                           0x40, 0x92, 0x8d, 0xee, 0x41, 0x51,
                                           0xf6, 0x3b, 0x28, 0x65};
const uint8_t MBIT_MORE_CH_PIN_EVENT[] = {0xa6, 0x2d, 0x01, 0x10, 0x1b, 0x34,
                                          0x40, 0x92, 0x8d, 0xee, 0x41, 0x51,
                                          0xf6, 0x3b, 0x28, 0x65};
const uint8_t MBIT_MORE_CH_BUTTON_EVENT[] = {0xa6, 0x2d, 0x01, 0x11, 0x1b, 0x34,
                                             0x40, 0x92, 0x8d, 0xee, 0x41, 0x51,
                                             0xf6, 0x3b, 0x28, 0x65};
const uint8_t MBIT_MORE_CH_GESTURE_EVENT[] = {
    0xa6, 0x2d, 0x01, 0x12, 0x1b, 0x34, 0x40, 0x92,
    0x8d, 0xee, 0x41, 0x51, 0xf6, 0x3b, 0x28, 0x65};
const uint8_t MBIT_MORE_CH_ANALOG_IN_P0[] = {0xa6, 0x2d, 0x01, 0x20, 0x1b, 0x34,
                                             0x40, 0x92, 0x8d, 0xee, 0x41, 0x51,
                                             0xf6, 0x3b, 0x28, 0x65};
const uint8_t MBIT_MORE_CH_ANALOG_IN_P1[] = {0xa6, 0x2d, 0x01, 0x21, 0x1b, 0x34,
                                             0x40, 0x92, 0x8d, 0xee, 0x41, 0x51,
                                             0xf6, 0x3b, 0x28, 0x65};
const uint8_t MBIT_MORE_CH_ANALOG_IN_P2[] = {0xa6, 0x2d, 0x01, 0x22, 0x1b, 0x34,
                                             0x40, 0x92, 0x8d, 0xee, 0x41, 0x51,
                                             0xf6, 0x3b, 0x28, 0x65};
const uint8_t MBIT_MORE_CH_SHARED_DATA[] = {0xa6, 0x2d, 0x01, 0x30, 0x1b, 0x34,
                                            0x40, 0x92, 0x8d, 0xee, 0x41, 0x51,
                                            0xf6, 0x3b, 0x28, 0x65};

const uint8_t MBIT_MORE_ANALOG_IN[] = {0xa6, 0x2d, 0x00, 0x03, 0x1b, 0x34,
                                       0x40, 0x92, 0x8d, 0xee, 0x41, 0x51,
                                       0xf6, 0x3b, 0x28, 0x65};
const uint8_t MBIT_MORE_SENSORS[] = {0xa6, 0x2d, 0x00, 0x04, 0x1b, 0x34,
                                     0x40, 0x92, 0x8d, 0xee, 0x41, 0x51,
                                     0xf6, 0x3b, 0x28, 0x65};
const uint8_t MBIT_MORE_SHARED_DATA[] = {0xa6, 0x2d, 0x00, 0x10, 0x1b, 0x34,
                                         0x40, 0x92, 0x8d, 0xee, 0x41, 0x51,
                                         0xf6, 0x3b, 0x28, 0x65};

/**
 * Class definition for the Scratch MicroBit More Service.
 * Provides a BLE service to remotely controll Micro:bit from Scratch3.
 */

/**
 * Constructor.
 * Create a representation of the Microbit More BLE Service
 * @param _uBit The instance of a MicroBit runtime.
 */
MbitMoreServiceDAL::MbitMoreServiceDAL() : uBit(pxt::uBit) {
  mbitMore = &MbitMoreDevice::getInstance();
  mbitMore->moreService = this;

  commandCh = new GattCharacteristic(
      MBIT_MORE_CH_COMMAND, commandBuffer, 0, MM_CH_BUFFER_SIZE_DEFAULT,
      GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_WRITE |
          GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_WRITE_WITHOUT_RESPONSE);
  commandCh->requireSecurity(SecurityManager::MICROBIT_BLE_SECURITY_LEVEL);

  digitalInCh = new GattCharacteristic(
      MBIT_MORE_CH_DIGITAL_IN, (uint8_t *)&digitalInBuffer, 0,
      MM_CH_BUFFER_SIZE_DEFAULT,
      GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_READ);
  digitalInCh->requireSecurity(SecurityManager::MICROBIT_BLE_SECURITY_LEVEL);

  buttonEventCh = new GattCharacteristic(
      MBIT_MORE_CH_BUTTON_EVENT, (uint8_t *)&buttonEventBuffer,
      MM_CH_BUFFER_SIZE_DEFAULT, MM_CH_BUFFER_SIZE_DEFAULT,
      GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_READ |
          GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY);
  buttonEventCh->requireSecurity(SecurityManager::MICROBIT_BLE_SECURITY_LEVEL);

  lightLevelCh = new GattCharacteristic(
      MBIT_MORE_CH_LIGHT_LEVEL, (uint8_t *)&lightLevelBuffer, 0,
      MM_CH_BUFFER_SIZE_DEFAULT,
      GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_READ);
  lightLevelCh->requireSecurity(SecurityManager::MICROBIT_BLE_SECURITY_LEVEL);

  // analogInChar =
  //     new GattCharacteristic(MBIT_MORE_ANALOG_IN, (uint8_t *)&analogInBuffer,
  //     0,
  //                            sizeof(analogInBuffer),
  //                            GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_READ);
  // analogInChar->setReadAuthorizationCallback(
  //     this, &MbitMoreServiceDAL::onReadAnalogIn);
  // analogInChar->requireSecurity(SecurityManager::MICROBIT_BLE_SECURITY_LEVEL);

  // sharedDataChar = new GattCharacteristic(
  //     MBIT_MORE_SHARED_DATA, (uint8_t *)&sharedBuffer, 0,
  //     sizeof(sharedBuffer), GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_READ
  //     |
  //         GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY |
  //         GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_WRITE |
  //         GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_WRITE_WITHOUT_RESPONSE);
  // sharedDataChar->requireSecurity(SecurityManager::MICROBIT_BLE_SECURITY_LEVEL);

  GattCharacteristic *mbitMoreChars[] = {commandCh, digitalInCh, buttonEventCh,
                                         lightLevelCh};
  GattService mbitMoreService(MBIT_MORE_SERVICE, mbitMoreChars,
                              sizeof(mbitMoreChars) /
                                  sizeof(GattCharacteristic *));
  uBit.ble->addService(mbitMoreService);

  // Setup callbacks for events.
  uBit.ble->onDataWritten(this, &MbitMoreServiceDAL::onDataWritten);

  uBit.messageBus.listen(MICROBIT_ID_BLE, MICROBIT_BLE_EVT_CONNECTED, this,
                         &MbitMoreServiceDAL::onBLEConnected,
                         MESSAGE_BUS_LISTENER_IMMEDIATE);
  uBit.messageBus.listen(MICROBIT_ID_BLE, MICROBIT_BLE_EVT_DISCONNECTED, this,
                         &MbitMoreServiceDAL::onBLEDisconnected,
                         MESSAGE_BUS_LISTENER_IMMEDIATE);
  // uBit.messageBus.listen(MICROBIT_ID_GESTURE, MICROBIT_EVT_ANY, this,
  //                        &MbitMoreServiceDAL::onGestureChanged,
  //                        MESSAGE_BUS_LISTENER_IMMEDIATE);

  fiber_add_idle_component(this);
}

/**
 * Callback. Invoked when AnalogIn is read via BLE.
 */
void MbitMoreServiceDAL::onReadAnalogIn(
    GattReadAuthCallbackParams *authParams) {
  // updateAnalogValues();
  // // analog value (0 to 1023) is sent as uint16_t little-endian.
  // memcpy(&(analogInBuffer[0]), &(analogValues[0]), 2);
  // memcpy(&(analogInBuffer[2]), &(analogValues[1]), 2);
  // memcpy(&(analogInBuffer[4]), &(analogValues[2]), 2);
  // // voltage of power supply [mV] is sent as uint16_t little-endian.
  // memcpy(&(analogInBuffer[6]), &powerVoltage, 2);
  // authParams->data = (uint8_t *)&analogInBuffer;
  // authParams->offset = 0;
  // authParams->len = sizeof(analogInBuffer) / sizeof(analogInBuffer[0]);
  // authParams->authorizationReply = AUTH_CALLBACK_REPLY_SUCCESS;
}

/**
 * Callback. Invoked when any of our attributes are written via BLE.
 */
void MbitMoreServiceDAL::onDataWritten(const GattWriteCallbackParams *params) {
  uint8_t *data = (uint8_t *)params->data;

  if (params->handle == commandCh->getValueHandle() && params->len > 0) {
    if (data[0] == ScratchBLECommand::CMD_DISPLAY_TEXT) {
      char text[params->len - 2] = {0};
      memcpy(text, &(data[2]), (params->len) - 2);
      mbitMore->displayText(text, data[1]);
    } else if (data[0] == ScratchBLECommand::CMD_DISPLAY_LED) {
      mbitMore->displayPattern(&data[1]);
    }
  }
}

/**
 * @brief Notify button event.
 *
 * @param data Data to notify.
 * @param length Lenght of the data.
 */
void MbitMoreServiceDAL::notifyButtonEvent(uint8_t *data, uint16_t length) {
  memcpy(buttonEventBuffer, data, length);
  buttonEventBuffer[DATA_FORMAT_INDEX] = MbitMoreDataFormat::BUTTON_EVENT;
  uBit.ble->gattServer().notify(
      buttonEventCh->getValueHandle(), buttonEventBuffer,
      (sizeof(buttonEventBuffer) / sizeof(buttonEventBuffer[0])));
}

/**
 * Notify shared data to Scratch3
 */
void MbitMoreServiceDAL::notifySharedData() {
  // for (size_t i = 0; i < sizeof(sharedData) / sizeof(sharedData[0]); i++) {
  //   memcpy(&(sharedBuffer[(i * 2)]), &sharedData[i], 2);
  // }
  // sharedBuffer[DATA_FORMAT_INDEX] = MbitMoreDataFormat::SHARED_DATA;
  // uBit.ble->gattServer().notify(sharedDataCh->getValueHandle(),
  //                               (uint8_t *)&sharedBuffer,
  //                               sizeof(sharedBuffer) / sizeof(sharedBuffer[0]));
}

/**
 * Notify data to Scratch3
 */
void MbitMoreServiceDAL::notify() {}

/**
 * Set value to shared data.
 * shared data (0, 1, 2, 3)
 */
void MbitMoreServiceDAL::setSharedData(int index, int value) {
  // value (-32768 to 32767) is sent as int16_t little-endian.
  // int16_t data = (int16_t)value;
  // sharedData[index] = data;
  // notifySharedData();
}

/**
 * Get value of a shared data.
 * shared data (0, 1, 2, 3)
 */
int MbitMoreServiceDAL::getSharedData(int index) {
  return (int)(mbitMore->sharedData[index]);
}

void MbitMoreServiceDAL::onBLEConnected(MicroBitEvent _e) {
  mbitMore->initConfiguration();
}

void MbitMoreServiceDAL::onBLEDisconnected(MicroBitEvent _e) {}

/**
 * Periodic callback from MicroBit idle thread.
 */
void MbitMoreServiceDAL::idleCallback() {
  // if (!(uBit.ble->gap().getState().connected)) {
  //   mbitMore->displayFriendlyName();
  // }
}

/**
 * Update sensors.
 */
void MbitMoreServiceDAL::update() {
  if (uBit.ble->gap().getState().connected) {
    mbitMore->updateDigitalIn(digitalInBuffer);
    uBit.ble->gattServer().write(digitalInCh->getValueHandle(), digitalInBuffer,
                                 MM_CH_BUFFER_SIZE_DIGITAL_IN);
    mbitMore->updateLightLevel(lightLevelBuffer);
    uBit.ble->gattServer().write(lightLevelCh->getValueHandle(),
                                 lightLevelBuffer, 1);
  } else {
    mbitMore->displayFriendlyName();
  }
}

#endif // !MICROBIT_CODAL
