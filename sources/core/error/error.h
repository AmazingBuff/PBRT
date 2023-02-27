#pragma once

namespace pbrt
{
    #define PRINTF_FUNC

    //message severity
    void Warning(const char*, ...) PRINTF_FUNC;
    void Error(const char*, ...) PRINTF_FUNC;  
}

