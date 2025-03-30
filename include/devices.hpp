#pragma once
#include <string>
#include <chrono>
using namespace std::chrono_literals;

class power_switch
{
private:
    int pin;
    const std::string name;

public:
    power_switch(int pin, const std::string &name);
    void turn_on();
    void turn_off();
    ~power_switch();
    bool on{false};
};

class linear_motor
{
public:
    linear_motor(int pin_pos, int pin_neg, int extension_time, int contraction_time, const std::string &name);
    void contract();
    void extend();
    ~linear_motor();
    bool extended{false};

private:
    int pin_pos;
    int pin_neg;
    const std::chrono::seconds extension_time;
    const std::chrono::seconds contraction_time;
    std::string name;
};

class temperature_sensor
{
public:
    temperature_sensor(const std::string &sensor_id);
    float read_temperature() const;

private:
    std::string sensor_path;
};