# Architecture

This project uses a simple layered structure:

- `Domain` holds the core entities and shared helpers.
- `ParkingSystem` contains the business rules and state.
- `ConsoleInput` isolates input validation and parsing.
- `ConsoleApp` owns the interactive menu and user flow.
- `main` only starts the app.

## Module Overview

```mermaid
flowchart TD
    Main[src/main.cpp] --> App[ConsoleApp]
    App --> Input[ConsoleInput]
    App --> Service[ParkingSystem]
    Service --> Domain[Domain Model]
    Input --> Domain
    Service --> Domain

    Domain --> Vehicle[Vehicle hierarchy]
    Domain --> Slot[ParkingSlot]
    Domain --> Tariff[Tariff]
    Domain --> Tx[Transaction]
```

## Runtime Flow

```mermaid
sequenceDiagram
    participant User
    participant App as ConsoleApp
    participant Input as ConsoleInput
    participant Service as ParkingSystem

    User->>App: choose menu option
    App->>Input: read and validate user input
    Input-->>App: validated value
    App->>Service: call operation
    Service-->>App: result / report text
    App-->>User: print output
```

## Core Data Flow

```mermaid
classDiagram
    class Vehicle {
        +plate()
        +type()
        +typeNameStr()
    }

    class Motorcycle
    class Car
    class Truck

    Vehicle <|-- Motorcycle
    Vehicle <|-- Car
    Vehicle <|-- Truck

    class ParkingSlot {
        -id_
        -type_
        -zone_
        -occupied_
        -occupantPlate_
        +occupy()
        +release()
    }

    class Tariff {
        -rate_
        +get()
        +set()
    }

    class ActiveParking {
        +vehicle
        +slotId
        +entryMin
    }

    class Transaction {
        +nbr
        +plate
        +vehicleType
        +slotId
        +zone
        +entryMin
        +exitMin
        +billedHours
        +rateApplied
        +fee
    }

    class ParkingSystem {
        -slots_
        -active_
        -history_
        -tariff_
        +addSlot()
        +removeSlot()
        +vehicleEntry()
        +vehicleExit()
        +displaySlots()
        +displayParked()
        +vehicleHistory()
        +dailyRevenue()
        +showTariffs()
        +setTariff()
    }

    ParkingSystem --> ParkingSlot
    ParkingSystem --> ActiveParking
    ParkingSystem --> Transaction
    ParkingSystem --> Tariff
    ActiveParking --> Vehicle
```

## Design Notes

- `map<string, ParkingSlot>` keeps slot iteration sorted by ID.
- `unordered_map<string, ActiveParking>` gives fast lookup for parked vehicles.
- `vector<Transaction>` keeps completed transactions in exit order.
- Tariff changes are applied only at the moment of exit, so history stays immutable.
