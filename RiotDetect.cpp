// RiotDetect.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <filesystem>
#include <Windows.h>
#include <wincrypt.h>
#include <wintrust.h>
#include <wtsapi32.h>

void FindAndCloseRiotGamesProcesses(); // For actively running Games
void FindAndDeleteRiotGamesFiles(); // For files in your directory
std::string GetLastErrorAsString(); // Prints prev windows error message

int main()
{
    // This only runs a single time at start up - its more expensive than the process one
    FindAndDeleteRiotGamesFiles();

    while (1)
    {
        FindAndCloseRiotGamesProcesses();
        Sleep(1000);
    }
    return 0;
}

void FindAndDeleteRiotGamesFiles()
{
    static uint64_t cnt = 0;
    volatile bool success = false;
    DWORD dwEncoding, dwContentType, dwFormatType, dwSignerInfo;
    HCERTSTORE hStore = NULL;
    HCRYPTMSG hMsg = NULL;
    WCHAR szFileName[MAX_PATH];
    volatile PCMSG_SIGNER_INFO pSignerInfo = NULL;
    PCMSG_SIGNER_INFO pCounterSignerInfo = NULL;

    try
    {
        for (auto const& p : std::filesystem::recursive_directory_iterator("C:\\", std::filesystem::directory_options::skip_permission_denied))
        {
            if (p.path().extension() == ".exe")
            {
                lstrcpynW(szFileName, p.path().c_str(), MAX_PATH);
                // Get info about the file
                success = CryptQueryObject(CERT_QUERY_OBJECT_FILE,
                    szFileName,
                    CERT_QUERY_CONTENT_FLAG_ALL,
                    CERT_QUERY_FORMAT_FLAG_ALL,
                    0,
                    &dwEncoding,
                    &dwContentType,
                    &dwFormatType,
                    &hStore,
                    &hMsg,
                    NULL);
                if (!success)
                {
                    std::cerr << GetLastErrorAsString();
                    continue;
                }
                
                success = CryptMsgGetParam(hMsg,
                    CMSG_SIGNER_INFO_PARAM,
                    0,
                    NULL,
                    &dwSignerInfo);
                if (!success)
                {
                    std::cerr << GetLastErrorAsString();
                    continue;
                }

                pSignerInfo = (PCMSG_SIGNER_INFO)LocalAlloc(LPTR, dwSignerInfo);
                if (!pSignerInfo)
                {
                    std::cerr << "Unable to allocate memory for signer information!\n";
                    continue;
                }

                // Get Signer Information.
                success = CryptMsgGetParam(hMsg,
                    CMSG_SIGNER_INFO_PARAM,
                    0,
                    (PVOID)pSignerInfo,
                    &dwSignerInfo);
                if (!success)
                {
                    std::cerr << GetLastErrorAsString();
                    continue;
                }
            }
            
            // Sleep every now and again to be nice to the OS
            cnt++;
            if (0 == (cnt % 1000))
            {
                Sleep(100);
            }
        }
    }
    catch (std::filesystem::filesystem_error& e)
    {
        std::cerr << e.what() << "\n";
    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << "\n";
    }
}

void FindAndCloseRiotGamesProcesses()
{
    WTS_PROCESS_INFO* pWPIs = NULL;
    DWORD dwProcCount = 0;
    // If you have a version other than the english version then this needs to be updated
    const wchar_t* leagueClientName = L"LeagueClient.exe";
    const wchar_t* leagueGameName = L"League of Legends.exe";
    // These two are guesses I do not play these games. If someone does know the names please update me :)
    const wchar_t* ValorantClientName = L"ValorantClient.exe";
    const wchar_t* valorantGameName = L"Valorant.exe";

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

            if (0 == wcscmp(pWPIs[i].pProcessName, ValorantClientName))
            {
                terminate = true;
                std::cout << "Valorant Detected Sending It To the Shadow Realm\n";
            }

            if (0 == wcscmp(pWPIs[i].pProcessName, valorantGameName))
            {
                terminate = true;
                std::cout << "Valorant Detected Sending It To the Shadow Realm\n";
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

//Returns the last Win32 error, in string format. Returns an empty string if there is no error.
std::string GetLastErrorAsString()
{
    //Get the error message ID, if any.
    DWORD errorMessageID = ::GetLastError();
    if (errorMessageID == 0) {
        return std::string(); //No error message has been recorded
    }

    LPSTR messageBuffer = nullptr;

    //Ask Win32 to give us the string version of that message ID.
    //The parameters we pass in, tell Win32 to create the buffer that holds the message for us (because we don't yet know how long the message string will be).
    size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

    //Copy the error message into a std::string.
    std::string message(messageBuffer, size);

    //Free the Win32's string's buffer.
    LocalFree(messageBuffer);

    return message;
}