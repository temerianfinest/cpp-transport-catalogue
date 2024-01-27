#include <iostream>
#include <string>
#include "json_reader.h"

int main() {
    TransportCatalogue::TransportCatalogue catalogue;

    JSON::Reader jsonReader(std::cin);
    const auto& rendererSettings = jsonReader.GetRendererSettingsReader();
    const auto& baseRequestReader = jsonReader.GetBaseRequestReader();
    const auto& statRequestReader = jsonReader.GetStatRequestReader();

    TransportCatalogue::RequestHandler requestHandler(catalogue, rendererSettings.GetRenderSettings());
    requestHandler.ProcessRequests(baseRequestReader.GetBaseRequests());
    auto responses = requestHandler.GetResponse(statRequestReader.GetStatRequests());

    JSON::Print(JSON::ParseResponses(responses), std::cout);
}