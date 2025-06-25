import streamlit as st
from streamlit_autorefresh import st_autorefresh
from serial import Serial # PySerial library for serial communication
import serial.tools.list_ports
import threading, os
import time, datetime
import pandas as pd
from save_data import SensorDatabase

# Configurable directory (default to 'databases' but can be changed)
DB_DIR = os.getenv("DB_DIRECTORY", "databases")

st.set_page_config(
    page_title="Perfusion Dashboard",
    page_icon="📊",
    layout="wide",
)

# Trigger auto‑refresh every 1000 ms (infinite)
st_autorefresh(interval=1000, limit=None, key="serial_refresh")


# --- Helper to make a unique filename ---
def make_db_filename():
    now = datetime.datetime.now().strftime("%Y%m%d_%H%M%S")
    #return f"databases/perfusion_{now}.db"
    return os.path.join(DB_DIR, f"perfusion_{now}.db")


# --- Initialization in Streamlit main thread ---
@st.cache_resource
def init_db():
    db = [None, None, False]  # [db_instance, db_path, perfusion_on]
    return db

db = init_db()

# ————— CONFIG ————— and ————— SETUP SERIAL —————
@st.cache_resource(ttl=3600)
def connect():
    BAUD_RATE    = 115200
    
    ports = serial.tools.list_ports.comports()
    choices = []
    for index, value in enumerate(sorted(ports)):
        if (value.hwid != 'n/a'):
            choices.append(index)
            print(index, '\t', value.name, '\t', value.manufacturer)

    #choice = -1
    #while choice not in choices:
        #answer = input("➜ Select your port: ")
        #if answer.isnumeric() and int(answer) <= int(max(choices)):
        #    choice = int(answer)
        
    choice = 0  # Default to the first port for simplicity in this example
    print('selecting: ', ports[choice].device)
    port = ports[choice].device

    ser = Serial(port=port, baudrate=BAUD_RATE, timeout=1)
    time.sleep(1)  # wait for the serial connection to initialize
    return ser

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

# ---- Initialize history ----
if "cmd_history" not in st.session_state:
    st.session_state.cmd_history = ["IDLE",  "1.0", "1.7", "0", "0"]  # Initialize with Five strings


# ---- Send function ----
def send_all_commands():
    # join every command with ',' and terminate with newline
    packet = ",".join(filter(None, st.session_state.cmd_history)) + "\n"
    ser.write(packet.encode())
    st.success(f"Sent: {packet.strip()}")



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


col1, col2, col3, col4= st.columns(4)
with col1:
    new_pressure = st.number_input("Pressure (mmHg)", min_value=0.0, max_value=100.0, value=1.0, step=1.0, key="pressure_input")
    if new_pressure == None or new_pressure < 0:
        new_pressure = 1
    else:
        new_pressure = new_pressure
    clockwise = serial_write_button("SET PRESSURE", "pressure", str(new_pressure), 1, button_icon='🔧')

with col2:
    new_flow_rate = st.number_input("Flow Rate (ml/day) -> min value 1.7", min_value=1.7, max_value=100.0, value=1.7, step=0.01)
    if new_flow_rate == None or new_flow_rate < 0:
        new_flow_rate = 1.7
    else:
        new_flow_rate = new_flow_rate
    counterclockwise = serial_write_button("SET FLOW RATE", "flowRate", str(new_flow_rate), 2, button_icon="💉")

with col3:
    raw_low = st.number_input("Set raw low pressure", min_value=0, max_value=500, value=50, step=1, key="raw_low_input")
    if raw_low == None or raw_low < 0:
        raw_low = 1
    else:
        raw_low = raw_low
    set_raw_low = serial_write_button("SET RAW LOW", "low_pressure", str(raw_low), 3, button_icon='⤵️')
    
with col4:
    raw_high = st.number_input("Set raw high pressure", min_value=0, max_value=500, value=100, step=1, key="raw_high_input")
    if raw_high == None or raw_high < 0:
        raw_high = 1
    else:
        raw_high = raw_high
    set_raw_high = serial_write_button("SET RAW HIGH", "high_pressure", str(raw_high), 4, button_icon='⤴️')


# ————— BACKGROUND READER —————
@st.cache_resource
def read_serial(buffer, db):
    """Continuously read lines from serial and append parsed values."""
    chunk_Array = bytearray()
    while True:
        if ser.in_waiting:
            chunk = ser.read(55)

        else:
            time.sleep(0.1)  # wait a bit if no data is available
            continue
        #chunk = ser.read(ser.in_waiting or 2)
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
            #print(frame.decode('utf-8'))  # print the raw data for debugging
            del chunk_Array[:end_index + 1]  # remove the processed frame
            
            raw_data = frame.decode('utf-8').strip()
            data_list = [item.strip() for item in raw_data.split(',')]

            cmd = data_list[0]
            # 1) If START_PERFUSION (cmd == "1"), open a new DB
            if cmd == "1" and not db[2]:
                new_path = make_db_filename()
                db[1] = new_path
                db[0] = SensorDatabase(database_path=new_path)
                db[2] = True
                print(f"Perfusion started → logging to: {new_path}")
            
            # 2) If STOP_PERFUSION (cmd == "0"), close out current session
            if cmd == "0" and db[2]:
                db[2] = False
                print(f"Perfusion stopped for: {db[1]}")
                # (Optionally: db = None)

            # 3) If perfusion is active and we have a db, insert
            if db[2] and db[0] is not None:
                try:
                    db[0].insert_reading(data_list)
                except Exception as e:
                    print(f"DB insert failed: {e}")
            """        
            try:
                if data_list[0] in ("1", "2"):
                    save_db.insert_reading(data_list)
            except:
                pass
            """
            buffer.append((time.time(), frame.decode('utf-8')))
            if len(buffer) > 100:  # keep only the last 100
                buffer.pop(0)


# start background thread once
if "reader" not in st.session_state:
    buffer = get_buffer()
    t = threading.Thread(target=read_serial, args=(buffer, db), daemon=True)
    t.start()
    st.session_state.reader = t


# ————— ST UI —————
st.title("Raspberry Pi ↔️ Arduino Dashboard")


# Command buttons style
st.markdown("""
    <style>
    [data-testid="stAppViewContainer"] {
        background-color: white;
    }
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
            

placeholder = st.empty()
if buffer:
    latest_time, latest_line = buffer[-1]
        
    placeholder.text(latest_line)

    # Optional: show full log
    with st.sidebar:
            serial_log()

else:
    st.warning("No data received yet.")


#try:
#    df = db[0].get_recent_readings(1000)
#df = save_db.get_recent_readings(1000)
#except Exception as e:
#    df = pd.DataFrame()
#    st.info(f"No data to retrieve: {e}")

df = pd.DataFrame(columns=[
    "timestamp", "perfusion_state", "valve_state",
    "humidity", "temperature", "current_pressure", "target_pressure",
    "motor_speed", "tilt", "gyro_x", "gyro_y", "gyro_z"
])

@st.fragment(run_every=1)
def read_db(df, db):
    try:
        new_row = db[0].get_recent_readings(1)

        if new_row.empty:
            st.info("No data available to display.")
            st.stop()

        new_row = new_row.dropna(axis=1, how="all")
        df = df.dropna(axis=1, how="all")
        df = pd.concat([new_row, df], ignore_index=True)
        if len(df) > 1000:
            df = df.iloc[:-1]

        return df

    except Exception as e:
        st.info(f"Database not loaded: {e}")
        return df


if "df" not in st.session_state:
    st.session_state.df = df

try:
    if st.session_state.df.iat[0, 1] == st.session_state.df.iat[1, 1]:
        print(st.session_state.df.iat[0, 1], st.session_state.df.iat[1, 1])
        df = st.session_state.df.iloc[1:].reset_index(drop=True)
        #df = st.session_state.df
        

    else:
        print("printing new data")
        df = read_db(st.session_state.df, db)
    

except:
    print("I am here")
    df = read_db(st.session_state.df, db)


#df = read_db(st.session_state.df, db)
st.session_state.df = df
st.subheader("Sensor Data Plot")
st.dataframe(df)