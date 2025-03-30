#include "devices.hpp"
#include <string>
#include <thread>
#include <fstream>

#ifndef MOCK
#include <wiringPi.h>
#endif

#include <iostream>

power_switch::power_switch(int pin, const std::string &name)
    : pin(pin), name(name)
{
#ifndef MOCK
    pinMode(pin, OUTPUT);
#endif
    turn_off();
}

void power_switch::turn_on()
{
    if (on)
    {
        return;
    }

    on = true;
#ifndef MOCK
    digitalWrite(pin, HIGH);
#endif
    std::cout << "Switching " << name << " on (pin " << pin << ")\n";
}

void power_switch::turn_off()
{
    if (!on)
    {
        return;
    }
    
    on = false;
#ifndef MOCK
    digitalWrite(pin, LOW);
#endif
    std::cout << "Switching " << name << " off (pin " << pin << ")\n";
}

power_switch::~power_switch()
{
    turn_off();
}

linear_motor::linear_motor(int pin_pos, int pin_neg, int extension_time, int contraction_time, const std::string &name)
    : pin_pos(pin_pos), pin_neg(pin_neg), extension_time(extension_time), contraction_time(contraction_time), name(name)
{
#ifndef MOCK
    pinMode(pin_pos, OUTPUT);
    pinMode(pin_neg, OUTPUT);
#endif
    // Recalibrate the motor by exending and contracting,
    // Exending will extend it all the way to the end
    // Currently there's an assumption that we want the motor
    // to be operating towards it's maximum extension.
    // i.e. fully extended motor means closed door
    extend();
    // contract();
}

void linear_motor::contract()
{
    if (!extended)
    {
        return;
    }

    std::cout << "Contracting " << name << " motor (pins: " << pin_pos << ", " << pin_neg << ") for "<< contraction_time <<"\n";
#ifndef MOCK
    digitalWrite(pin_pos, LOW);
    digitalWrite(pin_neg, HIGH);
    std::this_thread::sleep_for(contraction_time);
    digitalWrite(pin_neg, LOW);
    digitalWrite(pin_pos, LOW);
#endif
    extended = false;
}

void linear_motor::extend()
{
    if (extended)
    {
        return;
    }

    std::cout << "Extending " << name << " motor (pins: " << pin_pos << ", " << pin_neg << ") for "<< extension_time <<"\n";
#ifndef MOCK
    digitalWrite(pin_pos, HIGH);
    digitalWrite(pin_neg, LOW);
    std::this_thread::sleep_for(extension_time);
    digitalWrite(pin_neg, LOW);
    digitalWrite(pin_pos, LOW);
#endif
    extended = true;
}

linear_motor::~linear_motor()
{
    // We don't want the chickens to be closed inside, better open the door
    contract();
}

temperature_sensor::temperature_sensor(const std::string &sensor_id)
    : sensor_path("/sys/bus/w1/devices/" + sensor_id + "/w1_slave")
{
}

float temperature_sensor::read_temperature() const
{
    std::ifstream file(sensor_path);
    if (!file.is_open())
    {
        throw std::runtime_error("Failed to open sensor file");
    }

    std::string line1, line2;
    std::getline(file, line1);
    std::getline(file, line2);

    // Check CRC
    if (line1.find("YES") == std::string::npos)
    {
        throw std::runtime_error("CRC check failed");
    }

    // Parse temperature
    std::size_t pos = line2.find("t=");
    if (pos == std::string::npos)
    {
        throw std::runtime_error("Failed to find temperature data");
    }

    int temp_milli = std::stoi(line2.substr(pos + 2));
    return temp_milli / 1000.0f; // Convert to Celsius
}
