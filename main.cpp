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

void clean_log(const std::string& filename, int seconds_limit) {
    if (!fs::exists(filename)) return;
    std::vector<std::pair<std::time_t, double>> entries;
    std::string line;
    std::time_t now = std::time(nullptr);
    std::ifstream in(filename);
    while (std::getline(in, line)) {
        std::stringstream ss(line);
        std::time_t t;
        double temp;
        if (ss >> t >> temp) {
            if (now - t <= seconds_limit) {
                entries.push_back({t, temp});
            }
        }
    }
    in.close();
    std::ofstream out(filename, std::ios::trunc);
    for (const auto& e : entries) {
        out << e.first << " " << std::fixed << std::setprecision(2) << e.second << "\n";
    }
}

void append_log(const std::string& filename, std::time_t t, double temp) {
    std::ofstream out(filename, std::ios::app);
    out << t << " " << std::fixed << std::setprecision(2) << temp << "\n";
}

int main(int argc, char* argv[]) {
    std::string port = (argc > 1) ? argv[1] : "COM3";
    SerialPort sp(port);
    if (!sp.isOpen()) {
        std::cerr << "Port error: " << port << std::endl;
        return 1;
    }
    std::vector<double> hour_buffer;
    std::vector<double> day_buffer;
    auto last_hour = std::chrono::system_clock::now();
    auto last_day = std::chrono::system_clock::now();
    while (true) {
        std::string data;
        if (sp.readLine(data) && !data.empty()) {
            try {
                double current_temp = std::stod(data);
                std::time_t now_t = std::time(nullptr);
                append_log("raw.log", now_t, current_temp);
                clean_log("raw.log", 24 * 3600);
                hour_buffer.push_back(current_temp);
                day_buffer.push_back(current_temp);
                auto now = std::chrono::system_clock::now();
                if (std::chrono::duration_cast<std::chrono::hours>(now - last_hour).count() >= 1) {
                    if (!hour_buffer.empty()) {
                        double avg = std::accumulate(hour_buffer.begin(), hour_buffer.end(), 0.0) / hour_buffer.size();
                        append_log("hourly.log", now_t, avg);
                        clean_log("hourly.log", 30 * 24 * 3600);
                        hour_buffer.clear();
                    }
                    last_hour = now;
                }
                if (std::chrono::duration_cast<std::chrono::hours>(now - last_day).count() >= 24) {
                    if (!day_buffer.empty()) {
                        double avg = std::accumulate(day_buffer.begin(), day_buffer.end(), 0.0) / day_buffer.size();
                        append_log("daily.log", now_t, avg);
                        clean_log("daily.log", 365 * 24 * 3600);
                        day_buffer.clear();
                    }
                    last_day = now;
                }
                std::cout << "Temp: " << current_temp << std::endl;
            } catch (...) {}
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    return 0;
}