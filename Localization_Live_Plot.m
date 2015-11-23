%% Localization value liveplotting
%
% Reads Position values from the M2 microcontroller.
% Plots the data realtime into Plot overlaying field.
% Logged into Pos_Log[time, _Value] variable.
% Hit Ctrl-C to quit the program
%
% By Pele Collins in Fall 2015 for Robockey

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%


%% Initialize program and USB port

if(exist('M2USB'))
  fclose(M2USB);
 else
%  fclose(instrfindall);
 delete(instrfindall);
end

%% If the above initialization does not work, please run the following commands manually and disconnect and reconnect USB.
%  fclose(serial('COM5','Baudrate', 9600));
%  fclose(instrfindall);
clear all;
close all;

%% VARIABLES
maxPoints = 1;                                 % Max number of data points displayed at a time.
t = 1:1:maxPoints;                              % Create an evenly spaced matrix for X-axis.
x_pos = zeros(1,maxPoints);                     % Pre-allocate arrays for IMU values
y_pos = zeros(1,maxPoints);
angle = zeros(1,maxPoints);

%% SERIAL
%----> for ***MAC***
M2USB = serial('/dev/tty.usbmodem411','Baudrate',9600);
% *** Check where your device is by opening terminal and entering the command:
% 'ls /dev/tty.usb' and tab-completing.

fopen(M2USB);                                   % Open up the port to the M2 microcontroller
flushinput(M2USB);                              % Flush the input buffer

% Send initial packet to get first set of data from microcontroller
fwrite(M2USB,1);                                % Send a packet to the M2
time = 0;                                       % Set the start time to 0
i = 1;                                          % Start the indexer at 1
tic;                                            % Start timer.


%% Plotting

% Initializing figure and rink

figure('units','normalized','outerposition',[0 0 1 1]);
title({ ; 'Robockey Localization';'Team 5'}, 'FontSize',36 )

% Main Rink
pos1 = [-115,-60,230,120];
cur1 = 0.5;
rectangle('Position',pos1,'Curvature',cur1, 'FaceColor',[1,1,1],'EdgeColor','k',...
    'LineWidth',3);
axis equal
axis off

hold on 

% Goal 1
pos2 = [-125,-25,10,50];
cur2 = 0;
rectangle('Position',pos2,'Curvature',cur2, 'FaceColor',[1 1 1],'EdgeColor','k',...
    'LineWidth',3);

%Goal 2
pos3 = [115,-25,10,50];
cur3 = 0;
rectangle('Position',pos3,'Curvature',cur3, 'FaceColor',[1 1 1],'EdgeColor','k',...
    'LineWidth',3);

% Constellation
Star_co_ords = [0, 14.5; 11.655, 8.741; 0. -14.5; -10.563, 2.483];
scatter(Star_co_ords(:,1),Star_co_ords(:,2),20,'r', 'filled');

%Origin
scatter(0,0,50,'k','filled');

%Compass
U = [0,5];
V = [5,0];

compass(U,V);


%Zones
lines_y = [56,-56; 60,-60; 60,-60; 56, -56];
lines_x = [-100, -100; -40, -40; 40, 40; 100, 100];

for i = 1:4
    plot(lines_x(i,:), lines_y(i,:), 'b', 'LineWidth', 1);
end


%% Plotting position of Robots and orientation

position = scatter(x_pos, y_pos, 'm', 'LineWidth', 10);

orientation = plot([x_pos, x_pos+4*cos(angle)], [y_pos, y_pos+4*sin(angle)],'k', 'LineWidth', 2); 
    

position.XDataSource = 'x_pos';
position.YDataSource = 'y_pos';
orientation.XDataSource = ['x_pos', 'angle'];
orientation.YDataSource = ['y_pos', 'angle'];

% Send initial confirmation packet
fwrite(M2USB,1);                                % Confirmation packet

%% Run program forever
try
   while 1
        
        %% Read in data and send confirmation packet
        m2_buffer = fgetl(M2USB);               % Load buffer
        fwrite(M2USB,1);                        % Confirmation packet
        
        %% Parse microcontroller data
        [m2_y_pos, remain] = strtok(m2_buffer);
        [m2_x_pos, remain2] = strtok(remain);
        [m2_angle, remain3] = strtok(remain2);
        m2_buffer;
        time = toc;                             % Stamp the time the value was received
        
        % Remove the oldest entry 
        x_pos = [str2double(m2_x_pos)/3 x_pos(1:maxPoints-1)] ;
        y_pos = [str2double(m2_y_pos)/3 y_pos(1:maxPoints-1)] ;
        angle = [str2double(m2_angle)/100 angle(1:maxPoints-1)] ;

        %Update plots
        %refreshdata;
        %drawnow;
      hold on;
        delete(position);
        delete(orientation);
        position = scatter(x_pos, y_pos, 10000, 'm', 'LineWidth', 10);
        orientation = plot([x_pos, x_pos+15*cos(angle)], [y_pos, y_pos+15*sin(angle)],'k', 'LineWidth', 2); 
        pause(.01);
        
        hold off
        
        i=i+1;
    end
catch ME
    ME.stack
    fclose(M2USB);                              % Close serial object
end


