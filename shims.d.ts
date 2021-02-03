// Auto-generated. Do not edit.



    //% color=#FF9900 weight=95 icon="\uf1b0"
declare namespace MbitMore {

    /**
     * Starts a Scratch extension service.
     */
    //% shim=MbitMore::startMbitMoreService
    function startMbitMoreService(): void;

    /**
     * Events can have arguments before the handler
     */
    //% shim=MbitMore::call_registerWaitingMessage
    function call_registerWaitingMessage(messageLabel: string): int32;

    /**
     * @brief Get type of value for the message ID
     *
     * @param messageID
     * @return content type
     */
    //% shim=MbitMore::call_messageType
    function call_messageType(messageID: int32): MbitMoreMessageType;

    /**
     * Send a labeled message with content in float.
     * @param label - label of the message
     * @param content - number for content
     */
    //% shim=MbitMore::call_sendMessageWithNumber
    function call_sendMessageWithNumber(messageLabel: string, messageContent: number): void;

    /**
     * Send a labeled message with content in string.
     * @param label - label of the message
     * @param content - string for content
     */
    //% shim=MbitMore::call_sendMessageWithText
    function call_sendMessageWithText(messageLabel: string, messageContent: string): void;
}

// Auto-generated. Do not edit. Really.
