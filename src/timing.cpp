#include <timing.hpp>
#include <chrono>
#include <numbers>
#include <tuple>
#include <cmath>

using namespace std::chrono_literals;

namespace timing
{

    std::ostream &operator<<(std::ostream &os, const dtime &tod)
    {
        switch (tod)
        {
        case dtime::sunrise:
            os << "sunrise";
            break;
        case dtime::morning:
            os << "morning";
            break;
        case dtime::sunset:
            os << "sunset";
            break;
        case dtime::evening:
            os << "evening";
            break;
        case dtime::night:
            os << "night";
            break;
        default:
            os << "unknown";
            break;
        }

        return os;
    }

    dtime next_dtime(dtime tod)
    {
        return static_cast<dtime>((static_cast<int>(tod) + 1) % (static_cast<int>(dtime::night) + 1));
    }

    daylight::daylight(float lat, float lon) : lat(lat), lon(lon) {}

    std::tuple<unsigned long, unsigned long> daylight::get_sunrise_sunset(unsigned long unix_time)
    {
        using std::numbers::pi;
        // Convert Unix Timestamp to Julian day
        unsigned long J_date = round(unix_time / 86400.0 + 2440587.5);
        // Number of days since Jan 1st, 2000 12:00
        double n = (double)J_date - 2451545.0 + 0.0008;
        // Mean solar noon
        double J_star = -lon / 360 + n;
        // Solar mean anomaly
        double M = fmod((357.5291 + 0.98560028 * J_star), 360);
        // Equation of the center
        double C = 1.9148 * sin(M / 360 * 2 * pi) + 0.02 * sin(2 * M / 360 * 2 * pi) + 0.0003 * sin(3 * M / 360 * 2 * pi);
        // Ecliptic longitude
        double lambda = fmod((M + C + 180 + 102.9372), 360);
        // Solar transit
        double J_transit = J_star + (0.0053 * sin(M / 360.0 * 2.0 * pi) - 0.0069 * sin(2.0 * (lambda / 360.0 * 2.0 * pi)));
        // Declination of the Sun
        double delta = asin(sin(lambda / 360 * 2 * pi) * sin(23.44 / 360 * 2 * pi)) / (2 * pi) * 360;
        // Hour angle
        double omega_0 = 360 / (2 * pi) * acos((sin(-0.83 / 360 * 2 * pi) - sin(lat / 360 * 2 * pi) * sin(delta / 360 * 2 * pi)) / (cos(lat / 360 * 2 * pi) * cos(delta / 360 * 2 * pi)));
        // Julian day sunrise, sunset
        double J_set = J_transit + omega_0 / 360;
        double J_rise = J_transit - omega_0 / 360;
        // Convert to Unix Timestamp
        // TODO: perhaps rewrite with std::chrono
        unsigned long unix_rise = J_rise * 86400 + 946728000;
        unsigned long unix_set = J_set * 86400 + 946728000;

        return {unix_rise, unix_set};
    }

    unsigned long daylight::sunrise(unsigned long unix_time)
    {
        auto [sunrise, _] = get_sunrise_sunset(unix_time);
        return sunrise;
    }

    unsigned long daylight::sunset(unsigned long unix_time)
    {
        auto [_, sunset] = get_sunrise_sunset(unix_time);
        return sunset;
    }

    time_tracker::time_tracker(float lat, float lon) : today(lat, lon) {}

    dtime time_tracker::current_tod()
    {
        return tod(unix_timestamp());
    }

    dtime time_tracker::tod(unsigned long now)
    {
        auto sunrise_time = today.sunrise(now); // at sunrise
        auto morning_time = today.sunrise(now) + 1800; // 30 minutes after sunrise
        auto sunset_time = today.sunset(now) - 1800;  // 30 minutes before sunset
        auto evening_time = today.sunset(now) + 2700; // 45 minutes after sunset
        auto night_time = evening_time + 300;         // 5 minutes after evening

        if (now < sunrise_time || now >= night_time)
        {
            return dtime::night;
        }
        else if (now < morning_time)
        {
            return dtime::sunrise;
        }
        else if (now < sunset_time)
        {
            return dtime::morning;
        }
        else if (now < evening_time)
        {
            return dtime::sunset;
        }
        else
        {
            return dtime::evening;
        }
    }

    unsigned long time_tracker::unix_timestamp()
    {
        return std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    }

} // namespace timing
