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
    explicit InputReader(std::istream& input_stream);
    
     void FirstPassParseLine(std::string_view line, TransportCatalogue& catalogue);
    void SecondPassParseLine(std::string_view line, TransportCatalogue& catalogue);

    void ParseAndSetDistances(const std::string& desc, const std::string& stop_name, TransportCatalogue& catalogue);
    
    
private:
    std::istream& input_stream_; 
};

} // namespace TransportSystem
