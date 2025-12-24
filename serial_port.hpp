#pragma once

#include <string>
#include <fstream>
#include <iostream>

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#else
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#endif

class SerialPort {
public:
    SerialPort(const std::string& portName) {
        name = portName;
#if defined(_WIN32) || defined(_WIN64)
        hSerial = CreateFileA(name.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hSerial != INVALID_HANDLE_VALUE) {
            DCB dcbSerialParams = {0};
            dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
            GetCommState(hSerial, &dcbSerialParams);
            dcbSerialParams.BaudRate = CBR_9600;
            dcbSerialParams.ByteSize = 8;
            dcbSerialParams.StopBits = ONESTOPBIT;
            dcbSerialParams.Parity = NOPARITY;
            SetCommState(hSerial, &dcbSerialParams);
        }
#else
        fd = open(name.c_str(), O_RDWR | O_NOCTTY);
        if (fd != -1) {
            struct termios tty;
            tcgetattr(fd, &tty);
            cfsetospeed(&tty, B9600);
            cfsetispeed(&tty, B9600);
            tty.c_cflag |= (CLOCAL | CREAD);
            tty.c_cflag &= ~PARENB;
            tty.c_cflag &= ~CSTOPB;
            tty.c_cflag &= ~CSIZE;
            tty.c_cflag |= CS8;
            tcsetattr(fd, TCSANOW, &tty);
        }
#endif
    }

    ~SerialPort() {
#if defined(_WIN32) || defined(_WIN64)
        if (hSerial != INVALID_HANDLE_VALUE) CloseHandle(hSerial);
#else
        if (fd != -1) close(fd);
#endif
    }

    bool isOpen() const {
#if defined(_WIN32) || defined(_WIN64)
        return hSerial != INVALID_HANDLE_VALUE;
#else
        return fd != -1;
#endif
    }

    bool readLine(std::string& line) {
        line.clear();
        char c;
        while (true) {
#if defined(_WIN32) || defined(_WIN64)
            DWORD bytesRead;
            if (ReadFile(hSerial, &c, 1, &bytesRead, NULL) && bytesRead > 0) {
#else
            if (read(fd, &c, 1) > 0) {
#endif
                if (c == '\n') return true;
                if (c != '\r') line += c;
            } else {
                return false;
            }
        }
    }

private:
    std::string name;
#if defined(_WIN32) || defined(_WIN64)
    HANDLE hSerial;
#else
    int fd;
#endif
};