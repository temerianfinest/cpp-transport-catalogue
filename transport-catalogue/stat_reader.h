#pragma once

#include <iosfwd>
#include <string_view>
#include "transport_catalogue.h"

namespace TransportSystem {

class StatReader {
public:
    explicit StatReader(std::istream& input_stream, std::ostream& output_stream);
   
    double CalculateCurvature(const TransportCatalogue& transport_catalogue, const std::string& bus_name) const;

    void ParseAndPrintStat(const TransportCatalogue& transport_catalogue, std::string_view request);

private:
    std::istream& input_stream_;
    std::ostream& output_stream_;
};

} // namespace TransportSystem