import streamlit as st
from streamlit_autorefresh import st_autorefresh
from serial import Serial
import threading
import time
import pandas as pd



# Trigger auto‑refresh every 500 ms (infinite)
st_autorefresh(interval=500, limit=None, key="serial_refresh")


# ————— CONFIG ————— and ————— SETUP SERIAL —————
@st.cache_resource
def connect():
    BAUD_RATE    = 115200
    
    try:
        ser = Serial(port='COM7', baudrate=BAUD_RATE, timeout=1)
        return ser
    
    except:
        try:
            ser = Serial('/dev/ttyUSB0', 9600, timeout=1)
            return ser
        
        except:
            try:
                ser = Serial('/dev/ttyUSB1', 9600, timeout=1)
                return ser
        
            except Exception as error:
                st.error("No serial port found. Please check your connection.")
                return None



ser = connect()

# ————— DATA BUFFER —————
@st.cache_resource
def get_buffer():
    """Return a mutable list to hold recent readings."""
    return []

buffer = get_buffer()

# ————— BACKGROUND READER —————
def read_serial(buffer):
    """Continuously read lines from serial and append parsed values."""
    while True:
        try:
            line = ser.readline().decode("utf-8").strip()
            if line == "":
                continue
            line = (f"Arduino response: {line}")
            
        
        except:
            continue
        
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


# Command buttons
st.markdown("""
    <style>
    .st-key-clockwise button {
        background-color: #28a745; /* green */
        color: white;
    }
    .st-key-clockwise button:hover {
        background-color: #218838; /* darker green */
        color: white;
        border: 1px solid #218838;
    }
            
    .st-key-counterclockwise button {
        background-color: #28a745; /* green */
        color: white;
    }
    .st-key-counterclockwise button:hover {
        background-color: #218838; /* darker green */
        color: white;
        border: 1px solid #218838;
    }
            
    .st-key-stop button {
        background-color: #dc3545; /* red */
        color: white;
    }
    .st-key-stop button:hover {
        background-color: #c82333; /* darker red */
        color: white;
        border: 1px solid #c82333;
    }
    </style>
""", unsafe_allow_html=True)


col1, col2, col3 = st.columns(3)
with col1:
    if st.button("RUN CLOCKWISE", key="clockwise"):
        ser.write(b"RUN_CLOCKWISE\n")

with col2:
    if st.button("RUN COUNTERCLOCKWISE", key="counterclockwise"):
        ser.write(b"RUN_COUNTERCLOCKWISE\n")

with col3:
    if st.button("STOP MOTOR", key="stop"): 
        ser.write(b"STOP_MOTOR\n")

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

