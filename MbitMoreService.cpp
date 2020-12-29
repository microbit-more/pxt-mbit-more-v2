#include "pxt.h"

#if MICROBIT_CODAL

#include "nrf_saadc.h"

/**
 * Class definition for the Scratch3 MicrobitMore Service.
 * Provides a BLE service to Scratch3.
 */
#include "MicroBitConfig.h"

#if CONFIG_ENABLED(DEVICE_BLE)

#include "MbitMoreService.h"
#include "MicroBitButton.h"
#include "ble_advdata.h"

/**
 * Position of data format in a value holder.
 */
#define MBIT_MORE_DATA_FORMAT_INDEX 19

#define MBIT_MORE_DATA_FORMAT_BUTTON_EVENT 0x11

const uint8_t MbitMoreService::baseUUID[16] = {
    0xa6, 0x2d, 0x57, 0x4e, 0x1b, 0x34, 0x40, 0x92,
    0x8d, 0xee, 0x41, 0x51, 0xf6, 0x3b, 0x28, 0x65};
const uint16_t MbitMoreService::serviceUUID = 0x574e;
const uint16_t MbitMoreService::charUUID[mbitmore_cIdx_COUNT] = {
    0x0001, // COMMAND
    0x0002, // DIGITAL_IN
    0x0003, // PIN_EVENT
    0x0004, // BUTTON_EVENT
    0x0005, // LIGHT_LEVEL
    0x0006, // ACCELERATION
    0x0007, // MAGNET
    0x0008, // TEMPERATURE
    0x0009, // MICROPHONE
    0x0020, // ANALOG_IN_P0
    0x0021, // ANALOG_IN_P1
    0x0022, // ANALOG_IN_P2
    0x0030  // SHARED_DATA
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

  // Initialize buffers.
  buttonEventBuffer[MBIT_MORE_DATA_FORMAT_INDEX] =
      MBIT_MORE_DATA_FORMAT_BUTTON_EVENT;

  // Add each of our characteristics.
  CreateCharacteristic(
      mbitmore_cIdx_DIGITAL_IN, charUUID[mbitmore_cIdx_DIGITAL_IN],
      (uint8_t *)(digitalInBuffer), MM_CH_BUFFER_SIZE_DEFAULT,
      MM_CH_BUFFER_SIZE_DEFAULT, microbit_propREAD | microbit_propREADAUTH);

  CreateCharacteristic(
      mbitmore_cIdx_BUTTON_EVENT, charUUID[mbitmore_cIdx_BUTTON_EVENT],
      (uint8_t *)(buttonEventBuffer), MM_CH_BUFFER_SIZE_DEFAULT,
      MM_CH_BUFFER_SIZE_DEFAULT, microbit_propREAD | microbit_propNOTIFY);

  CreateCharacteristic(mbitmore_cIdx_COMMAND, charUUID[mbitmore_cIdx_COMMAND],
                       (uint8_t *)(commandBuffer), MM_CH_BUFFER_SIZE_DEFAULT,
                       MM_CH_BUFFER_SIZE_DEFAULT,
                       microbit_propWRITE | microbit_propWRITE_WITHOUT);

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
void MbitMoreService::onDisconnect(const microbit_ble_evt_t *p_ble_evt) {
}

/**
 * Callback. Invoked when any of our attributes are written via BLE.
 */
void MbitMoreService::onDataWritten(const microbit_ble_evt_write_t *params) {
  if (params->handle == valueHandle(mbitmore_cIdx_COMMAND) && params->len > 0) {
    uint8_t *data = (uint8_t *)params->data;

    if (data[0] == ScratchBLECommand::CMD_DISPLAY_TEXT) {
      char text[params->len - 2] = {0};
      memcpy(text, &(data[2]), (params->len) - 2);
      mbitMore->displayText(text, data[1]);
    } else if (data[0] == ScratchBLECommand::CMD_DISPLAY_LED) {
      mbitMore->displayPattern(&data[1]);
    }
    // } else if (data[0] == ScratchBLECommand::CMD_LAYER_LED) {
    //   mbitMore->;layerPattern(data[2], data[1]);
    // }
    // else if (data[0] == ScratchBLECommand::CMD_PIN)
    // {
    //   if (data[1] == MbitMorePinCommand::SET_PULL)
    //   {
    //     switch (data[3])
    //     {
    //     case MbitMorePullNone:
    //       setPullMode(data[2], PinMode::PullNone);
    //       break;
    //     case MbitMorePullUp:
    //       setPullMode(data[2], PinMode::PullUp);
    //       break;
    //     case MbitMorePullDown:
    //       setPullMode(data[2], PinMode::PullDown);
    //       break;

    //     default:
    //       break;
    //     }
    //   }
    //   else if (data[1] == MbitMorePinCommand::SET_TOUCH)
    //   {
    //     setPinModeTouch(data[2]);
    //   }
    //   else if (data[1] == MbitMorePinCommand::SET_OUTPUT)
    //   {
    //     setDigitalValue(data[2], data[3]);
    //   }
    //   else if (data[1] == MbitMorePinCommand::SET_PWM)
    //   {
    //     // value is read as uint16_t little-endian.
    //     int value;
    //     memcpy(&value, &(data[3]), 2);
    //     setAnalogValue(data[2], value);
    //   }
    //   else if (data[1] == MbitMorePinCommand::SET_SERVO)
    //   {
    //     int pinIndex = (int)data[2];
    //     // angle is read as uint16_t little-endian.
    //     uint16_t angle;
    //     memcpy(&angle, &(data[3]), 2);
    //     // range is read as uint16_t little-endian.
    //     uint16_t range;
    //     memcpy(&range, &(data[5]), 2);
    //     // center is read as uint16_t little-endian.
    //     uint16_t center;
    //     memcpy(&center, &(data[7]), 2);
    //     if (range == 0)
    //     {
    //       uBit.io.pin[pinIndex].setServoValue(angle);
    //     }
    //     else if (center == 0)
    //     {
    //       uBit.io.pin[pinIndex].setServoValue(angle, range);
    //     }
    //     else
    //     {
    //       uBit.io.pin[pinIndex].setServoValue(angle, range, center);
    //     }
    //   }
    //   else if (data[1] == MbitMorePinCommand::SET_EVENT)
    //   {
    //     listenPinEventOn((int)data[2], (int)data[3]);
    //   }
    // }
    // else if (data[0] == ScratchBLECommand::CMD_SHARED_DATA)
    // {
    //   // value is read as int16_t little-endian.
    //   int16_t value;
    //   memcpy(&value, &(data[2]), 2);
    //   sharedData[data[1]] = value;
    // }
    // else if (data[0] == ScratchBLECommand::CMD_PROTOCOL)
    // {
    //   mbitMoreProtocol = data[1];
    // }
    // else if (data[0] == ScratchBLECommand::CMD_LIGHT_SENSING)
    // {
    //   setLightSensingDuration(data[1]);
    // }
  }
}

/**
 * Callback. Invoked when any of our attributes are read via BLE.
 * Set  params->data and params->length to update the value
 */
void MbitMoreService::onDataRead(microbit_onDataRead_t *params) {
  if (params->handle == valueHandle(mbitmore_cIdx_DIGITAL_IN)) {
    mbitMore->updateDigitalValues();
    params->data = (uint8_t *)&(mbitMore->digitalValues);
    params->length = 4;
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
 * @brief Notify button event.
 *
 * @param data Data to notify.
 * @param length Lenght of the data.
 */
void MbitMoreService::notifyButtonEvent(uint8_t *data, uint16_t length) {
  notifyChrValue(mbitmore_cIdx_BUTTON_EVENT, data, length);
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
void MbitMoreService::notify() {
}

/**
 * Update sensors.
 */
void MbitMoreService::update() {
  if (getConnected()) {
  } else {
    mbitMore->displayFriendlyName();
  }
}

/**
 * Write IO characteristics.
 */
void MbitMoreService::writeDigitalIn() {
  writeChrValue(mbitmore_cIdx_DIGITAL_IN, (uint8_t *)&(mbitMore->digitalValues),
                4);
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