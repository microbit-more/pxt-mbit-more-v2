namespace MbitMore {
  const MBIT_MORE_DATA_RECEIVED = 8000;

  /**
  * Starts BLE services for Scratch Microbit-More extension.
  */
  //% blockId=MbitMore_startMbitMoreService block="start Microbit-More service"
  //% shim=MbitMore::startMbitMoreService
  export function startService(): void {
    console.log("Microbit-More started");
  }

  /**
  * Register a label and return its ID.
  */
  //% shim=MbitMore::call_registerWaitingDataLabel
  export function registerWaitingDataLabel(label: string, type: MbitMoreDataContentType): number {
    console.log("Microbit-More registered label: " + label);
    return 1; // dummy for sim
  }

  /**
   * Read received data as a number
   */
  //% shim=MbitMore::call_dataContentAsNumber
  export function dataContentAsNumber(labelID: number): number {
    return 0.0; // dummy for sim
  }

  /**
   * Read received data as a text
   */
  //% shim=MbitMore::call_dataContentAsText
  export function dataContentAsText(labelID: number): string {
    return "text"; // dummy for sim
  }

  /**
   * Run blocks with data when a number data with the label is received.
   * @param label - label of the data
   * @param handler - blocks to run
   */
  //% blockId=MbitMore_onReceivedNumberWithLabel
  //% block="on number $numberData with label $label"
  //% label.defl="label-01"
  //% draggableParameters
  export function onReceivedNumberWithLabel(label: string, handler: (numberData: number) => void) {
    let labelID = MbitMore.registerWaitingDataLabel(label, MbitMoreDataContentType.MM_DATA_NUMBER);
    if (0 === labelID) {
      throw "max waiting label counts exceed";
    }
    control.onEvent(MBIT_MORE_DATA_RECEIVED, labelID, function () {
      handler(MbitMore.dataContentAsNumber(labelID));
      return;
    });
  }

  /**
   * Run blocks with data when a text data with the label is received.
   * @param label - label of the data
   * @param handler - blocks to run
   */
  //% blockId=MbitMore_onReceivedTextWithLabel
  //% block="on text $textData with label $label"
  //% label.defl="label-01"
  //% draggableParameters
  export function onReceivedTextWithLabel(label: string, handler: (textData: string) => void) {
    let labelID = MbitMore.registerWaitingDataLabel(label, MbitMoreDataContentType.MM_DATA_TEXT);
    if (0 === labelID) {
      throw "max waiting label counts exceed";
    }
    control.onEvent(MBIT_MORE_DATA_RECEIVED, labelID, function () {
      handler(MbitMore.dataContentAsText(labelID));
      return;
    });
  }

  /**
   * Send number with label
   * @param label lavel of the data 
   * @param data number value to send
   */
  //% blockId=MbitMore_sendNumberWithLabel
  //% block="send number $numberData with label $label"
  //% shim=MbitMore::call_sendNumberWithLabel
  //% label.defl="label-01"
  //% numberData.defl=0.0
  export function sendNumberWithLabel(label: string, numberData: number): void {
    console.log("Microbit-More send a number: " + label + " = " + numberData);
  }

  /**
   * Send text with label
   * @param label lavel of the data
   * @param data text to send
   */
  //% blockId=MbitMore_sendTextWithLabel
  //% block="send text $textData with label $label"
  //% shim=MbitMore::call_sendTextWithLabel
  //% label.defl="label-01"
  //% textData.defl="text"
  export function sendTextWithLabel(label: string, textData: string): void {
    console.log("Microbit-More send a text: " + label + " = " + textData);
  }

} // namespace MbitMore