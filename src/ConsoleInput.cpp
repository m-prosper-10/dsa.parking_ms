#include "parking/ConsoleInput.h"

#include <cctype>
#include <cstdlib>
#include <ctime>
#include <iostream>

namespace parking {

std::string readRawLine(const std::string& prompt) {
    std::cout << prompt;
    std::string line;
    if (!std::getline(std::cin, line)) {
        std::cout << "\n[Input stream ended - exiting safely]\n";
        std::exit(0);
    }
    return trim(line);
}

std::string readNonEmpty(const std::string& prompt) {
    while (true) {
        if (std::string s = readRawLine(prompt); !s.empty()) {
            return s;
        }
        std::cout << "  Value cannot be empty, try again.\n";
    }
}

bool parseIntStrict(const std::string& s, long long& out) {
    if (s.empty()) {
        return false;
    }

    std::size_t pos = 0;
    try {
        out = std::stoll(s, &pos);
    } catch (...) {
        return false;
    }
    return pos == s.size();
}

int readIntInRange(const std::string& prompt, const int lo, const int hi) {
    while (true) {
        const std::string s = readRawLine(prompt);
        if (long long v; parseIntStrict(s, v) && v >= lo && v <= hi) {
            return static_cast<int>(v);
        }
        std::cout << "  Invalid input. Enter a whole number between " << lo << " and " << hi
                  << ".\n";
    }
}

double readPositiveAmount(const std::string& prompt) {
    while (true) {
        const std::string s = readRawLine(prompt);
        try {
            std::size_t pos = 0;
            if (const double v = std::stod(s, &pos); pos == s.size() && v > 0 && v <= 1e12) {
                return v;
            }
        } catch (...) {
        }
        std::cout << "  Invalid amount. Enter a positive number.\n";
    }
}

int parseTimeHHMM(const std::string& s) {
    const std::size_t c = s.find(':');
    if (c == std::string::npos || c == 0 || c + 1 >= s.size()) {
        return -1;
    }

    const std::string hs = s.substr(0, c);
    const std::string ms = s.substr(c + 1);
    if (hs.size() > 2 || ms.size() != 2) {
        return -1;
    }
    for (const char ch : hs) {
        if (!std::isdigit(static_cast<unsigned char>(ch))) {
            return -1;
        }
    }
    for (const char ch : ms) {
        if (!std::isdigit(static_cast<unsigned char>(ch))) {
            return -1;
        }
    }

    const int h = std::stoi(hs);
    const int m = std::stoi(ms);
    if (h > 23 || m > 59) {
        return -1;
    }
    return h * 60 + m;
}

int readTime(const std::string& prompt) {
    while (true) {
        const std::string s = readRawLine(prompt + " (HH:MM, blank = now): ");
        if (s.empty()) {
            const std::time_t t = std::time(nullptr);
            const std::tm* lt = std::localtime(&t);
            const int minutes = lt->tm_hour * 60 + lt->tm_min;
            std::cout << "  Using current time " << fmtTime(minutes) << ".\n";
            return minutes;
        }
        if (const int t = parseTimeHHMM(s); t >= 0) {
            return t;
        }
        std::cout << "  Invalid time. Use 24-hour HH:MM (e.g. 08:30).\n";
    }
}

VehicleType readVehicleType() {
    std::cout << "  Vehicle type: 1 = Motorcycle, 2 = Car, 3 = Truck\n";
    return static_cast<VehicleType>(readIntInRange("  Choose type (1-3): ", 1, 3));
}

}
