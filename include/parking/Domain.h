#pragma once

#include <algorithm>
#include <cctype>
#include <iomanip>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <utility>

namespace parking {

inline std::string trim(const std::string& s) {
    const std::size_t a = s.find_first_not_of(" \t\r\n");
    const std::size_t b = s.find_last_not_of(" \t\r\n");
    if (a == std::string::npos) {
        return "";
    }
    return s.substr(a, b - a + 1);
}

inline std::string toUpperCopy(std::string s) {
    for (char& c : s) {
        c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    }
    return s;
}

inline std::string fmtTime(const int minutes) {
    std::ostringstream o;
    o << std::setw(2) << std::setfill('0') << minutes / 60 << ":"
      << std::setw(2) << std::setfill('0') << minutes % 60;
    return o.str();
}

enum class VehicleType { Motorcycle = 1, Car = 2, Truck = 3 };

inline std::string typeName(const VehicleType type) {
    switch (type) {
        case VehicleType::Motorcycle: return "Motorcycle";
        case VehicleType::Car: return "Car";
        case VehicleType::Truck: return "Truck";
    }
    return "?";
}

class Vehicle {
    std::string plate_;

public:
    explicit Vehicle(std::string plate) : plate_(std::move(plate)) {}
    virtual ~Vehicle() = default;

    virtual VehicleType type() const = 0;
    std::string typeNameStr() const { return typeName(type()); }
    const std::string& plate() const { return plate_; }
};

class Motorcycle final : public Vehicle {
public:
    using Vehicle::Vehicle;
    VehicleType type() const override { return VehicleType::Motorcycle; }
};

class Car final : public Vehicle {
public:
    using Vehicle::Vehicle;
    VehicleType type() const override { return VehicleType::Car; }
};

class Truck final : public Vehicle {
public:
    using Vehicle::Vehicle;
    VehicleType type() const override { return VehicleType::Truck; }
};

inline std::unique_ptr<Vehicle> makeVehicle(const VehicleType type, const std::string& plate) {
    switch (type) {
        case VehicleType::Motorcycle: return std::make_unique<Motorcycle>(plate);
        case VehicleType::Car: return std::make_unique<Car>(plate);
        case VehicleType::Truck: return std::make_unique<Truck>(plate);
    }
    return nullptr;
}

class ParkingSlot {
    std::string id_;
    VehicleType type_;
    std::string zone_;
    bool occupied_ = false;
    std::string occupantPlate_;

public:
    ParkingSlot(std::string id, const VehicleType type, std::string zone)
        : id_(std::move(id)), type_(type), zone_(std::move(zone)) {}

    const std::string& id() const { return id_; }
    VehicleType type() const { return type_; }
    const std::string& zone() const { return zone_; }
    bool occupied() const { return occupied_; }
    const std::string& occupantPlate() const { return occupantPlate_; }

    void occupy(const std::string& plate) {
        occupied_ = true;
        occupantPlate_ = plate;
    }

    void release() {
        occupied_ = false;
        occupantPlate_.clear();
    }
};

class Tariff {
    std::map<VehicleType, double> rate_;

public:
    Tariff() {
        rate_[VehicleType::Motorcycle] = 500.0;
        rate_[VehicleType::Car] = 1000.0;
        rate_[VehicleType::Truck] = 2000.0;
    }

    double get(const VehicleType type) const { return rate_.at(type); }
    void set(const VehicleType type, const double rate) { rate_.at(type) = rate; }
};

struct ActiveParking {
    std::unique_ptr<Vehicle> vehicle;
    std::string slotId;
    int entryMin = 0;
};

struct Transaction {
    int nbr = 0;
    std::string plate;
    std::string vehicleType;
    std::string slotId;
    std::string zone;
    int entryMin = 0;
    int exitMin = 0;
    long long billedHours = 0;
    double rateApplied = 0.0;
    double fee = 0.0;
};

}  // namespace parking
