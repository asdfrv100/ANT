var ocf = require('ocf');
console.log('OCF server example app');

var oa = ocf.getAdapter();
oa.onInitialize(function() {
  console.log('onInitialize()');
  oa.setPlatform('ant');
  oa.addDevice('/oic/d', 'oic.d.light', 'Light', 'ocf.1.0.0', 'ocf.res.1.0.0');
}); 

var g_light_state = false;
oa.onPrepareServer(function() {
  console.log('onPrepareServer()');
  device = oa.getDevice(0);
  var lightRes = ocf.createResource(
      device, 'lightbulb', '/light/1', ['oic.r.light'], [ocf.OC_IF_RW]);
  lightRes.setDiscoverable(true);
  lightRes.setPeriodicObservable(1);
  lightRes.setHandler(ocf.OC_GET, getLightHandler);
  oa.addResource(lightRes);
});

function getLightHandler(request, method) {
  oa.repStartRootObject();
  oa.repSet('light', g_light_state);
  g_light_state = !g_light_state;
  oa.repEndRootObject();
  oa.sendResponse(request, ocf.OC_STATUS_OK);
}

oa.start();
setTimeout(function() {
  console.log('10s elapsed');
  oa.stop();
}, 10000);