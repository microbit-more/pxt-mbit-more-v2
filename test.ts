// tests go here; this will not be compiled when this package is used as an extension.
MbitMore.startService()
MbitMore.onMessage("label-01", function (content) {
  MbitMore.sendMessage("label-01", content);
  basic.showString("label-01 = " + (content))
})