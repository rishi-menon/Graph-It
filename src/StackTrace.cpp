#include <stdio.h>
#include <stdlib.h>
#include "DebugFinal.h"

#define USE_STACK_TRACE 0

#if !USE_STACK_TRACE
    void stack_trace() {

    }
#else

    #ifdef _WIN32
    #include <windows.h>
    #include <DbgHelp.h>

    void stack_trace() {
    HANDLE process = GetCurrentProcess();
    HANDLE thread = GetCurrentThread();
    
    CONTEXT context;
    memset(&context, 0, sizeof(CONTEXT));
    context.ContextFlags = CONTEXT_FULL;
    RtlCaptureContext(&context);
    
    SymInitialize(process, NULL, TRUE);
    
    DWORD image;
    STACKFRAME64 stackframe;
    ZeroMemory(&stackframe, sizeof(STACKFRAME64));
    
    #ifdef _M_IX86
    image = IMAGE_FILE_MACHINE_I386;
    stackframe.AddrPC.Offset = context.Eip;
    stackframe.AddrPC.Mode = AddrModeFlat;
    stackframe.AddrFrame.Offset = context.Ebp;
    stackframe.AddrFrame.Mode = AddrModeFlat;
    stackframe.AddrStack.Offset = context.Esp;
    stackframe.AddrStack.Mode = AddrModeFlat;
    #elif _M_X64
    image = IMAGE_FILE_MACHINE_AMD64;
    stackframe.AddrPC.Offset = context.Rip;
    stackframe.AddrPC.Mode = AddrModeFlat;
    stackframe.AddrFrame.Offset = context.Rsp;
    stackframe.AddrFrame.Mode = AddrModeFlat;
    stackframe.AddrStack.Offset = context.Rsp;
    stackframe.AddrStack.Mode = AddrModeFlat;
    #elif _M_IA64
    image = IMAGE_FILE_MACHINE_IA64;
    stackframe.AddrPC.Offset = context.StIIP;
    stackframe.AddrPC.Mode = AddrModeFlat;
    stackframe.AddrFrame.Offset = context.IntSp;
    stackframe.AddrFrame.Mode = AddrModeFlat;
    stackframe.AddrBStore.Offset = context.RsBSP;
    stackframe.AddrBStore.Mode = AddrModeFlat;
    stackframe.AddrStack.Offset = context.IntSp;
    stackframe.AddrStack.Mode = AddrModeFlat;
    #endif

        char buffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR)];
        for (size_t i = 0; i < 25; i++) {
            BOOL result = StackWalk64(
                image, process, thread,
                &stackframe, &context, NULL, 
                SymFunctionTableAccess64, SymGetModuleBase64, NULL);

            if (!result) { break; }

            PSYMBOL_INFO symbol = (PSYMBOL_INFO)buffer;
            symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
            symbol->MaxNameLen = MAX_SYM_NAME;

            DWORD64 displacement = 0;
            if (i == 0)
                continue;   //The first element will always be StackTrace() function. Printing it isnt helpful
            
            BOOL bfuncName = SymFromAddr(process, stackframe.AddrPC.Offset, &displacement, symbol);
            if (bfuncName)
            {
                const char* strFuncName = symbol->Name;
                IMAGEHLP_LINE64 lineStruct;
                lineStruct.SizeOfStruct = sizeof(IMAGEHLP_LINE64);
                DWORD disp = 0;

                BOOL bLineNum = SymGetLineFromAddr(process, stackframe.AddrPC.Offset, &disp, &lineStruct);
                const char* strFile = (bLineNum) ? lineStruct.FileName : "UnknownFile";
                int line = (bLineNum) ? lineStruct.LineNumber : 0;

                Log("[%i] %s ~ %s:%d\n", i, strFile, strFuncName, line);
            }
            else
            {
                Log("[%i] ???\n", i);
            }
            

            // if (bfuncName || bLineNum) {
            //     Log("[%i] %s\n", i, symbol->Name);
            // } else {
            //     Log("[%i] ???\n", i);
            // }
        }

        Log("\n");
        SymCleanup(process);
    }
    #else


    void stack_trace() {

    }

    #endif // _WIN32 was not defined

#endif //USE_STACK_TRACE == 1