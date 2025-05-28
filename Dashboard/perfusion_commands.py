
class Perfusion:
    
    #Holds perfusion state, sensor placeholders, current command name, and target pressure.
    
    def __init__(self, target_pressure: float = 1.0, motor_speed: float = 0):

        self._perfusion_state = "IDLE"
        self._current_command = None

        # sensors (to be updated externally if needed)
        self._motor_speed = motor_speed          # rpm
        self._motor_direction = ""  # "CW" or "CCW" or "STOP"
        self._valve_on = False
        self._target_pressure = target_pressure
        self._current_pressure = 0.0
        self._flow_rate = 0.0 # In future a flow rate should be pre-calculate to determine the motorspeed and user should set flow rate instead of speed. 
        self._syringe_current_position = 0
        self._syringe_desired_position = 0
        self._syringe_end_position = 0 # This could be calculated based on number of steps or by a sensor.
        self._tilt = 0.0
        self._gyro = (0.0, 0.0, 0.0)    # x,y,z


    def update_data(self, data: str):
        """Extract sensor data from a string."""
        try:
            parts = data.split(',')
            #self._humidity = float(parts[0])
            #self._temperature = float(parts[1])
            self._current_pressure = float(parts[2])
            self._tilt = float(parts[3])
            self._gyro = (float(parts[4]), float(parts[5]), float(parts[6]))
            self._motor_speed = float(parts[7])
            self._motor_direction = parts[8].strip()
            self._syringe_current_position = int(parts[9])
        except (ValueError, IndexError) as e:
            print(f"Error extracting sensor data: {e}")



    def _move_motor(self, speed: int, direction: str):
        """Move motor at given speed and direction."""
        self._motor_speed = speed
        self._motor_direction = direction

    
    def _open_valve(self):
        """Open valve to allow flow."""
        if self._current_pressure > self._target_pressure:
            self._valve_on = True
        else:
            self._valve_on = False

    """
    def adjust_syringe_position(self, syringe_desired_position: int):
        #Move motor to absolute syringe position x.
        self._perfusion_state = "ADJUSTING_SYRINGE"
        self._current_command = "Adjust syringe position"
        self._syringe_desired_position = syringe_desired_position
        self._syringe_current_position = self._syringe_current_position  # Placeholder for actual current position
        while True:
            if (self._syringe_current_position - self._syringe_desired_position) > 0:
                self._move_motor(-1, "CCW")
            elif (self._syringe_current_position - self._syringe_desired_position) < 0:
                self._move_motor(1, "CW")   
            else:
                self._move_motor(0, "STOP")
                break
    """
                

    def start_perfusion(self):
        """Begin perfusion run at given speed and direction. Auto‐adjust valve to hit pressure_target."""
        self._perfusion_state = "PERFUSING"
        self._current_command = "Start perfusion"
        while True:
            if (self._syringe_end_position - self._syringe_current_position) > 0:
                self._move_motor(1, "CCW")
            else:
                self._move_motor(0, "STOP")
                self._perfusion_state = "IDLE"
                self._current_command = "End perfusion"
                break

            Perfusion._open_valve(self)



    def pause_perfusion(self):
        """Pause motor run, keep sensors streaming."""
        self._perfusion_state = "PAUSED"
        self._current_command = "Pause perfusion"
        self._valve_on = False
        self._move_motor(0, "STOP")
        

    def continue_perfusion(self):
        self.start_perfusion()


    def end_perfusion(self):
        """End perfusion run, stop motor and close valve."""
        self._perfusion_state = "IDLE"
        self._current_command = "End perfusion"
        self._valve_on = False
        self._move_motor(0, "STOP")


    def set_pressure(self, desired_pressure: float):
        self._target_pressure = desired_pressure


    def set_speed(self, desired_speed: float):
        self._motor_speed = desired_speed


    def set_flow_rate(self, desired_flow_rate: float):
        pass
    