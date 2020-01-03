(c) 2019 M. Okincha

This package includes drivers for communicating with ASCOM devices over the Alpaca HTP interface. 
 
ASCOM (See https://ascom-standards.org) is a cross-platform protocol for communicating with astronomy equipment - cameras, telescopes, domes, filter wheels, etc.  Alpaca (https://ascom-standards.org/Developer/Alpaca.htm) is a HTTP-based protocol build on the ASCOM libraries.  

These files implement a base class (ASCOMDdriver) and derived classes for communicating with generic camera, telescope, filter wheel, and focuser devices.  The drivers can be easily extended to support other ASCOM devices such as safety controllers, domes, etc.

To use these drivers you must install the ASCOM platform and Alpaca (ASCOM Remote) packages.  Run and configure the ASCOM Remote server with your devices.  

Download the ASCOM platform here: https://ascom-standards.org/
Download the ASCOM Remote package here: https://ascom-standards.org/Developer/Alpaca.htm

Go to https://ascom-standards.org/api/ to find what devices are supported and the meaning of the API methods.

Examples:

cam = ASCOMCamera;				% Create a camera object
cam.ccd_temperature				% get the CCD temperature
cam.camera_size_xy				% Get the camera's native resolution
cam.integration_time = 10;		% set a 10 second exposure time
cam.Start_Exposure;				% Trigger a new image capture and for it to complete
im = cam.image;					% download the new image

telescope = ASCOMTelescope;		% create a telescope object
telescope.Unpark;				% Unpark the telescope
telescope.Slew_To_Alt_Az( 90, 0 );		% move to  celestial north
telescope.site_elevation = 100;			% Set site elevation to 100 m


The MATLAB driver assumes your Alpaca server is running on the local host.  You can change the host IP address and port as required:

telescope.host_string = 'https://virtserver.swaggerhub.com/ASCOMInitiative';	% Change host to the ASCOM Alpaca public testing server
telescope.host_string = 'http://127.0.0.1:11111';		% Default server and port on local machine

