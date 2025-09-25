import dash
from dash import dcc, html, Input, Output, State
import dash_bootstrap_components as dbc
from dash import dash_table  # Import dash_table module
from dash.exceptions import PreventUpdate
from serial import Serial
import serial.tools.list_ports
import threading
import time
from collections import deque
import datetime
from pathlib import Path
import os
from waitress import serve
from save_data import SensorDatabase  # Uncomment when ready

# --- Global Variables and Initialization ---
# Setup database directory
DB_DIR = Path(os.path.expanduser("~/Downloads/Perfusion_System/databases"))
DB_DIR.mkdir(parents=True, exist_ok=True)

# Shared buffer for serial data
buffer = deque(maxlen=15)  # For storing serial messages
data_rows = [("timestamp", "perfusion_state", "valve_state", "humidity", "temperature", 
              "envir_pressure", "AQI", "current_pressure", "target_pressure", "motor_speed")]
DB_STATE = [None, None, False]  # [db_instance, db_path, perfusion_on]

# Timing variables
last_displayed_second = None  # Track the last second we displayed a message
last_data_table_update = 0  # Track last data table update time
table_update_interval = 0.5  # Update table every 1 second
is_connected = False  # Track connection status

# --- Helper Functions ---
def make_db_filename():
    """Creates a unique, timestamped filename for the database."""
    now = datetime.datetime.now().strftime("%Y%m%d_%H%M%S")
    return DB_DIR / f"perfusion_{now}.db"

def ensure_db_file(db_path: Path):
    """Ensures the database file exists."""
    if not db_path.exists():
        db_path.touch()

def connect_serial():
    """Finds available serial ports and connects to the first one."""
    global is_connected
    try:
        ports = serial.tools.list_ports.comports()
        active_ports = [p for p in sorted(ports) if p.hwid != 'n/a']
        if not active_ports:
            print("❌ No active serial ports found.")
            return None
        
        port_device = active_ports[0].device
        for p in active_ports:
            print(f"Found serial port: {p.device} - {p.description}")
        print(f"🔌 Connecting to port: {port_device}")
        ser = Serial(port=port_device, baudrate=115200, timeout=1)
        time.sleep(1) # Wait for connection to establish
        is_connected = True
        return ser
    except Exception as e:
        print(f"Error connecting to serial port: {e}")
        return None

def send_all_commands(cmd_history):
    """Joins commands and sends them over serial."""
    if not SER or not SER.is_open or not is_connected:
        print("Serial port not available. Cannot send command.")
        return
    packet = ",".join(filter(None, cmd_history)) + "\n"
    try:
        SER.write(packet.encode())
        print(f"Sent: {packet.strip()}")
    except Exception as e:
        print(f"Error sending command: {e}")

# --- Background thread for reading serial ---
def read_serial():
    global buffer, DB_STATE, last_displayed_second, last_data_table_update
    if not SER or not is_connected:
        return
        
    chunk_array = bytearray()
    
    while is_connected:
        try:
            if SER.in_waiting:
                chunk = SER.read(55)
                chunk_array.extend(chunk)
                
                while True:
                    try:
                        start_index = chunk_array.index(b'<')
                        end_index = chunk_array.index(b'>', start_index + 1)
                    except ValueError:
                        break

                    frame = chunk_array[start_index + 1:end_index]
                    del chunk_array[:end_index + 1]
                    
                    raw_data = frame.decode('utf-8').strip()
                    data_list = [item.strip() for item in raw_data.split(',')]
                    data_list.insert(0, time.strftime('%H:%M:%S', time.localtime()))  # Add timestamp at the start
                    
                    # Get current time
                    current_time = time.time()
                    current_second = int(current_time)
                    
                    # Only store one message per second
                    if last_displayed_second != current_second:
                        # Database handling
                        cmd = data_list[1]
                        if cmd == "1" and not DB_STATE[2]:  # START_PERFUSION
                            new_path = make_db_filename()
                            ensure_db_file(new_path)
                            DB_STATE[1] = new_path
                            DB_STATE[0] = SensorDatabase(database_path=new_path)  # Uncomment when ready
                            DB_STATE[2] = True
                            print(f"Perfusion started → logging to: {new_path}")
                        
                        if cmd == "0" and DB_STATE[2]:  # STOP_PERFUSION
                            DB_STATE[2] = False
                            print(f"Perfusion stopped for: {DB_STATE[1]}")
                        
                        if DB_STATE[2] and DB_STATE[0] is not None:
                            try:
                                DB_STATE[0].insert_reading(data_list[1:])  # Uncomment when ready
                                pass
                            except Exception as e:
                                pass
                        
                        # Add to buffer with formatted timestamp
                        formatted_time = time.strftime('%H:%M:%S', time.localtime(current_time))
                        buffer.appendleft((formatted_time, raw_data))
                        
                        # Update last displayed second
                        last_displayed_second = current_second
                        
                        # Update data table immediately
                        if len(data_list) > 1 and data_list[1] != "0":
                            global data_rows
                            data_rows.insert(1, data_list)
                            if len(data_rows) > 11:
                                data_rows.pop()
                            
                            # Update last update time
                            last_data_table_update = current_time
            
            else:
                time.sleep(0.1)
                
        except Exception as e:
            print(f"Serial error: {e}")
            time.sleep(1)

# Initialize serial connection globally
SER = connect_serial()

# Start the background thread once
if SER and is_connected:
    reader_thread = threading.Thread(target=read_serial, daemon=True)
    reader_thread.start()

# --- Dash App Definition ---
app = dash.Dash(__name__, external_stylesheets=[dbc.themes.BOOTSTRAP])
server = app.server  # For deployment environments

# --- App Layout ---
app.layout = dbc.Container(fluid=True, children=[
    # Store for session state, equivalent to st.session_state
    dcc.Store(id='cmd-history-store', data=["IDLE", "15.0", "2.5", "0", "0", "0"]),
    
    # Timer to trigger UI updates (only for display refresh)
    dcc.Interval(id='interval-live-update', interval=1 * 1000, n_intervals=0),  # 1-second update
    
    html.H1("📊 Raspberry Pi ↔️ Arduino Dashboard"),
    html.Hr(),

    # --- Control Panel ---
    dbc.Row([
        dbc.Col([
            dbc.Label("Pressure (mmHg)"),
            dbc.Input(id="pressure-input", type="number", value=15.0, step=1.0),
            dbc.Button("SET PRESSURE", id="pressure-btn", color="success", className="mt-2 w-100"),
        ], width=3),
        dbc.Col([
            dbc.Label("Flow Rate (ml/day)"),
            dbc.Input(id="flow-rate-input", type="number", value=2.5, step=0.1),
            dbc.Button("SET FLOW RATE", id="flow-rate-btn", color="success", className="mt-2 w-100"),
        ], width=3),
        dbc.Col([
            dbc.Label("Set Raw Low Pressure"),
            dbc.Input(id="raw-low-input", type="number", value=50, step=1),
            dbc.Button("SET RAW LOW", id="low-pressure-btn", color="success", className="mt-2 w-100"),
        ], width=3),
        dbc.Col([
            dbc.Label("Set Raw High Pressure"),
            dbc.Input(id="raw-high-input", type="number", value=100, step=1),
            dbc.Button("SET RAW HIGH", id="high-pressure-btn", color="success", className="mt-2 w-100"),
        ], width=3),
    ], className="mb-3"),

    dbc.Row([
        dbc.Col(dbc.Button("▶️ START PERFUSION", id="start-btn", className="w-100")),
        dbc.Col(dbc.Button("⏸️ PAUSE PERFUSION", id="pause-btn", className="w-100")),
        dbc.Col(dbc.Button("⏯️ CONTINUE PERFUSION", id="continue-btn", className="w-100")),
    ], className="mb-3 g-2"),
    
    dbc.Row([
        dbc.Col(dbc.Button("⏹️ END PERFUSION", id="end-btn", className="w-100")),
        dbc.Col(dbc.Button("🔄 TOGGLE VALVE", id="valve-btn", className="w-100")),
        dbc.Col(dbc.Button("🔙 REVERSE FLOW", id="reverse-btn", className="w-100")),
    ], className="mb-4 g-2"),
    
    html.Hr(),
    
    # --- Live Data Display ---
    dbc.Row([
        # Main content area
        dbc.Col([
            html.H3("Sensor Data Log", className="mt-4"),
            html.Div(id='data-table-container'),
        ], width=9),

        # Sidebar for raw serial log
        dbc.Col([
            html.H3("🔄 Serial Log"),
            dbc.Checkbox(id='show-log-checkbox', label="Show full log", value=False),
            html.Div(id='serial-log-output', style={
                'height': '400px',
                'overflowY': 'scroll',
                'border': '1px solid #ccc',
                'padding': '10px',
                'marginTop': '10px'
            }),
        ], width=3),
    ]),
])

# --- Optimized Callbacks ---

# Callback for serial log
@app.callback(
    Output('serial-log-output', 'children'),
    Input('interval-live-update', 'n_intervals'),
    Input('show-log-checkbox', 'value')
)
def update_serial_log(n, show_log):
    if not show_log or not buffer:
        return []
    
    # Simply return pre-formatted buffer contents
    return [html.P(f"{ts} → {msg}") for ts, msg in buffer]

# Callback for data table
@app.callback(
    Output('data-table-container', 'children'),
    Input('interval-live-update', 'n_intervals')
)
def update_data_table(n):
    global last_data_table_update
    current_time = time.time()
    
    # Only update if enough time has passed since last update
    if current_time - last_data_table_update < table_update_interval:
        raise PreventUpdate
    
    # Create table only when necessary
    return dash_table.DataTable(
        id='data-table',
        columns=[{"name": col, "id": str(i)} for i, col in enumerate(data_rows[0])],
        data=[{str(i): val for i, val in enumerate(row)} for row in data_rows[1:]],
        page_size=10,
        style_table={'overflowX': 'auto'}
    )

# Command history callback
@app.callback(
    Output('cmd-history-store', 'data'),
    [
        Input('pressure-btn', 'n_clicks'), 
        Input('flow-rate-btn', 'n_clicks'),
        Input('low-pressure-btn', 'n_clicks'), 
        Input('high-pressure-btn', 'n_clicks'),
        Input('start-btn', 'n_clicks'), 
        Input('pause-btn', 'n_clicks'),
        Input('continue-btn', 'n_clicks'), 
        Input('end-btn', 'n_clicks'),
        Input('valve-btn', 'n_clicks'), 
        Input('reverse-btn', 'n_clicks'),
    ],
    [
        State('pressure-input', 'value'), 
        State('flow-rate-input', 'value'),
        State('raw-low-input', 'value'), 
        State('raw-high-input', 'value'),
        State('cmd-history-store', 'data')
    ],
    prevent_initial_call=True
)
def update_command_history(press_btn, flow_btn, low_press_btn, high_press_btn, 
                          start_btn, pause_btn, continue_btn, end_btn, 
                          valve_btn, reverse_btn,
                          pressure_val, flow_val, raw_low_val, raw_high_val,
                          current_history):
    # Identify which button was clicked
    ctx = dash.callback_context
    if not ctx.triggered:
        raise PreventUpdate
    
    button_id = ctx.triggered[0]['prop_id'].split('.')[0]
    new_history = current_history.copy()
    
    # Update command history based on button clicked
    if button_id == 'pressure-btn' and pressure_val is not None:
        new_history[1] = str(pressure_val)
    elif button_id == 'flow-rate-btn' and flow_val is not None:
        new_history[2] = str(flow_val)
    elif button_id == 'low-pressure-btn' and raw_low_val is not None:
        new_history[3] = str(raw_low_val)
    elif button_id == 'high-pressure-btn' and raw_high_val is not None:
        new_history[4] = str(raw_high_val)
    elif button_id == 'start-btn':
        new_history[0] = "START_PERFUSION"
        new_history[5] = "0"
    elif button_id == 'pause-btn':
        new_history[0] = "PAUSE_PERFUSION"
    elif button_id == 'continue-btn':
        new_history[0] = "CONTINUE_PERFUSION"
    elif button_id == 'end-btn':
        new_history[0] = "END_PERFUSION"
    elif button_id == 'valve-btn':
        # Toggle valve state
        new_history[5] = "1" if current_history[5] == "0" else "0"
    elif button_id == 'reverse-btn':
        # Toggle flow direction
        if float(current_history[2]) >= 0:
            new_history[2] = "-2000"
            new_history[0] = "IDLE"
        else:
            new_history[2] = "2.5"
    
    # Send commands immediately after updating history
    send_all_commands(new_history)
    return new_history

if __name__ == '__main__':
    #app.run(debug=True, use_reloader=False)
    print("Server running on http://127.0.0.50:8050")
    serve(app, host="127.0.0.50", port=8050)
    