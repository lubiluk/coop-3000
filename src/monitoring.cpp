#include "monitoring.hpp"
#include <fstream>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <string>
#include <filesystem>

std::string current_datetime()
{
    const auto now = std::chrono::system_clock::now();
    const auto time_t_now = std::chrono::system_clock::to_time_t(now);

    std::ostringstream oss;
    oss << std::put_time(std::localtime(&time_t_now), "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

void append_status_line(const std::string &filename, const automation_status &status)
{
    namespace fs = std::filesystem;

    std::ifstream file_check(filename);
    bool file_exists = file_check.good();
    file_check.close();

    // Create destination directory if it does not exist
    const auto dir = fs::path(filename).parent_path();
    
    if (!dir.empty() && !fs::exists(dir))
    {
        fs::create_directories(dir);
    }


    // Open file in append mode
    std::ofstream file(filename, std::ios::app);
    if (!file.is_open())
    {
        throw std::ios_base::failure("Failed to open file: " + filename);
    }

    // Write headers if the file is new
    if (!file_exists)
    {
        file << "Timestamp" << ",";
        file << "Time of Day" << ",";
        file << "Temperature" << ",";
        file << "Heater" << ",";
        file << "Light" << ",";
        file << "Door" << std::endl;
    }

    // Write the status line
    file << current_datetime() << ",";
    file << status.tod << ",";
    file << status.temperature << ",";
    file << status.heating << ",";
    file << status.light << ",";
    file << status.door << std::endl;
}
