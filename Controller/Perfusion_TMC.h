#ifndef PERFUSION_H
#define PERFUSION_H

#include <Arduino.h>
#include <TMC2209.h>

class Perfusion {
public:
    enum PerfusionState { IDLE, PERFUSING, PAUSED };
    enum MotorDirection { STOP, CW, CCW };
    enum ValveState { CLOSED, OPEN };

    // Constructor with hardware pin configuration
    Perfusion(byte RX_PIN, byte TX_PIN, byte VALVE_PIN,
              float target_pressure = 1.0, float flow_rate=0);
    
    // Sensor data update
    void update_data(const String& data);
    
    // Motor control commands
    void start_perfusion();
    void pause_perfusion();
    void continue_perfusion();
    void end_perfusion();
    
    // Main update function (non-blocking, call in loop)
    void update();
    
    // Configuration setters
    void set_target_pressure(float desired_pressure);
    void set_current_pressure(float new_pressure);
    void set_speed(float desired_speed);
    void set_flow_rate(float desired_flow_rate);
    void set_end_position(int position);
    void set_current_motor_speed(float rpm) ;
    
    // State accessors
    PerfusionState get_state() const;
    float get_current_pressure() const;
    float get_target_pressure() const;
    double get_motor_speed() const;
    float get_steps_per_second() const;
    MotorDirection get_motor_direction() const;
    //int get_syringe_position() const;
    float get_tilt() const;
    float get_gyro_x() const;
    float get_gyro_y() const;
    float get_gyro_z() const;
    ValveState get_valve_state() const;
    float get_current_motor_speed() const;
    void open_valve();
    void toggle_valve();

private:
    void move_motor(float speed);
    
    void stop_motor();
    
    // Hardware components
    TMC2209 stepper;
    byte RX_PIN, byte TX_PIN, byte VALVE_PIN, valvePin;
    const long stepsPerRev;
    const uint8_t RUN_CURRENT_PERCENT;
    
    // Perfusion state
    PerfusionState perfusion_state;
    const char* current_command;
    
    // Configuration parameters
    float target_pressure;
    int syringe_end_position;
    
    // Sensor values
    float motor_speed;
    float current_motor_speed;
    ValveState valve_state;
    float current_pressure;
    float flow_rate;
    //int syringe_current_position;
    float tilt;
    float gyro_x, gyro_y, gyro_z;
};

#endif
