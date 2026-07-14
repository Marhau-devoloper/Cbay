#include "iostream"
#include "curl/curl.h"
#include "include/json.hpp"
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
using namespace std;
using json = nlohmann::json;



size_t curl_to_string(void* ptr, size_t size, size_t nmemb, void* data)
{
    auto* str = static_cast<std::string*>(data);
    str->append(static_cast<char*>(ptr), size * nmemb);
    return size * nmemb;
}



json GetList(string* RawJson){
    
    
    json CookedJson = json::parse(*RawJson);
    if (CookedJson[0]["id"] == "0" && CookedJson[0]["info_hash"] == "0000000000000000000000000000000000000000"){
        cout << "Invalid Name" << endl;
        exit(0);
    }else{
        if (CookedJson.size() > 1){
            for (auto it = CookedJson.begin(); it != CookedJson.end(); )
            {
                int seeders = stoi((*it)["seeders"].get<string>());
                int leechers = stoi((*it)["leechers"].get<string>());

                if (seeders <= 2 && leechers <= 2)
                {
                    it = CookedJson.erase(it);
                }
                else
                {
                    ++it;
                }
            }
            if (CookedJson.empty()){
                cout << "All links are dead";
                exit(0);
            }
            return CookedJson;
    }    
    return {};
    }
    
}



string MakeRequest(string Query){
    string URL = "https://apibay.org/q.php?q=" + Query + "&cat=100";
    CURL* handle = curl_easy_init();
    string RawJson;
    
    curl_easy_setopt(handle,CURLOPT_URL, URL.c_str());
    curl_easy_setopt(handle,CURLOPT_WRITEFUNCTION,curl_to_string);
    curl_easy_setopt(handle,CURLOPT_WRITEDATA,&RawJson);
    curl_easy_perform(handle);
    curl_easy_cleanup(handle);
    return RawJson;
    

}
pid_t aria_pid = -1;

void Download(string Hash, string Path)
{
    string magnet =
        "magnet:?xt=urn:btih:" + Hash +
        "&tr=udp://tracker.opentrackr.org:1337/announce"
        "&tr=udp://tracker.openbittorrent.com:80/announce"
        "&tr=udp://tracker.torrent.eu.org:451/announce";

    aria_pid = fork();

    if (aria_pid == 0)
    {
        execlp(
            "aria2c",
            "aria2c",
            magnet.c_str(),
            "-d",
            Path.c_str(),
            "--seed-time=0",
            "--enable-dht=true",
            "--max-overall-download-limit=0",
            "--bt-max-peers=100",
            nullptr
        );

        exit(1);
    }

    cout << "aria2c PID: " << aria_pid << endl;
}

void StopDownload()
{
    if (aria_pid > 0)
    {
        kill(aria_pid, SIGTERM);
        waitpid(aria_pid, nullptr, 0);
    }
}
void SignalHandler(int signal)
{
    StopDownload();
    exit(0);
}

int main(){

    signal(SIGINT, SignalHandler); 
    signal(SIGTERM, SignalHandler); 
    string Query;
    string Path;
    system("clear");
    cout << R"(
          ░██████  ░██                              
         ░██   ░██ ░██                              
        ░██        ░████████   ░██████   ░██    ░██ 
        ░██        ░██    ░██       ░██  ░██    ░██ 
        ░██        ░██    ░██  ░███████  ░██    ░██ 
         ░██   ░██ ░███   ░██ ░██   ░██  ░██   ░███ 
          ░██████  ░██░█████   ░█████░██  ░█████░██ 
                                                ░██ 
                                          ░███████   by Marhau

        
    =========================================================================================
    |                        Cbay - BitTorrent Utility                                      |
    |                                                                                       |
    |  Thank you for using Cbay.                                                            |
    |                                                                                       |
    | By using this software, you agree that it is intended and will used only for          |
    | educational purposes and for downloading/sharing legal, freely distributable content. |
    |                                                                                       |
    | The author does not host, distribute, or promote copyrighted material.                |
    | Users are responsible for following all applicable local laws and regulations.        |
    =========================================================================================


)" << endl;
    cout << "Search:";
    getline(cin, Query);
    replace(Query.begin(), Query.end(), ' ', '-');
    cout << "==============" << endl;
    cout << "Download Path: ";
    cin >> Path;
    system("clear");
    string Raw = MakeRequest(Query);
    json List = GetList(&Raw);
    if (List == "{}"){
        exit(0);
    }
    cout << "===============================================================================" << endl;
    for(int x = 0; x < List.size(); x++){
        cout << "|" << " Num " << x << " Name " << List[x]["name"] << " size " << to_string(stoll(List[x]["size"].get<string>()) / 1000000) + " MB" << " Seeders " << List[x]["seeders"] << " leechers " << List[x]["leechers"]<< endl; 
    }
    cout << "===============================================================================" << endl;

    cout << "Choose NUM from " << "0 to " << (List.size() - 1 ) << endl;
    int choice;
    cout << "Num:";
    cin >> choice;
    
    if (choice > List.size()){
        cout << "Please Select Smaller Number" << endl;
        return 0;
    }else{
        system("clear");
        cout << "You Choose Num "<< choice << " Which is " << List[choice]["name"] << " Also it has " << List[choice]["seeders"] << " Seeders and " <<  List[choice]["leechers"] << " leechers " << endl;
        cout << "You will need " << to_string(stoll(List[choice]["size"].get<string>()) / 1000000) + " MB" << " of Space" << endl;
        cout << "Download Path is " << Path << endl;
        string confirm;
        cout << "Do you want to Download it ?  yes or no" << endl;
        cout << "confirm: ";
        cin >>confirm;
        if (confirm == "Yes" or confirm == "yes" or confirm == "y" or confirm == "Y"){
            Download(List[choice]["info_hash"],Path);
            waitpid(aria_pid, nullptr, 0); 

            StopDownload();

            return 0;
        }else{
            system("clear");
            cout << "Download Was Canceled" << endl;
            return 0;
        }
    
    }

    }