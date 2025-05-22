import streamlit as st
from streamlit_autorefresh import st_autorefresh
import serial, serial.tools.list_ports
import threading, sys, queue
import time
import pandas as pd


# Trigger auto‑refresh every 500 ms (infinite)
#st_autorefresh(interval=500, limit=None, key="serial_refresh")


# ————— CONFIG ————— and ————— SETUP SERIAL —————
baudRate = 115200
arduinoQueue = queue.Queue()
localQueue = queue.Queue()

#@st.cache_resource
def selectArduino():
    ports = serial.tools.list_ports.comports()
    choices = []
    st.write('PORT\tDEVICE\t\t\tMANUFACTURER')
    for index,value in enumerate(sorted(ports)):
        if (value.hwid != 'n/a'):
            choices.append(index)
            st.write(index, '\t', value.name, '\t', value.manufacturer) # https://pyserial.readthedocs.io/en/latest/tools.html#serial.tools.list_ports.ListPortInfo

    choice = -1
    while choice not in choices:
        #answer = input("➜ Select your port: ")
        answer = st.number_input("Insert a number", value= None)
        while answer is None:
            continue

        if int(answer) <= int(max(choices)):
            choice = int(answer)
    print('selecting: ', ports[choice].device)
    return ports[choice].device


#def connect():
#    BAUD_RATE    = 115200
#    
#    try:
#        ser = Serial(port='COM7', baudrate=BAUD_RATE, timeout=1)
#        return ser
#    
#    except:
#        try:
#            ser = Serial('/dev/ttyUSB0', baudrate=BAUD_RATE, timeout=1)
#            return ser
#        
#        except:
#            try:
#                ser = Serial('/dev/ttyUSB1', baudrate=BAUD_RATE, timeout=1)
#                return ser
#        
#            except Exception as error:
#                st.error("No serial port found. Please check your connection.")
#                return None


#ser = connect()
#time.sleep(0.01)  # Wait for Arduino to reset
#@st.cache_resource
def listenToArduino():
    message = b''
    while True:
        incoming = arduino.read()
        if (incoming == b'\n'):
            arduinoQueue.put(message.decode('utf-8').strip().upper())
            message = b''
        else:
            if ((incoming != b'') and (incoming != b'\r')):
                 message += incoming


#@st.cache_resource
def listenToLocal():
    while True:
        command = sys.stdin.readline().strip().upper()
        localQueue.put(command)


#@st.cache_resource
def configureUserInput():
    localThread = threading.Thread(target=listenToLocal, args=())
    localThread.daemon = True
    localThread.start()


#@st.cache_resource
def configureArduino():
    global arduinoPort
    arduinoPort = selectArduino()
    global arduino
    arduino = serial.Serial(arduinoPort, baudrate=baudRate, timeout=.1)
    arduinoThread = threading.Thread(target=listenToArduino, args=())
    arduinoThread.daemon = True
    arduinoThread.start()


# ---- CALLBACKS UPON MESSAGES -----
#@st.cache_resource
def handleLocalMessage(aMessage):
    print("=> [" + aMessage + "]")
    arduino.write(aMessage.encode('utf-8'))
    arduino.write(bytes('\n', encoding='utf-8'))


#@st.cache_resource
def handleArduinoMessage(aMessage):
    print("<= [" + aMessage + "]")



# ————— DATA BUFFER —————
@st.cache_resource
def get_buffer():
    """Return a mutable list to hold recent readings."""
    return []


configureArduino()                                      # will reboot AVR based Arduinos
configureUserInput() 
buffer = get_buffer()

while True:
    if not arduinoQueue.empty():
        if arduinoQueue.get() == "OK":
            break
print("Arduino Ready")

#while True:
#    if not arduinoQueue.empty():
#        handleArduinoMessage(arduinoQueue.get())
#
#    if not localQueue.empty():
#        handleLocalMessage(localQueue.get())


# ————— BACKGROUND READER —————
def read_serial(buffer):
    """Continuously read lines from serial and append parsed values."""
    while True:
        if not arduinoQueue.empty():
            #line = arduino.read_until(b"\n").decode("utf-8").strip()
            line = arduinoQueue.get()
            line = (f"Arduino response: {line}")
                
            if line:
                buffer.append((time.time(), line))
                if len(buffer) > 100:  # keep only the last 100
                    buffer.pop(0)

#        if line:
 #           try:
  #              # assume CSV-like: "temp,hum"
   #             temp, hum = map(float, line.split(","))
    #            timestamp = time.time()
     #           buffer.append({"time": timestamp, "temp": temp, "hum": hum})
      #          # keep only the last 100 points
       #         if len(buffer) > 100:
        #            buffer.pop(0)
         #   except Exception:
                # ignore malformed lines
          #      pass


# start background thread once
if "reader" not in st.session_state:
    buffer = get_buffer()
    t = threading.Thread(target=read_serial, args=(buffer,), daemon=True)
    t.start()
    st.session_state.reader = t


# ————— ST UI —————
st.title("Raspberry Pi ↔️ Arduino Dashboard")


# ---- Initialize history ----
if "cmd_history" not in st.session_state:
    st.session_state.cmd_history = ["STOP_MOTOR", "OFF"]  # Initialize with two empty strings


# ---- Send function ----
def send_all_commands():
    # join every command with ';' and terminate with newline
    packet = ",".join(filter(None, st.session_state.cmd_history)) + "\n"
    handleLocalMessage(packet)
    #arduino.write(packet.encode())
    st.success(f"Sent: {packet.strip()}")
# Command buttons style


st.markdown("""
    <style>
    .st-key-clockwise button {
        background-color: #04AA6D; /* Green */
        color: white;
        border: 1px solid #28a745; /* Green */
    }
    .st-key-clockwise button:hover {
        background-color: white;
        color: black;
        border: 3px solid #04AA6D; /* Green */
    }
    .st-key-clockwise button:focus {
        background-color: white;
        color: white;   
        border: 3px solid #04AA6D; /* Green */
    }
            
    .st-key-counterclockwise button {
        background-color: #04AA6D; /* Green */
        color: white;
        border: 1px solid #04AA6D; /* Green */
    }
    .st-key-counterclockwise button:hover {
        background-color: white;
        color: black;
        border: 3px solid #04AA6D; /* Green */
    }
    .st-key-counterclockwise button:focus {
        background-color: white;
        color: black;
        border: 3px solid #04AA6D;
    }
            
    .st-key-stop button {
        background-color: #008CBA; /* Blue */
        color: white;   
        border: 1px solid #008CBA; /* Blue */
    }
    .st-key-stop button:hover {
        background-color: white;
        color: black;
        border: 3px solid #008CBA; /* Blue */
    }
    .st-key-stop button:focus {
        background-color: white; 
        color: black;
        border: 3px solid #f44336; /* red */
    }
    </style>
""", unsafe_allow_html=True)


#col1, col2, col3 = st.columns(3)
#with col1:
#    if st.button("RUN CLOCKWISE", key="clockwise"):
#        ser.write(b"RUN_CLOCKWISE\n")
#
#with col2:
#    if st.button("RUN COUNTERCLOCKWISE", key="counterclockwise"):
#        ser.write(b"RUN_COUNTERCLOCKWISE\n")

#with col3:
#    if st.button("STOP MOTOR", key="stop"): 
#        ser.write(b"STOP_MOTOR\n")


def serial_write_button(name: str, button_key:str, command:str, position:int)-> None:
    """ Send command to Arduino when button is pressed. 
    Args: name (str): Button name.
    button_key (str): Unique key for the button to avoid Streamlit's key collision and to give it a unique style.
    command (str): Command to send to Arduino."""
    
    if st.button(name, key=button_key):
        st.session_state.cmd_history[position] = command
        send_all_commands()
        print(st.session_state.cmd_history)
        #ser.write(f"{command}\n".encode('utf-8'))


col1, col2, col3= st.columns(3)
with col1:
    clockwise = serial_write_button("RUN CLOCKWISE", "clockwise", "RUN_CLOCKWISE", 0)
with col2:
    counterclockwise = serial_write_button("RUN COUNTERCLOCKWISE", "counterclockwise", "RUN_COUNTERCLOCKWISE", 0)
with col3:
    counterclockwise = serial_write_button("STOP MOTOR", "stop", "STOP_MOTOR", 0)
with col1:
    RelayON = serial_write_button("RELAY ON", "relay_on", "ON", 1)
with col2:
    RelayOFF = serial_write_button("RELAY OFF", "relay_off", "OFF", 1)

# Sensor data plot
st.subheader("Live Motor Status")
#df = pd.DataFrame(buffer)
#if not df.empty:
    # convert timestamp to datetime for plotting
#    df["dt"] = pd.to_datetime(df["time"], unit="s")
#    st.line_chart(df.set_index("dt")[["temp", "hum"]])
#else:
#    st.write("Waiting for data...")

# Raw data view
#if st.checkbox("Show raw buffer"):
#    st.dataframe(df, use_container_width=True)

placeholder = st.empty()

if buffer:
    latest_time, latest_line = buffer[-1]
    
    placeholder.text(latest_line)

    # Optional: show full log
    with st.sidebar:
        st.markdown("### 🔄 Latest Serial Response:")
        st.subheader("Full Log")
        st.write("This log shows the last 100 lines received from the Arduino.")
        st.write("If you want to see the full log, please check the checkbox below.")
        if st.checkbox("Show log"):
            for t, msg in reversed(buffer):
                st.text(f"{time.strftime('%H:%M:%S', time.localtime(t))} → {msg}")

else:
    st.warning("No data received yet.")

