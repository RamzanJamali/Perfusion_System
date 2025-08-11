import sqlite3


class SensorDatabase:
    def __init__(self, database_path='sensor_data.db'):
        self.database_path = database_path
        self._create_table()
        
    def _create_table(self):
        """Create optimized table structure with necessary index"""
        with sqlite3.connect(self.database_path, isolation_level=None) as conn:
            cursor = conn.cursor()
            cursor.execute('''
                CREATE TABLE IF NOT EXISTS sensor_readings (
                    id INTEGER PRIMARY KEY AUTOINCREMENT,
                    timestamp DATETIME DEFAULT (datetime('now', 'localtime')),
                    perfusion_state INTEGER,
                    valve_state INTEGER,
                    humidity REAL,
                    temperature REAL,
                    envir_pressure REAL,
                    AQI REAL,
                    current_pressure REAL,
                    target_pressure REAL,
                    motor_speed REAL
                )
            ''')
            # Create index for timestamp queries only
            cursor.execute('CREATE INDEX IF NOT EXISTS idx_timestamp ON sensor_readings(timestamp)')
            conn.commit()

    def insert_reading(self, sensor_data):
        """Efficiently insert a sensor reading with optimized cleaning"""
        if len(sensor_data) != 9:
            raise ValueError(
                f"Expected 9 numeric fields, but got {len(sensor_data)}: {sensor_data!r}"
            )
        
        cleaned = []
        for item in sensor_data[:9]:  # Process only first 9 items
            s = str(item).strip()
            if not s:
                continue
            try:
                # Convert directly to float (covers both int and float)
                val = float(s)
                cleaned.append(val)
            except ValueError:
                continue  # Skip non-numeric
        
        if len(cleaned) != 9:
            raise ValueError(
                f"Expected 9 numeric fields, but got {len(cleaned)} after cleaning"
            )
        
        with sqlite3.connect(self.database_path, isolation_level=None) as conn:
            conn.execute('''
                INSERT INTO sensor_readings (
                    perfusion_state, valve_state, humidity, temperature, 
                    envir_pressure, AQI, current_pressure, target_pressure, motor_speed
                ) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)
            ''', cleaned)
            conn.commit()

        