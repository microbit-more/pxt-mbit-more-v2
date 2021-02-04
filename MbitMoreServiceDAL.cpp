#include "pxt.h"

#include "MicroBit.h"
#include "MicroBitConfig.h"

#if !MICROBIT_CODAL

#include "MicroBitButton.h"

#include "MbitMoreServiceDAL.h"

#define MBIT_MORE_DATA_FORMAT_BUTTON_EVENT 0x11

/**
 * @brief Service ID of Microbit More.
 *
 */
const uint8_t MBIT_MORE_SERVICE[] = {0x0b, 0x50, 0xf3, 0xe4, 0x60, 0x7f, 0x41, 0x51, 0x90, 0x91, 0x7d, 0x00, 0x8d, 0x6f, 0xfc, 0x5c};

/**
 * @brief Characteristics in Microbit More Service.
 *
 */
const uint8_t MBIT_MORE_CH_COMMAND[] = {0x0b, 0x50, 0x01, 0x00, 0x60, 0x7f, 0x41, 0x51, 0x90, 0x91, 0x7d, 0x00, 0x8d, 0x6f, 0xfc, 0x5c};
const uint8_t MBIT_MORE_CH_STATE[] = {0x0b, 0x50, 0x01, 0x01, 0x60, 0x7f, 0x41, 0x51, 0x90, 0x91, 0x7d, 0x00, 0x8d, 0x6f, 0xfc, 0x5c};
const uint8_t MBIT_MORE_CH_DIRECTION[] = {0x0b, 0x50, 0x01, 0x02, 0x60, 0x7f, 0x41, 0x51, 0x90, 0x91, 0x7d, 0x00, 0x8d, 0x6f, 0xfc, 0x5c};
const uint8_t MBIT_MORE_CH_PIN_EVENT[] = {0x0b, 0x50, 0x01, 0x10, 0x60, 0x7f, 0x41, 0x51, 0x90, 0x91, 0x7d, 0x00, 0x8d, 0x6f, 0xfc, 0x5c};
const uint8_t MBIT_MORE_CH_ACTION_EVENT[] = {0x0b, 0x50, 0x01, 0x11, 0x60, 0x7f, 0x41, 0x51, 0x90, 0x91, 0x7d, 0x00, 0x8d, 0x6f, 0xfc, 0x5c};
const uint8_t MBIT_MORE_CH_ANALOG_IN_P0[] = {0x0b, 0x50, 0x01, 0x20, 0x60, 0x7f, 0x41, 0x51, 0x90, 0x91, 0x7d, 0x00, 0x8d, 0x6f, 0xfc, 0x5c};
const uint8_t MBIT_MORE_CH_ANALOG_IN_P1[] = {0x0b, 0x50, 0x01, 0x21, 0x60, 0x7f, 0x41, 0x51, 0x90, 0x91, 0x7d, 0x00, 0x8d, 0x6f, 0xfc, 0x5c};
const uint8_t MBIT_MORE_CH_ANALOG_IN_P2[] = {0x0b, 0x50, 0x01, 0x22, 0x60, 0x7f, 0x41, 0x51, 0x90, 0x91, 0x7d, 0x00, 0x8d, 0x6f, 0xfc, 0x5c};

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

  stateCh = new GattCharacteristic(
      MBIT_MORE_CH_STATE, (uint8_t *)&stateChBuffer,
      MM_CH_BUFFER_SIZE_STATE, MM_CH_BUFFER_SIZE_STATE,
      GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_READ);
  stateCh->requireSecurity(SecurityManager::MICROBIT_BLE_SECURITY_LEVEL);

  directionCh = new GattCharacteristic(
      MBIT_MORE_CH_DIRECTION, (uint8_t *)&motionChBuffer,
      MM_CH_BUFFER_SIZE_MOTION, MM_CH_BUFFER_SIZE_MOTION,
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

  /*
  stateCh = digitalIn[4], lightLevel[1], temperature[1], microphone[1]
  directionCh = acceleration[10], magnet[8]
  pinEventCh = pinEvent
  actionEventCh = buttonEvent, gestureEvent
  analogInP0Ch, analogInP1Ch, analogInP2Ch
  */

  GattCharacteristic *mbitMoreChs[] = {
      commandCh,
      stateCh,
      directionCh,
      pinEventCh,
      actionEventCh,
      analogInP0Ch,
      analogInP1Ch,
      analogInP2Ch,
  };

  GattService mbitMoreService(MBIT_MORE_SERVICE, mbitMoreChs,
                              sizeof(mbitMoreChs) /
                                  sizeof(GattCharacteristic *));
  uBit.ble->addService(mbitMoreService);

  // Setup callbacks for events.
  uBit.ble->onDataWritten(this, &MbitMoreServiceDAL::onDataWritten);

}

/**
 * Callback. Invoked when AnalogIn is read via BLE.
 */
void MbitMoreServiceDAL::onReadAnalogIn(
    GattReadAuthCallbackParams *authParams) {
  if (authParams->handle == analogInP0Ch->getValueHandle()) {
    mbitMore->updateAnalogIn(analogInP0ChBuffer, 0);
    authParams->data = (uint8_t *)&analogInP0ChBuffer;
    authParams->offset = 0;
    authParams->len = MM_CH_BUFFER_SIZE_ANALOG_IN;
    authParams->authorizationReply = AUTH_CALLBACK_REPLY_SUCCESS;
  } else if (authParams->handle == analogInP1Ch->getValueHandle()) {
    mbitMore->updateAnalogIn(analogInP1ChBuffer, 1);
    authParams->data = (uint8_t *)&analogInP1ChBuffer;
    authParams->offset = 0;
    authParams->len = MM_CH_BUFFER_SIZE_ANALOG_IN;
    authParams->authorizationReply = AUTH_CALLBACK_REPLY_SUCCESS;
  } else if (authParams->handle == analogInP2Ch->getValueHandle()) {
    mbitMore->updateAnalogIn(analogInP2ChBuffer, 2);
    authParams->data = (uint8_t *)&analogInP2ChBuffer;
    authParams->offset = 0;
    authParams->len = MM_CH_BUFFER_SIZE_ANALOG_IN;
    authParams->authorizationReply = AUTH_CALLBACK_REPLY_SUCCESS;
  }
}

/**
 * Callback. Invoked when any of our attributes are written via BLE.
 */
void MbitMoreServiceDAL::onDataWritten(const GattWriteCallbackParams *params) {
  mbitMore->onCommandReceived((uint8_t *)params->data, params->len);
}

/**
 * @brief Notify action event.
 */
void MbitMoreServiceDAL::notifyActionEvent() {
  uBit.ble->gattServer().notify(actionEventCh->getValueHandle(),
                                actionEventChBuffer, MM_CH_BUFFER_SIZE_NOTIFY);
}

/**
 * @brief Notify pin event.
 */
void MbitMoreServiceDAL::notifyPinEvent() {
  uBit.ble->gattServer().notify(pinEventCh->getValueHandle(), pinEventChBuffer,
                                MM_CH_BUFFER_SIZE_NOTIFY);
}

/**
 * Notify data to Scratch3
 */
void MbitMoreServiceDAL::notify() {}

/**
 * Update all GPIO and sensors state.
 */
void MbitMoreServiceDAL::update() {
  if (uBit.ble->gap().getState().connected) {
    mbitMore->updateState(stateChBuffer);
    uBit.ble->gattServer().write(stateCh->getValueHandle(), stateChBuffer,
                                 MM_CH_BUFFER_SIZE_STATE);
    mbitMore->updateMotion(motionChBuffer);
    uBit.ble->gattServer().write(directionCh->getValueHandle(),
                                 motionChBuffer,
                                 MM_CH_BUFFER_SIZE_MOTION);
  } else {
    mbitMore->displayFriendlyName();
  }
}

#endif // !MICROBIT_CODAL
