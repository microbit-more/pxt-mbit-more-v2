// tests go here; this will not be compiled when this package is used as an extension.
MbitMore.onReceivedTextWithLabel("label-01", function (content) {
  MbitMore.sendTextWithLabel("label-01", content);
  // basic.showString("label-01 = T " + (content))
})
MbitMore.onReceivedNumberWithLabel("label-01", function (content) {
  MbitMore.sendNumberWithLabel("label-01", content);
  // basic.showString("label-01 = N " + (content))
})
MbitMore.startService()
