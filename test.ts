// tests go here; this will not be compiled when this package is used as an extension.
MbitMore.startService()
input.onButtonPressed(Button.A, function () {
  MbitMore.setSharedData(SharedDataIndex.Data7, MbitMore.getSharedData(SharedDataIndex.Data0))
})
input.onButtonPressed(Button.B, function () {
  MbitMore.setSharedData(SharedDataIndex.Data0, MbitMore.getSharedData(SharedDataIndex.Data7))
})
