class Sensor_Data:
    def __init__(self):
        self._humidity = 0.0
        self._temperature = 0.0
        self._pressure = 0.0
        self._tilt = 0.0
        self._gyro = (0.0, 0.0, 0.0)
        self._motor_speed = 0.0
        self._motor_direction = "" # could be "CW", "CCW", or "STOP"
        self._syringe_current_position = 0


    def extract_sensor_data(self, data: str):
        """Extract sensor data from a string."""
        try:
            parts = data.split(',')
            self._humidity = float(parts[0])
            self._temperature = float(parts[1])
            self._pressure = float(parts[2])
            self._tilt = float(parts[3])
            self._gyro = (float(parts[4]), float(parts[5]), float(parts[6]))
            self._motor_speed = float(parts[7])
            self._motor_direction = parts[8].strip()
            self._syringe_current_position = int(parts[9])
        except (ValueError, IndexError) as e:
            print(f"Error extracting sensor data: {e}")


    def get_sensor_data(self):
        """Return sensor data as a dictionary."""
        return {
            "humidity": self._humidity,
            "temperature": self._temperature,
            "pressure": self._pressure,
            "tilt": self._tilt,
            "gyro": self._gyro,
            "motor_speed": self._motor_speed,
            "motor_direction": self._motor_direction,
            "syringe_current_position": self._syringe_current_position
        }
