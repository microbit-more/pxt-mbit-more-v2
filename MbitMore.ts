namespace MbitMore {
  const MBIT_MORE_MESSAGE = 8000;

  /**
  * Starts BLE services for Scratch Microbit-More extension.
  */
  //% blockId=MbitMore_startMbitMoreService block="start Microbit-More service"
  //% shim=MbitMore::startMbitMoreService
  export function startService(): void {
    console.log("Microbit-More started");
  }


  /**
   * @brief Get type of value for the message ID
   *
   * @param messageID
   * @return content type
   */
  //% shim=MbitMore::call_messageType
  export function messageType(messageID: number): number {
    console.log("Microbit-More message value type: text ");
    return MbitMoreMessageType.MM_MSG_TEXT;
  }

  /**
  * Register message label and return ID
  */
  //% shim=MbitMore::call_registerWaitingMessage
  export function registerWaitingMessage(label: string): number {
    console.log("Microbit-More registered message: " + label);
    return 0; // dummy for sim
  }

  /**
   * Read message as number
   */
  //% shim=MbitMore::call_messageContentAsNumber
  export function messageContentAsNumber(messageID: number): number {
    console.log("Microbit-More read number message");
    return 0.1; // dummy for sim
  }

  /**
   * Read message as text
   */
  //% shim=MbitMore::call_messageContentAsText
  export function messageContentAsText(messageID: number): string {
    console.log("Microbit-More read test message");
    return "text"; // dummy for sim
  }

  /**
   * Run blocks with content when a message with the label is received.
   * @param label - label of the message
   * @param handler - blocks to run
   */
  //% blockId=MbitMore_onMessage 
  //% block="on message $label with $content"
  //% draggableParameters
  export function onMessage(label: string, handler: (content: number | string) => void) {
    let messageID = MbitMore.registerWaitingMessage(label);
    control.onEvent(MBIT_MORE_MESSAGE, messageID, function () {
      let contentType = MbitMore.messageType(messageID);
      if (contentType === MbitMoreMessageType.MM_MSG_TEXT) {
        handler(MbitMore.messageContentAsText(messageID));
      } else {
        handler(MbitMore.messageContentAsNumber(messageID));
      }
      return;
    });
  }

  //% shim=MbitMore::call_sendMessageWithNumber
  export function sendMessageWithNumber(label: string, content: number): void {
    console.log("Microbit-More send a number message: " + label + " = " + content);
  }

  //% shim=MbitMore::call_sendMessageWithText
  export function sendMessageWithText(label: string, content: string): void {
    console.log("Microbit-More send a text message: " + label + " = " + content);
  }

  /**
   * Send a labeled message with content.
   * @param label - label of the message
   * @param content - content of the message [number | string]
   */
  //% blockId=MbitMore_sendMessage
  //% block="send message $label with $content"
  export function sendMessage(label: string, content: number | string): void {
    if (typeof content === 'number') {
      sendMessageWithNumber(label, content);
    } else {
      sendMessageWithText(label, content);
    }
  }

} // namespace MbitMore