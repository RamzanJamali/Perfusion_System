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
                    humidity REAL,
                    temperature REAL,
                    pressure REAL,
                    tilt REAL,
                    gyro_x REAL,
                    gyro_y REAL,
                    gyro_z REAL,
                    motor_speed REAL,
                    motor_direction TEXT,
                    syringe_position INTEGER
                )
            ''')
            # Create indexes for fast timestamp-based queries
            cursor.execute('CREATE INDEX IF NOT EXISTS idx_timestamp ON sensor_readings(timestamp)')
            cursor.execute('CREATE INDEX IF NOT EXISTS idx_id ON sensor_readings(id)')
            conn.commit()

    def insert_reading(self, sensor_data):
        """Insert a sensor reading with efficient batch processing"""
        with sqlite3.connect(self.database_path, isolation_level=None) as conn:
            cursor = conn.cursor()
            cursor.execute('''
                INSERT INTO sensor_readings (
                    humidity, temperature, pressure, tilt,
                    gyro_x, gyro_y, gyro_z,
                    motor_speed, motor_direction, syringe_position
                ) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
            ''', (
                sensor_data['humidity'],
                sensor_data['temperature'],
                sensor_data['pressure'],
                sensor_data['tilt'],
                sensor_data['gyro'][0],
                sensor_data['gyro'][1],
                sensor_data['gyro'][2],
                sensor_data['motor_speed'],
                sensor_data['motor_direction'],
                sensor_data['syringe_position']
            ))
            conn.commit()

    def get_recent_readings(self, limit=1000):
        """Retrieve last N readings efficiently using index"""
        with sqlite3.connect(self.database_path, isolation_level=None) as conn:
            # Use pandas for direct DataFrame conversion
            return read_sql_query(
                f'SELECT * FROM sensor_readings ORDER BY id DESC LIMIT {limit}',
                conn
            )