#pragma once

#include <iosfwd>
#include <string_view>
#include "transport_catalogue.h"

namespace TransportSystem {

class StatReader {
public:
    static void ParseAndPrintStat(const TransportCatalogue& transport_catalogue, std::string_view request,
                                  std::ostream& output);
};

} // namespace TransportSystem
