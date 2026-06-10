#pragma once

#include "parking/Domain.h"

#include <map>
#include <string>
#include <unordered_map>
#include <vector>

namespace parking {

class ParkingSystem {
    std::map<std::string, ParkingSlot> slots_;
    std::unordered_map<std::string, ActiveParking> active_;
    std::vector<Transaction> history_;
    Tariff tariff_;

    ParkingSlot* findFreeSlot(VehicleType type);
    void countSlots(VehicleType type, int& total, int& occupied) const;
    static void printTransaction(const Transaction& tr);
    static void printTransactionHeader();

public:
    double tariffFor(VehicleType type) const;
    void setTariff(const VehicleType type, const double rate);
    void addSlot();
    void removeSlot();
    void displaySlots() const;
    void vehicleEntry();
    void vehicleExit();
    void showTariffs() const;
    void displayParked() const;
    void vehicleHistory() const;
    void dailyRevenue() const;
};

}  // namespace parking
