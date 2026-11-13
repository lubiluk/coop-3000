#include "automation.hpp"
#include <iostream>
#include <iomanip>

automation::automation(const automation_config &config)
    : light(config.pin_light, "light"),
      heating(config.pin_heating, "heating"),
      door(config.pin_door_pos, config.pin_door_neg, config.door_close_time, config.door_open_time, "door"),
      thermometer(config.temp_sensor_id), config(config) {}
automation_status automation::update(timing::dtime tod)

{
    using timing::dtime;
    // Fallback temperature
    float temp = 15.0f;

    try
    {
        temp = thermometer.read_temperature();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error reading temperature: " << e.what() << std::endl;
    }

    float temp_heating_on = config.temp_heating_on;
    float temp_heating_off = config.temp_heating_off;

    switch (tod)
    {
    case dtime::sunrise:
        heating.turn_off();
        light.turn_off();
        door.extend();
        break;
    case dtime::morning:
        heating.turn_off();
        light.turn_off();
        door.contract();
        break;
    case dtime::sunset:
        heating.turn_off();
        light.turn_on();
        door.contract();
        break;
    case dtime::evening:
        if (temp < temp_heating_on)
        {
            heating.turn_on();
        }
        else if (temp > temp_heating_off)
        {
            heating.turn_off();
        }
        
        light.turn_on();

        if (temp > config.temp_door_open)
        {
            door.contract();
        }
        else
        {
            door.extend();
        }
        break;
    case dtime::night:
        if (temp < temp_heating_on)
        {
            heating.turn_on();
        }
        else if (temp > temp_heating_off)
        {
            heating.turn_off();
        }
        
        light.turn_off();
        
        if (temp > config.temp_door_open)
        {
            door.contract();
        }
        else if (temp < config.temp_door_close)
        {
            door.extend();
        }

        break;
    }

    return {tod, temp, heating.on, light.on, door.extended};
}

std::ostream &operator<<(std::ostream &os, const automation_status &status)
{
    // Output as one line as name: value
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    os << "[" << std::put_time(std::localtime(&time), "%c") << "] ";
    os << "Time of Day: " << status.tod << ", ";
    os << "Temperature: " << status.temperature << ", ";
    os << "Heating: " << status.heating << ", ";
    os << "Light: " << status.light << ", ";
    os << "Door: " << (status.door ? "extended" : "contracted");
    return os;
}
