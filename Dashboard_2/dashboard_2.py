import dash
from dash import dcc, html, Input, Output, State, callback_context
import dash_bootstrap_components as dbc
from serial import Serial
import serial.tools.list_ports
import threading
import time
import datetime
from pathlib import Path
import os
from save_data import SensorDatabase # Assuming this class is in a separate file

# --- Global Variables and Initialization ---
# These variables are initialized once when the app starts.
# They are shared across all user sessions and callbacks.

# 1. Setup database directory
DB_DIR = Path(os.path.expanduser("~/Downloads/Perfusion_System/databases"))
DB_DIR.mkdir(parents=True, exist_ok=True)

# 2. Shared buffer for serial data
# The background thread writes to this, and Dash callbacks read from it.
BUFFER = []
DB_STATE = [None, None, False]  # [db_instance, db_path, perfusion_on]


# 3. Serial Port Connection
def connect_serial():
    """Finds available serial ports and connects to the first one."""
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
        return ser
    except Exception as e:
        print(f"Error connecting to serial port: {e}")
        return None

# Initialize serial connection globally
SER = connect_serial()


# --- Helper Functions ---
def make_db_filename():
    """Creates a unique, timestamped filename for the database."""
    now = datetime.datetime.now().strftime("%Y%m%d_%H%M%S")
    return DB_DIR / f"perfusion_{now}.db"

def ensure_db_file(db_path: Path):
    """Ensures the database file exists."""
    if not db_path.exists():
        db_path.touch()

def send_all_commands(cmd_history):
    """Joins commands and sends them over serial."""
    if not SER or not SER.is_open:
        print("Serial port not available. Cannot send command.")
        return
    packet = ",".join(filter(None, cmd_history)) + "\n"
    SER.write(packet.encode())
    print(f"Sent: {packet.strip()}")
    

# --- Background Serial Reader Thread ---
def read_serial_thread(buffer, db_state):
    """
    Continuously reads from the serial port in a background thread.
    Parses data packets, updates the buffer, and handles database logging.
    """
    if not SER:
        print("Serial reader thread not started: no serial connection.")
        return
        
    chunk_array = bytearray()
    while True:
        try:
            if SER.in_waiting:
                chunk = SER.read(SER.in_waiting)
                chunk_array.extend(chunk)
                
                while True:
                    # Find start ('<') and end ('>') markers of a data frame
                    start_index = chunk_array.find(b'<')
                    if start_index == -1: break
                    
                    end_index = chunk_array.find(b'>', start_index + 1)
                    if end_index == -1: break

                    frame = chunk_array[start_index + 1:end_index]
                    del chunk_array[:end_index + 1] # Remove processed frame
                    
                    raw_data = frame.decode('utf-8', errors='ignore').strip()
                    data_list = [item.strip() for item in raw_data.split(',')]

                    # Handle perfusion state and database logging
                    cmd = data_list[0]
                    perfusion_is_on = db_state[2]

                    if cmd == "1" and not perfusion_is_on: # Start Perfusion
                        new_path = make_db_filename()
                        ensure_db_file(new_path)
                        db_state[1] = new_path
                        db_state[0] = SensorDatabase(database_path=new_path)
                        db_state[2] = True
                        print(f"Perfusion started → logging to: {new_path}")
                    
                    elif cmd == "0" and perfusion_is_on: # Stop Perfusion
                        db_state[2] = False
                        print(f"Perfusion stopped for: {db_state[1]}")
                    
                    if db_state[2] and db_state[0] is not None:
                        try:
                            db_state[0].insert_reading(data_list)
                        except Exception as e:
                            # Pass silently to avoid flooding console
                            pass
                    
                    # Update the shared buffer for the UI
                    buffer.append((time.time(), raw_data))
                    if len(buffer) > 50: # Keep buffer size manageable
                        buffer.pop(0)

            else:
                time.sleep(0.1) # Avoid busy-waiting
        except Exception as e:
            print(f"Error in serial reader thread: {e}")
            time.sleep(1)

# Start the background thread once
reader_thread = threading.Thread(target=read_serial_thread, args=(BUFFER, DB_STATE), daemon=True)
reader_thread.start()

# --- Dash App Definition ---
app = dash.Dash(__name__, external_stylesheets=[dbc.themes.BOOTSTRAP])
server = app.server # For deployment environments

# --- App Layout ---
app.layout = dbc.Container(fluid=True, children=[
    # Store for session state, equivalent to st.session_state
    dcc.Store(id='cmd-history-store', data=["IDLE", "15.0", "2.5", "50", "100", "0"]),
    
    # Timers to trigger UI updates
    dcc.Interval(id='interval-live-update', interval=1 * 1000, n_intervals=0), # 1-second update
    dcc.Interval(id='interval-command-repeat', interval=10 * 1000, n_intervals=0), # 10-second repeat
    
    html.H1("📊 Raspberry Pi ↔️ Arduino Dashboard"),
    html.Hr(),

    # --- Control Panel ---
    dbc.Row([
        dbc.Col([
            dbc.Label("Pressure (mmHg)"),
            dbc.Input(id="pressure-input", type="number", value=1.0, step=1.0),
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
            html.H3("Live Motor Status"),
            html.Pre(id='live-motor-status-output', style={'fontSize': '1.2em'}),
            
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

# --- Callbacks ---

@app.callback(
    Output('cmd-history-store', 'data'),
    [
        Input('pressure-btn', 'n_clicks'), Input('flow-rate-btn', 'n_clicks'),
        Input('low-pressure-btn', 'n_clicks'), Input('high-pressure-btn', 'n_clicks'),
        Input('start-btn', 'n_clicks'), Input('pause-btn', 'n_clicks'),
        Input('continue-btn', 'n_clicks'), Input('end-btn', 'n_clicks'),
        Input('valve-btn', 'n_clicks'), Input('reverse-btn', 'n_clicks'),
        Input('interval-command-repeat', 'n_clicks')
    ],
    [
        State('pressure-input', 'value'), State('flow-rate-input', 'value'),
        State('raw-low-input', 'value'), State('raw-high-input', 'value'),
        State('cmd-history-store', 'data')
    ],
    prevent_initial_call=True
)
def handle_commands(
    p_clicks, fr_clicks, rl_clicks, rh_clicks,
    start_clicks, pause_clicks, continue_clicks, end_clicks,
    valve_clicks, reverse_clicks, n_intervals,
    pressure_val, flow_rate_val, raw_low_val, raw_high_val,
    cmd_history):
    """A single callback to handle all button clicks and the periodic command repeat."""
    
    ctx = callback_context
    if not ctx.triggered:
        return cmd_history
    
    button_id = ctx.triggered[0]['prop_id'].split('.')[0]
    new_history = cmd_history[:] # Create a mutable copy
    send_command_flag = False

    # Map button IDs to their actions
    if button_id == 'pressure-btn' and pressure_val is not None:
        new_history[1] = str(pressure_val)
        send_command_flag = True
    elif button_id == 'flow-rate-btn' and flow_rate_val is not None:
        new_history[2] = str(flow_rate_val)
        send_command_flag = True
    elif button_id == 'low-pressure-btn' and raw_low_val is not None:
        new_history[3] = str(raw_low_val)
        send_command_flag = True
    elif button_id == 'high-pressure-btn' and raw_high_val is not None:
        new_history[4] = str(raw_high_val)
        send_command_flag = True
    elif button_id in ['start-btn', 'pause-btn', 'continue-btn', 'end-btn']:
        commands = {'start-btn': 'START_PERFUSION', 'pause-btn': 'PAUSE_PERFUSION',
                    'continue-btn': 'CONTINUE_PERFUSION', 'end-btn': 'END_PERFUSION'}
        new_history[0] = commands[button_id]
        new_history[5] = "0" # Reset valve state on perfusion command
        send_command_flag = True
    elif button_id == 'valve-btn':
        new_history[5] = "1" if new_history[5] == "0" else "0"
        send_command_flag = True
    elif button_id == 'reverse-btn':
        new_history[0] = "IDLE"
        new_history[2] = "-1000" # Set reverse flow rate
        send_command_flag = True
    elif button_id == 'interval-command-repeat':
        # Don't change history, just resend the last state
        send_command_flag = True

    if send_command_flag:
        send_all_commands(new_history)

    return new_history


@app.callback(
    Output('live-motor-status-output', 'children'),
    Input('interval-live-update', 'n_intervals')
)
def update_live_status(n):
    """Updates the 'Live Motor Status' text."""
    if not BUFFER:
        return "Waiting for data..."
    latest_time, latest_line = BUFFER[-1]
    return latest_line

@app.callback(
    Output('serial-log-output', 'children'),
    [Input('interval-live-update', 'n_intervals'),
     Input('show-log-checkbox', 'value')]
)
def update_serial_log(n, show_log):
    """Updates the serial log display in the sidebar."""
    if not show_log:
        return ""
    if not BUFFER:
        return "No serial data received yet."
    
    log_lines = []
    for t, msg in reversed(BUFFER):
        timestamp = time.strftime('%H:%M:%S', time.localtime(t))
        log_lines.append(f"{timestamp} → {msg}")
    
    return html.Pre("\n".join(log_lines))

@app.callback(
    Output('data-table-container', 'children'),
    Input('interval-live-update', 'n_intervals')
)
def update_data_table(n):
    """Updates the data table with the latest readings from the buffer."""
    if not BUFFER:
        return dbc.Alert("Database not loaded or no data available!", color="info")

    headers = ["Timestamp", "Perfusion State", "Valve", "Humidity", "Temp", "Envir Press", "AQI", "Current Press", "Target Press", "Motor Speed"]
    
    # Get the last 10 valid readings
    rows = []
    for t, msg in reversed(BUFFER):
        if len(rows) >= 10: break
        
        data_list = [item.strip() for item in msg.split(',')]
        if len(data_list) > 1 and data_list[0] != "0":
            timestamp = time.strftime('%H:%M:%S', time.localtime(t))
            rows.append([timestamp] + data_list)

    if not rows:
        return dbc.Alert("Waiting for perfusion data...", color="info")
        
    table_header = [html.Thead(html.Tr([html.Th(h) for h in headers]))]
    table_body = [html.Tbody([html.Tr([html.Td(col) for col in row]) for row in rows])]
    
    return dbc.Table(table_header + table_body, bordered=True, striped=True, hover=True, responsive=True)


if __name__ == '__main__':
    app.run(debug=True, use_reloader=False)