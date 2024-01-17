#pragma once

#include <string>
#include <string_view>
#include "transport_catalogue.h"

namespace TransportSystem {

struct CommandDescription {
    explicit operator bool() const;
    bool operator!() const;

    std::string_view command;
    std::string_view id;
    std::string_view description;
};

class InputReader {
public:
    void ParseLine(std::string_view line, TransportCatalogue& catalogue);
};

} // namespace TransportSystem
