#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <memory>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cctype>
#include <ctime>

using namespace std;

string trim(const string& s){
    size_t a = s.find_first_not_of(" \t\r\n");
    size_t b = s.find_last_not_of(" \t\r\n");
    if (a == string::npos) return "";
    return s.substr(a, b - a + 1);
}

string toUpperCopy(string s){
    for (char& c : s) c = (char)toupper((unsigned char)c);
    return s;
}

string readRawLine(const string& prompt){
    cout << prompt;
    string line;
    if (!getline(cin, line)){
        cout << "\n[Input stream ended - exiting safely]\n";
        exit(0);
    }
    return trim(line);
}

string readNonEmpty(const string& prompt){
    while (true){
        string s = readRawLine(prompt);
        if (!s.empty()) return s;
        cout << "  Value cannot be empty, try again.\n";
    }
}

bool parseIntStrict(const string& s, long long& out){
    if (s.empty()) return false;
    size_t pos = 0;
    try { out = stoll(s, &pos); }
    catch (...) { return false; }            // junk or overflow
    return pos == s.size();
}

int readIntInRange(const string& prompt, int lo, int hi){
    while (true){
        string s = readRawLine(prompt);
        long long v;
        if (parseIntStrict(s, v) && v >= lo && v <= hi) return (int)v;
        cout << "  Invalid input. Enter a whole number between "
             << lo << " and " << hi << ".\n";
    }
}

double readPositiveAmount(const string& prompt){
    while (true){
        string s = readRawLine(prompt);
        try {
            size_t pos = 0;
            double v = stod(s, &pos);
            if (pos == s.size() && v > 0 && v <= 1e12) return v;
        } catch (...) {}
        cout << "  Invalid amount. Enter a positive number.\n";
    }
}

int parseTimeHHMM(const string& s){
    size_t c = s.find(':');
    if (c == string::npos || c == 0 || c + 1 >= s.size()) return -1;
    string hs = s.substr(0, c), ms = s.substr(c + 1);
    if (hs.size() > 2 || ms.size() != 2) return -1;
    for (char ch : hs) if (!isdigit((unsigned char)ch)) return -1;
    for (char ch : ms) if (!isdigit((unsigned char)ch)) return -1;
    int h = stoi(hs), m = stoi(ms);
    if (h > 23 || m > 59) return -1;
    return h * 60 + m;
}

int nowMinutes(){
    time_t t = time(nullptr);
    tm* lt = localtime(&t);
    return lt->tm_hour * 60 + lt->tm_min;
}

string fmtTime(int minutes){
    ostringstream o;
    o << setw(2) << setfill('0') << minutes / 60 << ":"
      << setw(2) << setfill('0') << minutes % 60;
    return o.str();
}

int readTime(const string& prompt){
    while (true){
        string s = readRawLine(prompt + " (HH:MM, blank = now): ");
        if (s.empty()){
            int t = nowMinutes();
            cout << "  Using current time " << fmtTime(t) << ".\n";
            return t;
        }
        int t = parseTimeHHMM(s);
        if (t >= 0) return t;
        cout << "  Invalid time. Use 24-hour HH:MM (e.g. 08:30).\n";
    }
}

enum class VehicleType { MOTORCYCLE = 1, CAR = 2, TRUCK = 3 };

string typeName(VehicleType t){
    switch (t){
        case VehicleType::MOTORCYCLE: return "Motorcycle";
        case VehicleType::CAR:        return "Car";
        case VehicleType::TRUCK:      return "Truck";
    }
    return "?";
}

VehicleType readVehicleType(){
    cout << "  Vehicle type: 1 = Motorcycle, 2 = Car, 3 = Truck\n";
    return (VehicleType)readIntInRange("  Choose type (1-3): ", 1, 3);
}

// Abstract base class: abstraction + inheritance + polymorphism.
class Vehicle {
    string plate_;
public:
    explicit Vehicle(string plate) : plate_(move(plate)) {}
    virtual ~Vehicle() = default;
    virtual VehicleType type() const = 0;          // pure virtual
    string typeNameStr() const { return typeName(type()); }
    const string& plate() const { return plate_; }
};

class Motorcycle : public Vehicle {
public:
    using Vehicle::Vehicle;
    VehicleType type() const override { return VehicleType::MOTORCYCLE; }
};
class Car : public Vehicle {
public:
    using Vehicle::Vehicle;
    VehicleType type() const override { return VehicleType::CAR; }
};
class Truck : public Vehicle {
public:
    using Vehicle::Vehicle;
    VehicleType type() const override { return VehicleType::TRUCK; }
};

unique_ptr<Vehicle> makeVehicle(VehicleType t, const string& plate){
    switch (t){
        case VehicleType::MOTORCYCLE: return make_unique<Motorcycle>(plate);
        case VehicleType::CAR:        return make_unique<Car>(plate);
        case VehicleType::TRUCK:      return make_unique<Truck>(plate);
    }
    return nullptr;
}

// ================= core entities =================================
class ParkingSlot {
    string id_;
    VehicleType type_;
    string zone_;
    bool occupied_ = false;
    string occupantPlate_;        // back-link kept in sync with `active`
public:
    ParkingSlot(string id, VehicleType t, string zone)
        : id_(move(id)), type_(t), zone_(move(zone)) {}
    const string& id() const            { return id_; }
    VehicleType type() const            { return type_; }
    const string& zone() const          { return zone_; }
    bool occupied() const               { return occupied_; }
    const string& occupantPlate() const { return occupantPlate_; }
    void occupy(const string& plate){ occupied_ = true; occupantPlate_ = plate; }
    void release()                  { occupied_ = false; occupantPlate_.clear(); }
};

class Tariff {
    map<VehicleType, double> rate_;
public:
    Tariff(){
        rate_[VehicleType::MOTORCYCLE] = 500.0;   // default (paper)
        rate_[VehicleType::CAR]        = 1000.0;  // default (paper)
        rate_[VehicleType::TRUCK]      = 2000.0;  // assumption: not in paper
    }
    double get(VehicleType t) const { return rate_.at(t); }
    void set(VehicleType t, double r){ rate_.at(t) = r; }  // caller validates r > 0
};

struct ActiveParking {
    unique_ptr<Vehicle> vehicle;   // polymorphic: held through base pointer
    string slotId;
    int entryMin = 0;
};

// Full snapshot at exit time: later price updates or slot removals can
// NEVER change a completed transaction (Task 3 rule 4).
struct Transaction {
    int nbr = 0;
    string plate;
    string vehicleType;
    string slotId;
    string zone;
    int entryMin = 0;
    int exitMin = 0;
    long long billedHours = 0;
    double rateApplied = 0.0;
    double fee = 0.0;
};

// ================= the system ====================================
class ParkingSystem {
    map<string, ParkingSlot> slots_;                 // key = slot ID (UPPER)
    unordered_map<string, ActiveParking> active_;    // key = plate  (UPPER)
    vector<Transaction> history_;
    Tariff tariff_;

    // first available slot (by sorted ID) whose type matches exactly
    ParkingSlot* findFreeSlot(VehicleType t){
        for (auto& kv : slots_)
            if (kv.second.type() == t && !kv.second.occupied())
                return &kv.second;
        return nullptr;
    }

    void countSlots(VehicleType t, int& total, int& occupied) const {
        total = occupied = 0;
        for (const auto& kv : slots_)
            if (kv.second.type() == t){
                ++total;
                if (kv.second.occupied()) ++occupied;
            }
    }

    static void printTransaction(const Transaction& tr){
        cout << "  " << left << setw(5) << tr.nbr
             << setw(12) << tr.plate
             << setw(12) << tr.vehicleType
             << setw(8)  << tr.slotId
             << setw(10) << tr.zone
             << setw(7)  << fmtTime(tr.entryMin)
             << setw(7)  << fmtTime(tr.exitMin)
             << setw(6)  << tr.billedHours
             << right << setw(10) << fixed << setprecision(2) << tr.rateApplied
             << setw(12) << tr.fee << "\n" << left;
    }

    static void printTransactionHeader(){
        cout << "  " << left << setw(5) << "Nbr" << setw(12) << "Plate"
             << setw(12) << "Type" << setw(8) << "Slot" << setw(10) << "Zone"
             << setw(7) << "In" << setw(7) << "Out" << setw(6) << "Hrs"
             << right << setw(10) << "Rate" << setw(12) << "Fee(RWF)"
             << "\n" << left;
        cout << "  " << string(87, '-') << "\n";
    }

public:
    // ---------- Task 1: slot configuration -----------------------
    void addSlot(){
        string id = toUpperCopy(readNonEmpty("  Slot ID: "));
        if (slots_.count(id)){
            cout << "  Slot '" << id << "' already exists (IDs are unique, "
                    "case-insensitive).\n";
            return;
        }
        VehicleType t = readVehicleType();
        string zone = readNonEmpty("  Zone (location): ");
        slots_.emplace(id, ParkingSlot(id, t, zone));
        cout << "  Slot " << id << " (" << typeName(t) << ", zone "
             << zone << ") configured. Status: Available.\n";
    }

    void removeSlot(){
        if (slots_.empty()){ cout << "  No slots configured yet.\n"; return; }
        string id = toUpperCopy(readNonEmpty("  Slot ID to remove: "));
        auto it = slots_.find(id);
        if (it == slots_.end()){
            cout << "  No slot with ID '" << id << "'.\n";
            return;
        }
        if (it->second.occupied()){
            cout << "  Cannot remove slot " << id << ": occupied by vehicle "
                 << it->second.occupantPlate() << ". Exit the vehicle first.\n";
            return;
        }
        slots_.erase(it);
        cout << "  Slot " << id << " removed.\n";
    }

    void displaySlots() const {
        if (slots_.empty()){ cout << "  No slots configured yet.\n"; return; }
        cout << "\n  " << left << setw(8) << "ID" << setw(13) << "Type"
             << setw(12) << "Zone" << setw(11) << "Status" << "Occupant\n";
        cout << "  " << string(55, '-') << "\n";
        for (const auto& kv : slots_){
            const ParkingSlot& s = kv.second;
            cout << "  " << left << setw(8) << s.id()
                 << setw(13) << typeName(s.type())
                 << setw(12) << s.zone()
                 << setw(11) << (s.occupied() ? "Occupied" : "Available")
                 << (s.occupied() ? s.occupantPlate() : string("-")) << "\n";
        }
        cout << "\n  Availability summary:\n";
        for (VehicleType t : { VehicleType::MOTORCYCLE, VehicleType::CAR,
                               VehicleType::TRUCK }){
            int total, occ;
            countSlots(t, total, occ);
            cout << "    " << left << setw(12) << typeName(t)
                 << ": " << (total - occ) << " available / "
                 << total << " total\n";
        }
    }

    // ---------- Task 2: vehicle entry -----------------------------
    void vehicleEntry(){
        if (slots_.empty()){
            cout << "  No slots configured yet. Configure slots first (menu 1).\n";
            return;
        }
        string plate = toUpperCopy(readNonEmpty("  Vehicle plate number: "));
        auto it = active_.find(plate);
        if (it != active_.end()){
            cout << "  Vehicle " << plate << " is ALREADY parked in slot "
                 << it->second.slotId << " (entered at "
                 << fmtTime(it->second.entryMin)
                 << "). A vehicle cannot be parked twice at the same time.\n";
            return;
        }
        VehicleType t = readVehicleType();
        ParkingSlot* slot = findFreeSlot(t);
        if (!slot){
            int total, occ;
            countSlots(t, total, occ);
            cout << "  Sorry, no available " << typeName(t) << " slot right now ("
                 << occ << "/" << total << " occupied). Please try again later.\n";
            return;
        }
        int entry = readTime("  Entry time");

        ActiveParking rec;
        rec.vehicle  = makeVehicle(t, plate);
        rec.slotId   = slot->id();
        rec.entryMin = entry;
        slot->occupy(plate);                          // both sides of the
        active_.emplace(plate, std::move(rec));       // link updated together
        cout << "  Vehicle " << plate << " (" << typeName(t)
             << ") allocated slot " << slot->id() << " (zone "
             << slot->zone() << ") at " << fmtTime(entry) << ".\n";
    }

    // ---------- Tasks 3 & 4: exit, duration, fee ------------------
    void vehicleExit(){
        if (active_.empty()){ cout << "  No vehicles are currently parked.\n"; return; }
        string plate = toUpperCopy(readNonEmpty("  Vehicle plate number: "));
        auto it = active_.find(plate);
        if (it == active_.end()){
            // distinguish "never seen" from "already exited"
            for (auto h = history_.rbegin(); h != history_.rend(); ++h)
                if (h->plate == plate){
                    cout << "  Vehicle " << plate << " is not parked now "
                         << "(last exited at " << fmtTime(h->exitMin)
                         << " from slot " << h->slotId << ").\n";
                    return;
                }
            cout << "  No parked vehicle with plate '" << plate << "'.\n";
            return;
        }

        ActiveParking& rec = it->second;
        cout << "  Found: " << plate << " (" << rec.vehicle->typeNameStr()
             << "), slot " << rec.slotId << ", entered at "
             << fmtTime(rec.entryMin) << ".\n";

        int exitMin;
        while (true){
            exitMin = readTime("  Exit time");
            if (exitMin >= rec.entryMin) break;
            cout << "  Exit time cannot be before entry time ("
                 << fmtTime(rec.entryMin) << "). Try again.\n";
        }

        long long minutes = exitMin - rec.entryMin;
        // partial hours billed as full; minimum charge = 1 hour
        long long hours = max(1LL, (minutes + 59) / 60);
        VehicleType t = rec.vehicle->type();
        double rate = tariff_.get(t);        // CURRENT rate, at exit time
        double fee  = (double)hours * rate;

        Transaction tr;
        tr.nbr         = (int)history_.size() + 1;
        tr.plate       = plate;
        tr.vehicleType = rec.vehicle->typeNameStr();
        tr.slotId      = rec.slotId;
        tr.entryMin    = rec.entryMin;
        tr.exitMin     = exitMin;
        tr.billedHours = hours;
        tr.rateApplied = rate;
        tr.fee         = fee;

        auto slotIt = slots_.find(rec.slotId);
        if (slotIt != slots_.end()){                 // defensive: should always exist
            tr.zone = slotIt->second.zone();
            slotIt->second.release();
        }
        active_.erase(it);
        history_.push_back(tr);

        cout << "\n  ---------- PARKING RECEIPT ----------\n";
        cout << "  Plate     : " << tr.plate << "\n";
        cout << "  Type      : " << tr.vehicleType << "\n";
        cout << "  Slot      : " << tr.slotId << " (zone " << tr.zone << ")\n";
        cout << "  Entry     : " << fmtTime(tr.entryMin)
             << "    Exit: " << fmtTime(tr.exitMin) << "\n";
        cout << "  Duration  : " << minutes << " minute(s) -> billed "
             << hours << " hour(s)"
             << (minutes == 0 ? " (minimum charge)" : "") << "\n";
        cout << "  Rate      : " << fixed << setprecision(2) << rate
             << " RWF/hour\n";
        cout << "  TOTAL FEE : " << fixed << setprecision(2) << fee
             << " RWF\n";
        cout << "  Slot " << tr.slotId << " is now Available.\n";
        cout << "  -------------------------------------\n";
    }

    // ---------- Task 3: tariff management -------------------------
    void showTariffs() const {
        cout << "\n  Current tariffs (RWF per hour, partial hour = full hour):\n";
        for (VehicleType t : { VehicleType::MOTORCYCLE, VehicleType::CAR,
                               VehicleType::TRUCK })
            cout << "    " << left << setw(12) << typeName(t) << ": "
                 << fixed << setprecision(2) << tariff_.get(t) << "\n";
    }

    void updateTariff(){
        showTariffs();
        VehicleType t = readVehicleType();
        double oldRate = tariff_.get(t);
        double newRate = readPositiveAmount("  New hourly rate (RWF) for "
                                            + typeName(t) + ": ");
        string ans = toUpperCopy(readNonEmpty(
            "  Confirm change " + typeName(t) + ": "
            + to_string((long long)oldRate) + " -> "
            + to_string((long long)newRate) + " RWF/h ? (Y/N): "));
        if (ans != "Y" && ans != "YES"){
            cout << "  Price update cancelled.\n";
            return;
        }
        tariff_.set(t, newRate);
        cout << "  Tariff updated. Applies to vehicles exiting from now on;\n"
                "  previously completed transactions keep their recorded fees.\n";
    }

    // ---------- Task 5: reports -----------------------------------
    void displayParked() const {
        if (active_.empty()){ cout << "  No vehicles are currently parked.\n"; return; }
        cout << "\n  " << left << setw(12) << "Plate" << setw(13) << "Type"
             << setw(8) << "Slot" << setw(12) << "Zone" << "Entry\n";
        cout << "  " << string(50, '-') << "\n";
        // iterate slots map so output is deterministic (sorted by slot ID)
        for (const auto& kv : slots_){
            if (!kv.second.occupied()) continue;
            auto it = active_.find(kv.second.occupantPlate());
            if (it == active_.end()) continue;       // defensive
            cout << "  " << left << setw(12) << it->first
                 << setw(13) << it->second.vehicle->typeNameStr()
                 << setw(8) << it->second.slotId
                 << setw(12) << kv.second.zone()
                 << fmtTime(it->second.entryMin) << "\n";
        }
        cout << "  Total parked: " << active_.size() << " vehicle(s).\n";
    }

    void vehicleHistory() const {
        string plate = toUpperCopy(readNonEmpty("  Plate number to look up: "));
        bool any = false;
        for (const Transaction& tr : history_){
            if (tr.plate != plate) continue;
            if (!any) printTransactionHeader();
            printTransaction(tr);
            any = true;
        }
        auto it = active_.find(plate);
        if (it != active_.end())
            cout << "  Note: " << plate << " is CURRENTLY parked in slot "
                 << it->second.slotId << " since "
                 << fmtTime(it->second.entryMin) << " (not billed yet).\n";
        if (!any && it == active_.end())
            cout << "  No records found for plate '" << plate << "'.\n";
        else if (!any)
            cout << "  No completed transactions yet for '" << plate << "'.\n";
    }

    void dailyRevenue() const {
        if (history_.empty()){
            cout << "  No completed transactions yet - revenue is 0.00 RWF.\n";
            return;
        }
        cout << "\n  All completed transactions (this session = one day):\n";
        printTransactionHeader();
        double total = 0;
        map<string, pair<int, double>> byType;       // type -> (count, sum)
        for (const Transaction& tr : history_){
            printTransaction(tr);
            total += tr.fee;
            byType[tr.vehicleType].first  += 1;
            byType[tr.vehicleType].second += tr.fee;
        }
        cout << "\n  Breakdown by vehicle type:\n";
        for (const auto& kv : byType)
            cout << "    " << left << setw(12) << kv.first << ": "
                 << kv.second.first << " transaction(s), "
                 << fixed << setprecision(2) << kv.second.second << " RWF\n";
        cout << "  DAILY REVENUE: " << fixed << setprecision(2) << total
             << " RWF from " << history_.size() << " transaction(s).\n";
    }
};

// ================= menu ==========================================
int main(){
    ParkingSystem system;

    cout << "=================================================\n";
    cout << "   KIGALI SMART PARKING MANAGEMENT SYSTEM (NE)\n";
    cout << "=================================================\n";
    cout << "Default tariffs: Motorcycle 500, Car 1000, Truck 2000 RWF/h\n";
    cout << "(Truck rate is an assumption - not given in the paper.)\n";

    while (true){
        cout << "\nMenu:\n"
                "  1. Add parking slot\n"
                "  2. Remove parking slot\n"
                "  3. Display all slots\n"
                "  4. Vehicle entry\n"
                "  5. Vehicle exit (parking fee calculated.)\n"
                "  6. View parked vehicles\n"
                "  7. Vehicle history (by plate number)\n"
                "  8. Daily revenue report\n"
                "  9. View current tariffs\n"
                " 10. Update tariff\n"
                "  0. Exit\n";
        int choice = readIntInRange("Enter your choice: ", 0, 10);

        switch (choice){
            case 1:  system.addSlot();        break;
            case 2:  system.removeSlot();     break;
            case 3:  system.displaySlots();   break;
            case 4:  system.vehicleEntry();   break;
            case 5:  system.vehicleExit();    break;
            case 6:  system.displayParked();  break;
            case 7:  system.vehicleHistory(); break;
            case 8:  system.dailyRevenue();   break;
            case 9:  system.showTariffs();    break;
            case 10: system.updateTariff();   break;
            case 0:
                cout << "Goodbye. (Data is in-memory only, as required";
                return 0;
        }
    }
}
