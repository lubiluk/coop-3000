#pragma once

#include "devices.hpp"
#include "timing.hpp"
#include <string>

struct automation_config
{
    int pin_light;
    int pin_heating;
    int pin_door_pos;
    int pin_door_neg;
    int door_open_time;
    int door_close_time;
    std::string temp_sensor_id;
    float temp_heating_on;
    float temp_heating_off;
    float temp_door_open;
    float temp_door_close;
};

struct automation_status
{
    timing::dtime tod;
    float temperature;
    bool heating;
    bool light;
    bool door;
};

std::ostream &operator<<(std::ostream &os, const automation_status &status);

class automation
{
private:
    automation_config config;
    power_switch light;
    power_switch heating;
    linear_motor door;
    temperature_sensor thermometer;

public:
    automation(const automation_config &config);
    automation_status update(timing::dtime tod);
};