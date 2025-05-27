import streamlit as st
from streamlit_autorefresh import st_autorefresh
from serial import Serial
import threading
import time
import pandas as pd


# Trigger auto‑refresh every 1000 ms (infinite)
st_autorefresh(interval=1000, limit=None, key="serial_refresh")


# ————— CONFIG ————— and ————— SETUP SERIAL —————
@st.cache_resource
def connect():
    BAUD_RATE    = 115200
    
    try:
        ser = Serial(port='COM7', baudrate=BAUD_RATE, timeout=1)
        return ser
    
    except:
        try:
            ser = Serial('/dev/ttyUSB0', baudrate=BAUD_RATE, timeout=1)
            return ser
        
        except:
            try:
                ser = Serial('/dev/ttyUSB1', baudrate=BAUD_RATE, timeout=1)
                return ser
        
            except Exception as error:
                st.error("No serial port found. Please check your connection.")
                return None



# --- Serial port initialization (only once) ---
if 'ser' not in st.session_state:
    st.session_state.ser = connect()
ser = st.session_state.ser


# ————— DATA BUFFER —————
@st.cache_resource
def get_buffer():
    """Return a mutable list to hold recent readings."""
    return []


buffer = get_buffer()


# ————— BACKGROUND READER —————
@st.cache_resource
def read_serial(buffer):
    """Continuously read lines from serial and append parsed values."""
    chunk_Array = bytearray()
    while True:

        chunk = ser.read(ser.in_waiting or 2)
        #print(chunk.decode('utf-8'))  # print the raw data for debugging

        if not chunk:
            continue
        #print(chunk.decode('utf-8'))  # print the raw data for debugging
        chunk_Array.extend(chunk)
        
        while True:
            try:
                start_index = chunk_Array.index(b'<')
                end_index = chunk_Array.index(b'>', start_index + 1)
            except ValueError:
                break

            frame = chunk_Array[start_index + 1:end_index]  
            del chunk_Array[:end_index + 1]  # remove the processed frame

            buffer.append((time.time(), frame.decode('utf-8')))
            if len(buffer) > 100:  # keep only the last 100
                buffer.pop(0)


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
    # join every command with ',' and terminate with newline
    packet = ",".join(filter(None, st.session_state.cmd_history)) + "\n"
    ser.write(packet.encode())
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

