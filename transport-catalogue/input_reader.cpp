#include "input_reader.h"

namespace TransportSystem {

InputReader::InputReader(std::istream& input_stream) : input_stream_(input_stream) {}

Coordinates ParseCoordinates(std::string_view str) {
    static const double nan = std::nan("");

    auto not_space = str.find_first_not_of(' ');
    auto comma = str.find(',');

    if (comma == str.npos) {
        return {nan, nan};
    }

    auto not_space2 = str.find_first_not_of(' ', comma + 1);

    double lat = std::stod(std::string(str.substr(not_space, comma - not_space)));
    double lng = std::stod(std::string(str.substr(not_space2)));

    return {lat, lng};
}

std::string_view Trim(std::string_view string) {
    const auto start = string.find_first_not_of(' ');
    if (start == string.npos) {
        return {};
    }
    return string.substr(start, string.find_last_not_of(' ') + 1 - start);
}

std::vector<std::string> Split(std::string_view string, char delim) {
    std::vector<std::string> result;

    size_t pos = 0;
    while ((pos = string.find_first_not_of(' ', pos)) < string.length()) {
        auto delim_pos = string.find(delim, pos);
        if (delim_pos == string.npos) {
            delim_pos = string.size();
        }
        if (auto substr = Trim(string.substr(pos, delim_pos - pos)); !substr.empty()) {
            result.push_back(std::string(substr));
        }
        pos = delim_pos + 1;
    }

    return result;
}

std::vector<std::string> ParseRoute(std::string_view route) {
    if (route.find('>') != route.npos) {
        return Split(route, '>');
    }

    auto stops = Split(route, '-');
    std::vector<std::string> results(stops.begin(), stops.end());
    results.insert(results.end(), std::next(stops.rbegin()), stops.rend());

    return results;
}

CommandDescription ParseCommandDescription(std::string_view line) {
    auto colon_pos = line.find(':');
    if (colon_pos == line.npos) {
        return {};
    }

    auto space_pos = line.find(' ');
    if (space_pos >= colon_pos) {
        return {};
    }

    auto not_space = line.find_first_not_of(' ', space_pos);
    if (not_space >= colon_pos) {
        return {};
    }

    return {line.substr(0, space_pos),
            line.substr(not_space, colon_pos - not_space),
            line.substr(colon_pos + 1)};
}

// Добавлено определение функций-членов CommandDescription
CommandDescription::operator bool() const {
    return !command.empty() && !id.empty() && !description.empty();
}

bool CommandDescription::operator!() const {
    return !static_cast<bool>(*this);
}

void InputReader::ParseLine(std::string_view line, TransportCatalogue& catalogue) {
    auto command_description = ParseCommandDescription(line);
    if (command_description) {
        std::string command_str = std::string(command_description.command);
        std::string id_str = std::string(command_description.id);
        std::string desc_str = std::string(command_description.description);

        if (command_str == "Stop") {
            Coordinates coordinates = ParseCoordinates(desc_str);
            catalogue.AddStop(id_str, coordinates);
        } else if (command_str == "Bus") {
            std::vector<std::string> stops = ParseRoute(desc_str);
            catalogue.AddBusRoute(id_str, stops);
        }
    }
}

} // namespace TransportSystem
