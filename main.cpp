#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <numeric>
#include <thread>
#include <algorithm>
#include <filesystem>
#include "serial_port.hpp"

namespace fs = std::filesystem;

struct Entry {
    std::time_t time;
    double temp;
};

void clean_log(const std::string& filename, int seconds_limit) {
    if (!fs::exists(filename)) return;
    std::vector<std::string> lines;
    std::string line;
    std::time_t now = std::time(nullptr);
    std::ifstream in(filename);
    while (std::getline(in, line)) {
        std::stringstream ss(line);
        std::string ts_str;
        ss >> ts_str;
        std::time_t t = std::stoll(ts_str);
        if (now - t <= seconds_limit) {
            lines.push_back(line);
        }
    }
    in.close();
    std::ofstream out(filename, std::ios::trunc);
    for (const auto& l : lines) out << l << "\n";
}

void append_log(const std::string& filename, std::time_t t, double temp) {
    std::ofstream out(filename, std::ios::app);
    out << t << " " << std::fixed << std::setprecision(2) << temp << "\n";
}

int main(int argc, char* argv[]) {
    std::string port = (argc > 1) ? argv[1] : "COM3";
    SerialPort sp(port);

    if (!sp.isOpen()) {
        std::cerr << "Could not open " << port << ". Check if device is connected or simulation file exists.\n";
        return 1;
    }

    std::vector<double> hour_buffer;
    std::vector<double> day_buffer;
    
    auto last_hour_check = std::chrono::system_clock::now();
    auto last_day_check = std::chrono::system_clock::now();

    while (true) {
        std::string data;
        if (sp.readLine(data)) {
            try {
                double current_temp = std::stod(data);
                std::time_t now_t = std::time(nullptr);
                
                append_log("raw.log", now_t, current_temp);
                clean_log("raw.log", 24 * 3600);

                hour_buffer.push_back(current_temp);
                day_buffer.push_back(current_temp);

                auto now = std::chrono::system_clock::now();
                
                if (std::chrono::duration_cast<std::chrono::hours>(now - last_hour_check).count() >= 1) {
                    double avg = std::accumulate(hour_buffer.begin(), hour_buffer.end(), 0.0) / hour_buffer.size();
                    append_log("hourly.log", now_t, avg);
                    clean_log("hourly.log", 30 * 24 * 3600);
                    hour_buffer.clear();
                    last_hour_check = now;
                }

                if (std::chrono::duration_cast<std::chrono::hours>(now - last_day_check).count() >= 24) {
                    double avg = std::accumulate(day_buffer.begin(), day_buffer.end(), 0.0) / day_buffer.size();
                    append_log("daily.log", now_t, avg);
                    clean_log("daily.log", 365 * 24 * 3600);
                    day_buffer.clear();
                    last_day_check = now;
                }

                std::cout << "Read temp: " << current_temp << std::endl;
            } catch (...) {}
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    return 0;
}