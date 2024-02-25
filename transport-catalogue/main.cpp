#include "request_handler.h"

int main(){
    using namespace transport_catalogue;
    json_reader::JsonReader json_input(json::Load(std::cin));
    TransportCatalogue t(json_input.BaseRequestsReturn());
    TransportRouter tr(t, json_input.RouterSettingsReturn());
    request_handler::RequestHandler answers(t, json_input.StatRequestsReturn(), json_input.RenderSettingsReturn(), tr);
    auto answers_map = json_input.MakeJSON(answers.GetAnswers());
    Print(answers_map, std::cout);
    return 0;
}