%=======================================================================
%
%   Camera class for communicating with ASCOM cameras over Alpaca interface
%
%   ASCOM Platform: https://ascom-standards.org/Downloads/Index.htm
%   Alpaca Developer's page: https://ascom-standards.org/Developer/Alpaca.htm
%
%   (c) 2019 M. Okincha
%
%   Exmaple:
%       
%       cam = ASCOMCamera;      % Create camera object on server 127.0.0.1:11111
%       cam.camera_size_xy      % Get the default camera resolution
%       
%=======================================================================
classdef ASCOMCamera < ASCOMDevice
   
        % Inherited ASCOM Device properties
    properties ( SetAccess = protected, GetAccess = public )
        
        alpaca_device_name;         % ASCOM-recognized device name
    end        

    properties ( Dependent = true )
        
        % ASCOM Camera properties
        bin_xy;
        cooler_on;
        fast_readout;
        gain;
        num_xy;
        readout_mode;
        ccd_temperature_setpoint;
        start_xy;
        
        % Convenient properties
        integration_time;
        
    end
    
    % Read only properties
    properties ( Dependent = true, SetAccess = protected, GetAccess = public )
        
        % ASCOM Camera properties
        bayer_offset_xy;
        camera_size_xy;
        can_abort_exposure;
        can_asymmetric_bin;
        can_fast_readout;
        can_pulse_guide;
        can_stop_exposure;
        ccd_temperature;
        cooler_power_level;
        electrons_per_DN;
        exposure_max;
        exposure_min;
        exposure_resolution;
        full_well_capacity;
        gain_max;
        gain_min;
        gains;
        has_shutter
        heatsink_temperature;
        image;
        is_pulse_guiding;
        last_exposure_duration;
        last_exposure_start_time;
        max_adu;
        max_bin;
        percent_completed;
        readout_modes;
        sensor_name;
        state;

    end
        
    % Internal properties
    properties ( Access = public )
        is_light_frame;
    end    

    properties ( Access = protected )
        integration_time_current;
    end    
    
    % Public Methods
    methods
        % Constructor
        function obj = ASCOMCamera

            % Call the base class constructor.  Not sure if this is
            % required
            @obj.ASCOMDevice;
            
            % Device that this is a camera device
            obj.alpaca_device_name = 'camera';         % ASCOM-recognized device name

            % try connecting to the camera
            try
                obj.Alpaca_Get( 'connected' );
            catch
                error( [ 'Camera ' int2str( obj.alpaca_device_number ) ' not responding over ASCOM Alpaca HTTP interface' ] );
            end
            
            
            % Set default parameters
            obj.integration_time_current = 1.0;     % default to 1.0 sec.
            obj.is_light_frame = true;
        end
   end
   
   % Public methods
   methods
        
        %-----------------------------------------------------
        % Camera Properties
        %-----------------------------------------------------

        %-----------------------------------------------------
        %% SET.integration_time
        %-----------------------------------------------------
        function obj = set.integration_time( obj, value )
            
            exp_max = obj.exposure_max;
            exp_min = obj.exposure_min;
            if value > exp_max
                error( [ 'Exposure time of ' num2str( value, '%i' ) ' sec too long.  Must be less than ' num2str( exp_max, '%i' ) ' sec' ] );
            end
            if value < exp_min
                error( [ 'Exposure time of ' num2str( value, '%i' ) ' sec too short.  Must be greater than ' num2str( exp_min, '%i' ) ' sec' ] );
            end
           
            % integration time is in sec
            obj.integration_time_current = value;
            
        end
   
        %-----------------------------------------------------
        %% GET.integration_time
        %-----------------------------------------------------
        function value = get.integration_time( obj )
            
            % integration time is in seconds
            value = obj.integration_time_current;
        end

       
        % Camera-specific READ-ONLY values
       
        %-----------------------------------------------------
        %% GET.bayer_offset_xy
        %-----------------------------------------------------
        function value = get.bayer_offset_xy( obj )
            value(1) = obj.Alpaca_Get( 'bayeroffsetx' );
            value(2) = obj.Alpaca_Get( 'bayeroffsety' );
        end
        
       
        %-----------------------------------------------------
        %% GET.camera_size_xy
        %-----------------------------------------------------
        function value = get.camera_size_xy( obj )
            value(1) = obj.Alpaca_Get( 'cameraxsize' );
            value(2) = obj.Alpaca_Get( 'cameraysize' );
        end
        
       
        
        %-----------------------------------------------------
        %% GET.state
        %-----------------------------------------------------
        function value = get.state( obj )
            value = obj.Alpaca_Get( 'camerastate' );
        end
        
       
        %-----------------------------------------------------
        %% GET.can_abort_exposure
        %-----------------------------------------------------
        function value = get.can_abort_exposure( obj )
            value = obj.Alpaca_Get( 'canabortexposure' );
        end
        
       
        %-----------------------------------------------------
        %% GET.can_asymmetric_bin
        %-----------------------------------------------------
        function value = get.can_asymmetric_bin( obj )
            value = obj.Alpaca_Get( 'canasymmetricbin' );
        end
        
       
        %-----------------------------------------------------
        %% GET.can_fast_readout
        %-----------------------------------------------------
        function value = get.can_fast_readout( obj )
            value = obj.Alpaca_Get( 'canfastreadout' );
        end
        
       
        %-----------------------------------------------------
        %% GET.can_pulse_guide
        %-----------------------------------------------------
        function value = get.can_pulse_guide( obj )
            value = obj.Alpaca_Get( 'canpulseguide' );
        end
        
       
        %-----------------------------------------------------
        %% GET.can_stop_exposure
        %-----------------------------------------------------
        function value = get.can_stop_exposure( obj )
            value = obj.Alpaca_Get( 'canstopexposure' );
        end
        
       
        %-----------------------------------------------------
        %% GET.ccd_temperature
        %-----------------------------------------------------
        function value = get.ccd_temperature( obj )
            value = obj.Alpaca_Get( 'ccdtemperature' );
        end
        
       
        %-----------------------------------------------------
        %% GET.cooler_power_level
        %-----------------------------------------------------
        function value = get.cooler_power_level( obj )
            value = obj.Alpaca_Get( 'coolerpower' );
        end
        
       
        %-----------------------------------------------------
        %% GET.electrons_per_DN
        %-----------------------------------------------------
        function value = get.electrons_per_DN( obj )
            value = obj.Alpaca_Get( 'electrons_per_DN' );
        end
        
       
        %-----------------------------------------------------
        %% GET.exposure_max
        %-----------------------------------------------------
        function value = get.exposure_max( obj )
            value = obj.Alpaca_Get( 'exposuremax' );
        end
        
       
        %-----------------------------------------------------
        %% GET.exposure_min
        %-----------------------------------------------------
        function value = get.exposure_min( obj )
            value = obj.Alpaca_Get( 'exposuremin' );
        end
        
       
        %-----------------------------------------------------
        %% GET.exposure_resolution
        %-----------------------------------------------------
        function value = get.exposure_resolution( obj )
            value = obj.Alpaca_Get( 'exposureresolution' );
        end
        
       
        %-----------------------------------------------------
        %% GET.full_well_capacity
        %-----------------------------------------------------
        function value = get.full_well_capacity( obj )
            value = obj.Alpaca_Get( 'fullwellcapacity' );
        end
        
        
        %-----------------------------------------------------
        %% GET.gain_max
        %-----------------------------------------------------
        function value = get.gain_max( obj )
            value = obj.Alpaca_Get( 'gainmax' );
        end
        
        
        %-----------------------------------------------------
        %% GET.gain_min
        %-----------------------------------------------------
        function value = get.gain_min( obj )
            value = obj.Alpaca_Get( 'gainmin' );
        end
        
        
        %-----------------------------------------------------
        %% GET.gains
        %-----------------------------------------------------
        function value = get.gains( obj )
            value = obj.Alpaca_Get( 'gains' );
        end

        
        %-----------------------------------------------------
        %% GET.has_shutter
        %-----------------------------------------------------
        function value = get.has_shutter( obj )
            value = obj.Alpaca_Get( 'hasshutter' );
        end
        
        
        %-----------------------------------------------------
        %% GET.heatsink_temperature
        %-----------------------------------------------------
        function value = get.heatsink_temperature( obj )
            value = obj.Alpaca_Get( 'heatsinktemperature' );
        end
        
        
        %-----------------------------------------------------
        %% GET.image
        %-----------------------------------------------------
        function im = get.image( obj )
            im = obj.Alpaca_Get( 'imagearray' );
        end
        
        
        %-----------------------------------------------------
        %% GET.is_pulse_guiding
        %-----------------------------------------------------
        function value = get.is_pulse_guiding( obj )
            value = obj.Alpaca_Get( 'ispulseguiding' );
        end
        
        
        %-----------------------------------------------------
        %% GET.last_exposure_duration
        %-----------------------------------------------------
        function value = get.last_exposure_duration( obj )
            value = obj.Alpaca_Get( 'lastexposureduration' );
        end
        
        
        %-----------------------------------------------------
        %% GET.last_exposure_start_time
        %-----------------------------------------------------
        function value = get.last_exposure_start_time( obj )
            value = obj.Alpaca_Get( 'lastexposurestarttime' );
        end
        
        
        %-----------------------------------------------------
        %% GET.max_adu
        %-----------------------------------------------------
        function value = get.max_adu( obj )
            value = obj.Alpaca_Get( 'maxadu' );
        end
        
        
        %-----------------------------------------------------
        %% GET.max_bin
        %-----------------------------------------------------
        function value = get.max_bin( obj )
            value(1) = obj.Alpaca_Get( 'maxbinx' );
            value(2) = obj.Alpaca_Get( 'maxbiny' );
        end
        
        
        %-----------------------------------------------------
        %% GET.percent_completed
        %-----------------------------------------------------
        function value = get.percent_completed( obj )
            value = obj.Alpaca_Get( 'percentcompleted' );
        end
        
        
        %-----------------------------------------------------
        %% GET.readout_modes
        %-----------------------------------------------------
        function value = get.readout_modes( obj )
            value = obj.Alpaca_Get( 'readoutmodes' );
        end
        
        
        %-----------------------------------------------------
        %% GET.sensor_name
        %-----------------------------------------------------
        function value = get.sensor_name( obj )
            value = obj.Alpaca_Get( 'sensorname' );
        end
        
        
        % Camera-specific Read/write properties
        
        %-----------------------------------------------------
        %% SET.bin_xy
        %-----------------------------------------------------
        function obj = set.bin_xy( obj, xy )
            obj.AlpacaSet( 'binx', [ 'BinX=' int2str( xy(1) ) ] );
            obj.AlpacaSet( 'biny', [ 'BinY=' int2str( xy(2) ) ] );
        end
        
        
        %-----------------------------------------------------
        %% GET.bin_xy
        %-----------------------------------------------------
        function value = get.bin_xy( obj )
            value(1) = obj.Alpaca_Get( 'binx' );
            value(2) = obj.Alpaca_Get( 'biny' );
        end
        

       %-----------------------------------------------------
        %% SET.cooler_on
        %-----------------------------------------------------
        function obj = set.cooler_on( obj, power_state )
            if power_state == 0
                state_str = 'false';
            else
                state_str = 'true';
            end
            obj.AlpacaSet( 'cooleron', [ 'CoolerOn=' state_str ] );
        end
        
       
        %-----------------------------------------------------
        %% GET.cooler_on
        %-----------------------------------------------------
        function value = get.cooler_on( obj )
            value = obj.Alpaca_Get( 'cooleron' );
        end
        
       
        %-----------------------------------------------------
        %% SET.fast_readout
        %-----------------------------------------------------
        function obj = set.fast_readout( obj, state )
            if state == 0
                state_str = 'false';
            else
                state_str = 'true';
            end
            obj.AlpacaSet( 'fastreadout', [ 'FastReadout=' state_str ] );
        end
        
       
        %-----------------------------------------------------
        %% GET.fast_readout
        %-----------------------------------------------------
        function value = get.fast_readout( obj )
            value = obj.Alpaca_Get( 'fastreadout' );
        end
        
       
        %-----------------------------------------------------
        %% SET.gain
        %-----------------------------------------------------
        function obj = set.gain( obj, gain )
            obj.AlpacaSet( 'gain', [ 'Gain=' int2str( gain ) ] );
        end
        
       
        %-----------------------------------------------------
        %% GET.gain
        %-----------------------------------------------------
        function value = get.gain( obj )
            value = obj.Alpaca_Get( 'gain' );
        end
        
       
        %-----------------------------------------------------
        %% SET.num_xy
        %-----------------------------------------------------
        function obj = set.num_xy( obj, xy )
            obj.AlpacaSet( 'numx', [ 'NumX=' int2str( xy(1) ) ] );
            obj.AlpacaSet( 'numy', [ 'NumY=' int2str( xy(2) ) ] );
        end
        
       
        %-----------------------------------------------------
        %% GET.num_xy
        %-----------------------------------------------------
        function value = get.num_xy( obj )
            value(1) = obj.Alpaca_Get( 'numx' );
            value(2) = obj.Alpaca_Get( 'numy' );
        end
        
       
        %-----------------------------------------------------
        %% SET.readout_mode
        %-----------------------------------------------------
        function obj = set.readout_mode( obj, mode )
            obj.AlpacaSet( 'readoutmode', [ 'ReadoutMode=' int2str( mode ) ] );
        end
        
       
        %-----------------------------------------------------
        %% GET.readout_mode
        %-----------------------------------------------------
        function value = get.readout_mode( obj )
            value = obj.Alpaca_Get( 'readoutmode' );
        end
        
       
        %-----------------------------------------------------
        %% SET.ccd_temperature_setpoint
        %-----------------------------------------------------
        function obj = set.ccd_temperature_setpoint( obj, temp )
            obj.AlpacaSet( 'setccdtemperature', [ 'SetCCDTemperature=' num2str( temp, '%0.1f' ) ] );
        end
        
       
        %-----------------------------------------------------
        %% GET.ccd_temperature_setpoint
        %-----------------------------------------------------
        function value = get.ccd_temperature_setpoint( obj )
            value = obj.Alpaca_Get( 'setccdtemperature' );
        end
                
       
        %-----------------------------------------------------
        %% SET.start_xy
        %-----------------------------------------------------
        function obj = set.start_xy( obj, xy )
            obj.AlpacaSet( 'startx', [ 'StartX=' int2str( xy(1) ) ] );
            obj.AlpacaSet( 'starty', [ 'StartY=' int2str( xy(2) ) ] );
        end
        
       
        %-----------------------------------------------------
        %% GET.start_xy
        %-----------------------------------------------------
        function value = get.start_xy( obj )
            value(1) = obj.Alpaca_Get( 'startx' );
            value(2) = obj.Alpaca_Get( 'starty' );
        end
              
       
        %-----------------------------------------------------
        %% Start_Exposure
        %-----------------------------------------------------
        function Start_Exposure( obj )

            if isempty( obj.integration_time_current ) 
                fprintf( 'Integration time is empty.  Needs to be set before starting exposure.' );
            end
            
            exp_min = obj.exposure_min;
            if obj.integration_time_current < exp_min 
                fprintf( [ 'Integration time must be longer than ' num2str( exp_min, '%0.3f' ) ' sec\n' ] );
            end

            exp_max = obj.exposure_max;
            if obj.integration_time_current > exp_max 
                fprintf( [ 'Integration time must be less than ' num2str( exp_max, '%0.3f' ) ' sec\n' ] );
            end
            
            % Wait for camera to be ready
            camera_state = obj.Alpaca_Get( 'camerastate' );
            while  camera_state ~= 0
                operation_percent_completed = obj.Alpaca_Get( 'percentcompleted' );
                switch camera_state
                    case 1
                        fprintf( ['Camera is waiting: ' int2str( operation_percent_completed ) '% complete.\n' ] );
                    case 2
                        fprintf( ['Camera is exposing: ' int2str( operation_percent_completed ) '% complete.\n' ] );
                    case 3
                        fprintf( ['Camera is reading out: ' int2str( operation_percent_completed ) '% complete.\n' ] );
                    case 4
                        fprintf( ['Camera is downloading: ' int2str( operation_percent_completed ) '% complete.\n' ] );
                    case 5
                        fprintf( ['Camera error\n' ]);
                end
                camera_state = obj.Alpaca_Get( 'camerastate' );
            end

            if obj.is_light_frame
                light_frame_str = 'true';
            else
                light_frame_str = 'false';
            end
            % start the esposure
            obj.Alpaca_Set( 'startexposure', [ 'Light=' light_frame_str '&Duration=' num2str( obj.integration_time_current, '%0.3f' ) ] );

            % Wait for the image capture to complete.  Some applications
            % might not want the capture to be blocking
            while obj.Alpaca_Get( 'imageready' ) == false
                fprintf( ['Exposure ' int2str( obj.percent_completed ) '%% complete.\n' ]);
            end

            % Once the capture is complete, downalod the image from the 
            %   'image' property.
        end

   end
   
end