#pragma once

#include "parking/Domain.h"

#include <string>

namespace parking {

std::string readRawLine(const std::string& prompt);
std::string readNonEmpty(const std::string& prompt);
int readIntInRange(const std::string& prompt, int lo, int hi);
double readPositiveAmount(const std::string& prompt);
int parseTimeHHMM(const std::string& s);
int readTime(const std::string& prompt);
VehicleType readVehicleType();

}  // namespace parking
