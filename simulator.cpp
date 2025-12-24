#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>
#include <random>

int main() {
    std::ofstream out("virtual_com", std::ios::trunc);
    std::mt19937 rng(std::random_device{}());
    std::uniform_real_distribution<double> dist(20.0, 30.0);

    std::cout << "Simulating device. Writing to 'virtual_com'.\n";

    while (true) {
        double temp = dist(rng);
        out << temp << "\n";
        out.flush();
        std::cout << "Sent: " << temp << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    return 0;
}