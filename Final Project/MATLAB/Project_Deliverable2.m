clear;
clc;

% Set up serial port
port = "COM4";              % Define the COM port the microcontroller is connected to
baudRate = 115200;          % Set baud rate to match UART configuration on microcontroller
s = serialport(port, baudRate); % Create serial port object
flush(s);                   % Clear any residual data in the serial buffer

% Scan configuration
total_scans = 3;              % Total number of scans to collect
measurements = 32;       % Number of distance measurements per 360° scan
angles_deg = linspace(0, 360, measurements + 1); % Generate angles from 0 to 360°
angles_deg(end) = [];       % Remove last angle (360°) to avoid overlap with 0°
angles_rad = deg2rad(angles_deg); % Convert angles to radians for trig functions

% Storage arrays
x_coords = [];              % Stores all X positions (scan number)
y_coords = [];              % Stores all Y coordinates (cosine-projected distance)
z_coords = [];              % Stores all Z coordinates (sine-projected distance)

disp("Waiting for 3 complete scans...");

scan_count = 0;             % Counter to track how many full scans were received

while scan_count < total_scans
    measurement_values = [];   % Reset array to store one full scan of 32 distance values

    % Wait until 32 valid distance readings are received
    while length(measurement_values) < measurements
        if s.NumBytesAvailable > 0         % Check if data is available on serial port
            line = readline(s);            % Read a line from serial (should be a distance value)
            dist = str2double(strtrim(line)); % Convert the string to a double after trimming whitespace

            % Validate parsed value
            if ~isnan(dist) && isfinite(dist)  % Ensure the value is numeric and finite
                measurement_values(end+1) = dist; % Append the valid distance
                fprintf("Scan %d | Step %d | Distance: %.2f mm\n", scan_count+1, length(measurement_values), dist);
            end
        end
        pause(0.01); % short wait to prevent CPU overload and allow buffer to fill
    end

    % Convert polar coordinates to Cartesian (Y-Z plane, scan moves in X direction)
    y = measurement_values .* cos(angles_rad);   % Project to Y using cos(θ)
    z = measurement_values .* sin(angles_rad);   % Project to Z using sin(θ)
    x = ones(1, measurements) * (scan_count + 1); % X is constant per scan layer

    % Append this scan's data to the full list
    x_coords = [x_coords, x];     % Stack all x coordinates
    y_coords = [y_coords, y];     % Stack all y coordinates
    z_coords = [z_coords, z];     % Stack all z coordinates

    scan_count = scan_count + 1;  % Move to next scan
 
    fprintf("Completed scan %d of %d.\n\n", scan_count, total_scans);
end

% Reshape collected data into matrices with dimensions: (steps per scan) x (num scans)
x_matrix = reshape(x_coords, measurements, total_scans);
y_matrix = reshape(y_coords, measurements, total_scans);
z_matrix = reshape(z_coords, measurements, total_scans);

% Plot the 3D connected graph
figure;
hold on;
grid on;
xlabel("X Depth");          % X axis represents scan layers
ylabel("Y Width");          % Y axis shows horizontal distance (cosine projection)
zlabel("Z Height");         % Z axis shows vertical height (sine projection)
title("2DX3 Project Visualization"); % Title of the plot

% Connect points within each circular scan
for i = 1:total_scans
    plot3(x_matrix(:, i), y_matrix(:, i), z_matrix(:, i), '-o', 'LineWidth', 1); % Connect circular scan points
end

% Connect corresponding points between different scans (same angle across scans)
for j = 1:measurements
    plot3(x_matrix(j, :), y_matrix(j, :), z_matrix(j, :), 'k-', 'LineWidth', 0.5); % Connect same-angle points across scans
end

view(45, 25); % Optional: Set a good 3D viewing angle for visualization