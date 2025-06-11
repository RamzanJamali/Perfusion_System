import streamlit as st
from streamlit_autorefresh import st_autorefresh
from serial import Serial
import threading
import time
import pandas as pd
from save_data import SensorDatabase


st.set_page_config(
    page_title="Perfusion Dashboard",
    page_icon="📊",
    layout="wide",
)

# Trigger auto‑refresh every 1000 ms (infinite)
st_autorefresh(interval=1000, limit=None, key="serial_refresh")


save_db = SensorDatabase(database_path='sensor_data.db')

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
    st.session_state.cmd_history = ["IDLE",  "1", "1.7"]  # Initialize with THREE strings


# ---- Send function ----
def send_all_commands():
    # join every command with ',' and terminate with newline
    packet = ",".join(filter(None, st.session_state.cmd_history)) + "\n"
    ser.write(packet.encode())
    st.success(f"Sent: {packet.strip()}")


# Command buttons style
st.markdown("""
    <style>
    .stButton > button {
        background-color: #04AA6D; /* Green */
        color: white;
        border: 3px solid #28a745; /* Green */
    }
    .stButton > button:hover {
        background-color: white;
        color: black;
        border: 3px solid #04AA6D; /* Green */
    }
    .stButton > button:focus {
        background-color: white;
        color: #28a745; /* Green */
        border: 3px solid #04AA6D; /* Green */
    }

    </style>
""", unsafe_allow_html=True)


def serial_write_button(name: str, button_key:str, command:str, position:int, button_icon)-> None:
    """ Send command to Arduino when button is pressed. 
    Args: name (str): Button name.
    button_key (str): Unique key for the button to avoid Streamlit's key collision and to give it a unique style.
    command (str): Command to send to Arduino."""
    
    if st.button(name, key=button_key, type="tertiary", use_container_width=False, icon=button_icon):
        st.session_state.cmd_history[position] = command
        send_all_commands()
        print(st.session_state.cmd_history)
        #ser.write(f"{command}\n".encode('utf-8'))


col1, col2, col3, col4= st.columns(4)
with col1:
    clockwise = serial_write_button("START PERFUSION", "start", "START_PERFUSION", 0, button_icon='▶️')
with col2:
    counterclockwise = serial_write_button("PAUSE PERFUSION", "pause", "PAUSE_PERFUSION", 0, button_icon='⏸️')
with col3:
    counterclockwise = serial_write_button("CONTINUE PERFUSION", "continue", "CONTINUE_PERFUSION", 0, button_icon="⏯️")
with col4:
    counterclockwise = serial_write_button("END PERFUSION", "end", "END_PERFUSION", 0, button_icon="⏹️")



col1, col2= st.columns(2)
with col1:
    new_pressure = st.number_input("Pressure (mmHg)", min_value=0.0, max_value=100.0, value=1.0, step=1.0)
    if new_pressure == None or new_pressure < 0:
        new_pressure = 1
    else:
        new_pressure = new_pressure
    clockwise = serial_write_button("SET PRESSURE", "pressure", str(new_pressure), 1, button_icon='💨')
with col2:
    new_flow_rate = st.number_input("Flow Rate (ml/day) -> min value 1.7", min_value=1.7, max_value=100.0, value=1.7, step=0.01)
    if new_flow_rate == None or new_flow_rate < 0:
        new_flow_rate = 1.7
    else:
        new_flow_rate = new_flow_rate
    counterclockwise = serial_write_button("SET FLOW RATE", "flowRate", str(new_flow_rate), 2, button_icon="💉")

# Sensor data plot
st.subheader("Live Motor Status")

@st.fragment(run_every=1)
def serial_log():
    st.markdown("### 🔄 Latest Serial Response:")
    st.subheader("Full Log")
    st.write("This log shows the last 100 lines received from the Arduino.")
    st.write("If you want to see the full log, please check the checkbox below.")
    if st.checkbox("Show log"):
        for t, msg in reversed(buffer):
            st.text(f"{time.strftime('%H:%M:%S', time.localtime(t))} → {msg}")
            data_list = [item.strip() for item in msg.split(',')]
            try:
                save_db.insert_reading(data_list)
            except:
                pass
            

placeholder = st.empty()
if buffer:
    latest_time, latest_line = buffer[-1]
        
    placeholder.text(latest_line)

    # Optional: show full log
    with st.sidebar:
            serial_log()

else:
    st.warning("No data received yet.")


df = save_db.get_recent_readings(limit=1000)

if df.empty:
    st.warning("No data available to display.")
    st.stop()

df['timestamp'] = pd.to_datetime(df['timestamp'])
df = df.sort_values(by='timestamp').reset_index(drop=True)
df["elapsed_time"] = (df['timestamp'] - df['timestamp'].iloc[0]).dt.total_seconds()

st.subheader("Sensor Data Plot")
st.dataframe(df)