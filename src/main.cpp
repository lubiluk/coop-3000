#include "automation.hpp"
#include "timing.hpp"
#include "monitoring.hpp"

#ifndef MOCK
#include <wiringPi.h>
#endif

#include <iostream>
#include <chrono>
#include <thread>
#include <fstream>
#include <csignal>
#include <boost/program_options.hpp>
#include <stdexcept>
#include <vector>
#include <cstdlib>

using namespace std::chrono_literals;
namespace po = boost::program_options;

// Using pointer to call destructors on abrubt exit
std::unique_ptr<automation> coop;

void signal_handler(int signal)
{
    std::cout << "\nSignal received: " << signal << ". Cleaning up..." << std::endl;
    coop.reset(); // Destructor is called
    exit(signal); // Exit the program
}

std::string find_config_file(const std::vector<std::string> &paths)
{
    for (const auto &path : paths)
    {
        std::ifstream ifs(path);
        if (ifs.good())
        {
            return path;
        }
    }
    
    throw std::runtime_error("Config file not found in default locations.");
}

void start(po::variables_map &vm)
{
#ifndef MOCK
    // Initialize WiringPi
    if (wiringPiSetup() == -1)
    {
        std::cerr << "WiringPi setup failed. Exiting.\n";
        return;
    }
#endif

    int pin_light = vm["pin-light"].as<int>();
    int pin_heating = vm["pin-heating"].as<int>();
    int pin_door_pos = vm["pin-door-pos"].as<int>();
    int pin_door_neg = vm["pin-door-neg"].as<int>();
    int door_open_time = vm["door-open-time"].as<int>();
    int door_close_time = vm["door-close-time"].as<int>();
    float lat = vm["latitude"].as<float>();
    float lon = vm["longitude"].as<float>();
    std::string temp_sensor_id = vm["temp-sensor-id"].as<std::string>();
    float temp_min = vm["temp-min"].as<float>();
    float temp_max = vm["temp-max"].as<float>();
    std::string monitor_file = vm["monitor-file"].as<std::string>();

    std::cout << "Starting coop automation at location (" << lat << ", " << lon << ") \n";

    automation_config config;
    config.pin_light = pin_light;
    config.pin_heating = pin_heating;
    config.pin_door_pos = pin_door_pos;
    config.pin_door_neg = pin_door_neg;
    config.door_open_time = door_open_time;
    config.door_close_time = door_close_time;
    config.temp_sensor_id = temp_sensor_id;
    config.temp_min = temp_min;
    config.temp_max = temp_max;

    coop = std::make_unique<automation>(config);

    // Register the signal handler
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);

    timing::time_tracker tracker{lat, lon};

    std::cout << "Entering update loop" << std::endl;

    while (true)
    {
        auto tod = tracker.current_tod();
        auto status = coop->update(tod);
        std::cout << status << std::endl;
        append_status_line(monitor_file, status);
        std::this_thread::sleep_for(5min);
    }
}

int main(int argc, char **argv)
{
    using std::cout;
    using std::exception;
    using std::ifstream;
    using std::string;

    try
    {
        string config_file;
        // Declare a group of options that will be
        // allowed only on command line
        po::options_description visible("Program options");
        visible.add_options()("help", "produce help message")("config,c", po::value<string>(&config_file)->default_value(""), "name of a file of a configuration.");

        int pin_light;       // WiringPi pin number for light
        int pin_heatig;      // WiringPi pin number for heating
        int pin_door_pos{2}; // WiringPi pin number for door positive
        int pin_door_neg{3}; // WiringPi pin number for door negative
        float latitude;      // Latitude of the location
        float longitude;     // Longitude of the location
        // Declare a group of options that should be in a config file
        po::options_description config("Configuration");
        config.add_options()("pin-light", po::value<int>(&pin_light), "pin number for light")
                            ("pin-heating", po::value<int>(&pin_heatig), "pin number for heating")
                            ("pin-door-pos", po::value<int>(&pin_door_pos), "pin number for door positive")
                            ("pin-door-neg", po::value<int>(&pin_door_neg), "pin number for door negative")
                            ("door-open-time", po::value<int>(), "time to open the door")
                            ("door-close-time", po::value<int>(), "time to close the door")
                            ("latitude", po::value<float>(&latitude), "latitude")
                            ("longitude", po::value<float>(&longitude), "longitude")
                            ("temp-sensor-id", po::value<string>(), "ID of the temperature sensor")
                            ("temp-min", po::value<float>(), "minimum temperature")
                            ("temp-max", po::value<float>(), "maximum temperature")
                            ("monitor-file", po::value<string>(), "file to save monitoring data");

        po::options_description cmdline_options;
        cmdline_options.add(visible);

        po::options_description config_file_options;
        config_file_options.add(config);

        po::variables_map vm;
        store(po::parse_command_line(argc, argv, cmdline_options), vm);
        notify(vm);

        // If the config file is not specified, try to find it in the default locations
        if (config_file.empty())
        {
            // Construct the default config file paths
            std::vector<std::string> default_config_files = {
                std::string(std::getenv("HOME")) + "/.local/etc/coop-3000/config.cfg",
                "/usr/local/etc/coop-3000/config.cfg"};

            // Find the first available config file
            config_file = find_config_file(default_config_files);
        }


        ifstream ifs(config_file.c_str());
        if (!ifs)
        {
            throw std::runtime_error("Cannot open config file: " + config_file);
        }
        else
        {
            store(parse_config_file(ifs, config_file_options), vm);
            notify(vm);
        }

        if (vm.count("help"))
        {
            cout << visible << "\n";
            return 0;
        }

        start(vm);
    }
    catch (exception &e)
    {
        cout << e.what() << "\n";
        return 1;
    }

    return 0;
}