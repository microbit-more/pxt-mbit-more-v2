#include "pxt.h"

#if MICROBIT_CODAL
#include "LevelDetector.h"
#include "LevelDetectorSPL.h"
#endif // MICROBIT_CODAL

#define MICROPHONE_MIN 52.0f
#define MICROPHONE_MAX 120.0f

namespace pxt {
#if MICROBIT_CODAL
  codal::LevelDetectorSPL *getMicrophoneLevel();
#endif // MICROBIT_CODAL
} // namespace pxt

#include "MicroBit.h"
#include "MicroBitConfig.h"

#include "MbitMoreCommon.h"

#define UPDATE_PERIOD 11
#define NOTIFY_PERIOD 101

#if MICROBIT_CODAL
#include "MbitMoreService.h"
#else // MICROBIT_CODAL
#include "MbitMoreServiceDAL.h"
using MbitMoreService = MbitMoreServiceDAL;
#endif // NOT MICROBIT_CODAL

//% color=#FF9900 weight=95 icon="\uf1b0"
namespace MbitMore {
  MbitMoreService *_pService = NULL;

  int soundLevel() {
#if MICROBIT_CODAL
    auto level = pxt::getMicrophoneLevel();
    if (NULL == level)
      return 0;
    const int micValue = level->getValue();
    const int scaled = max(MICROPHONE_MIN, min(micValue, MICROPHONE_MAX)) - MICROPHONE_MIN;
    return min(0xff, scaled * 0xff / (MICROPHONE_MAX - MICROPHONE_MIN));
#else // NOT MICROBIT_CODAL
    target_panic(PANIC_VARIANT_NOT_SUPPORTED);
    return 0;
#endif // NOT MICROBIT_CODAL
  }

  void update() {
    while (NULL != _pService) {
      _pService->update();
#if MICROBIT_CODAL
      if (_pService->isMicInUse()) {
        _pService->setSoundLevel(soundLevel());
      }
#endif // MICROBIT_CODAL
      fiber_sleep(UPDATE_PERIOD);
    }
  }

  void notifyScratch() {
    while (NULL != _pService) {
      // notyfy data to Scratch
      _pService->notify();
      fiber_sleep(NOTIFY_PERIOD);
    }
  }

  /**
   * Starts a Scratch extension service.
   */
  //%
  void startMbitMoreService() {
    if (NULL != _pService)
      return;

    _pService = new MbitMoreService();

    create_fiber(update);
    create_fiber(notifyScratch);
  }

  /**
   * Events can have arguments before the handler
   */
  //%
  int call_registerWaitingMessage(String messageLabel) {
#if MICROBIT_CODAL
    if (NULL == _pService)
      return 0;

    int messageID = _pService->registerWaitingMessage(MSTR(messageLabel));
    return messageID;
#else // NOT MICROBIT_CODAL
    return 0;
#endif // NOT MICROBIT_CODAL
  }

  /**
   * @brief Get type of value for the message ID
   *
   * @param messageID
   * @return content type
   */
  //%
  MbitMoreMessageType call_messageType(int messageID) {
#if MICROBIT_CODAL
    return _pService->messageType(messageID);
#else // NOT MICROBIT_CODAL
    return MbitMoreMessageType::MM_MSG_NUMBER;
#endif // NOT MICROBIT_CODAL
  }

  //%
  float call_messageContentAsNumber(int messageID) {
#if MICROBIT_CODAL
    return _pService->messageContentAsNumber(messageID);
#else // NOT MICROBIT_CODAL
    return 0.0;
#endif // NOT MICROBIT_CODAL
  }

  //%
  String call_messageContentAsText(int messageID) {
#if MICROBIT_CODAL
    return PSTR(_pService->messageContentAsText(messageID));
#else // NOT MICROBIT_CODAL
    return String("");
#endif // NOT MICROBIT_CODAL
  }

  /**
   * Send a labeled message with content in float.
   * @param label - label of the message
   * @param content - number for content
   */
  //%
  void call_sendMessageWithNumber(String messageLabel, float messageContent) {
#if MICROBIT_CODAL
    if (NULL == _pService)
      return;

    _pService->sendMessageWithNumber(MSTR(messageLabel), messageContent);
#endif // MICROBIT_CODAL
  }

  /**
   * Send a labeled message with content in string.
   * @param label - label of the message
   * @param content - string for content
   */
  //%
  void call_sendMessageWithText(String messageLabel, String messageContent) {
#if MICROBIT_CODAL
    if (NULL == _pService)
      return;

    _pService->sendMessageWithText(MSTR(messageLabel), MSTR(messageContent));
#endif // MICROBIT_CODAL
  }

} // namespace MbitMore
