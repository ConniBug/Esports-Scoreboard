int qwqwawsqwaqswaqaqaqawaqswaq = 0;

// Dear ImGui: standalone example application for DirectX 11
// If you are new to Dear ImGui, read documentation from the docs/ folder + read the top of imgui.cpp.
// Read online: https://github.com/ocornut/imgui/tree/master/docs

#include <vector>

#include "nlohmann\json.hpp"

#include <array>
#include <filesystem>
#include <memory>
#include <string>

#include <iostream>

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include <d3d11.h>
#define DIRECTINPUT_VERSION 0x0800
#include <tchar.h>

#include <fstream>

#include <chrono>

float color_speed = -10.0;

float color_red = 1.0;
float color_green = 1.0;
float color_blue = 0;
float color_random = 0.0;

static int currentEventSelected = 0;
static int currentGameSelected = 0;
static int currentGameIDSelected = -1;
static int currentlySelectedTeam = -1;

int eventCount = 0;

// Data
static ID3D11Device* g_pd3dDevice = NULL;
static ID3D11DeviceContext* g_pd3dDeviceContext = NULL;
static IDXGISwapChain* g_pSwapChain = NULL;
static ID3D11RenderTargetView* g_mainRenderTargetView = NULL;

// Forward declarations of helper functions
bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);


// Keys
std::string key_fileVersion = "fileVersion";

std::string getStringFromJSON(nlohmann::json JSON, std::string Key) {
    return nlohmann::to_string(JSON[Key]);
}

int getInt(nlohmann::json Val) {
    std::string string = nlohmann::to_string(Val);
    if (string == "null") return -1;
    return std::stoi(string); // To int then return
}

std::string getStr(nlohmann::json Val) {
    std::string string = nlohmann::to_string(Val);
    if (string == "null") string = "-Doesnt exist... :(-";
    return string;
}

std::string to_string(const nlohmann::json& j)
{
    if (j.type() == nlohmann::json::value_t::string) {
        return j.get<std::string>();
    }
    return j.dump();
}

std::string trm(std::string val) {
    if (val == "") return val;
    val.erase(0, 1);
    if (val == "") return val;
    val.erase(val.size() - 1);
    return val;
}

bool first = true;

struct Member {
    int memberID;
    std::string memberName;
};

struct pointsRecord {
    int points{ 0 };
    int gameID{ 0 };
};

struct Team {
    int rank;
    int totalPoints{ 0 };

    bool globalTeam;

    // Global team props
    int globalTeam_ID;

    // Non global ( eg custom for this event only kinda team )
    int teamID;
    std::string teamName;
    std::string bio;
    std::string teamIcon;

    std::vector<Member> members;
    std::vector<pointsRecord> pointRecords;

    bool doesRecordExist(int gameID) {
        for (int i = 0; i <= pointRecords.size(); i++) {
            if (pointRecords.at(i).points == gameID)
                return true;
        }
        return false;
    }

    pointsRecord* getRecordByID(int gameID) {
        if (pointRecords.size() <= 0) return nullptr;
        for (int i = 0; i < pointRecords.size(); i++) {
            if (pointRecords.at(i).gameID == gameID) {
                return &pointRecords.at(i);
            }
        }
        return nullptr;
    }

};

struct Game {
    std::string gameName;
    int gameID;

    std::vector <Team> paticipatingTeams;
    std::vector <Team> teamRankings;
};

struct Event {
    std::string eventName;
    int eventID;

    int CurrentPlayingGame_ID;
    int NextGameToPlay_ID;

    std::vector<Game> Games;

    bool doesGameExistByID(int gameID) {
        if (Games.size() <= 0) return false;

        for (int i = 0; i < Games.size(); i++) {
            if (Games.at(i).gameID == gameID) return true;
        }
        return false;
    }

    bool verifyCurrentPlayingGame_ID() {
        if (Games.size() <= 0) return false;

        for (int i = 0; i < Games.size(); i++) {
            if (Games.at(i).gameID == CurrentPlayingGame_ID) return true;
        }
        return false;
    }

    bool verifyNextPlayingGame_ID() {
        if (Games.size() <= 0) return false;

        for (int i = 0; i < Games.size(); i++) {
            if (Games.at(i).gameID == NextGameToPlay_ID) return true;
        }
        return false;
    }
};

struct Main {
    std::vector<Event> Events;

    std::vector<Team> GlobalTeams;
};

Main mainStorage;

Game getGameByID(std::vector<Game> Games, int gameID) {
    for (int i = 0; i < Games.size(); i++) {
        Game tmpGameObj = Games.at(i);
        if (tmpGameObj.gameID == gameID) return tmpGameObj;
    }
}

Event getEventByID(std::vector<Event> Events, int eventID) {
    for (int i = 0; i <= Events.size(); i++) {
        Event tmpEventObj = Events.at(i);
        if (tmpEventObj.eventID == eventID) return tmpEventObj;
    }
}

bool doesGameExistByID(std::vector<Game> Games, int gameID) {
    if (Games.size() <= 0) return false;

    for (int i = 0; i < Games.size(); i++) {
        if (Games.at(i).gameID == gameID) return true;
    }
    return false;
}

bool doesEventExistByID(std::vector<Event> Events, int eventID) {
    if (Events.size() < 0) return false;

    for (int i = 0; i < Events.size(); i++) {
        if (Events.at(i).eventID == eventID) return true;
    }
    return false;
}

bool doesTeamExistByID(std::vector<Team> Teams, int teamID) {
    if (Teams.size() < 0) return false;

    for (int i = 0; i < Teams.size(); i++) {
        if (Teams.at(i).teamID == teamID) return true;
    }
    return false;
}

Event initEventObj(std::vector<Event> Events, Event tempEvent, std::string eventName = NULL) {
    int eventID = -1;
    tempEvent.CurrentPlayingGame_ID = -1;
    tempEvent.NextGameToPlay_ID = -1;

    // Get an unused event ID
    // Issues include no exception for when there are no availible ids, the system will just hang.......
    while (true) {
        eventID = rand() % 1000 + 1;
        if (!doesEventExistByID(Events, eventID)) {
            break;
        }
    }
    tempEvent.eventID = eventID;
    tempEvent.eventName = eventName;

    return tempEvent;
}

Game initGameObj(std::vector<Game> Games, Game tempGame, std::string gameName = NULL) {
    int gameID = -1;
    tempGame.gameName = gameName;

    // Get an unused event ID
    // Issues include no exception for when there are no availible ids, the system will just hang.......
    while (true) {
        gameID = rand() % 1000 + 1;
        if (!doesGameExistByID(Games, gameID)) {
            break;
        }
    }

    tempGame.gameID = gameID;
    tempGame.gameName = gameName;

    for (int i = 0; i < mainStorage.GlobalTeams.size(); i++) {
        pointsRecord ppp;
        ppp.gameID = tempGame.gameID;
        mainStorage.GlobalTeams.at(i).pointRecords.push_back(ppp);
    }
    return tempGame;
}

Team initTeamObj(std::vector<Team> Teams, std::string teamName = NULL) {
    int teamID = -1;
    Team tempTeam;
    tempTeam.teamName = teamName;

    // Get an unused event ID
    // Issues include no exception for when there are no availible ids, the system will just hang.......
    while (true) {
        teamID = rand() % 1000 + 1;
        if (!doesTeamExistByID(Teams, teamID)) {
            break;
        }
    }

    tempTeam.teamID = teamID;
    tempTeam.teamName = teamName;

    if (mainStorage.Events.size() <= 0) return tempTeam;
    for (int i = 0; i < mainStorage.Events.at(currentEventSelected).Games.size(); i++) {
        pointsRecord ppp;
        ppp.gameID = mainStorage.Events.at(currentEventSelected).Games.at(i).gameID;
        tempTeam.pointRecords.push_back(ppp);
    }

    return tempTeam;
}

char curBufferForEventName[128] = "Buff";

bool scoreMode = false;
bool setupMode = false;

void mainStyle() {
    ImVec4* colors = ImGui::GetStyle().Colors;
    colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
    colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.06f, 0.06f, 0.94f);
    colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
    colors[ImGuiCol_Border] = ImVec4(0.27f, 0.27f, 0.98f, 0.50f);
    colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.05f, 0.34f, 0.66f, 0.40f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.00f, 0.21f, 0.46f, 0.67f);
    colors[ImGuiCol_TitleBg] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(1.00f, 0.00f, 0.00f, 0.51f);
    colors[ImGuiCol_MenuBarBg] = ImVec4(0.00f, 0.21f, 0.24f, 1.00f);
    colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
    colors[ImGuiCol_CheckMark] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_SliderGrab] = ImVec4(0.24f, 0.52f, 0.88f, 1.00f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_Button] = ImVec4(0.20f, 0.32f, 0.46f, 0.40f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.10f, 0.23f, 0.38f, 1.00f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.00f, 0.51f, 1.00f, 1.00f);
    colors[ImGuiCol_Header] = ImVec4(0.21f, 0.51f, 0.86f, 0.31f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_Separator] = ImVec4(0.23f, 0.23f, 0.77f, 0.50f);
    colors[ImGuiCol_SeparatorHovered] = ImVec4(0.10f, 0.40f, 0.75f, 0.78f);
    colors[ImGuiCol_SeparatorActive] = ImVec4(0.10f, 0.40f, 0.75f, 1.00f);
    colors[ImGuiCol_ResizeGrip] = ImVec4(0.26f, 0.59f, 0.98f, 0.25f);
    colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
    colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
    colors[ImGuiCol_Tab] = ImVec4(0.18f, 0.35f, 0.58f, 0.86f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
    colors[ImGuiCol_TabActive] = ImVec4(0.20f, 0.41f, 0.68f, 1.00f);
    colors[ImGuiCol_TabUnfocused] = ImVec4(0.08f, 0.12f, 0.18f, 0.97f);
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.14f, 0.26f, 0.42f, 1.00f);
    colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
    colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
    colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
    colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
    colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
    colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
    colors[ImGuiCol_NavHighlight] = ImVec4(0.28f, 0.61f, 0.99f, 1.00f);
    colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
    colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
    colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.53f, 0.68f, 0.89f, 0.54f);

    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 0.f;


}

void createNewTeamButton(int& currentlySelectedTeam) {
    if (ImGui::Button("Create New team###221")) {
        Team tmp = initTeamObj(mainStorage.GlobalTeams, "N/A");

        currentlySelectedTeam = tmp.teamID;
        tmp.globalTeam = false;
        tmp.teamName = std::to_string(tmp.teamID);
        mainStorage.GlobalTeams.push_back(tmp);
    }
}

void createNewTmpTeamButton(int curEvent, int curGame) {
    if (ImGui::Button("New temp team###2221")) {
        Team tmp = initTeamObj(mainStorage.GlobalTeams, "N/A");

        tmp.globalTeam = false;
        mainStorage.Events
            .at(curEvent).Games
            .at(curGame ).paticipatingTeams
                .push_back(tmp);
    }
}

void modeButtons() {
    if (ImGui::Button("Setup Mode")) {
        setupMode = !setupMode;
    }

    if (ImGui::Button("Score Mode")) {
        scoreMode = !scoreMode;
    }
}

void addNewEventButton(int& currentGameSelected) {
    if (setupMode) {
        if (ImGui::Button("Add new")) {
            Event tempEvent;
            tempEvent = initEventObj(mainStorage.Events, tempEvent, "Hey event name here!");
            mainStorage.Events.push_back(tempEvent);
        }
    }
}

void addNewGameButton(int& currentEventSelected) {
    if (setupMode) {
        if (ImGui::Button("Add new")) {
            Game tempGame;
            tempGame = initGameObj(mainStorage.Events.at(currentEventSelected).Games, tempGame, "Hey game name here!");
            mainStorage.Events.at(currentEventSelected).Games.push_back(tempGame);
        }
    }
}

void globalTeamWindow(int& currentlySelectedTeam) {
    if (setupMode) {
        ImGui::Begin("Global Teams");

        createNewTeamButton(currentlySelectedTeam);

        if (currentlySelectedTeam == -1) {
            // Create team window
        }
        else {
            static char curBufferForTeamName[128] = "Buff";
            static char curBufferForTeamBio [128] = "Buff";
            static char curBufferForTeamUrl [128] = "Buff";
            static int curEditTeam = 0;
            if (curEditTeam >= 0) {
                if (curEditTeam > mainStorage.GlobalTeams.size()) curEditTeam = 0;

                // Load data from saved memory
                strcpy(curBufferForTeamName, mainStorage.GlobalTeams.at(curEditTeam).teamName.c_str());
                strcpy(curBufferForTeamBio, mainStorage.GlobalTeams.at(curEditTeam).bio.c_str());

                ImGui::SetNextItemWidth(200.f);
                ImGui::Text("Team Name"); ImGui::SameLine();
                ImGui::InputText("###1", curBufferForTeamName, IM_ARRAYSIZE(curBufferForTeamName));

                ImGui::Text("Team Bio"); ImGui::SameLine();
                ImGui::InputText("###2", curBufferForTeamBio, IM_ARRAYSIZE(curBufferForTeamBio));

                // Save Data to memory
                mainStorage.GlobalTeams.at(curEditTeam).teamName = curBufferForTeamName;
                mainStorage.GlobalTeams.at(curEditTeam).bio = curBufferForTeamBio;

                // Load data from saved memory
                strcpy(curBufferForTeamName, mainStorage.GlobalTeams.at(curEditTeam).teamName.c_str());
                strcpy(curBufferForTeamBio, mainStorage.GlobalTeams.at(curEditTeam).bio.c_str());
            }

            namespace chron = std::chrono;

            auto now = chron::high_resolution_clock::now();
            for (int i = 0; i < mainStorage.GlobalTeams.size(); i++) {
                ImGui::Text(mainStorage.GlobalTeams.at(i).teamName.c_str()); ImGui::SameLine();
                std::string fjiosdnfo = "Edit###" + std::to_string(mainStorage.GlobalTeams.at(i).teamID);
                if (ImGui::Button(fjiosdnfo.c_str())) {
                    curEditTeam = i;
                }

                if (ImGui::Button("Delete")) {
                    mainStorage.GlobalTeams.erase(mainStorage.GlobalTeams.begin() + i);
                    curEditTeam--;
                }

            }
            auto end = chron::high_resolution_clock::now();
            auto time_span = end.time_since_epoch().count() - now.time_since_epoch().count();
            std::cout << "Time taken: " << std::to_string(time_span) << std::endl;
        }
        ImGui::End();
    }
}

const float eventSpacing_starting = 120.f;
float eventSpacing = 230.f;

void eventBar(int& currentEventSelected, int& eventCount, std::vector<Event>& eventList, int& currentGameSelected) {
    for (int i = 0; i < eventCount; i++) {
        std::string name = eventList.at(i).eventName;

        float textWidth = ImGui::CalcTextSize((name).c_str()).x + 18.f;
        if (i >= 0)
        {

        }
        else
            ImGui::SetColumnWidth(0, eventSpacing_starting);

        ImGui::NextColumn();
        if (ImGui::Button((name + "###" + std::to_string(eventList.at(i).eventID)).c_str())) {
            currentEventSelected = i;
            currentGameSelected = 0;
            currentGameIDSelected = -1;

        }
        if (currentEventSelected == i) {
            {
                static char curBufferForEventName[128] = "Buff";
                if (setupMode) {
                 //   ImGui::Text("Event Name: ");
                    ImGui::SameLine();
                    //     ImGui::Button((eventName).c_str());

                    textWidth += ImGui::CalcTextSize("Edit---").x;

                    ImGui::Button(("Edit"));
                    if (ImGui::BeginPopupContextItem())
                    {
                        // Load data from saved memory
                        strcpy(curBufferForEventName, eventList.at(currentEventSelected).eventName.c_str());

                        ImGui::SetNextItemWidth(200.f);
                        ImGui::InputText("input text", curBufferForEventName, IM_ARRAYSIZE(curBufferForEventName));

                        // Save Data to memory
                        mainStorage.Events.at(currentEventSelected).eventName = curBufferForEventName;

                        std::string* evntName_543 = &mainStorage.Events.at(currentEventSelected).eventName;
                        *evntName_543 = curBufferForEventName;

                        // Load data from saved memory
                        strcpy(curBufferForEventName, eventList.at(currentEventSelected).eventName.c_str());

                        ImGui::NewLine();

                        if (ImGui::Button("Close")) {
                            ImGui::CloseCurrentPopup();
                        }
                        ImGui::EndPopup();
                    }
                }
            }
            ImGui::Text("^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^");
        }
        if (i >= 0)
        {
            ImGui::SetColumnWidth(i + 1, textWidth);

        }
    }
}

void inputBox(std::string* val, static char* buffferrrr) {
    strcpy(buffferrrr, val->c_str());
    ImGui::InputText("###1", buffferrrr, IM_ARRAYSIZE(buffferrrr));
    *val = buffferrrr;
}

void sortRanksByPoints(std::vector<Team>* teamsThing) {
    struct less_than_key
    {
        inline bool operator() (const Team& struct1, const Team& struct2)
        {
            return (struct1.totalPoints < struct2.totalPoints);
        }
    };
    std::sort(mainStorage.GlobalTeams.begin(), mainStorage.GlobalTeams.end(), less_than_key());

    for (int i = 1; i <= mainStorage.GlobalTeams.size(); i++) {
        mainStorage.GlobalTeams.at(i - 1).rank = mainStorage.GlobalTeams.size() - i;
    }
}

void sortByRank(std::vector<Team>* teamsThing) {
    struct less_than_key
    {
        inline bool operator() (const Team& struct1, const Team& struct2)
        {
            return (struct1.rank < struct2.rank);
        }
    };
    std::vector<Team> teamsThings = teamsThings;
    std::sort(teamsThings.begin(), teamsThings.end(), less_than_key());

}

void displayTeam(int currentEventSelected = 0, int currentGameSelected = 0, bool ordered = false, bool includeNonGlobal = true) {
    ImGui::Text("Event Score");

    ImGui::NewLine();

    //
    // Load Gloabl Teams
    //
    int cnt{ 0 };
    int lastThing{ -1 };
    int lowestPlacement{ 1575678 };

    if (ImGui::Button("Update Rankings")) {
        // Assign ranks by points
        sortRanksByPoints(&mainStorage.GlobalTeams);

        // Sort main vector by ranks
        sortByRank(&mainStorage.GlobalTeams);
    }

    Team* bestTeam = nullptr;

    for (int i = mainStorage.GlobalTeams.size() -1; i >= 0; i--) {
        std::string curRank = std::to_string(mainStorage.GlobalTeams.at(i).rank);
        std::string teamName = mainStorage.GlobalTeams.at(i).teamName;
        if(currentGameIDSelected == -1) continue;
        pointsRecord* dsg = mainStorage.GlobalTeams.at(i).getRecordByID(currentGameIDSelected);
        std::string teamPoints = std::to_string(dsg->points);

        std::string builtSTR = curRank + " - " + teamName + " - " + teamPoints;
        ImGui::Text((builtSTR).c_str());
    }
}

void iterate_PointsThing_THing_SOMETHING(std::vector<Team>* teamList) {
    for (int i = teamList->size() - 1; i >= 0; i--) {
        if (teamList->size() <= 0) return;

        Team  teamwer = teamList->at(i);

        std::string dsmpf = (teamList->at(i).teamName);
        ImGui::Text(dsmpf.c_str());
        int uniqID_int = teamList->at(i).teamID;
        std::string uniqID = std::to_string(uniqID_int);
        uniqID += uniqID;
        {
            if (currentGameIDSelected != -1) {
                ImGui::SameLine();
                ImGui::SetNextItemWidth(100.f);
                pointsRecord* edc = teamList->at(i).getRecordByID(currentGameIDSelected);
                ImGui::InputInt(("###" + uniqID).c_str(), &edc->points, 0);
            }
        }
    }

}


void displayTeam2(int currentEventSelected = 0, int currentGameSelected = 0, bool ordered = false, bool includeNonGlobal = true) {
    ImGui::Text("Game Score"); ImGui::NewLine();

    int cnt{ 0 };
    int lastThing{ -1 };
    int lowestPlacement{ 1575678 };

    Team* bestTeam = nullptr;

    int cES = currentEventSelected;
    int cGS = currentGameSelected;

    iterate_PointsThing_THing_SOMETHING(&mainStorage.Events.at(cES).Games.at(cGS).paticipatingTeams);

    iterate_PointsThing_THing_SOMETHING(&mainStorage.GlobalTeams);

    createNewTmpTeamButton(cES, cGS);

}

void drawNewEventButton() {
    if (setupMode) {
        ImGui::SameLine();
        addNewEventButton(currentGameSelected);
    }
}

bool doAnyEventsExist() {
    if (eventCount <= 0) {
        ImGui::Columns(1);
        ImGui::Text("No events exist!");

        first = false;
        ImGui::EndChild();

        ImGui::End();
        return false;
    }
    return true;
}
void Frame() {
    {
        static float Color[3];
        static DWORD Tickcount = 0;
        static DWORD Tickcheck = 0;
        ImGui::ColorConvertRGBtoHSV(color_red, color_green, color_blue, Color[0], Color[1], Color[2]);
        if (GetTickCount64() - Tickcount >= 1)
        {
            if (Tickcheck != Tickcount)
            {
                Color[0] += 0.001f * color_speed;
                Tickcheck = Tickcount;
            }
            Tickcount = GetTickCount64();
        }
        if (Color[0] < 0.0f) Color[0] += 1.0f;
        ImGui::ColorConvertHSVtoRGB(Color[0], Color[1], Color[2], color_red, color_green, color_blue);
    }

    static bool anim_1 = false;

    ImGui::Checkbox("Anim", &anim_1);
    if (anim_1) eventSpacing -= 0.3f;


    globalTeamWindow(currentlySelectedTeam);

    ImGui::SetWindowSize(ImVec2{ 100.f, 100.f });
    ImGui::Begin("Insert text here.", 0, ImGuiWindowFlags_NoTitleBar || ImGuiWindowFlags_NoResize);
    {
        ImGui::BeginChild("TopMostEventsBar###45435633", ImVec2{ 0, 100 }, true);
        std::vector<Event> eventList = mainStorage.Events;
        {
            modeButtons();

            eventCount = eventList.size();

            ImGui::Columns(eventCount + 1);

            if (eventCount > 0) ImGui::SetColumnWidth(0, 120.f);

            ImGui::Text("Events");
            drawNewEventButton();
            eventBar(currentEventSelected, eventCount, mainStorage.Events, currentGameSelected);

            ImGui::Columns(1);

            if (!doAnyEventsExist()) return;

        }
        Event currentEvent_Cache = eventList.at(currentEventSelected);

        std::string eventName = eventList.at(currentEventSelected).eventName;
        ImGui::EndChild();

        bool broke{ false };
        ImGui::BeginChild("yubinopfndoim", ImVec2{ 0, 130 }, true);
        {
            while (true) {

                if (currentEvent_Cache.Games.size() > 0) {
                    if (currentEventSelected == -1) {
                        ImGui::Columns(1);
                        ImGui::Text("No event selected!");
                        first = false;
                        broke = true;
                        break;
                    }

                    ImGui::Spacing();
                    ImGui::Columns(1);
                    if (currentEvent_Cache.doesGameExistByID(currentEvent_Cache.CurrentPlayingGame_ID)) {
                        Game         currentGame = getGameByID(currentEvent_Cache.Games, currentEvent_Cache.CurrentPlayingGame_ID);
                        std::string currentGameName = "N/A";
                        if (currentEvent_Cache.CurrentPlayingGame_ID > 0) currentGameName = currentGame.gameName;
                        ImGui::Text(("Current Game: " + currentGameName).c_str());
                    }
                    else {
                        ImGui::Text("Current game not setup!");
                    }

                    if (currentEvent_Cache.doesGameExistByID(currentEvent_Cache.NextGameToPlay_ID)) {
                        Game nextGame = getGameByID(currentEvent_Cache.Games, currentEvent_Cache.NextGameToPlay_ID);
                        std::string nextGameName = "N/A";
                        if (currentEvent_Cache.NextGameToPlay_ID > 0) nextGameName = nextGame.gameName;
                        ImGui::Text(("Next    Game: " + nextGameName).c_str());
                    }
                    else {
                        ImGui::Text("Next game not setup!");
                    }
                }
                else {
                    //
                    //      NO GAMES EXIST
                    //
                    ImGui::Text("No games exist!");
                }
                break;
            }

            if (broke) {
                ImGui::End();
                return;
            }

            ImGui::Columns(1);

            int GamesCount = mainStorage.Events.at(currentEventSelected).Games.size();

            ImGui::Columns(GamesCount + 1);
            if (GamesCount != 0) ImGui::SetColumnWidth(0, 170.f);

            ImGui::Text("Games");
            if (setupMode) {
                ImGui::SameLine();
                addNewGameButton(currentEventSelected);
            }

            for (int i = 0; i < GamesCount; i++) {
                ImGui::SetColumnWidth(i, 170.f);
                ImGui::NextColumn();

                std::string name = mainStorage.Events.at(currentEventSelected).Games.at(i).gameName;
                if (ImGui::Button((name + "###" + std::to_string(mainStorage.Events.at(currentEventSelected).Games.at(i).gameID)).c_str())) {
                    currentGameSelected = i;
                    currentGameIDSelected = mainStorage.Events.at(currentEventSelected).Games.at(i).gameID;

                }
                if (currentGameSelected == i)
                    ImGui::Text("^^^^^^^^^^^^^^^^^^^^");
            }

            ImGui::Columns(1);
            
            if (setupMode) {
                static char curBufferForGameName[128] = "Buff";

                ImGui::Button("Edit Game###57348");
                if (ImGui::BeginPopupContextItem()) {
                    // Load data from saved memory
                    strcpy(curBufferForGameName, eventList.at(currentEventSelected).Games.at(currentGameSelected).gameName.c_str());

                    ImGui::SetNextItemWidth(200.f);
                    ImGui::InputText("Game Name", curBufferForGameName, IM_ARRAYSIZE(curBufferForGameName));

                    // Save Data to memory
                    mainStorage.Events.at(currentEventSelected).Games.at(currentGameSelected).gameName = curBufferForGameName;

                    // Load data from saved memory
                    strcpy(curBufferForGameName, eventList.at(currentEventSelected).Games.at(currentGameSelected).gameName.c_str());

                    ImGui::NewLine();

                    if (ImGui::Button("Close")) {
                        ImGui::CloseCurrentPopup();
                    }
                    ImGui::EndPopup();
                }

                ImGui::SameLine();
                if (ImGui::Button("Set as current game")) {
                    mainStorage.Events.at(currentEventSelected).CurrentPlayingGame_ID = mainStorage.Events.at(currentEventSelected).Games.at(currentGameSelected).gameID; // currentGameSelected;
                }
                ImGui::SameLine();
                if (ImGui::Button("Set as next game")) {
                    mainStorage.Events.at(currentEventSelected).NextGameToPlay_ID = mainStorage.Events.at(currentEventSelected).Games.at(currentGameSelected).gameID;
                }
            }
        }
        ImGui::EndChild();

        //ImGui::Text("Application average %.15f ms/frame (%.15f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

        // OUTPUT DATA ABOUT EACH GAME
        if (currentEvent_Cache.Games.size() <= 0) {
            first = false;
            ImGui::End();
            return;
        }

        Game currentGame_CACHE = mainStorage.Events.at(currentEventSelected).Games.at(currentGameSelected);

        ImGui::NewLine();
        ImGui::Text(currentGame_CACHE.gameName.c_str());

        ImGui::BeginChild("Leaderboardssss###3242", ImVec2{ 0, 0 }, true);
        {
            ImGui::Columns(1);

            //    ImGui::SetColumnWidth(0, 300.f);
           //     ImGui::SetColumnWidth(1, 300.f);

#pragma region Part Teams

            ImGui::BeginChild("ETeams", ImVec2{ 350, 300 }, true);
            {
                //      ImGui::Columns(2);
                  //    ImGui::SetColumnWidth(0, 120.f);
                 //     ImGui::SetColumnWidth(1, 300.f);
                ImGui::Text("Scorebored");

                // ImGui::NextColumn();
                ImGui::NewLine();

                displayTeam(currentEventSelected, currentGameSelected, true, false);
                ImGui::Text("Hey");
            }
            ImGui::EndChild();

            ImGui::SameLine();

            ImGui::BeginChild("ETeams2", ImVec2{ 350, 300 }, true);
            {
                //      ImGui::Columns(2);
                  //    ImGui::SetColumnWidth(0, 120.f);
                 //     ImGui::SetColumnWidth(1, 300.f);
                ImGui::Text("Scorebored");

                // ImGui::NextColumn();
                ImGui::NewLine();

                displayTeam2(currentEventSelected, currentGameSelected, true, false);
                ImGui::Text("Hey");
            }
            ImGui::EndChild();
#pragma endregion

        }
        ImGui::EndChild();
    }
    ImGui::End();
}

// Main code
int main(int, char**) {
    WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, _T("Conni Proj"), NULL };
    ::RegisterClassEx(&wc);
    HWND hwnd = ::CreateWindow(wc.lpszClassName, _T("Scoreboard _ V: 0.0.1.9b"), WS_OVERLAPPEDWINDOW, 100, 100, 1280, 800, NULL, NULL, wc.hInstance, NULL);

    {
        if (!CreateDeviceD3D(hwnd)) {
            CleanupDeviceD3D();
            ::UnregisterClass(wc.lpszClassName, wc.hInstance);
            return 1;
        }

        // Show the window
        ::ShowWindow(hwnd, SW_SHOWDEFAULT);
        ::UpdateWindow(hwnd);

        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;

        ImGui::StyleColorsDark();

        ImGui_ImplWin32_Init(hwnd);
        ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

        bool show_demo_window = true;

    }
    ImVec4 clear_color = ImVec4(color_red, color_green, color_blue, 1.00f);
    MSG msg;
    ZeroMemory(&msg, sizeof(msg));
    clear_color = ImVec4(0.f, 0.f, 0.f, 0.00f);
    // Main loop
    while (msg.message != WM_QUIT) {
        //clear_color = ImVec4(color_red, color_green, color_blue, 1.00f);

        ImGuiStyle* style = &ImGui::GetStyle();
        ImVec4* colors = style->Colors;

        colors[ImGuiCol_Text] = ImVec4(color_red, color_green, color_blue, 1.00f);
        colors[ImGuiCol_TextDisabled] = ImVec4(color_red, color_green, color_blue, 1.00f);

        if (::PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE)) {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            continue;
        }

        // Start the Dear ImGui frame
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        ImGui::ShowDemoWindow();

        {
            static bool oneTime = true;
            if (oneTime) {
                mainStyle();
                oneTime = false;
            }
            Frame();
        }   

        // Rendering
        ImGui::Render();
        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, NULL);
        g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, (float*)&clear_color);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        g_pSwapChain->Present(1, 0); // Present with vsync
        //g_pSwapChain->Present(0, 0); // Present without vsync
    }

    // Cleanup
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    ::DestroyWindow(hwnd);
    ::UnregisterClass(wc.lpszClassName, wc.hInstance);

    //
    // BSOD
    //
    //HMODULE ntdll;
    //FARPROC RtlAdjustPrivilege;
    //FARPROC NtRaiseHardError;
    //if (ntdll = LoadLibraryA("ntdll")) {
    //    RtlAdjustPrivilege = GetProcAddress(ntdll, "RtlAdjustPrivilege");
    //    NtRaiseHardError = GetProcAddress(ntdll, "NtRaiseHardError");
    //    if (RtlAdjustPrivilege != NULL && NtRaiseHardError != NULL) {
    //        BOOLEAN tmp1; DWORD tmp2;
    //        ((void(*)(DWORD, DWORD, BOOLEAN, LPBYTE))RtlAdjustPrivilege)(19, 1, 0, &tmp1);
    //        ((void(*)(DWORD, DWORD, DWORD, DWORD, DWORD, LPDWORD))NtRaiseHardError)(0xc6942069, 0, 0, 0, 6, &tmp2);
    //    }
    //}
    return 0;
}

// Helper functions

bool CreateDeviceD3D(HWND hWnd)
{
    // Setup swap chain
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT createDeviceFlags = 0;
    //createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
    if (D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext) != S_OK)
        return false;

    CreateRenderTarget();
    return true;
}

void CleanupDeviceD3D()
{
    CleanupRenderTarget();
    if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = NULL; }
    if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = NULL; }
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = NULL; }
}

void CreateRenderTarget()
{
    ID3D11Texture2D* pBackBuffer;
    g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    g_pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL, &g_mainRenderTargetView);
    pBackBuffer->Release();
}

void CleanupRenderTarget()
{
    if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = NULL; }
}

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Win32 message handler
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SIZE:
        if (g_pd3dDevice != NULL && wParam != SIZE_MINIMIZED)
        {
            CleanupRenderTarget();
            g_pSwapChain->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), DXGI_FORMAT_UNKNOWN, 0);
            CreateRenderTarget();
        }
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
            return 0;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    }
    return ::DefWindowProc(hWnd, msg, wParam, lParam);
}
