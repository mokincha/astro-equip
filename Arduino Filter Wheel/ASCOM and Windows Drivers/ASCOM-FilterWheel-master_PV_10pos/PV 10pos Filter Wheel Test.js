//*** CHECK THIS ProgID ***
var X = new ActiveXObject("ASCOM.PV 10pos Filter Wheel.FilterWheel");
WScript.Echo("This is " + X.Name + ")");
// You may want to uncomment this...
// X.Connected = true;
X.SetupDialog();
