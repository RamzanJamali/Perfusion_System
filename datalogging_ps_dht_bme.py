#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Environmental Sensor Data Logging Script
======================================

Author: Robert Lohmüller
Version: 2.0
Last Updated: 18/11/2024

Description
----------
This script establishes a serial connection with an Arduino board to collect and log
environmental sensor data. It automatically creates dated directories and timestamped
CSV files for data storage. The script handles multiple sensor types including pressure
sensors, DHT temperature/humidity sensors, and BME680 environmental sensors.

Key Features
-----------
1. Real-time data collection from Arduino via serial connection
2. Automatic directory creation with date-based organization
3. CSV file generation with timestamps and sensor readings
4. User-friendly command interface
5. Graceful error handling and termination
6. Support for multiple sensor types:
   - Pressure sensor
   - DHT temperature/humidity sensor
   - BME680 environmental sensor (temperature, humidity, pressure, gas)

Required Hardware
---------------
- Arduino board with configured sensors
- USB connection to host computer
- Supported sensors:
  * Pressure sensor
  * DHT temperature/humidity sensor
  * BME680 environmental sensor

Directory Structure
-----------------
base_directory/
└── YYYY-MM-DD/
    └── YYYY-MM-DD_HH_MM_SS_data_logging.csv

Input
-----
Serial data from Arduino in format:
zeroValue,sensorValue1,pressureValue,Temperature,Humidity,BMEt,BMEh,BMEp,BMEg

Output
------
CSV files containing:
- Timestamp (ISO format)
- Zero calibration value
- Raw sensor reading
- Pressure value 
- Temperature 
- Humidity 
- BME680 temperature 
- BME680 humidity 
- BME680 pressure 
- BME680 gas resistance 

Usage
-----
1. Connect Arduino via USB
2. Run script: python datalogging_ps_dht_bme.py
3. Enter 'start' to begin logging
4. Press Ctrl+C to stop logging

Dependencies
-----------
- pyserial
- datetime
- csv
- os

Notes
-----
- Ensure correct USB port is specified in serial connection
- Arduino must be programmed to output data in expected format
- Data is flushed immediately to prevent loss on interruption
"""

# Import required libraries
import serial                      # For serial communication with Arduino
import time                        # For adding delays and timing functions
import csv                         # For CSV file operations
from datetime import datetime      # For timestamp generation
import os                          # For directory and file operations
import serial.tools.list_ports     # For listing available serial ports


def get_available_ports():
    """
    Get list of available serial ports.
    
    Returns
    -------
    list
        List of available serial COM ports with their descriptions
        
    Notes
    -----
    - Uses serial.tools.list_ports to detect all available ports
    - Returns both device paths and human-readable descriptions
    - Useful for debugging connection issues
    
    Example
    -------
    >>> ports = get_available_ports()
    >>> for port in ports:
    ...     print(f"{port.device}: {port.description}")
    /dev/cu.usbserial-2110: USB Serial Port
    """
    return list(serial.tools.list_ports.comports())

def select_serial_port(default_port='/dev/ttyUSB0'):
    """
    Let user select or confirm serial port.
    
    Parameters
    ----------
    default_port : str
        Default serial port to suggest (platform dependent)
        
    Returns
    -------
    str
        Selected serial port device path
        
    Notes
    -----
    - Lists all available ports with descriptions
    - Allows user to select port by number
    - Falls back to default port if input is invalid
    - Provides interactive selection interface
    
    Example
    -------
    >>> port = select_serial_port()
    Available serial ports:
    1: /dev/cu.usbserial-2110 - USB Serial Port
    2: /dev/ttyUSB0 - Arduino Uno
    
    Default port is: /dev/cu.usbserial-2110
    Press Enter to use default port or enter number to select different port: 
    """
    available_ports = get_available_ports()
    
    print("\nAvailable serial ports:")
    for i, port in enumerate(available_ports):
        print(f"{i+1}: {port.device} - {port.description}")
    
    print(f"\nDefault port is: {default_port}")
    choice = input("Press Enter to use default port or enter number to select different port: ").strip()
    
    if not choice:  # User pressed Enter
        selected_port = default_port
    else:
        try:
            port_index = int(choice) - 1
            if 0 <= port_index < len(available_ports):
                selected_port = available_ports[port_index].device
            else:
                print("Invalid selection, using default port.")
                selected_port = default_port
        except ValueError:
            print("Invalid input, using default port.")
            selected_port = default_port
    
    print(f"Using port: {selected_port}")
    return selected_port

def setup_serial_connection(port, baud_rate=9600):
    """
    Establish serial connection with Arduino.
    
    Parameters
    ----------
    port : str
        Serial port identifier (e.g., 'COM3', '/dev/ttyUSB0')
    baud_rate : int
        Communication speed in bauds (default: 9600)
        
    Returns
    -------
    serial.Serial
        Configured serial connection object
        
    Raises
    ------
    serial.SerialException
        If connection cannot be established
        
    Notes
    -----
    - Includes 2-second delay for connection stabilization
    - Verifies connection before returning
    - Uses standard Arduino baud rate by default
    
    Example
    -------
    >>> try:
    ...     arduino = setup_serial_connection('/dev/ttyUSB0')
    ...     print("Connected successfully")
    ... except serial.SerialException as e:
    ...     print(f"Connection failed: {e}")
    """
    try:
        connection = serial.Serial(port, baud_rate)
        time.sleep(2)  # Allow connection to stabilize
        return connection
    except serial.SerialException as e:
        print(f"Error establishing serial connection: {e}")
        raise

def create_output_directory():
    """
    Create dated directory for storing output files.
    
    Returns
    -------
    str
        Path to created/existing directory
        
    Notes
    -----
    - Creates directory with current date as name
    - Uses format YYYY-MM-DD for directory naming
    - Creates parent directories if needed
    - Returns existing directory if already present
    
    Example
    -------
    >>> output_dir = create_output_directory()
    >>> print(output_dir)
    ./2024-11-18
    """
    today = datetime.now().strftime("%Y-%m-%d")
    output_dir = os.path.join('.', today)
    if not os.path.exists(output_dir):
        os.makedirs(output_dir)
        print(f"Created new directory: {output_dir}")
    else:
        print(f"Using existing directory: {output_dir}")
    return output_dir

def generate_filename(output_dir):
    """
    Generate timestamped filename for CSV output.
    
    Parameters
    ----------
    output_dir : str
        Directory path for file creation
        
    Returns
    -------
    str
        Full path to new CSV file with timestamp
        
    Notes
    -----
    - Uses format YYYY-MM-DD_HH_MM_SS for timestamps
    - Appends '_data_logging.csv' to filename
    - Creates unique filename for each logging session
    
    Example
    -------
    >>> filename = generate_filename('./2024-11-18')
    >>> print(filename)
    ./2024-11-18/2024-11-18_14_30_45_data_logging.csv
    """
    now = datetime.now().strftime("%Y-%m-%d_%H_%M_%S")
    return os.path.join(output_dir, f"{now}_data_logging.csv")

def write_csv_header(file):
    """
    Write header row to CSV file.
    
    Parameters
    ----------
    file : file object
        Open CSV file object in write mode
        
    Notes
    -----
    - Writes standard column headers for all sensor types
    - Headers match Arduino data output format
    - Includes timestamp and all sensor readings
    
    Example
    -------
    >>> with open('data.csv', 'w', newline='') as f:
    ...     write_csv_header(f)
    """
    writer = csv.writer(file)
    header = [
        'Timestamp',
        'zeroValue',
        'sensorValue1',
        'pressureValue',
        'Temperature',
        'Humidity',
        'BMEt',
        'BMEh',
        'BMEp',
        'BMEg'
    ]
    writer.writerow(header)

def process_arduino_data(data_string):
    """
    Process raw data string from Arduino.
    
    Parameters
    ----------
    data_string : str
        Comma-separated string of sensor values from Arduino
        
    Returns
    -------
    list
        Processed data with timestamp and parsed sensor values
        
    Notes
    -----
    - Adds ISO format timestamp to each reading
    - Splits Arduino string into individual values
    - Preserves original data precision
    - Order matches CSV header structure
    
    Example
    -------
    >>> data = "0,123,456,25.5,60.0,24.5,58.5,1013.25,12000"
    >>> processed = process_arduino_data(data)
    >>> print(processed)
    ['2024-11-18T14:30:45.123456', '0', '123', '456', '25.5', ...]
    """
    timestamp = datetime.now().isoformat()
    parts = data_string.split(',')
    return [
        timestamp,
        parts[0],  # zeroValue
        parts[1],  # sensorValue1
        parts[2],  # pressureValue
        parts[3],  # Temperature
        parts[4],  # Humidity
        parts[5],  # BME680t
        parts[6],  # BME680h
        parts[7],  # BME680p
        parts[8]   # BME680g
    ]

def main():
    """
    Main execution function for data logging.
    
    Notes
    -----
    - Handles complete logging workflow:
        1. Serial port selection and connection
        2. Directory and file creation
        3. Continuous data collection
        4. Error handling and cleanup
    - Supports user interaction via commands
    - Implements graceful shutdown on Ctrl+C
    - Ensures proper resource cleanup
    
    Example
    -------
    >>> if __name__ == "__main__":
    ...     main()
    
    Start the Datalogging with => (start)
    Press Ctrl+C to stop logging
    """
    try:
        # Let user select serial port
        selected_port = select_serial_port()
        
        # Setup serial connection
        arduino = setup_serial_connection(selected_port)
        
        # Create output directory
        output_dir = create_output_directory()
        
        # Display user instructions
        print("\nStart the Datalogging with => (start)")
        print("Press Ctrl+C to stop Datalogging")
        
        file_name = None
        writer = None
        last_rows = []
        
        while True:
            input_str = input("Enter command: ")
            if input_str.lower() == "start":
                file_name = generate_filename(output_dir)
                
                with open(file_name, 'w', newline='') as file:
                    write_csv_header(file)
                    writer = csv.writer(file)
                    
                    print("Waiting for data from Arduino. Press Ctrl+C to stop logging.")
                    
                    try:
                        while True:
                            # Read and process data
                            data = arduino.readline().decode().strip()
                            row = process_arduino_data(data)
                            
                            try:
                                writer.writerow(row)
                                file.flush()  # Immediate flush to prevent data loss
                                print("\nData Datalogging is running!")
                                print("Press Ctrl+C to stop Datalogging")
                                print(row)
                            except Exception as e:
                                print(f'Writing error: {e}')
                            
                    except KeyboardInterrupt:
                        print("\nData logging stopped")
                        break
                    
    except Exception as e:
        print(f"Error in main execution: {e}")
    finally:
        if 'arduino' in locals():
            arduino.close()

if __name__ == "__main__":
    main()