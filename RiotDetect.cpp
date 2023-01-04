// RiotDetect.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <Windows.h>
#include <wtsapi32.h>

void FindAndCloseRiotGamesProcesses();

int main()
{
    while (1)
    {
        FindAndCloseRiotGamesProcesses();
        Sleep(1000);
    }
    return 0;
}

void FindAndCloseRiotGamesProcesses()
{
    WTS_PROCESS_INFO* pWPIs = NULL;
    DWORD dwProcCount = 0;
    const wchar_t* leagueClientName = L"LeagueClient.exe";
    const wchar_t* leagueGameName = L"League of Legends.exe";

    bool terminate = false;
    if (WTSEnumerateProcesses(WTS_CURRENT_SERVER_HANDLE, 0, 1, &pWPIs, &dwProcCount))
    {
        //Go through all processes retrieved
        for (DWORD i = 0; i < dwProcCount; i++)
        {
            if (0 == wcscmp(pWPIs[i].pProcessName, leagueClientName))
            {
                terminate = true;
                std::cout << "League of Legends Detected Sending It To the Shadow Realm\n";
            }

            if (0 == wcscmp(pWPIs[i].pProcessName, leagueGameName))
            {
                terminate = true;
                std::cout << "League of Legends Detected Sending It To the Shadow Realm\n";
            }

            if (terminate)
            {
                HANDLE h = OpenProcess(PROCESS_TERMINATE, false, pWPIs[i].ProcessId);
                TerminateProcess(h, 0);
                CloseHandle(h);
            }
        }
    }

    //Free memory
    if (pWPIs)
    {
        WTSFreeMemory(pWPIs);
        pWPIs = NULL;
    }
}