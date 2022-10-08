// SepProgram.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <windows.h>
#include <stdio.h> 
#include <StaticLib.h>
extern "C" __declspec(dllimport) void maxOccs();
#pragma comment(lib, "DllDynamic.lib")
typedef int(__cdecl* MYFUNC)();
void dynamic() {
    std::cout << "Hey, sir, I'm running myCode function from .dll file dynamically!" << std::endl;
    HINSTANCE hinstLib;
    BOOL fFreeResult, fRunTimeLinkSuccess = FALSE;
    MYFUNC functionAddress;
    // Get a handle to the DLL module.

    hinstLib = LoadLibrary(TEXT("DllDynamic.dll"));

    // If the handle is valid, try to get the function address.

    if (hinstLib != NULL)
    {
        functionAddress = (MYFUNC)GetProcAddress(hinstLib, "myCode");

        // If the function address is valid, call the function.

        if (NULL != functionAddress)
        {
            fRunTimeLinkSuccess = TRUE;
            (functionAddress)();
        }
        // Free the DLL module.

        fFreeResult = FreeLibrary(hinstLib);
    }

    // If unable to call the DLL function, use an alternative.
    if (!fRunTimeLinkSuccess)
        std::cout << ("Didn't link it :(") << std::endl;;



}
void staticLinking() {
    std::cout << "Hey, sir, I'm running maxOccs function from .lib file statically!" << std::endl;
    fnStaticLib1();
}
int main()
{
    dynamic();
    staticLinking(); 
}

