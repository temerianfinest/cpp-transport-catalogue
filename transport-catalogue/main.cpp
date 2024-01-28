#include <iostream>
#include <string>

#include "json_reader.h"

int main() 
{
    transport_catalogue::TransportCatalogue catalogue;

    json::Reader reader(std::cin);
    const auto& rsr = reader.GetRendererSettingsReader();
    const auto& brr = reader.GetBaseRequestReader();
    const auto& srr = reader.GetStatRequestReader();

    transport_catalogue::RequestHandler rh(catalogue, rsr.GetRenderSettings());
    rh.ProcessRequests(brr.GetBaseRequests());
    json::Print(json::ParseResponses(rh.GetResponse(srr.GetStatRequests())), std::cout);
}