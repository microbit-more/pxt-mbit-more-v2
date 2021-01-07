#include "pxt.h"

#if MICROBIT_CODAL

#include "nrf_saadc.h"

/**
 * Class definition for the Scratch3 MicrobitMore Service.
 * Provides a BLE service to Scratch3.
 */
#include "MicroBitConfig.h"

#if CONFIG_ENABLED(DEVICE_BLE)

// #include "ble_advdata.h"

#include "MbitMoreService.h"
#include "MicroBitButton.h"

const uint8_t MbitMoreService::baseUUID[16] = {
    0xa6, 0x2d, 0x57, 0x4e, 0x1b, 0x34, 0x40, 0x92,
    0x8d, 0xee, 0x41, 0x51, 0xf6, 0x3b, 0x28, 0x65};
const uint16_t MbitMoreService::serviceUUID = 0x574e;
const uint16_t MbitMoreService::charUUID[mbitmore_cIdx_COUNT] = {
    0x0100, // COMMAND
    0x0101, // SENSORS
    0x0102, // DIRECTION
    0x0110, // PIN_EVENT
    0x0111, // ACTION_EVENT
    0x0120, // ANALOG_IN_P0
    0x0121, // ANALOG_IN_P1
    0x0122, // ANALOG_IN_P2
    0x0130  // SHARED_DATA
};

/**
 * Constructor.
 * Create a representation of default extension for Scratch3.
 */
MbitMoreService::MbitMoreService() : uBit(pxt::uBit) {
  mbitMore = &MbitMoreDevice::getInstance();
  mbitMore->moreService = this;

  // Create the service.
  bs_uuid_type = BLE_UUID_TYPE_UNKNOWN;
  RegisterBaseUUID(baseUUID);
  CreateService(serviceUUID);

  // Add each of our characteristics.
  CreateCharacteristic(mbitmore_cIdx_COMMAND, charUUID[mbitmore_cIdx_COMMAND],
                       (uint8_t *)(commandChBuffer), 0, MM_CH_BUFFER_SIZE_MAX,
                       microbit_propWRITE | microbit_propWRITE_WITHOUT);

  CreateCharacteristic(mbitmore_cIdx_SENSORS, charUUID[mbitmore_cIdx_SENSORS],
                       (uint8_t *)(sensorsChBuffer), MM_CH_BUFFER_SIZE_SENSORS,
                       MM_CH_BUFFER_SIZE_SENSORS, microbit_propREAD);

  CreateCharacteristic(
      mbitmore_cIdx_DIRECTION, charUUID[mbitmore_cIdx_DIRECTION],
      (uint8_t *)(directionChBuffer), MM_CH_BUFFER_SIZE_DIRECTION,
      MM_CH_BUFFER_SIZE_DIRECTION, microbit_propREAD);

  CreateCharacteristic(
      mbitmore_cIdx_PIN_EVENT, charUUID[mbitmore_cIdx_PIN_EVENT],
      (uint8_t *)(pinEventChBuffer), MM_CH_BUFFER_SIZE_NOTIFY,
      MM_CH_BUFFER_SIZE_NOTIFY, microbit_propREAD | microbit_propNOTIFY);

  CreateCharacteristic(
      mbitmore_cIdx_ACTION_EVENT, charUUID[mbitmore_cIdx_ACTION_EVENT],
      (uint8_t *)(actionEventChBuffer), MM_CH_BUFFER_SIZE_NOTIFY,
      MM_CH_BUFFER_SIZE_NOTIFY, microbit_propREAD | microbit_propNOTIFY);

  CreateCharacteristic(
      mbitmore_cIdx_ANALOG_IN_P0, charUUID[mbitmore_cIdx_ANALOG_IN_P0],
      (uint8_t *)(analogInP0ChBuffer), MM_CH_BUFFER_SIZE_ANALOG_IN,
      MM_CH_BUFFER_SIZE_ANALOG_IN, microbit_propREAD | microbit_propREADAUTH);

  CreateCharacteristic(
      mbitmore_cIdx_ANALOG_IN_P1, charUUID[mbitmore_cIdx_ANALOG_IN_P1],
      (uint8_t *)(analogInP1ChBuffer), MM_CH_BUFFER_SIZE_ANALOG_IN,
      MM_CH_BUFFER_SIZE_ANALOG_IN, microbit_propREAD | microbit_propREADAUTH);

  CreateCharacteristic(
      mbitmore_cIdx_ANALOG_IN_P2, charUUID[mbitmore_cIdx_ANALOG_IN_P2],
      (uint8_t *)(analogInP2ChBuffer), MM_CH_BUFFER_SIZE_ANALOG_IN,
      MM_CH_BUFFER_SIZE_ANALOG_IN, microbit_propREAD | microbit_propREADAUTH);

  CreateCharacteristic(
      mbitmore_cIdx_SHARED_DATA, charUUID[mbitmore_cIdx_SHARED_DATA],
      (uint8_t *)(sharedDataChBuffer), MM_CH_BUFFER_SIZE_SHARED_DATA,
      MM_CH_BUFFER_SIZE_SHARED_DATA, microbit_propREAD | microbit_propNOTIFY);

  // // Stop advertising.
  // uBit.ble->stopAdvertising();

  // // Configure advertising.
  // ble_uuid_t adv_uuids[] = {{serviceUUID, BLE_UUID_TYPE_BLE}};
  // ble_advdata_t advdata;
  // memset(&advdata, 0, sizeof(advdata));
  // advdata.name_type = BLE_ADVDATA_FULL_NAME;
  // advdata.include_appearance = true;
  // advdata.flags = BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE;
  // advdata.uuids_complete.uuid_cnt = sizeof(adv_uuids) / sizeof(adv_uuids[0]);
  // advdata.uuids_complete.p_uuids = adv_uuids;
  // bool connectable = true;
  // bool discoverable = true;
  // bool whitelist = false;
  // uBit.ble->configureAdvertising(connectable, discoverable, whitelist,
  //                                MICROBIT_BLE_ADVERTISING_INTERVAL,
  //                                MICROBIT_BLE_ADVERTISING_TIMEOUT, &advdata);

  // // Start advertising.
  // uBit.ble->advertise();

  fiber_add_idle_component(this);
}

/**
 * Invoked when BLE connects.
 */
void MbitMoreService::onConnect(const microbit_ble_evt_t *p_ble_evt) {
  mbitMore->initConfiguration();
}

/**
 * Invoked when BLE disconnects.
 */
void MbitMoreService::onDisconnect(const microbit_ble_evt_t *p_ble_evt) {}

/**
 * Callback. Invoked when any of our attributes are written via BLE.
 */
void MbitMoreService::onDataWritten(const microbit_ble_evt_write_t *params) {
  if (params->handle == valueHandle(mbitmore_cIdx_COMMAND) && params->len > 0) {
    mbitMore->onCommandReceived((uint8_t *)params->data, params->len);
  }
}

/**
 * Callback. Invoked when any of our attributes are read via BLE.
 * Set  params->data and params->length to update the value
 */
void MbitMoreService::onDataRead(microbit_onDataRead_t *params) {
  // if (params->handle == valueHandle(mbitmore_cIdx_ANALOG_IN_P0)) {
  //   mbitMore->updateAnalogInValue(0);
  //   params->data = (uint8_t *)&(mbitMore->analogInValues[0]);
  //   params->length = 2;
  // }
}

/**
 * Periodic callback from MicroBit idle thread.
 */
void MbitMoreService::idleCallback() {
  if (getConnected()) {
  } else {
    mbitMore->displayFriendlyName();
  }
}

/**
 * @brief Notify data of the basic extension.
 *
 * @param data Data to notify.
 * @param length Length of the data.
 */
void MbitMoreService::notifyBasicData(uint8_t *data, uint16_t length) {
  // notifyChrValue(mbbs_cIdxTX, data, length);
}

/**
 * @brief Notify action event.
 */
void MbitMoreService::notifyActionEvent() {
  notifyChrValue(mbitmore_cIdx_ACTION_EVENT, actionEventChBuffer,
                 MM_CH_BUFFER_SIZE_NOTIFY);
}

/**
 * @brief Notify IO event.
 *
 * @param data Data to notify.
 * @param length Lenght of the data.
 */
void MbitMoreService::notifyIOEvent(uint8_t *data, uint16_t length) {
  // notifyChrValue(mbbs_cIdxIOEvent, data, length);
}

/**
 * Notify shared data to Scratch3
 */
void MbitMoreService::notifySharedData() {
  // for (size_t i = 0; i < sizeof(sharedData) / sizeof(sharedData[0]); i++) {
  //   memcpy(&(sharedBuffer[(i * 2)]), &sharedData[i], 2);
  // }
  // moreService->notifySharedData(
  //     (uint8_t *)&sharedBuffer,
  //     sizeof(sharedBuffer) / sizeof(sharedBuffer[0]));
}

/**
 * Notify data to Scratch3
 */
void MbitMoreService::notify() {}

/**
 * Update sensors.
 */
void MbitMoreService::update() {
  if (getConnected()) {
    mbitMore->updateSensors(sensorsChBuffer);
    mbitMore->updateDirection(directionChBuffer);
  }
}

/**
 * Set value to shared data.
 * shared data (0, 1, 2, 3)
 */
void MbitMoreService::setSharedData(int index, int value) {
  // value (-32768 to 32767) is sent as int16_t little-endian.
  // int16_t data = (int16_t)value;
  // sharedData[index] = data;
  // notifySharedData();
}

/**
 * Get value of a shared data.
 * shared data (0, 1, 2, 3)
 */
int MbitMoreService::getSharedData(int index) {
  // return (int)(sharedData[index]);
}

#endif // CONFIG_ENABLED(DEVICE_BLE)
#endif // MICROBIT_CODAL