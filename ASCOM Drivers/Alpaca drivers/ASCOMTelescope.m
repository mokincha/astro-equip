%=======================================================================
%
%   Telescope class for communicating with ASCOM telescopes over Alpaca interface
%
%   ASCOM Platform: https://ascom-standards.org/Downloads/Index.htm
%   Alpaca Developer's page: https://ascom-standards.org/Developer/Alpaca.htm
%
%   (c) 2019 M. Okincha
%
%=======================================================================
classdef ASCOMTelescope < ASCOMDevice
   
    properties ( SetAccess = protected, GetAccess = public )
        
        % ASCOM Telescope properties
        alpaca_device_name;
    end

    % Read only
    properties ( Dependent = true, SetAccess = protected, GetAccess = public )
        
        % ASCOM Telescope properties
        alignment_mode;
        altitude;
        aperture_area;
        aperture_diameter;
        at_home;
        at_park;
        azimuth;
        can_find_home;
        can_park;
        can_pulse_guide;
        can_set_declination_rate;
        can_set_park;
        can_set_pier_side;
        can_set_right_ascension_rate;
        can_set_tracking;
        can_slew;
        can_slew_alt_az;
        can_slew_alt_az_async;
        can_slew_async;
        can_sync;
        can_sync_alt_az;
        declination;
        equatorial_system;
        focal_length;
        is_pulse_guiding;
        right_ascension;
        sidereal_time;
        slewing;
        tracking_rates;

    end
    
    % Read/Write
    properties ( Dependent = true, SetAccess = public, GetAccess = public )
        
        % ASCOM Telescope properties
        declination_rate;
        does_refraction;
        guide_rate_declination;
        guide_rate_right_ascension;
        right_ascension_rate;
        side_of_pier;
        site_elevation;
        site_latitude;
        site_longitude;
        slew_settle_time;
        target_declination;
        target_right_ascension;
        tracking;
        tracking_rate;
        utc_date;
        
    end


    methods
        % Constructor
        function obj = ASCOMTelescope

            % Call the base class constructor.  Not sure if this is
            % required
            @obj.ASCOMDevice;
            
            % Device that this is a telescope device
            obj.alpaca_device_name ='telescope';         % ASCOM-recognized device name

            % try connecting to the telescope
            try
                obj.Alpaca_Get( 'connected' );
            catch
                error( [ 'Telescope ' int2str( obj.alpaca_device_number ) ' not responding over ASCOM Alpaca HTTP interface' ] );
            end
            
        end
   end
   
   % Public methods
   methods
        
%         % Telescope Properties

        %-----------------------------------------------------
        %% GET.alignment_mode
        %-----------------------------------------------------
        function value = get.alignment_mode( obj )
            value = obj.Alpaca_Get(  'alignmentmode' );
        end

        %-----------------------------------------------------
        %% GET.altitude
        %-----------------------------------------------------
        function value = get.altitude( obj )
            value = obj.Alpaca_Get(  'altitude' );
        end

        %-----------------------------------------------------
        %% GET.aperture_area
        %-----------------------------------------------------
        function value = get.aperture_area( obj )
            value = obj.Alpaca_Get(  'aperturearea' );
        end

        %-----------------------------------------------------
        %% GET.aperture_diameter
        %-----------------------------------------------------
        function value = get.aperture_diameter( obj )
            value = obj.Alpaca_Get(  'aperturediameter' );
        end

        %-----------------------------------------------------
        %% GET.at_home
        %-----------------------------------------------------
        function value = get.at_home( obj )
            value = obj.Alpaca_Get(  'athome' );
        end

        %-----------------------------------------------------
        %% GET.at_park
        %-----------------------------------------------------
        function value = get.at_park( obj )
            value = obj.Alpaca_Get(  'atpark' );
        end

        %-----------------------------------------------------
        %% GET.azimuth
        %-----------------------------------------------------
        function value = get.azimuth( obj )
            value = obj.Alpaca_Get(  'azimuth' );
        end

        %-----------------------------------------------------
        %% GET.can_find_home
        %-----------------------------------------------------
        function value = get.can_find_home( obj )
            value = obj.Alpaca_Get(  'canfindhome' );
        end

        %-----------------------------------------------------
        %% GET.can_park
        %-----------------------------------------------------
        function value = get.can_park( obj )
            value = obj.Alpaca_Get(  'canpark' );
        end

        %-----------------------------------------------------
        %% GET.can_pulse_guide
        %-----------------------------------------------------
        function value = get.can_pulse_guide( obj )
            value = obj.Alpaca_Get(  'canpulseguide' );
        end

        %-----------------------------------------------------
        %% GET.can_set_declination_rate
        %-----------------------------------------------------
        function value = get.can_set_declination_rate( obj )
            value = obj.Alpaca_Get(  'cansetdeclinationrate' );
        end

        %-----------------------------------------------------
        %% GET.can_set_park
        %-----------------------------------------------------
        function value = get.can_set_park( obj )
            value = obj.Alpaca_Get(  'cansetpark' );
        end

        %-----------------------------------------------------
        %% GET.can_set_pier_side
        %-----------------------------------------------------
        function value = get.can_set_pier_side( obj )
            value = obj.Alpaca_Get(  'cansetpierside' );
        end

        %-----------------------------------------------------
        %% GET.can_set_right_ascension_rate
        %-----------------------------------------------------
        function value = get.can_set_right_ascension_rate( obj )
            value = obj.Alpaca_Get(  'cansetrightascensionrate' );
        end

        %-----------------------------------------------------
        %% GET.can_set_tracking
        %-----------------------------------------------------
        function value = get.can_set_tracking( obj )
            value = obj.Alpaca_Get(  'cansettracking' );
        end

        %-----------------------------------------------------
        %% GET.can_slew
        %-----------------------------------------------------
        function value = get.can_slew( obj )
            value = obj.Alpaca_Get(  'canslew' );
        end

        %-----------------------------------------------------
        %% GET.can_slew_alt_az
        %-----------------------------------------------------
        function value = get.can_slew_alt_az( obj )
            value = obj.Alpaca_Get(  'canslewaltaz' );
        end

        %-----------------------------------------------------
        %% GET.can_slew_alt_az_async
        %-----------------------------------------------------
        function value = get.can_slew_alt_az_async( obj )
            value = obj.Alpaca_Get( 'canslewaltazasync' );
        end

        %-----------------------------------------------------
        %% GET.can_slew_async
        %-----------------------------------------------------
        function value = get.can_slew_async( obj )
            value = obj.Alpaca_Get(  'canslewasync' );
        end

        %-----------------------------------------------------
        %% GET.can_sync
        %-----------------------------------------------------
        function value = get.can_sync( obj )
            value = obj.Alpaca_Get(  'cansync' );
        end

        %-----------------------------------------------------
        %% GET.can_sync_alt_az
        %-----------------------------------------------------
        function value = get.can_sync_alt_az( obj )
            value = obj.Alpaca_Get(  'cansyncaltaz' );
        end

        %-----------------------------------------------------
        %% GET.declination
        %-----------------------------------------------------
        function value = get.declination( obj )
            value = obj.Alpaca_Get(  'declination' );
        end

        %-----------------------------------------------------
        %% GET.equatorial_system
        %-----------------------------------------------------
        function value = get.equatorial_system( obj )
            value = obj.Alpaca_Get(  'equatorialsystem' );
        end

        %-----------------------------------------------------
        %% GET.focal_length
        %-----------------------------------------------------
        function value = get.focal_length( obj )
            value = obj.Alpaca_Get(  'focallength' );
        end

        %-----------------------------------------------------
        %% GET.is_pulse_guiding
        %-----------------------------------------------------
        function value = get.is_pulse_guiding( obj )
            value = obj.Alpaca_Get(  'ispulseguiding' );
        end

        %-----------------------------------------------------
        %% GET.right_ascension
        %-----------------------------------------------------
        function value = get.right_ascension( obj )
            value = obj.Alpaca_Get(  'rightascension' );
        end

        %-----------------------------------------------------
        %% GET.sidereal_time
        %-----------------------------------------------------
        function value = get.sidereal_time( obj )
            value = obj.Alpaca_Get(  'siderealtime' );
        end

        %-----------------------------------------------------
        %% GET.slewing
        %-----------------------------------------------------
        function value = get.slewing( obj )
            value = obj.Alpaca_Get(  'slewing' );
        end

        %-----------------------------------------------------
        %% GET.tracking_rates
        %-----------------------------------------------------
        function value = get.tracking_rates( obj )
            value = obj.Alpaca_Get(  'trackingrates' );
        end

        %============================================================
        % Read/Write
        %============================================================
    
        %-----------------------------------------------------
        %% GET.declination_rate
        %-----------------------------------------------------
        function value = get.declination_rate( obj )
            value = obj.Alpaca_Get(  'declinationrate' );
        end

        %-----------------------------------------------------
        %% SET.declination_rate
        %-----------------------------------------------------
       function obj = set.declination_rate( obj, rate )
            obj.Alpaca_Set(  'declinationrate', [ 'DeclinationRate=' num2str( rate, '%0.9f' ) ] );
        end
        
        %-----------------------------------------------------
        %% GET.does_refraction
        %-----------------------------------------------------
        function value = get.does_refraction( obj )
            value = obj.Alpaca_Get(  'doesrefraction' );
        end        

        %-----------------------------------------------------
        %% SET.position
        %-----------------------------------------------------
       function obj = set.does_refraction( obj, enable )
           
           if enable
               enable_str ='true';
           else
               enable_str ='false';
           end
           
            obj.Alpaca_Set(  'doesrefraction', [ 'DoesRefraction=' enable_str ] );
        end
        
        %-----------------------------------------------------
        %% GET.guide_rate_declination
        %-----------------------------------------------------
        function value = get.guide_rate_declination( obj )
            value = obj.Alpaca_Get(  'guideratedeclination' );
        end        
        
        %-----------------------------------------------------
        %% SET.guide_rate_declination
        %-----------------------------------------------------
       function obj = set.guide_rate_declination( obj, rate )
            obj.Alpaca_Set(  'guideratedeclination', [ 'GuideRateDeclination=' num2str( rate, '%0.9f' ) ] );
        end
        
        %-----------------------------------------------------
        %% GET.guide_rate_right_ascension
        %-----------------------------------------------------
        function value = get.guide_rate_right_ascension( obj )
            value = obj.Alpaca_Get(  'guideraterightascension' );
        end        
        
        %-----------------------------------------------------
        %% SET.guide_rate_ascension
        %-----------------------------------------------------
       function obj = set.guide_rate_right_ascension( obj, rate )
            obj.Alpaca_Set(  'guideraterightascension', [ 'GuideRateRightAscension=' num2str( rate, '%0.9f' ) ] );
        end
        
        %-----------------------------------------------------
        %% GET.right_ascension_rate
        %-----------------------------------------------------
        function value = get.right_ascension_rate( obj )
            value = obj.Alpaca_Get(  'rightascensionrate' );
        end        
        
        %-----------------------------------------------------
        %% SET.right_ascension_rate
        %-----------------------------------------------------
       function obj = set.right_ascension_rate( obj, rate )
            obj.Alpaca_Set(  'rightascensionrate', [ 'RightAscensionRate=' num2str( rate, '%0.9f' ) ] );
       end
       
       %-----------------------------------------------------
        %% GET.side_of_pier
        %-----------------------------------------------------
        function value = get.side_of_pier( obj )
            value = obj.Alpaca_Get(  'sideofpier' );
        end        
        
        %-----------------------------------------------------
        %% SET.side_of_pier
        %-----------------------------------------------------
       function obj = set.side_of_pier( obj, pier_side )
            obj.Alpaca_Set(  'sideofpier', [ 'SideOfPier=' int2str( pier_side ) ] );
        end
        
        %-----------------------------------------------------
        %% GET.site_elevation
        %-----------------------------------------------------
        function value = get.site_elevation( obj )
            value = obj.Alpaca_Get(  'siteelevation' );
        end        
        
        %-----------------------------------------------------
        %% SET.site_elevation
        %-----------------------------------------------------
       function obj = set.site_elevation( obj, elevation )
            obj.Alpaca_Set(  'site_elevation', [ 'SiteElevation=' num2str( elevation, '%0.9f' ) ] );
        end
        
        %-----------------------------------------------------
        %% GET.site_latitude
        %-----------------------------------------------------
        function value = get.site_latitude( obj )
            value = obj.Alpaca_Get(  'sitelatitude' );
        end        
        
        %-----------------------------------------------------
        %% SET.site_latitude
        %-----------------------------------------------------
       function obj = set.site_latitude( obj, latitude )
            obj.Alpaca_Set(  'site_latitude', [ 'SiteLatitude=' num2str( latitude, '%0.9f' ) ] );
        end
        
        %-----------------------------------------------------
        %% GET.site_longitude
        %-----------------------------------------------------
        function value = get.site_longitude( obj )
            value = obj.Alpaca_Get(  'sitelongitude' );
        end        
        
        %-----------------------------------------------------
        %% SET.site_longitude
        %-----------------------------------------------------
       function obj = set.site_longitude( obj, longitude )
            obj.Alpaca_Set(  'site_longitude', [ 'SiteLongitude=' num2str( longitude, '%0.9f' ) ] );
        end
        
        %-----------------------------------------------------
        %% GET.slew_settle_time
        %-----------------------------------------------------
        function value = get.slew_settle_time( obj )
            value = obj.Alpaca_Get(  'slewsettletime' );
        end        
        
        %-----------------------------------------------------
        %% SET.slew_settle_time
        %-----------------------------------------------------
       function obj = set.slew_settle_time( obj, longitude )
            obj.Alpaca_Set(  'slew_settle_time', [ 'SlewSettleTime=' num2str( longitude, '%0.9f' ) ] );
        end
        
        %-----------------------------------------------------
        %% GET.target_declination
        %-----------------------------------------------------
        function value = get.target_declination( obj )
            value = obj.Alpaca_Get(  'targetdeclination' );
        end        
        
        %-----------------------------------------------------
        %% SET.target_declination
        %-----------------------------------------------------
       function obj = set.target_declination( obj, target_declination )
            obj.Alpaca_Set(  'target_declination', [ 'TargetDeclination=' num2str( target_declination, '%0.9f' ) ] );
        end
        
        %-----------------------------------------------------
        %% GET.target_right_ascension
        %-----------------------------------------------------
        function value = get.target_right_ascension( obj )
            value = obj.Alpaca_Get(  'targetrightascension' );
        end        
        
        %-----------------------------------------------------
        %% SET.target_right_ascension
        %-----------------------------------------------------
       function obj = set.target_right_ascension( obj, target_right_ascension )
            obj.Alpaca_Set(  'targetrightascension', [ 'TargetRightAcension=' num2str( target_right_ascension, '%0.9f' ) ] );
        end
        
        %-----------------------------------------------------
        %% GET.tracking
        %-----------------------------------------------------
        function value = get.tracking( obj )
            value = obj.Alpaca_Get(  'tracking' );
        end        
        
        %-----------------------------------------------------
        %% SET.tracking
        %-----------------------------------------------------
       function obj = set.tracking( obj, enable )
           
           if enable
               enable_str ='true';
           else
               enable_str ='false';
           end
                      
           obj.Alpaca_Set(  'tracking', [ 'Tracking=' enable_str ] );
        end
        
        %-----------------------------------------------------
        %% GET.tracking_rate
        %-----------------------------------------------------
        function value = get.tracking_rate( obj )
            value = obj.Alpaca_Get(  'trackingrate' );
        end        
        
        %-----------------------------------------------------
        %% SET.tracking_rate
        %-----------------------------------------------------
       function obj = set.tracking_rate( obj, tracking_rate )
            obj.Alpaca_Set(  'tracking_rate', [ 'TrackingRate=' num2str( tracking_rate, '%0.9f' ) ] );
        end
        
        %-----------------------------------------------------
        %% GET.utc_date
        %-----------------------------------------------------
        function value = get.utc_date( obj )
            value = obj.Alpaca_Get(  'utcdate' );
        end        
        
        %-----------------------------------------------------
        %% SET.utc_date
        %-----------------------------------------------------
       function obj = set.utc_date( obj, date_str )
            obj.Alpaca_Set(  'utcdate', [ 'UTCDate=' date_str ] );
        end
        
        %============================================================
        % Write Only
        %============================================================
        
        %-----------------------------------------------------
        %% Abort_Slew
        %-----------------------------------------------------
        function obj = Abort_Slew( obj )
            obj.Alpaca_Set( 'abortslew', '' );
        end
        
        %-----------------------------------------------------
        %% Find_Home
        %-----------------------------------------------------
        function obj = Find_Home( obj )
            obj.Alpaca_Set( 'findhome', '', 45 );
        end
        
        %-----------------------------------------------------
        %% Park
        %-----------------------------------------------------
        function obj = Park( obj )
            obj.Alpaca_Set( 'park', '', 30 );
        end
        
        %-----------------------------------------------------
        %% Set_Park
        %-----------------------------------------------------
        function obj = Set_Park( obj )
            obj.Alpaca_Set( 'setpark' );
        end
        
        %-----------------------------------------------------
        %% Sync_To_Target
        %-----------------------------------------------------
        function obj = Sync_To_Target( obj )
            obj.Alpaca_Set( 'synctotarget', '' );
        end
        
        %-----------------------------------------------------
        %% Unpark
        %-----------------------------------------------------
        function obj = Unpark( obj )
            obj.Alpaca_Set( 'unpark', '' );
        end
        
        %-----------------------------------------------------
        %% Move_Axis
        %-----------------------------------------------------
        function obj = Move_Axis( obj, move_axis, move_rate )
            obj.Alpaca_Set( 'MoveAxis', [ 'Axis=' int2str(move_axis) '&Rate=' num2str( move_rate, '%0.9f' ) ], 45 );
        end
        
        %-----------------------------------------------------
        %% Pulse_Guide
        %-----------------------------------------------------
        function obj = Pulse_Guide( obj, direction, duration )
            obj.Alpaca_Set( 'pulseguide', [ 'Direction=' int2str( direction ) '&Duration=' int2str( duration ) ] );
        end
        
        %-----------------------------------------------------
        %% Slew_To_Alt_Az
        %-----------------------------------------------------
        function obj = Slew_To_Alt_Az( obj, alt, az )
            obj.Alpaca_Set( 'slewtoaltaz', [ 'Azimuth=' num2str( alt, '%0.9f' ) '&Altitude=' num2str( az, '%0.9f' ) ], 45 );
        end
        
        %-----------------------------------------------------
        %% Slew_To_Alt_Az_Sync
        %-----------------------------------------------------
        function obj = Slew_To_Alt_Az_Sync( obj, alt, az )
            obj.Alpaca_Set( 'slewtoaltazsync', [ 'Azimuth=' num2str( alt, '%0.9f' ) '&Altitude=' num2str( az, '%0.9f' ) ], 45 );
        end
        
        %-----------------------------------------------------
        %% Slew_To_Coordinates
        %-----------------------------------------------------
        function obj = Slew_To_Coordinates( obj, alt, az )
            obj.Alpaca_Set( 'slewtocoordinates', [ 'RightAscension=' num2str( alt, '%0.9f' ) '&Declination=' num2str( az, '%0.9f' ) ], 45 );
        end
        
        %-----------------------------------------------------
        %% Slew_To_Coordinates_Sync
        %-----------------------------------------------------
        function obj = Slew_To_Coordinates_Sync( obj, alt, az )
            obj.Alpaca_Set( 'slewtocoordinatessync', [ 'RightAscension=' num2str( alt, '%0.9f' ) '&Declination=' num2str( az, '%0.9f' ) ], 45 );
        end
        
        %-----------------------------------------------------
   end
end