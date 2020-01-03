%=======================================================================
%
%   FilterWheel class for communicating with ASCOM filterwheels over Alpaca interface
%
%   ASCOM Platform: https://ascom-standards.org/Downloads/Index.htm
%   Alpaca Developer's page: https://ascom-standards.org/Developer/Alpaca.htm
%
%   (c) 2019 M. Okincha
%
%=======================================================================
classdef ASCOMFilterWheel < ASCOMDevice
   
    properties ( SetAccess = protected, GetAccess = public )
        
        % ASCOM FilterWheel properties
        alpaca_device_name;
    end
    
    % Read only
    properties ( Dependent = true, SetAccess = protected, GetAccess = public )
        
        % ASCOM FilterWheel properties
        focus_offsets;
        names;

    end
    
    % Read/Write
    properties ( Dependent = true, SetAccess = public, GetAccess = public )
        
        % ASCOM FilterWheel properties
        position;

    end
    
   methods
        % Constructor
        function obj = ASCOMFilterWheel

            % Call the base class constructor.  Not sure if this is
            % required
            @obj.ASCOMDevice;
            
            % Device that this is a filter wheel device
            obj.alpaca_device_name = 'filterwheel';         % ASCOM-recognized device name

            % try connecting to the filter wheel
            try
                obj.Alpaca_Get( 'connected' );
            catch
                error( [ 'Filter Wheel ' int2str( obj.alpaca_device_number ) ' not responding over ASCOM Alpaca HTTP interface' ] );
            end
            
        end
   end
   
   % Public methods
   methods
        
%         % Filter Wheel Properties

        %-----------------------------------------------------
        %% GET.focus_offsets
        %-----------------------------------------------------
        function value = get.focus_offsets( obj )
            value = obj.Alpaca_Get(  'focusoffsets' );
        end
        
        %-----------------------------------------------------
        %% GET.names
        %-----------------------------------------------------
        function value = get.names( obj )
            value = obj.Alpaca_Get(  'names' );
        end
        
        %-----------------------------------------------------
        %% SET.position
        %-----------------------------------------------------
       function obj = set.position( obj, position )
            obj.Alpaca_Set(  'position', [ 'Position=' int2str( position ) ] );
        end
        
        %-----------------------------------------------------
        %% GET.position
        %-----------------------------------------------------
        function value = get.position( obj )
            value = obj.Alpaca_Get(  'position' );
        end        
        
   end
end