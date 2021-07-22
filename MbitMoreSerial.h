#include "MbitMoreCommon.h"
#if MBIT_MORE_USE_SERIAL

#ifndef MBIT_MORE_SERIAL_H
#define MBIT_MORE_SERIAL_H

#include "MbitMoreDevice.h"

#define MM_SFD 0xff
#define MM_RX_BUFFER_SIZE 254
#define MM_TX_BUFFER_SIZE 254

// // Forward declaration
class MbitMoreDevice;

/**
 * Class definition for main logics of Microbit More Service except bluetooth connectivity.
 *
 */
class MbitMoreSerial {
private:
  /**
   * @brief Communication route between Scratch and micro:bit
   * 
   */
  enum MbitMoreCommunicationRoute
  {
    BLE = 0,
    SERIAL = 1,
  };

  /**
   * @brief Request type from Scratch
   * 
   */
  enum ChRequest
  {
    REQ_READ = 0x01,
    REQ_WRITE = 0x10,
    REQ_WRITE_RESPONSE = 0x11,
    REQ_NOTIFY_STOP = 0x20,
    REQ_NOTIFY_START = 0x21,
  };

  /**
   * @brief Response type to Scratch
   * 
   */
  enum ChResponse
  {
    RES_READ = 0x01,
    RES_WRITE = 0x11,
    RES_NOTIFY = 0x21,
  };

public:
  /**
   * @brief Microbit More object.
   *
   */
  MbitMoreDevice &mbitMore;

  /**
   * @brief Construct a new Microbit More Serial service
   * 
   * @param _mbitMore An instance of Microbit More device controller
   */
  MbitMoreSerial(MbitMoreDevice &_mbitMore);

  /**
   * @brief Send a response for read request.
   * 
   * @param ch Characteristeic of the request
   * @param dataBuffer Buffer to send
   * @param len Length of the buffer to send
   */
  void readResponseOnSerial(uint16_t ch, uint8_t *dataBuffer, size_t len);

  /**
   * @brief Send a response for write request.
   * 
   * @param ch Characteristic of the request
   * @param response Response for the request
   */
  void writeResponseOnSerial(uint16_t ch, bool response);

  /**
   * @brief Notify data of the characteristic
   * 
   * @param ch Characteristic to notify
   * @param dataBuffer Buffer to notify
   * @param len Length of the buffer to notify
   */
  void notifyOnSerial(uint16_t ch, uint8_t *dataBuffer, size_t len);

  /**
   * @brief Start continuous receiving process from serial port.
   * 
   */
  void startSerialReceiving();

  /**
   * @brief Start continuous updating process to serial port.
   * 
   */
  void startSerialUpdating();
};
#endif // MBIT_MORE_SERIAL_H
#endif // MBIT_MORE_USE_SERIAL