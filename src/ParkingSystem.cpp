#include "parking/ParkingSystem.h"
#include "parking/ConsoleInput.h"

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <ranges>
#include <string>

namespace parking {

ParkingSlot* ParkingSystem::findFreeSlot(const VehicleType type) {
    for (auto& slot : slots_ | std::views::values) {
        if (slot.type() == type && !slot.occupied()) {
            return &slot;
        }
    }
    return nullptr;
}

void ParkingSystem::countSlots(const VehicleType type, int& total, int& occupied) const {
    total = 0;
    occupied = 0;
    for (const auto& slot : slots_ | std::views::values) {
        if (slot.type() != type) {
            continue;
        }
        ++total;
        if (slot.occupied()) {
            ++occupied;
        }
    }
}

void ParkingSystem::printTransaction(const Transaction& tr) {
    std::cout << "  " << std::left << std::setw(5) << tr.nbr
              << std::setw(12) << tr.plate
              << std::setw(12) << tr.vehicleType
              << std::setw(8) << tr.slotId
              << std::setw(10) << tr.zone
              << std::setw(7) << fmtTime(tr.entryMin)
              << std::setw(7) << fmtTime(tr.exitMin)
              << std::setw(6) << tr.billedHours
              << std::right << std::setw(10) << std::fixed << std::setprecision(2) << tr.rateApplied
              << std::setw(12) << tr.fee << "\n" << std::left;
}

void ParkingSystem::printTransactionHeader() {
    std::cout << "  " << std::left << std::setw(5) << "Nbr" << std::setw(12) << "Plate"
              << std::setw(12) << "Type" << std::setw(8) << "Slot" << std::setw(10) << "Zone"
              << std::setw(7) << "In" << std::setw(7) << "Out" << std::setw(6) << "Hrs"
              << std::right << std::setw(10) << "Rate" << std::setw(12) << "Fee(RWF)"
              << "\n" << std::left;
    std::cout << "  " << std::string(87, '-') << "\n";
}

void ParkingSystem::addSlot() {
    std::string id = toUpperCopy(readNonEmpty("  Slot ID: "));
    if (slots_.contains(id)) {
        std::cout << "  Slot '" << id << "' already exists (IDs are unique, case-insensitive).\n";
        return;
    }

    const VehicleType type = readVehicleType();
    const std::string zone = readNonEmpty("  Zone (location): ");
    slots_.emplace(id, ParkingSlot(id, type, zone));
    std::cout << "  Slot " << id << " (" << typeName(type) << ", zone " << zone
              << ") configured. Status: Available.\n";
}

void ParkingSystem::removeSlot() {
    if (slots_.empty()) {
        std::cout << "  No slots configured yet.\n";
        return;
    }

    const std::string id = toUpperCopy(readNonEmpty("  Slot ID to remove: "));
    const auto it = slots_.find(id);
    if (it == slots_.end()) {
        std::cout << "  No slot with ID '" << id << "'.\n";
        return;
    }
    if (it->second.occupied()) {
        std::cout << "  Cannot remove slot " << id << ": occupied by vehicle "
                  << it->second.occupantPlate() << ". Exit the vehicle first.\n";
        return;
    }

    slots_.erase(it);
    std::cout << "  Slot " << id << " removed.\n";
}

void ParkingSystem::displaySlots() const {
    if (slots_.empty()) {
        std::cout << "  No slots configured yet.\n";
        return;
    }

    std::cout << "\n  " << std::left << std::setw(8) << "ID" << std::setw(13) << "Type"
              << std::setw(12) << "Zone" << std::setw(11) << "Status" << "Occupant\n";
    std::cout << "  " << std::string(55, '-') << "\n";
    for (const auto& slot : slots_ | std::views::values) {
        std::cout << "  " << std::left << std::setw(8) << slot.id()
                  << std::setw(13) << typeName(slot.type())
                  << std::setw(12) << slot.zone()
                  << std::setw(11) << (slot.occupied() ? "Occupied" : "Available")
                  << (slot.occupied() ? slot.occupantPlate() : std::string("-")) << "\n";
    }

    std::cout << "\n  Availability summary:\n";
    for (const VehicleType type : {VehicleType::Motorcycle, VehicleType::Car, VehicleType::Truck}) {
        int total = 0;
        int occupied = 0;
        countSlots(type, total, occupied);
        std::cout << "    " << std::left << std::setw(12) << typeName(type)
                  << ": " << (total - occupied) << " available / " << total << " total\n";
    }
}

void ParkingSystem::vehicleEntry() {
    if (slots_.empty()) {
        std::cout << "  No slots configured yet. Configure slots first (menu 1).\n";
        return;
    }

    const std::string plate = toUpperCopy(readNonEmpty("  Vehicle plate number: "));
    if (const auto it = active_.find(plate); it != active_.end()) {
        std::cout << "  Vehicle " << plate << " is ALREADY parked in slot " << it->second.slotId
                  << " (entered at " << fmtTime(it->second.entryMin)
                  << "). A vehicle cannot be parked twice at the same time.\n";
        return;
    }

    const VehicleType type = readVehicleType();
    ParkingSlot* slot = findFreeSlot(type);
    if (!slot) {
        int total = 0;
        int occupied = 0;
        countSlots(type, total, occupied);
        std::cout << "  Sorry, no available " << typeName(type) << " slot right now ("
                  << occupied << "/" << total << " occupied). Please try again later.\n";
        return;
    }

    const int entry = readTime("  Entry time");
    ActiveParking rec;
    rec.vehicle = makeVehicle(type, plate);
    rec.slotId = slot->id();
    rec.entryMin = entry;
    slot->occupy(plate);
    active_.emplace(plate, std::move(rec));

    std::cout << "  Vehicle " << plate << " (" << typeName(type) << ") allocated slot "
              << slot->id() << " (zone " << slot->zone() << ") at " << fmtTime(entry) << ".\n";
}

void ParkingSystem::vehicleExit() {
    if (active_.empty()) {
        std::cout << "  No vehicles are currently parked.\n";
        return;
    }

    const std::string plate = toUpperCopy(readNonEmpty("  Vehicle plate number: "));
    const auto it = active_.find(plate);
    if (it == active_.end()) {
        for (auto h = history_.rbegin(); h != history_.rend(); ++h) {
            if (h->plate == plate) {
                std::cout << "  Vehicle " << plate << " is not parked now (last exited at "
                          << fmtTime(h->exitMin) << " from slot " << h->slotId << ").\n";
                return;
            }
        }
        std::cout << "  No parked vehicle with plate '" << plate << "'.\n";
        return;
    }

    auto& [vehicle, slotId, entryMin] = it->second;
    std::cout << "  Found: " << plate << " (" << vehicle->typeNameStr() << "), slot "
              << slotId << ", entered at " << fmtTime(entryMin) << ".\n";

    int exitMin = 0;
    while (true) {
        exitMin = readTime("  Exit time");
        if (exitMin >= entryMin) {
            break;
        }
        std::cout << "  Exit time cannot be before entry time (" << fmtTime(entryMin)
                  << "). Try again.\n";
    }

    const long long minutes = exitMin - entryMin;
    const long long hours = std::max(1LL, (minutes + 59) / 60);
    const VehicleType type = vehicle->type();
    const double rate = tariff_.get(type);
    const double fee = static_cast<double>(hours) * rate;

    Transaction tr;
    tr.nbr = static_cast<int>(history_.size()) + 1;
    tr.plate = plate;
    tr.vehicleType = vehicle->typeNameStr();
    tr.slotId = slotId;
    tr.entryMin = entryMin;
    tr.exitMin = exitMin;
    tr.billedHours = hours;
    tr.rateApplied = rate;
    tr.fee = fee;

    if (const auto slotIt = slots_.find(slotId); slotIt != slots_.end()) {
        tr.zone = slotIt->second.zone();
        slotIt->second.release();
    }

    active_.erase(it);
    history_.push_back(tr);

    std::cout << "\n  ---------- PARKING RECEIPT ----------\n";
    std::cout << "  Plate     : " << tr.plate << "\n";
    std::cout << "  Type      : " << tr.vehicleType << "\n";
    std::cout << "  Slot      : " << tr.slotId << " (zone " << tr.zone << ")\n";
    std::cout << "  Entry     : " << fmtTime(tr.entryMin) << "    Exit: " << fmtTime(tr.exitMin)
              << "\n";
    std::cout << "  Duration  : " << minutes << " minute(s) -> billed " << hours << " hour(s)"
              << (minutes == 0 ? " (minimum charge)" : "") << "\n";
    std::cout << "  Rate      : " << std::fixed << std::setprecision(2) << rate << " RWF/hour\n";
    std::cout << "  TOTAL FEE : " << std::fixed << std::setprecision(2) << fee << " RWF\n";
    std::cout << "  Slot " << tr.slotId << " is now Available.\n";
    std::cout << "  -------------------------------------\n";
}

void ParkingSystem::showTariffs() const {
    std::cout << "\n  Current tariffs (RWF per hour, partial hour = full hour):\n";
    for (const VehicleType type : {VehicleType::Motorcycle, VehicleType::Car, VehicleType::Truck}) {
        std::cout << "    " << std::left << std::setw(12) << typeName(type) << ": "
                  << std::fixed << std::setprecision(2) << tariff_.get(type) << "\n";
    }
}

void ParkingSystem::setTariff(const VehicleType type, const double rate) {
    tariff_.set(type, rate);
}

double ParkingSystem::tariffFor(const VehicleType type) const {
    return tariff_.get(type);
}

void ParkingSystem::displayParked() const {
    if (active_.empty()) {
        std::cout << "  No vehicles are currently parked.\n";
        return;
    }

    std::cout << "\n  " << std::left << std::setw(12) << "Plate" << std::setw(13) << "Type"
              << std::setw(8) << "Slot" << std::setw(12) << "Zone" << "Entry\n";
    std::cout << "  " << std::string(50, '-') << "\n";
    for (const auto& slot : slots_ | std::views::values) {
        if (!slot.occupied()) {
            continue;
        }
        const auto it = active_.find(slot.occupantPlate());
        if (it == active_.end()) {
            continue;
        }
        std::cout << "  " << std::left << std::setw(12) << it->first
                  << std::setw(13) << it->second.vehicle->typeNameStr()
                  << std::setw(8) << it->second.slotId
                  << std::setw(12) << slot.zone()
                  << fmtTime(it->second.entryMin) << "\n";
    }
    std::cout << "  Total parked: " << active_.size() << " vehicle(s).\n";
}

void ParkingSystem::vehicleHistory() const {
    const std::string plate = toUpperCopy(readNonEmpty("  Plate number to look up: "));
    bool any = false;
    for (const Transaction& tr : history_) {
        if (tr.plate != plate) {
            continue;
        }
        if (!any) {
            printTransactionHeader();
        }
        printTransaction(tr);
        any = true;
    }

    const auto it = active_.find(plate);
    if (it != active_.end()) {
        std::cout << "  Note: " << plate << " is CURRENTLY parked in slot " << it->second.slotId
                  << " since " << fmtTime(it->second.entryMin) << " (not billed yet).\n";
    }
    if (!any && it == active_.end()) {
        std::cout << "  No records found for plate '" << plate << "'.\n";
    } else if (!any) {
        std::cout << "  No completed transactions yet for '" << plate << "'.\n";
    }
}

void ParkingSystem::dailyRevenue() const {
    if (history_.empty()) {
        std::cout << "  No completed transactions yet - revenue is 0.00 RWF.\n";
        return;
    }

    std::cout << "\n  All completed transactions (this session = one day):\n";
    printTransactionHeader();
    double total = 0.0;
    std::map<std::string, std::pair<int, double>> byType;
    for (const Transaction& tr : history_) {
        printTransaction(tr);
        total += tr.fee;
        byType[tr.vehicleType].first += 1;
        byType[tr.vehicleType].second += tr.fee;
    }

    std::cout << "\n  Breakdown by vehicle type:\n";
    for (const auto& [type, data] : byType) {
        std::cout << "    " << std::left << std::setw(12) << type << ": " << data.first
                  << " transaction(s), " << std::fixed << std::setprecision(2) << data.second
                  << " RWF\n";
    }
    std::cout << "  DAILY REVENUE: " << std::fixed << std::setprecision(2) << total
              << " RWF from " << history_.size() << " transaction(s).\n";
}

}  // namespace parking
