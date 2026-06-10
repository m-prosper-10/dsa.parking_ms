#include "parking/ConsoleApp.h"

#include "parking/ParkingSystem.h"
#include "parking/ConsoleInput.h"

#include <iostream>
#include <string>

namespace parking {

void ParkingConsoleApp::run() {
    ParkingSystem system;

    std::cout << "***********************************************************************************************************\n";
    std::cout << "====================================== SMART PARKING MANAGEMENT SYSTEM ====================================\n";
    std::cout << "***********************************************************************************************************\n";
    std::cout << "Default tariffs: Motorcycle 500, Car 1000, Truck 2000 RWF/h\n";

    while (true) {
        std::cout << "\nMenu:\n"
                     "  1. Add parking slot\n"
                     "  2. Remove parking slot\n"
                     "  3. Display all slots\n"
                     "  4. Vehicle entry\n"
                     "  5. Vehicle exit (calculates parking fee)\n"
                     "  6. Vehicle history (by vehicle plate number)\n"
                     "  7. View parked vehicles\n"
                     "  8. Daily revenue report\n"
                     "  9. View current tariffs\n"
                     " 10. Update tariff\n"
                     "  0. Exit\n";

        switch (const int choice = readIntInRange("Enter your choice: ", 0, 10)) {
            case 1: system.addSlot(); break;
            case 2: system.removeSlot(); break;
            case 3: system.displaySlots(); break;
            case 4: system.vehicleEntry(); break;
            case 5: system.vehicleExit(); break;
            case 6: system.vehicleHistory(); break;
            case 7: system.displayParked(); break;
            case 8: system.dailyRevenue(); break;
            case 9: system.showTariffs(); break;
            case 10: {
                system.showTariffs();
                const VehicleType type = readVehicleType();
                const double oldRate = system.tariffFor(type);
                const double newRate = readPositiveAmount("  New hourly rate (RWF) for " + typeName(type) + ": ");
                const std::string ans = toUpperCopy(readNonEmpty(
                    "  Confirm change " + typeName(type) + ": " +
                    std::to_string(static_cast<long long>(oldRate)) + " -> " +
                    std::to_string(static_cast<long long>(newRate)) + " RWF/h ? (Y/N): "));
                if (ans != "Y" && ans != "YES") {
                    std::cout << "  Price update cancelled.\n";
                    break;
                }
                system.setTariff(type, newRate);
                std::cout << "  Tariff updated. Applies to vehicles exiting from now on;\n"
                             "  previously completed transactions keep their recorded fees.\n";
                break;
            }
            case 0:
                std::cout << "Goodbye. (Data is in-memory only)";
                return;
            default: break;
        }
    }
}

}  // namespace parking
