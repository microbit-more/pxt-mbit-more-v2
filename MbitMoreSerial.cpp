#include "MbitMoreSerial.h"

static MbitMoreSerial *serial;

void startMbitMoreSerialReceiving() {
  serial->startSerialReceiving();
}

void startMbitMoreSerialUpdating() {
  serial->startSerialUpdating();
}

MbitMoreSerial::MbitMoreSerial(MbitMoreDevice &_mbitMore) : mbitMore(_mbitMore) {
  serial = this;
  // // Change baud rate
  //   int rate = 57600;
  // #if MICROBIT_CODAL
  //   uBit.serial.setBaud(rate);
  // #else
  //   uBit.serial.baud((int)rate);
  // #endif
  create_fiber(startMbitMoreSerialReceiving);
}

void MbitMoreSerial::readResponseOnSerial(uint16_t ch, uint8_t *dataBuffer, size_t len) {
  uint8_t frame[5 + len] = {0};
  frame[0] = MM_SFD;
  frame[1] = ChResponse::RES_READ;
  frame[2] = ch >> 8;
  frame[3] = ch & 0x00FF;
  frame[4] = len;
  memcpy(&frame[5], dataBuffer, len);
  uBit.serial.send(frame, 5 + len, SYNC_SLEEP);
}

void MbitMoreSerial::writeResponseOnSerial(uint16_t ch, bool response) {
  uint8_t frame[6] = {0};
  frame[0] = MM_SFD;
  frame[1] = ChResponse::RES_WRITE;
  frame[2] = ch >> 8;
  frame[3] = ch & 0x00FF;
  frame[4] = 1;
  frame[5] = 1;
  uBit.serial.send(frame, 6, SYNC_SLEEP);
}

void MbitMoreSerial::notifyOnSerial(uint16_t ch, uint8_t *dataBuffer, size_t len) {
  uint8_t frame[5 + len] = {0};
  frame[0] = MM_SFD;
  frame[1] = ChResponse::RES_NOTIFY;
  frame[2] = ch >> 8;
  frame[3] = ch & 0x00FF;
  frame[4] = len;
  memcpy(&frame[5], dataBuffer, len);
  uBit.serial.send(frame, 5 + len, SYNC_SLEEP);
}

void MbitMoreSerial::startSerialUpdating() {
  MbitMoreService *moreService = mbitMore.moreService;
  uint16_t stateCh = 0x0101;
  uint16_t motionCh = 0x0102;
  uint8_t *responseBuffer;
  while (true) {
    responseBuffer = moreService->stateChBuffer;
    mbitMore.updateState(responseBuffer);
    readResponseOnSerial(stateCh, responseBuffer, MM_CH_BUFFER_SIZE_STATE);
    fiber_sleep(50);
    responseBuffer = moreService->motionChBuffer;
    mbitMore.updateMotion(responseBuffer);
    readResponseOnSerial(motionCh, responseBuffer, MM_CH_BUFFER_SIZE_MOTION);
    fiber_sleep(50);
  }
}

bool MbitMoreSerial::waitRxBufferedSize(size_t length) {
  int retry = 0;
  while (uBit.serial.rxBufferedSize() < (int)length) {
    if (retry > (10 * (int)length)) {
      return false;
    }
    retry++;
    fiber_sleep(1);
  }
  while (uBit.serial.rxInUse()) {
    fiber_sleep(1);
  }
  return true;
}

void MbitMoreSerial::startSerialReceiving() {
  MbitMoreService *moreService = mbitMore.moreService;
  int receivedChar;
  int requestType;
  uint16_t ch;
  uint8_t *responseBuffer;

  uBit.serial.setTxBufferSize(254);
  uBit.serial.clearTxBuffer();
  uBit.serial.setRxBufferSize(254);
  uBit.serial.clearRxBuffer();

  while (true) {
    receivedChar = uBit.serial.read(SYNC_SLEEP);
    if (receivedChar < 0) {
      continue; // MICROBIT_SERIAL_IN_USE or MICROBIT_NO_RESOURCES
    }
    if (MM_SFD == receivedChar) {
      if (!waitRxBufferedSize(1)) {
        continue;
      }
      requestType = uBit.serial.read(SYNC_SLEEP);
      if (requestType < 0 || requestType > ChRequest::REQ_NOTIFY_START) {
        continue;
      }
      if (!waitRxBufferedSize(1)) {
        continue;
      }
      ch = uBit.serial.read(SYNC_SLEEP) << 8;
      if (!waitRxBufferedSize(1)) {
        continue;
      }
      ch |= uBit.serial.read(SYNC_SLEEP);
      // uBit.display.scroll((int)ch); // debug

      // COMMAND
      if (0x0100 == ch) {
        if (ChRequest::REQ_READ == requestType) {
          // Start connection
          mbitMore.updateVersionData();
          responseBuffer = moreService->commandChBuffer;
          readResponseOnSerial(ch, responseBuffer, MM_CH_BUFFER_SIZE_COMMAND);
          if (!mbitMore.serialConnected) {
            mbitMore.onSerialConnected();
            create_fiber(startMbitMoreSerialUpdating);
          }
          continue;
        }
        if (ChRequest::REQ_WRITE == requestType || ChRequest::REQ_WRITE_RESPONSE == requestType) {
          if (!waitRxBufferedSize(1)) {
            continue;
          }
          uint8_t length = uBit.serial.read(SYNC_SLEEP);
          if (!waitRxBufferedSize(length)) {
            continue;
          }
          uBit.serial.read(moreService->commandChBuffer, length, SYNC_SLEEP);
          mbitMore.onCommandReceived(moreService->commandChBuffer, length);
          if (ChRequest::REQ_WRITE_RESPONSE == requestType) {
            writeResponseOnSerial(ch, true);
          }
          continue;
        }
      }

      // ANALOG_IN_P0
      if (0x0120 == ch) {
        if (ChRequest::REQ_READ == requestType) {
          mbitMore.updateAnalogIn(moreService->analogInP0ChBuffer, 0);
          readResponseOnSerial(ch, moreService->analogInP0ChBuffer, MM_CH_BUFFER_SIZE_ANALOG_IN);
          continue;
        }
      }

      // ANALOG_IN_P1
      if (0x0121 == ch) {
        if (ChRequest::REQ_READ == requestType) {
          mbitMore.updateAnalogIn(moreService->analogInP1ChBuffer, 1);
          readResponseOnSerial(ch, moreService->analogInP1ChBuffer, MM_CH_BUFFER_SIZE_ANALOG_IN);
          continue;
        }
      }

      // ANALOG_IN_P2
      if (0x0122 == ch) {
        if (ChRequest::REQ_READ == requestType) {
          mbitMore.updateAnalogIn(moreService->analogInP2ChBuffer, 2);
          readResponseOnSerial(ch, moreService->analogInP2ChBuffer, MM_CH_BUFFER_SIZE_ANALOG_IN);
          continue;
        }
      }

      uBit.display.print("E"); // debug
      continue;
    }
  }
}
