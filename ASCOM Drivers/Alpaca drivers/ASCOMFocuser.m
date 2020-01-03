%=======================================================================
%
%   Focuser class for communicating with ASCOM focus motors over Alpaca interface
%
%   ASCOM Platform: https://ascom-standards.org/Downloads/Index.htm
%   Alpaca Developer's page: https://ascom-standards.org/Developer/Alpaca.htm
%
%   (c) 2019 M. Okincha
%
%=======================================================================
classdef ASCOMFocuser < ASCOMDevice
   
    properties ( SetAccess = protected, GetAccess = public )
        
        % ASCOM Focuser properties
        alpaca_device_name;
    end
    
    % Read only
    properties ( Dependent = true, SetAccess = protected, GetAccess = public )
        
        % ASCOM Focuser properties
        absolute;
        is_moving;
        max_increment;
        max_step;
        step_size;
        temp_comp_available;
        temperature;

    end
    
    % Read/Write
    properties ( Dependent = true, SetAccess = public, GetAccess = public )
        
        % ASCOM Focuser properties
        position;
        temp_comp;

    end
    
    % Write-only Properties
    properties ( Dependent = true, SetAccess = public, GetAccess = protected )
        
        % ASCOM Focuser properties
        halt;
        move;

    end
    
   methods
        % Constructor
        function obj = ASCOMFocuser

            % Call the base class constructor.  Not sure if this is
            % required
            @obj.ASCOMDevice;
            
            % Device that this is a focuser device
            obj.alpaca_device_name = 'focuser';         % ASCOM-recognized device name

            % try connecting to the focuser
            try
                obj.Alpaca_Get( 'connected' );
            catch
                error( [ 'Focuser ' int2str( obj.alpaca_device_number ) ' not responding over ASCOM Alpaca HTTP interface' ] );
            end
            
        end
   end
   
   % Public methods
   methods
        
%         % Filter Wheel Properties

        %-----------------------------------------------------
        %% GET.absolute
        %-----------------------------------------------------
        function value = get.absolute( obj )
            value = obj.Alpaca_Get( 'absolute' );
        end
        
        %-----------------------------------------------------
        %% GET.is_moving
        %-----------------------------------------------------
        function value = get.is_moving( obj )
            value = obj.Alpaca_Get( 'ismoving' );
        end
        
        %-----------------------------------------------------
        %% GET.max_increment
        %-----------------------------------------------------
        function value = get.max_increment( obj )
            value = obj.Alpaca_Get( 'maxincrement' );
        end
        
        %-----------------------------------------------------
        %% GET.max_step
        %-----------------------------------------------------
        function value = get.max_step( obj )
            value = obj.Alpaca_Get( 'maxstep' );
        end
        
        %-----------------------------------------------------
        %% GET.position
        %-----------------------------------------------------
        function value = get.position( obj )
            value = obj.Alpaca_Get( 'position' );
        end
        
        %-----------------------------------------------------
        %% SET.position
        %-----------------------------------------------------
        function obj = set.position( obj, new_position )
            
            obj.move = new_position;
        end
        
        %-----------------------------------------------------
        %% GET.step_size
        %-----------------------------------------------------
        function value = get.step_size( obj )
            value = obj.Alpaca_Get( 'stepsize' );
        end
        
        %-----------------------------------------------------
        %% SET.temp_comp
        %-----------------------------------------------------
       function obj = set.temp_comp( obj, enable )
           
           if enable
               enable_str = 'true';
           else
               enable_str = 'false';
           end
           
            obj.Alpaca_Set( 'tempcomp', [ 'TempComp=' enable_str ] );
        end
        
        %-----------------------------------------------------
        %% GET.temp_comp
        %-----------------------------------------------------
        function value = get.temp_comp( obj )
            value = obj.Alpaca_Get( 'tempcomp' );
        end        
        
        %-----------------------------------------------------
        %% GET.temp_comp_available
        %-----------------------------------------------------
        function value = get.temp_comp_available( obj )
            value = obj.Alpaca_Get( 'tempcompavailable' );
        end        
        
        %-----------------------------------------------------
        %% GET.temperature
        %-----------------------------------------------------
        function value = get.temperature( obj )
            value = obj.Alpaca_Get( 'temperature' );
        end
        
        %-----------------------------------------------------
        %% SET.halt
        %-----------------------------------------------------
        function obj = set.halt( obj, value )
            obj.Alpaca_Set( 'halt', '' );
        end
        
        %-----------------------------------------------------
        %% SET.move
        %-----------------------------------------------------
        function obj = set.move( obj, new_position )
            obj.Alpaca_Set( 'move', [ 'Position=' int2str( new_position ) ] );
        end
        
   end
end