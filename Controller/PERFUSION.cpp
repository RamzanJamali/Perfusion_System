// PERFUSION.cpp
#include "PERFUSION.h"

Perfusion::Perfusion(uint8_t VALVE_PIN, float target_pressure = 1.0)
    :   perfusion_state(IDLE),
      current_command(nullptr),
      target_pressure(target_pressure),
      syringe_end_position(0),
      valve_state(CLOSED),
      current_pressure(2.0)
    {
        pinMode(VALVE_PIN, OUTPUT);
        //digitalWrite(VALVE_PIN, LOW); // Close valve initially
        
    }

/*
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
}*/


void Perfusion::opened_valve() {
    valve_state = OPEN;
    digitalWrite(VALVE_PIN, HIGH);
}

void Perfusion::closed_valve() {
    valve_state = CLOSED;
    digitalWrite(VALVE_PIN, LOW);
}


void Perfusion::open_valve() {

    valve_state = (current_pressure > target_pressure) ? OPEN : CLOSED;
    digitalWrite(VALVE_PIN, valve_state == OPEN ? HIGH : LOW);

}

void Perfusion::start_perfusion() {
    perfusion_state = PERFUSING;
    current_command = "Start perfusion";
    
    if (syringe_end_position == LOW) {
        open_valve();
    } else {
        end_perfusion();
    }
}


void Perfusion::pause_perfusion() {
    perfusion_state = PAUSED;
    current_command = "Pause perfusion";
    valve_state = CLOSED;

}

void Perfusion::continue_perfusion() {
    perfusion_state = PERFUSING;
    current_command = "Continue perfusion";

}

void Perfusion::end_perfusion() {
    perfusion_state = IDLE;
    current_command = "End perfusion";

}

void Perfusion::set_target_pressure(float desired_pressure) {
    target_pressure = desired_pressure;
}

void Perfusion::set_current_pressure(float new_pressure){
    current_pressure = new_pressure;
}

/*
void PERFUSION::set_speed(float desired_speed) {
    motor_speed = (desired_speed * 51200)/(60*0.715);
    //stepperDriver.moveAtVelocity(motor_speed);
}

void PERFUSION::set_current_motor_speed(float rpm) {
    current_motor_speed = rpm;
}

void PERFUSION::set_flow_rate(float desired_flow_rate) {
    if (desired_flow_rate >= 0.1) { // The lowest flow_rate limit
        motor_speed = desired_flow_rate * 65.77363377; // The number 65.77363377 is calculated according to perfusion system parameters. See Formulas.xlsx for further calculations
        flow_rate = desired_flow_rate;
    }
    else {
        motor_speed = 6.577363377; // microstepping period for flow_rate at 0.1 mL per day
        flow_rate = 0.1;
    }
}
*/
void Perfusion::set_end_position(int position) {
    syringe_end_position = position;
    end_perfusion();
}

// Accessor methods
Perfusion::PerfusionState Perfusion::get_state() const { return perfusion_state; }
float Perfusion::get_current_pressure() const { return current_pressure; }
float Perfusion::get_target_pressure() const { return target_pressure; }
//double PERFUSION::get_motor_speed() const { return motor_speed; }
//float PERFUSION::get_current_motor_speed() const { return current_motor_speed; }
//float PERFUSION::get_steps_per_second() const {return flow_rate;}
//PERFUSION::MotorDirection PERFUSION::get_motor_direction() const { return motor_direction; }
//int PERFUSION::get_syringe_position() const { return syringe_current_position; }
float Perfusion::get_tilt() const { return tilt; }
float Perfusion::get_gyro_x() const { return gyro_x; }
float Perfusion::get_gyro_y() const { return gyro_y; }
float Perfusion::get_gyro_z() const { return gyro_z; }
Perfusion::ValveState Perfusion::get_valve_state() const { return valve_state; }
