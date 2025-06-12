import sqlite3
from pandas import read_sql_query

class SensorDatabase:
    def __init__(self, database_path='sensor_data.db'):
        self.database_path = database_path
        self._create_table()
        
    def _create_table(self):
        """Create optimized table structure with indexes"""
        with sqlite3.connect(self.database_path, isolation_level=None) as conn:
            cursor = conn.cursor()
            cursor.execute('''
                CREATE TABLE IF NOT EXISTS sensor_readings (
                    id INTEGER PRIMARY KEY AUTOINCREMENT,
                    timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,
                    perfusion_state INTEGER,
                    valve_state INTEGER,
                    humidity REAL,
                    temperature REAL,
                    current_pressure REAL,
                    target_pressure REAL,
                    motor_speed REAL,
                    tilt REAL,
                    gyro_x REAL,
                    gyro_y REAL,
                    gyro_z REAL
                )
            ''')
            # Create indexes for fast timestamp-based queries
            cursor.execute('CREATE INDEX IF NOT EXISTS idx_timestamp ON sensor_readings(timestamp)')
            cursor.execute('CREATE INDEX IF NOT EXISTS idx_id ON sensor_readings(id)')
            conn.commit()

    def insert_reading(self, sensor_data):
        """Insert a sensor reading with efficient batch processing"""
        cleaned = []
        for item in sensor_data:
            s = str(item).strip()
            if not s:
                continue
            # try int first, then float
            try:
                val = int(s)
            except ValueError:
                try:
                    val = float(s)
                except ValueError:
                    # skip non-numeric
                    continue
            cleaned.append(val)

        # 2) Validate length
        if len(cleaned) != 11:
            raise ValueError(
                f"Expected 11 numeric fields, but got {len(cleaned)}: {cleaned!r}"
            )
        
        sensor_data = cleaned
        with sqlite3.connect(self.database_path, isolation_level=None) as conn:
            cursor = conn.cursor()
            cursor.execute('''
                INSERT INTO sensor_readings (
                    perfusion_state, valve_state,
                    humidity, temperature, current_pressure, target_pressure, motor_speed, 
                    tilt, gyro_x, gyro_y, gyro_z
                ) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
            ''', (
                sensor_data[0],
                sensor_data[1],
                sensor_data[2],
                sensor_data[3],
                sensor_data[4],
                sensor_data[5],
                sensor_data[6],
                sensor_data[7],
                sensor_data[8],
                sensor_data[9],
                sensor_data[10]
            ))
            conn.commit()

    def get_recent_readings(self, limit=1):
        """Retrieve last N readings efficiently using index"""
        with sqlite3.connect(self.database_path, isolation_level=None) as conn:
            # Use pandas for direct DataFrame conversion
            return read_sql_query(
                f'SELECT * FROM sensor_readings ORDER BY id DESC LIMIT {limit}',
                conn
            )