// tests go here; this will not be compiled when this package is used as an extension.
MbitMore.startService()
basic.forever(function () {
  MbitMore.setSharedData(SharedDataIndex.DATA0, input.compassHeading())
})
