#include <iostream>
#include <timing.hpp>

int main()
{
    float lat = 50.093243; 
    float lon = 19.793206; 

    timing::time_tracker tracker{lat, lon};

    auto now = tracker.unix_timestamp();
    auto tod = tracker.tod(now);
    auto today = timing::daylight(lat, lon);
    auto sunrise_time = today.sunrise(now);
    auto sunset_time = today.sunset(now);

    std::cout << "Time of day: " << tod << std::endl;
    std::cout << "Now: " << now << std::endl;
    std::cout << "Sunrise: " << sunrise_time << std::endl;
    std::cout << "Sunset: " << sunset_time << std::endl;

    double to_sunrise = ((double)sunrise_time - (double)now) / 3600.0;
    double to_sunset = ((double)sunset_time - (double)now) / 3600.0;

    std::cout << "To sunrise: " << to_sunrise << " hours" << std::endl;
    std::cout << "To sunset: " << to_sunset << " hours" << std::endl;

    return 0;
}