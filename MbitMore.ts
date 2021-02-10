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
  * Register message label and return ID
  */
  //% shim=MbitMore::call_registerWaitingMessage
  export function registerWaitingMessage(label: string, type: MbitMoreMessageType): number {
    console.log("Microbit-More registered message: " + label);
    return 1; // dummy for sim
  }

  /**
   * Read message as number
   */
  //% shim=MbitMore::call_messageContentAsNumber
  export function messageContentAsNumber(messageID: number): number {
    console.log("Microbit-More read number message");
    return -0.1; // dummy for sim
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
   * Run blocks with number content when a message with the label is received.
   * @param label - label of the message
   * @param handler - blocks to run
   */
  //% blockId=MbitMore_onMessageWithNumber
  //% block="on message $label with number $content"
  //% label.defl="label-01"
  //% draggableParameters
  export function onMessageWithNumber(label: string, handler: (content: number) => void) {
    let messageID = MbitMore.registerWaitingMessage(label, MbitMoreMessageType.MM_MSG_NUMBER);
    if (0 === messageID) {
      throw "waiting message size exceed";
    }
    control.onEvent(MBIT_MORE_MESSAGE, messageID, function () {
      handler(MbitMore.messageContentAsNumber(messageID));
      return;
    });
  }

  /**
   * Run blocks with text content when a message with the label is received.
   * @param label - label of the message
   * @param handler - blocks to run
   */
  //% blockId=MbitMore_onMessageWithText
  //% block="on message $label with text $content"
  //% label.defl="label-01"
  //% draggableParameters
  export function onMessageWithText(label: string, handler: (content: string) => void) {
    let messageID = MbitMore.registerWaitingMessage(label, MbitMoreMessageType.MM_MSG_TEXT);
    if (0 === messageID) {
      throw "waiting message size exceed";
    }
    control.onEvent(MBIT_MORE_MESSAGE, messageID, function () {
      handler(MbitMore.messageContentAsText(messageID));
      return;
    });
  }

  /**
   * Send laveled message with number content
   * @param label lavel of the message 
   * @param content content of the message
   */
  //% blockId=MbitMore_sendMessageWithNumber
  //% block="send message $label with number $content"
  //% shim=MbitMore::call_sendMessageWithNumber
  //% label.defl="label-01"
  //% content.defl=0.0
  export function sendMessageWithNumber(label: string, content: number): void {
    console.log("Microbit-More send a number message: " + label + " = " + content);
  }

  /**
   * Send laveled message with text content
   * @param label lavel of the message
   * @param content content of the message
   */
  //% blockId=MbitMore_sendMessageWithText
  //% block="send message $label with text $content"
  //% shim=MbitMore::call_sendMessageWithText
  //% label.defl="label-01"
  //% content.defl="..11 length"
  export function sendMessageWithText(label: string, content: string): void {
    console.log("Microbit-More send a text message: " + label + " = " + content);
  }

} // namespace MbitMore