/*!
 * \file    export.h
 * \brief   Some helper macros and functions for exporting functions as a dynamic link library.
 *
 *          This library is expected to be loaded with JavaScript, so the functions must be exported
 *          with C-style interface.
 * \author  Sun Ziping
 * \date    2017/1/11
 */

#ifndef GRAPHVISUALIZATION_EXPORT_H
#define GRAPHVISUALIZATION_EXPORT_H

#include <string>
#include <cstring>

/*!
 * \def    DllExport
 * \brief  Use before the definition of the function in `export.cpp` to make it visible outside.
 */

#if defined(_WIN32)
#define DllExport   __declspec(dllexport) extern "C"
#else
#define DllExport   extern "C"
#endif

std::string str_escape(const std::string &str) {
    std::string result;
    result.reserve(str.size());
    for (typename std::string::const_iterator iter = str.cbegin(); iter != str.cend(); ++iter) {
        if (*iter == '\t')
            result += "\\t";
        else if (*iter == '\n')
            result += "\\n";
        else if (*iter == '\\')
            result += "\\\\";
        else if (*iter == '\"')
            result += "\\\"";
        else
            result.push_back(*iter);
    }
    return result;
}

#endif //GRAPHVISUALIZATION_EXPORT_H
