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

// service ID: 0b50f3e4-607f-4151-9091-7d008d6ffc5c
const uint8_t MbitMoreService::baseUUID[16] = {0x0b, 0x50, 0xf3, 0xe4, 0x60, 0x7f, 0x41, 0x51, 0x90, 0x91, 0x7d, 0x00, 0x8d, 0x6f, 0xfc, 0x5c};
const uint16_t MbitMoreService::serviceUUID = 0xf3e4;
const uint16_t MbitMoreService::charUUID[mbitmore_cIdx_COUNT] = {
    0x0100, // COMMAND
    0x0101, // STATE
    0x0102, // MOTION
    0x0110, // PIN_EVENT
    0x0111, // ACTION_EVENT
    0x0120, // ANALOG_IN_P0
    0x0121, // ANALOG_IN_P1
    0x0122, // ANALOG_IN_P2
    0x0130  // MESSAGE
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
  CreateCharacteristic(
      mbitmore_cIdx_COMMAND,
      charUUID[mbitmore_cIdx_COMMAND],
      (uint8_t *)(commandChBuffer),
      MM_CH_BUFFER_SIZE_COMMAND,
      MM_CH_BUFFER_SIZE_COMMAND,
      microbit_propWRITE | microbit_propWRITE_WITHOUT | microbit_propREAD);

  CreateCharacteristic(
      mbitmore_cIdx_STATE,
      charUUID[mbitmore_cIdx_STATE],
      (uint8_t *)(stateChBuffer),
      MM_CH_BUFFER_SIZE_STATE,
      MM_CH_BUFFER_SIZE_STATE,
      microbit_propREAD);

  CreateCharacteristic(
      mbitmore_cIdx_MOTION,
      charUUID[mbitmore_cIdx_MOTION],
      (uint8_t *)(motionChBuffer),
      MM_CH_BUFFER_SIZE_MOTION,
      MM_CH_BUFFER_SIZE_MOTION,
      microbit_propREAD);

  CreateCharacteristic(
      mbitmore_cIdx_PIN_EVENT,
      charUUID[mbitmore_cIdx_PIN_EVENT],
      (uint8_t *)(pinEventChBuffer),
      MM_CH_BUFFER_SIZE_NOTIFY,
      MM_CH_BUFFER_SIZE_NOTIFY,
      microbit_propREAD | microbit_propNOTIFY);

  CreateCharacteristic(
      mbitmore_cIdx_ACTION_EVENT,
      charUUID[mbitmore_cIdx_ACTION_EVENT],
      (uint8_t *)(actionEventChBuffer),
      MM_CH_BUFFER_SIZE_NOTIFY,
      MM_CH_BUFFER_SIZE_NOTIFY,
      microbit_propREAD | microbit_propNOTIFY);

  CreateCharacteristic(
      mbitmore_cIdx_ANALOG_IN_P0,
      charUUID[mbitmore_cIdx_ANALOG_IN_P0],
      (uint8_t *)(analogInP0ChBuffer),
      MM_CH_BUFFER_SIZE_ANALOG_IN,
      MM_CH_BUFFER_SIZE_ANALOG_IN,
      microbit_propREAD | microbit_propREADAUTH);

  CreateCharacteristic(
      mbitmore_cIdx_ANALOG_IN_P1,
      charUUID[mbitmore_cIdx_ANALOG_IN_P1],
      (uint8_t *)(analogInP1ChBuffer),
      MM_CH_BUFFER_SIZE_ANALOG_IN,
      MM_CH_BUFFER_SIZE_ANALOG_IN,
      microbit_propREAD | microbit_propREADAUTH);

  CreateCharacteristic(
      mbitmore_cIdx_ANALOG_IN_P2,
      charUUID[mbitmore_cIdx_ANALOG_IN_P2],
      (uint8_t *)(analogInP2ChBuffer),
      MM_CH_BUFFER_SIZE_ANALOG_IN,
      MM_CH_BUFFER_SIZE_ANALOG_IN,
      microbit_propREAD | microbit_propREADAUTH);

  CreateCharacteristic(
      mbitmore_cIdx_DATA,
      charUUID[mbitmore_cIdx_DATA],
      (uint8_t *)(dataChBuffer),
      MM_CH_BUFFER_SIZE_NOTIFY,
      MM_CH_BUFFER_SIZE_NOTIFY,
      microbit_propREAD | microbit_propNOTIFY);

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
  mbitMore->updateVersionData();
}

/**
 * Invoked when BLE disconnects.
 */
void MbitMoreService::onDisconnect(const microbit_ble_evt_t *p_ble_evt) {
}

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
  if (params->handle == valueHandle(mbitmore_cIdx_ANALOG_IN_P0)) {
    mbitMore->updateAnalogIn(analogInP0ChBuffer, 0);
    params->data = analogInP0ChBuffer;
    params->length = 2;
  } else if (params->handle == valueHandle(mbitmore_cIdx_ANALOG_IN_P1)) {
    mbitMore->updateAnalogIn(analogInP1ChBuffer, 1);
    params->data = analogInP1ChBuffer;
    params->length = 2;
  } else if (params->handle == valueHandle(mbitmore_cIdx_ANALOG_IN_P2)) {
    mbitMore->updateAnalogIn(analogInP2ChBuffer, 2);
    params->data = analogInP2ChBuffer;
    params->length = 2;
  }
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
 * @brief Notify action event.
 */
void MbitMoreService::notifyActionEvent() {
  if (!getConnected())
    return;
  notifyChrValue(mbitmore_cIdx_ACTION_EVENT, actionEventChBuffer,
                 MM_CH_BUFFER_SIZE_NOTIFY);
}

/**
 * @brief Notify pin event.
 */
void MbitMoreService::notifyPinEvent() {
  if (!getConnected())
    return;
  notifyChrValue(mbitmore_cIdx_PIN_EVENT, pinEventChBuffer,
                 MM_CH_BUFFER_SIZE_NOTIFY);
}

/**
 * Notify message to Scratch3
 */
void MbitMoreService::notifyData() {
  if (!getConnected())
    return;
  notifyChrValue(mbitmore_cIdx_DATA, dataChBuffer, MM_CH_BUFFER_SIZE_NOTIFY);
}

/**
 * Notify data to Scratch3
 */
void MbitMoreService::notify() {}

/**
 * Update all sensors.
 */
void MbitMoreService::update() {
  if (getConnected()) {
    mbitMore->updateState(stateChBuffer);
    mbitMore->updateMotion(motionChBuffer);
  }
}

/**
 * @brief Register data label and retrun ID for the label.
 *
 * @param dataLabel label to register
 * @param dataType type of the data to be received
 * @return int ID for the label
 */
int MbitMoreService::registerWaitingDataLabel(ManagedString dataLabel, MbitMoreDataContentType dataType) {
  return mbitMore->registerWaitingDataLabel(dataLabel, dataType);
}

/**
 * @brief Get type of content for the label
 *
 * @param labelID ID for the label
 * @return type of content [number | string]
 */
MbitMoreDataContentType MbitMoreService::dataType(int labelID) {
  return mbitMore->dataType(labelID);
}

/**
 * @brief Return content of the data as number
 *
 * @param labelID ID for the label
 * @return content of the data
 */
float MbitMoreService::dataContentAsNumber(int labelID) {
  return mbitMore->dataContentAsNumber(labelID);
}

/**
 * @brief Return content of the data as string
 *
 * @param labelID ID for the label
 * @return content of the data
 */
ManagedString MbitMoreService::dataContentAsText(int labelID) {
  return mbitMore->dataContentAsText(labelID);
}

/**
 * @brief Send a float with label to Scratch.
 *  
 * @param dataLabel label of the data
 * @param dataContent content of the data
 */
void MbitMoreService::sendNumberWithLabel(ManagedString dataLabel, float dataContent) {
  mbitMore->sendNumberWithLabel(dataLabel, dataContent);
}

/**
 * @brief Send a string with label to Scratch.
 * 
 * @param dataLabel label of the data
 * @param dataContent content of the data
 */
void MbitMoreService::sendTextWithLabel(ManagedString dataLabel, ManagedString dataContent) {
  mbitMore->sendTextWithLabel(dataLabel, dataContent);
}

#endif // CONFIG_ENABLED(DEVICE_BLE)
#endif // MICROBIT_CODAL