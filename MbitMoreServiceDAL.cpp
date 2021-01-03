#include "pxt.h"

#if !MICROBIT_CODAL

#include "MicroBitConfig.h"

#include "MicroBitButton.h"

#include "MbitMoreServiceDAL.h"

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
const uint8_t MBIT_MORE_CH_SENSORS[] = {0xa6, 0x2d, 0x01, 0x01, 0x1b, 0x34,
                                        0x40, 0x92, 0x8d, 0xee, 0x41, 0x51,
                                        0xf6, 0x3b, 0x28, 0x65};
const uint8_t MBIT_MORE_CH_DIRECTION[] = {0xa6, 0x2d, 0x01, 0x02, 0x1b, 0x34,
                                          0x40, 0x92, 0x8d, 0xee, 0x41, 0x51,
                                          0xf6, 0x3b, 0x28, 0x65};
const uint8_t MBIT_MORE_CH_PIN_EVENT[] = {0xa6, 0x2d, 0x01, 0x10, 0x1b, 0x34,
                                          0x40, 0x92, 0x8d, 0xee, 0x41, 0x51,
                                          0xf6, 0x3b, 0x28, 0x65};
const uint8_t MBIT_MORE_CH_ACTION_EVENT[] = {0xa6, 0x2d, 0x01, 0x11, 0x1b, 0x34,
                                             0x40, 0x92, 0x8d, 0xee, 0x41, 0x51,
                                             0xf6, 0x3b, 0x28, 0x65};
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

/**
 * Class definition for the Scratch MicroBit More Service.
 * Provides a BLE service to remotely controll Micro:bit from Scratch3.
 */

/**
 * Constructor.
 * Create a representation of the Microbit More BLE Service
 */
MbitMoreServiceDAL::MbitMoreServiceDAL() : uBit(pxt::uBit) {
  mbitMore = &MbitMoreDevice::getInstance();
  mbitMore->moreService = this;

  commandCh = new GattCharacteristic(
      MBIT_MORE_CH_COMMAND, commandChBuffer, 0, MM_CH_BUFFER_SIZE_MAX,
      GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_WRITE |
          GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_WRITE_WITHOUT_RESPONSE);
  commandCh->requireSecurity(SecurityManager::MICROBIT_BLE_SECURITY_LEVEL);

  sensorsCh = new GattCharacteristic(
      MBIT_MORE_CH_SENSORS, (uint8_t *)&sensorsChBuffer,
      MM_CH_BUFFER_SIZE_SENSORS, MM_CH_BUFFER_SIZE_SENSORS,
      GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_READ);
  sensorsCh->requireSecurity(SecurityManager::MICROBIT_BLE_SECURITY_LEVEL);

  directionCh = new GattCharacteristic(
      MBIT_MORE_CH_DIRECTION, (uint8_t *)&directionChBuffer,
      MM_CH_BUFFER_SIZE_DIRECTION, MM_CH_BUFFER_SIZE_DIRECTION,
      GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_READ);
  directionCh->requireSecurity(SecurityManager::MICROBIT_BLE_SECURITY_LEVEL);

  pinEventCh = new GattCharacteristic(
      MBIT_MORE_CH_PIN_EVENT, (uint8_t *)&pinEventChBuffer,
      MM_CH_BUFFER_SIZE_NOTIFY, MM_CH_BUFFER_SIZE_NOTIFY,
      GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_READ |
          GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY);
  pinEventCh->requireSecurity(SecurityManager::MICROBIT_BLE_SECURITY_LEVEL);

  actionEventCh = new GattCharacteristic(
      MBIT_MORE_CH_ACTION_EVENT, (uint8_t *)&actionEventChBuffer,
      MM_CH_BUFFER_SIZE_NOTIFY, MM_CH_BUFFER_SIZE_NOTIFY,
      GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_READ |
          GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY);
  actionEventCh->requireSecurity(SecurityManager::MICROBIT_BLE_SECURITY_LEVEL);

  analogInP0Ch = new GattCharacteristic(
      MBIT_MORE_CH_ANALOG_IN_P0, (uint8_t *)&analogInP0ChBuffer,
      MM_CH_BUFFER_SIZE_ANALOG_IN, MM_CH_BUFFER_SIZE_ANALOG_IN,
      GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_READ);
  analogInP0Ch->setReadAuthorizationCallback(
      this, &MbitMoreServiceDAL::onReadAnalogIn);
  analogInP0Ch->requireSecurity(SecurityManager::MICROBIT_BLE_SECURITY_LEVEL);

  analogInP1Ch = new GattCharacteristic(
      MBIT_MORE_CH_ANALOG_IN_P1, (uint8_t *)&analogInP1ChBuffer,
      MM_CH_BUFFER_SIZE_ANALOG_IN, MM_CH_BUFFER_SIZE_ANALOG_IN,
      GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_READ);
  analogInP1Ch->setReadAuthorizationCallback(
      this, &MbitMoreServiceDAL::onReadAnalogIn);
  analogInP1Ch->requireSecurity(SecurityManager::MICROBIT_BLE_SECURITY_LEVEL);

  analogInP2Ch = new GattCharacteristic(
      MBIT_MORE_CH_ANALOG_IN_P2, (uint8_t *)&analogInP2ChBuffer,
      MM_CH_BUFFER_SIZE_ANALOG_IN, MM_CH_BUFFER_SIZE_ANALOG_IN,
      GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_READ);
  analogInP2Ch->setReadAuthorizationCallback(
      this, &MbitMoreServiceDAL::onReadAnalogIn);
  analogInP2Ch->requireSecurity(SecurityManager::MICROBIT_BLE_SECURITY_LEVEL);

  sharedDataCh = new GattCharacteristic(
      MBIT_MORE_CH_SHARED_DATA, (uint8_t *)&sharedDataChBuffer,
      MM_CH_BUFFER_SIZE_SHARED_DATA, MM_CH_BUFFER_SIZE_SHARED_DATA,
      GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_READ |
          GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY);
  sharedDataCh->requireSecurity(SecurityManager::MICROBIT_BLE_SECURITY_LEVEL);

  // sensorsCh = digitalIn[4], lightLevel[1], temperature[1], microphone[1]
  // directionCh = acceleration[10], magnet[8]
  // pinEventCh = pinEvent
  // actionEventCh = buttonEvent, gestureEvent

  GattCharacteristic *mbitMoreChs[] = {
      commandCh,    sensorsCh,    directionCh,  pinEventCh,  actionEventCh,
      analogInP0Ch, analogInP1Ch, analogInP2Ch, sharedDataCh};

  GattService mbitMoreService(MBIT_MORE_SERVICE, mbitMoreChs,
                              sizeof(mbitMoreChs) /
                                  sizeof(GattCharacteristic *));
  uBit.ble->addService(mbitMoreService);

  // Setup callbacks for events.
  uBit.ble->onDataWritten(this, &MbitMoreServiceDAL::onDataWritten);

  uBit.messageBus.listen(MICROBIT_ID_BLE, MICROBIT_BLE_EVT_CONNECTED, this,
                         &MbitMoreServiceDAL::onBLEConnected,
                         MESSAGE_BUS_LISTENER_QUEUE_IF_BUSY);
  uBit.messageBus.listen(MICROBIT_ID_BLE, MICROBIT_BLE_EVT_DISCONNECTED, this,
                         &MbitMoreServiceDAL::onBLEDisconnected,
                         MESSAGE_BUS_LISTENER_QUEUE_IF_BUSY);
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
 * @brief Notify action event.
 */
void MbitMoreServiceDAL::notifyActionEvent() {
  uBit.ble->gattServer().notify(actionEventCh->getValueHandle(),
                                actionEventChBuffer, MM_CH_BUFFER_SIZE_NOTIFY);
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
  //                               sizeof(sharedBuffer) /
  //                               sizeof(sharedBuffer[0]));
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
 * Update sensors.
 */
void MbitMoreServiceDAL::update() {
  if (uBit.ble->gap().getState().connected) {
    mbitMore->updateSensors(sensorsChBuffer);
    uBit.ble->gattServer().write(sensorsCh->getValueHandle(), sensorsChBuffer,
                                 MM_CH_BUFFER_SIZE_SENSORS);
  } else {
    mbitMore->displayFriendlyName();
  }
}

#endif // !MICROBIT_CODAL
