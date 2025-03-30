#pragma once

#include <string>

namespace timing
{
    enum class dtime
    {
        sunrise,
        morning,
        sunset,
        evening,
        night
    };

    std::ostream &operator<<(std::ostream &os, const dtime &tod);

    dtime next_dtime(dtime tod);

    // Credit: https://github.com/nplan/Arduino-Sun/blob/master/Sun.h
    class daylight
    {
    public:
        daylight(float lat, float lon);
        std::tuple<unsigned long, unsigned long> get_sunrise_sunset(unsigned long unix_time);
        unsigned long sunrise(unsigned long unix_time);
        unsigned long sunset(unsigned long unix_time);

    private:
        float lat;
        float lon;
    };

    class time_tracker
    {
    public:
        time_tracker(float lat, float lon);
        dtime current_tod();
        dtime tod(unsigned long timestamp);
        unsigned long unix_timestamp();

    private:
        daylight today;
    };

} // namespace timing