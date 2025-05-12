from serial import Serial
import time

# ser = serial.Serial('/dev/ttyUSB0', 9600, timeout=1)
ser = Serial(port='COM7', baudrate=9600, timeout=1)
time.sleep(2)

def send_command(command):
    ser.write((command + '\n').encode())
    response = ser.readline().decode().strip()
    print(f"Arduino response: {response}")
    
while True:
    send_command("LED_ON")
    time.sleep(0.5)
    send_command("LED_OFF")
    time.sleep(0.5)