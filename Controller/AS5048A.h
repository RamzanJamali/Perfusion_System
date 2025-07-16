#ifndef AS5048A_H
#define AS5048A_H

#include <Arduino.h>
#include <SPI.h>

class AS5048A {
public:
    AS5048A(int chipSelectPin);

    void SPI_setup();
    void end_SPI();

    uint16_t get_raw();            // Returns the raw 14-bit angle
    void update_info();            // Call this in the loop as frequently as possible

    bool get_DIR();                // Returns rotation direction: true = CW, false = CCW
    uint16_t get_pos();            // Returns last measured position
    float get_speed();             // Returns speed in RPM

private:
    int value;                     // Chip select pin
};

#endif // AS5048A_H
