#include <iostream>
#include <Windows.h>
#include <string>
#include "../Boring32/include/Boring32.hpp"

int main(int argc, char** args)
{
    if (argc > 0)
    {
        int writeHandle = std::stoi(args[1]);
        int readHandle = std::stoi(args[2]);
        Boring32::Async::AnonymousPipe pipe(false, 2048, L"||", (HANDLE)readHandle, (HANDLE)writeHandle);
        std::wcout << pipe.Read();
        pipe.DelimitedWrite(L"Hello from child!");
    }

    Boring32::Async::Event evt(false, true, true, false, L"TestEvent");
    evt.WaitOnEvent();
    std::wcout << L"Exiting after wait!" << std::endl;

    return 0;
}
