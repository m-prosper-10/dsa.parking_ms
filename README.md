# Smart Parking Management System

Console C++20 application for parking-slot management, vehicle entry/exit, tariff updates, and revenue reporting. All data stays in memory.

## Layout

- `include/parking/Domain.h` - core domain types and helpers
- `include/parking/ParkingSystem.h` - business logic interface
- `include/parking/ConsoleInput.h` - validated console input helpers
- `include/parking/ConsoleApp.h` - UI entry point
- `src/ParkingSystem.cpp` - business logic implementation
- `src/ConsoleInput.cpp` - input parsing and time handling
- `src/ConsoleApp.cpp` - interactive menu
- `src/main.cpp` - program entry point

## Build

```bash
cmake -S . -B build
cmake --build build
```

## Run

From the build directory generated above:

```bash
./build/DSA
```

On Windows with CMake/Ninja or Visual Studio generators, run the built executable from the matching build folder, for example:

```powershell
.\build\Debug\DSA.exe
```

## Behavior

- Prices are charged per hour, with partial hours rounded up.
- Tariff changes only affect future exits.
- Slot IDs and plates are normalized to uppercase.
- A vehicle cannot be parked twice at the same time.
- History is preserved in memory for the current session only.

## Notes

This project is intentionally in-memory only. Restarting the program clears all slots, parked vehicles, and transaction history.
