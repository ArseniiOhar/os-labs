#include <windows.h>
#include <iostream>

#define EOF (-1)

#ifdef __cplusplus    // If used by C++ code, 
extern "C" {          // we need to export the C interface
#endif
#pragma comment(linker, "/export:staticHelloWorld=helloWorldStatic")
    __declspec(dllexport) void CALLBACK helloWorldStatic()
    {

        std::cout << "Hello World I'M STATICALLY LINKED LIBRARY RIGHT HERE" << std::endl;
    }

#ifdef __cplusplus
}
#endif