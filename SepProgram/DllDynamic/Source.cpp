#include <windows.h>
#include <iostream>
#include "C:\Users\senya\source\repos\Lab3a\Lab3a\Source.cpp"
#define EOF (-1)
 
#ifdef __cplusplus    // If used by C++ code, 
extern "C" {          // we need to export the C interface
#endif
#pragma comment(linker, "/export:maxOccs=myCode")
__declspec(dllexport) void CALLBACK myCode()
{
    main();
}
 
#ifdef __cplusplus
}
#endif