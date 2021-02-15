// tests go here; this will not be compiled when this package is used as an extension.
MbitMore.onMessageWithText("label-01", function (content) {
  MbitMore.sendMessageWithText("label-01", content);
  // basic.showString("label-01 = " + (content))
})
MbitMore.onMessageWithNumber("label-01", function (content) {
  MbitMore.sendMessageWithNumber("label-01", content);
  // basic.showString("label-01 = N " + (content))
})
MbitMore.startService()
