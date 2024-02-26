classdef Gate < handle
    properties
        id (1,1) uint16 = uint16(0);         
        mode (1,1) uint8 = uint8(0);         
        orientation (1,1) uint8 = uint8(0);  
        x (1,1) int32 = int32(0);            
        y (1,1) int32 = int32(0);            
        z (1,1) int32 = int32(0);            
        color (1,1) uint32 = uint32(0);      
        destination uint16 = zeros(0, 'uint16');
    end
    
    properties (Dependent)
        num_dest (1,1) uint32;
    end
    
    properties (Constant)
        COLOR_DEFAULT = uint32(0x00DF7F01);
        COLOR_INPUT = uint32(0x0000EE00);
        COLOR_OUTPUT = uint32(0x000000EE);
        COLOR_DEBUG = uint32(0x00EE00EE);
        COLOR_WHITE = uint32(0x00EEEEEE);
        COLOR_BLACK = uint32(0x00222222);
    end
    
    properties (Constant)
        SHAPE_ID = '9f0f56e8-2c31-4d83-996c-d00a9b296c3f';
    end
    
    properties (Constant)
        MODE_AND  = 0;
        MODE_OR   = 1;
        MODE_XOR  = 2;
        MODE_NAND = 3;
        MODE_NOR  = 4;
        MODE_XNOR = 5;
    end
    
    properties (Constant)
        FACE_UP = uint8(0x32);
        FACE_LEFT = uint8(0xF3);
        FACE_RIGHT = uint8(0x13);
        FACE_FRONT = uint8(0xE3);
        FACE_BACK = uint8(0x23);
    end
    
    properties (Constant)
        TABLE_CENTER = 4;
        TABLE_XZ = Gate.InitXZTable();
        TABLE_OFFSET = Gate.InitOffsetTable();
    end
    
    methods
        function obj = Gate(id, mode, orientation, x, y, z, color, destination)
            if nargin >= 7
                obj.id = uint16(id);
                obj.mode = uint8(mode);
                obj.orientation = uint8(orientation);
                obj.x = int32(x);
                obj.y = int32(y);
                obj.z = int32(z);
                obj.color = uint32(color);
                
                if nargin == 8
                    obj.destination = uint16(destination);
                end
            end
            
        end
        
        function value = get.num_dest(obj)
            value = numel(obj.destination);
        end

        function obj = AddDest(obj, ids)
            obj.destination = [obj.destination, ids];
        end
    end
    
    methods (Access = private, Static)
        function value = InitXZTable()
            value = zeros(7,7,2);
            value(7,5,:) = [1,-2];
            value(7,6,:) = [2,1];
            value(7,3,:) = [-1,2];
            value(7,2,:) = [-2,-1];
            value(5,6,:) = [2,-3];
            value(5,7,:) = [3,2];
            value(5,2,:) = [-2,3];
            value(5,1,:) = [-3,-2];
            value(6,5,:) = [1,3];
            value(6,7,:) = [3,-1];
            value(6,3,:) = [-1,-3];
            value(6,1,:) = [-3,1];
            value(1,5,:) = [1,2];
            value(1,6,:) = [2,-1];
            value(1,3,:) = [-1,-2];
            value(1,2,:) = [-2,1];
            value(3,6,:) = [2,3];
            value(3,7,:) = [3,-2];
            value(3,2,:) = [-2,-3];
            value(3,1,:) = [-3,2];
            value(2,5,:) = [1,-3];
            value(2,7,:) = [3,1];
            value(2,3,:) = [-1,3];
            value(2,1,:) = [-3,-1];
        end
        function value = InitOffsetTable()
            value = zeros(7,7,3);
            value(7,5,:) = [0,-1,0];
            value(7,6,:) = [0,0,0];
            value(7,3,:) = [-1,0,0];
            value(7,2,:) = [-1,-1,0];
            value(5,6,:) = [0,0,-1];
            value(5,7,:) = [0,0,0];
            value(5,2,:) = [0,-1,0];
            value(5,1,:) = [0,-1,-1];
            value(6,5,:) = [0,0,0];
            value(6,7,:) = [-1,0,0];
            value(6,3,:) = [-1,0,-1];
            value(6,1,:) = [0,0,-1];
            value(1,5,:) = [0,-0,-1];
            value(1,6,:) = [-1,0,-1];
            value(1,3,:) = [-1,-1,-1];
            value(1,2,:) = [0,-1,0];
            value(3,6,:) = [-1,0,0];
            value(3,7,:) = [-1,-1,0];
            value(3,2,:) = [-1,-1,-1];
            value(3,1,:) = [-1,0,-1];
            value(2,5,:) = [0,-1,-1];
            value(2,7,:) = [0,-1,0];
            value(2,3,:) = [-1,-1,0];
            value(2,1,:) = [-1,-1,-1];
        end
    end
    
end
