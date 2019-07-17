#include "../SAM.Picker/MyClientSocket.h"
#include <string>
#include <iostream>

MySteam* g_steam = nullptr;
MainPickerWindow* g_main_gui = nullptr;
char* g_cache_folder = nullptr;
MySteamClient* g_steamclient = nullptr;

int main() {
    g_cache_folder = "/home/paul/.SamRewritten";

    MyClientSocket* client = new MyClientSocket(206690);

    std::string input;

    while(std::cin >> input) {
        std::string test = client->request_response(input.c_str());
        std::cout << "response: " << test << std::endl;
    }

    client->kill_server();
    delete client;

    return 0;
}