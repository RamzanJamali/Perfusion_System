#include "Perfusion.h"

Perfusion::Perfusion(byte RX_PIN, byte TX_PIN, byte VALVE_PIN,
                     float target_pressure, float flow_rate)
    : stepper(flow_rate),
      VALVE_PIN(VALVE_PIN),
      //stepsPerRev(motorStepsPerRev * microsteps),
      perfusion_state(IDLE),
      current_command(nullptr),
      target_pressure(target_pressure),
      syringe_end_position(0),
      motor_speed(motor_speed), // RUN_VELOCITY
      RUN_CURRENT_PERCENT(60);
      current_motor_speed(0),
      motor_direction(STOP),
      valve_state(CLOSED),
      current_pressure(2.0),
      flow_rate(flow_rate),
      //syringe_current_position(0),
      tilt(0.0),
      gyro_x(0.0),
      gyro_y(0.0),
      gyro_z(0.0) {
    
    // Initialize hardware
    stepper.setup(soft_serial);
    stepper.setRunCurrent(RUN_CURRENT_PERCENT);
    stepper.enableCoolStep();
    stepper.setMicrostepsPerStep(256); // Set microstepping
    stepper.enable();
    
    pinMode(VALVE_PIN, OUTPUT);
    digitalWrite(VALVE_PIN, LOW); // Close valve initially
}

void Perfusion::update_data(const String& data) {
    // Split comma-separated values
    int start_index = 0;
    int end_index = data.indexOf(',');
    int param_index = 0;
    
    while (end_index != -1 && param_index < 10) {
        String part = data.substring(start_index, end_index);
        start_index = end_index + 1;
        end_index = data.indexOf(',', start_index);
        
        switch (param_index) {
            case 2:  // Pressure
                current_pressure = part.toFloat();
                break;
            case 3:  // Tilt
                tilt = part.toFloat();
                break;
            case 4:  // Gyro X
                gyro_x = part.toFloat();
                break;
            case 5:  // Gyro Y
                gyro_y = part.toFloat();
                break;
            case 6:  // Gyro Z
                gyro_z = part.toFloat();
                break;
            case 7:  // Motor speed
                current_motor_speed = part.toFloat();
                break;
            case 8:  // Motor direction
                if (part == "CW") motor_direction = CW;
                else if (part == "CCW") motor_direction = CCW;
                else motor_direction = STOP;
                break;
            case 9:  // Current State
                //current_command = part;
                break;
        }
        param_index++;
    }
}

void Perfusion::move_motor(float flow) {

    
    // Enable motor driver
    stepper_driver.enable();
    
    // Configure stepper
    stepper.setRunCurrent(60);
    
    stepper_driver.moveAtVelocity(flow);
}

void Perfusion::stop_motor() {
    stepper.disable();
    motor_direction = STOP;
}

void Perfusion::toggle_valve() {
    if (valve_state == CLOSED){
        valve_state = OPEN;
    }
    else {
        valve_state = CLOSED;
    }
    digitalWrite(VALVE_PIN, valve_state == OPEN ? HIGH : LOW);

}

void Perfusion::open_valve() {

    valve_state = (current_pressure > target_pressure) ? OPEN : CLOSED;
    digitalWrite(VALVE_PIN, valve_state == OPEN ? HIGH : LOW);

}

void Perfusion::start_perfusion() {
    perfusion_state = PERFUSING;
    current_command = "Start perfusion";
    
    if (syringe_end_position == LOW) {
        move_motor(flow_rate);
        open_valve();
    } else {
        end_perfusion();
    }
}
/*
void Perfusion::update() {
    if (perfusion_state == PERFUSING) {
        // Check if we've reached the end position
        if (syringe_current_position >= syringe_end_position) {
            end_perfusion();
        } else {
            // Update valve based on current pressure
            open_valve();
        }
    }
}*/

void Perfusion::pause_perfusion() {
    perfusion_state = PAUSED;
    current_command = "Pause perfusion";
    valve_state = CLOSED;
    digitalWrite(VALVE_PIN, LOW);
    stop_motor();
}

void Perfusion::continue_perfusion() {
    perfusion_state = PERFUSING;
    current_command = "Continue perfusion";

    move_motor(flow_rate);
}

void Perfusion::end_perfusion() {
    perfusion_state = IDLE;
    current_command = "End perfusion";
    open_valve();
    //valve_state = CLOSED;
    //digitalWrite(VALVE_PIN, LOW);
    stop_motor();
}

void Perfusion::set_target_pressure(float desired_pressure) {
    target_pressure = desired_pressure;
}

void Perfusion::set_current_pressure(float new_pressure){
    current_pressure = new_pressure;
}

void Perfusion::set_speed(float desired_speed) {
    motor_speed = (desired_speed * 51200)/(60*0.715);
    stepper.moveAtVelocity(motor_speed);
}

void Perfusion::set_current_motor_speed(float rpm) {
    current_motor_speed = rpm;
}

void Perfusion::set_flow_rate(float desired_flow_rate) {
    if (0.58824 * desired_flow_rate >= 1.7) {
        flow_rate = 0.58824 * desired_flow_rate; // The number 0.58824 is calculated according to 10mL syringe. See notebook for further calculations
    }
    else {
    flow_rate = 1.7;
    // Implement flow-to-speed conversion logic here
    }
}

void Perfusion::set_end_position(int position) {
    syringe_end_position = position;
}

// Accessor methods
Perfusion::PerfusionState Perfusion::get_state() const { return perfusion_state; }
float Perfusion::get_current_pressure() const { return current_pressure; }
float Perfusion::get_target_pressure() const { return target_pressure; }
double Perfusion::get_motor_speed() const { return motor_speed; }
float Perfusion::get_current_motor_speed() const { return current_motor_speed; }
float Perfusion::get_steps_per_second() const {return flow_rate;}
Perfusion::MotorDirection Perfusion::get_motor_direction() const { return motor_direction; }
//int Perfusion::get_syringe_position() const { return syringe_current_position; }
float Perfusion::get_tilt() const { return tilt; }
float Perfusion::get_gyro_x() const { return gyro_x; }
float Perfusion::get_gyro_y() const { return gyro_y; }
float Perfusion::get_gyro_z() const { return gyro_z; }
Perfusion::ValveState Perfusion::get_valve_state() const { return valve_state; }
