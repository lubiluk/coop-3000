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
#include <memory>
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
    float temp_heating_on = vm["temp-heating-on"].as<float>();
    float temp_heating_off = vm["temp-heating-off"].as<float>();
    float temp_door_open = vm["temp-door-open"].as<float>();
    float temp_door_close = vm["temp-door-close"].as<float>();
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
    config.temp_heating_on = temp_heating_on;
    config.temp_heating_off = temp_heating_off;
    config.temp_door_open = temp_door_open;
    config.temp_door_close = temp_door_close;

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
        visible.add_options()("help", "produce help message")("config,c", po::value<string>(&config_file), "path to the configuration file (required)");

        // Declare a group of options that should be in a config file
        po::options_description config("Configuration");
        config.add_options()("pin-light", po::value<int>(), "pin number for light")
                            ("pin-heating", po::value<int>(), "pin number for heating")
                            ("pin-door-pos", po::value<int>(), "pin number for door positive")
                            ("pin-door-neg", po::value<int>(), "pin number for door negative")
                            ("door-open-time", po::value<int>(), "time to open the door")
                            ("door-close-time", po::value<int>(), "time to close the door")
                            ("latitude", po::value<float>(), "latitude")
                            ("longitude", po::value<float>(), "longitude")
                            ("temp-sensor-id", po::value<string>(), "ID of the temperature sensor")
                            ("temp-heating-on", po::value<float>(), "temperature to turn on heating")
                            ("temp-heating-off", po::value<float>(), "temperature to turn off heating")
                            ("temp-door-open", po::value<float>(), "temperature to open the door")
                            ("temp-door-close", po::value<float>(), "temperature to close the door")
                            ("monitor-file", po::value<string>(), "file to save monitoring data");

        po::variables_map vm;
        store(po::parse_command_line(argc, argv, visible), vm);
        notify(vm);

        if (vm.count("help"))
        {
            cout << visible << "\n";
            return 0;
        }

        if (config_file.empty())
        {
            throw std::runtime_error("Specify a configuration file with --config or -c.");
        }

        ifstream ifs(config_file);
        if (!ifs)
        {
            throw std::runtime_error("Cannot open config file: " + config_file);
        }
        store(parse_config_file(ifs, config), vm);
        notify(vm);

        start(vm);
    }
    catch (exception &e)
    {
        cout << e.what() << "\n";
        return 1;
    }

    cout << "Exiting normally\n";
    return 0;
}