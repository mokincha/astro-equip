%=======================================================================
%
%   Base class for communicating with ASCOM devices over Alpaca interface
%
%   This is an abstract class.  You can't instantiate it directly, since
%   there's no such thing as a generic ASCOM device.
%
%   ASCOM Platform: https://ascom-standards.org/Downloads/Index.htm
%   Alpaca Developer's page: https://ascom-standards.org/Developer/Alpaca.htm
%
%   (c) 2019 M. Okincha
%
%=======================================================================
classdef ASCOMDevice
   
    properties ( SetAccess = protected, GetAccess = public, Abstract = true )
        
        alpaca_device_name;         % ASCOM-recognized device name
    end
    
    properties ( SetAccess = public, GetAccess = public )
        
        % ASCOM Common properties
        alpaca_device_number;
        host_string;
    end
    
    properties ( Dependent = true )
        
        % ASCOM Common properties
        connected;
    end
    
    % Read only
    properties ( Dependent = true, SetAccess = protected, GetAccess = public )
        
        % ASCOM Common properties
        description;
        driver_info;
        driver_version;
        interface_version;
        name;
        supported_actions;
    end
    
   methods
        % Constructor
        function obj = ASCOMDevice

            %   Set default values
            obj.host_string = 'http://127.0.0.1:11111';       % local host.
            obj.alpaca_device_number = 0;
          
        end
   end
   
   % Public methods, abstracted from Sensor superclass
   methods
        
        % COMMON READ-ONLY Properties

        %-----------------------------------------------------
        %% Action
        %-----------------------------------------------------
       function Action( obj, action )
            obj.Alpaca_Set(  'action', [ 'Action=' action ] );
        end
        
       
        %-----------------------------------------------------
        %% Command_Blind
        %-----------------------------------------------------
       function Command_Blind( obj, command_str, parameter_str )
            obj.Alpaca_Set(  'commandblind', [ 'Command=' command_str '&Raw=' parameter_str ] );
        end
        
       
        %-----------------------------------------------------
        %% Command_Bool
        %-----------------------------------------------------
       function Command_Bool( obj, command_str, parameter_bool )
            obj.Alpaca_Set(  'commandbool', [ 'Command=' command_str '&Raw=' int2str( parameter_bool ) ] );
        end
        
       
        %-----------------------------------------------------
        %% Command_String
        %-----------------------------------------------------
       function Command_String( obj, command_str, parameter_str )
            obj.Alpaca_Set(  'commandstring', [ 'Command=' command_str '&Raw=' parameter_str ] );
        end
        
       
        %-----------------------------------------------------
        %% SET.connected
        %-----------------------------------------------------
       function set.connected( obj, power_state )
            if power_state == 0
                state_str = 'false';
            else
                state_str = 'true';
            end
            obj.Alpaca_Set(  'connected', [ 'Connected=' state_str ] );
        end
        
       
        %-----------------------------------------------------
        %% GET.connected
        %-----------------------------------------------------
        function value = get.connected( obj )
            value = obj.Alpaca_Get(  'connected' );
        end
        
        
        %-----------------------------------------------------
        %% GET.description
        %-----------------------------------------------------
        function value = get.description( obj )
            value = obj.Alpaca_Get(  'description' );
        end
        
       
        %-----------------------------------------------------
        %% GET.driver_info
        %-----------------------------------------------------
        function value = get.driver_info( obj )
            value = obj.Alpaca_Get(  'driverinfo' );
        end
        
       
        %-----------------------------------------------------
        %% GET.driver_version
        %-----------------------------------------------------
        function value = get.driver_version( obj )
            value = obj.Alpaca_Get(  'driverversion' );
        end
        
       
        %-----------------------------------------------------
        %% GET.interface_version
        %-----------------------------------------------------
        function value = get.interface_version( obj )
            value = obj.Alpaca_Get(  'interfaceversion' );
        end
        
       
        %-----------------------------------------------------
        %% GET.name
        %-----------------------------------------------------
        function value = get.name( obj )
            value = obj.Alpaca_Get(  'name' );
        end
        
        
        %-----------------------------------------------------
        %% GET.supported_actions
        %-----------------------------------------------------
        function value = get.supported_actions( obj )
            value = obj.Alpaca_Get(  'supportedactions' );
        end
      
        
        %-----------------------------------------------------
        %% ASCOM Alpaca write command 
        %-----------------------------------------------------
        function [ error_id, error_msg ] = Alpaca_Set( obj, method_str, method_data, time_out )
            
            if ~exist( 'time_out', 'var' )
                time_out = 5;
            end

            % import the required libraries
            import matlab.net.*
            import matlab.net.http.*
            import matlab.net.https.*

            % construct the message
            device_id_str = int2str( obj.alpaca_device_number );
            options = matlab.net.http.HTTPOptions( 'ConnectTimeout', time_out );

            if exist( 'method_data', 'var' ) && ~isempty( method_data ) 
                uri = URI( [ obj.host_string '/api/v1/' obj.alpaca_device_name '/' device_id_str '/' method_str ] );
                body = matlab.net.http.MessageBody( method_data );
                contentTypeField = matlab.net.http.field.ContentTypeField('text/plain');
                type1 = matlab.net.http.MediaType('text/*');
                acceptField = matlab.net.http.field.AcceptField( [ type1 ] );
                header = [acceptField contentTypeField];
                method = matlab.net.http.RequestMethod.PUT;
                request = matlab.net.http.RequestMessage( method, header, body );
            else
                % format parameter-less command
                uri = URI( [ obj.host_string '/api/v1/' obj.alpaca_device_name '/' device_id_str '/' method_str ] );
                contentTypeField = matlab.net.http.field.ContentTypeField('text/plain');
                type1 = matlab.net.http.MediaType('text/*');
                acceptField = matlab.net.http.field.AcceptField( [ type1 ] );
                header = [acceptField contentTypeField ];
                method = matlab.net.http.RequestMethod.PUT;
                request = matlab.net.http.RequestMessage( method );
            end
            
            % send the request and parse the response
            response = send( request, uri, options );

            if isstring( response.Body.Data )
                error( response.Body.Data );
            else
                error_id = response.Body.Data.ErrorNumber;
                error_msg = response.Body.Data.ErrorMessage;

                if error_id ~= 0
                    warning( error_msg );
                end
            end   
            
        end
        
        %-----------------------------------------------------
        %% ASCOM Alpaca read command 
        %-----------------------------------------------------  
        function [ result, error_id, error_msg ] = Alpaca_Get( obj, method_str, time_out )
            
            if ~exist( 'time_out', 'var' )
                time_out = 20;
            end

            % import the required libraries
            import matlab.net.*
            import matlab.net.http.*

            % construct the message
            device_id_str = int2str( obj.alpaca_device_number );
            uri = URI( [ obj.host_string '/api/v1/' obj.alpaca_device_name '/' device_id_str '/' method_str ] );
            request = RequestMessage;
            options = matlab.net.http.HTTPOptions( 'ConnectTimeout', time_out );

            % send the request and parse the response
            response = send( request, uri, options );
            
            if strcmp( response.StatusCode, 'BadRequest' )
                
                fprintf( [ 'Alpaca server unable to process request: to "' obj.alpaca_device_name '\\' int2str( obj.alpaca_device_number ) '".\n' cell2mat( response.Body.Data ) '\n' ] );

                % Requested device not found on the Alpaca server.  
                result = '';
                error_id = response.StatusCode;
                error_msg = response.StatusLine;
                
            else
            
                % Response was accepted.  
                
                result = response.Body.Data.Value;

                if isstring( response.Body.Data )

                    error( response.Body.Data );
                    
                else
                    
                    error_id = response.Body.Data.ErrorNumber;
                    error_msg = response.Body.Data.ErrorMessage;

                    switch error_id
                        
                        case 1024
                            fprintf( [ 'Property ''' method_str ''' not implemented by this driver\n' ] );

                        otherwise
                            warning( error_msg );
                    end
                    
                end

            end
            
        end
        
   end
   
end