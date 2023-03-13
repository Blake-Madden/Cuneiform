///////////////////////////////////////////////////////////////////////////////
// Name:        main.cpp
// Author:      Blake Madden
// Copyright:   (c) 2021-2023 Blake Madden
// License:     3-Clause BSD license
// SPDX-License-Identifier: BSD-3-Clause
///////////////////////////////////////////////////////////////////////////////

#include "cpp_i18n_review.h"
#include "cxxopts/include/cxxopts.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <filesystem>
#include <numeric>

namespace fs = std::filesystem;
using namespace i18n_check;
using namespace string_util;

int main(int argc, char* argv[])
    {
    cxxopts::Options options("i18n-check",
                             "Internationalization/localization analysis system");
    options.add_options()
    ("input", "The folder to analyze", cxxopts::value<std::string>())
    ("i,ignore", "Folder(s) to ignore", cxxopts::value<std::vector<std::string>>())
    ("h,help", "Print usage");

    cxxopts::ParseResult result;
    try
        {
        options.parse_positional({"input"});

        result = options.parse(argc, argv);

        if (result.count("help"))
            {
            std::cout << options.help() << "\n";
            return 0;
            }
        }
    catch (const cxxopts::exceptions::exception& exp)
        {
        std::cout << exp.what();
        return 0;
        }

    // paths being ignored
    std::vector<std::string> excludedPaths;
    if (result.count("ignore"))
        {
        const auto& providedExcFolders = result["ignore"].as<std::vector<std::string>>();
        for (const auto& excFolder : providedExcFolders)
            {
            excludedPaths.push_back(excFolder);
            for (const auto& p :
                fs::recursive_directory_iterator(excFolder))
                {
                if (p.is_directory())
                    { excludedPaths.push_back(p.path().string()); }
                }
            }
        }

    std::wcout << L"Searching for files to analyze...\n";
    std::vector<std::string> filesToAnalyze; 
    if (result.count("input"))
        {
        const auto& inputFolder = result["input"].as<std::string>();
        if (!fs::exists(inputFolder))
            {
            std::cout << "Input path does not exist: " << inputFolder;
            return 0;
            }
        for (const auto& p :
            fs::recursive_directory_iterator(inputFolder))
            {
            case_insensitive_wstring ext{ p.path().extension().c_str() };
            bool inExcludedPath{ false };
            for (const auto& ePath : excludedPaths)
                {
                fs::path excPath(ePath, fs::path::native_format);
                if (p.exists() && fs::exists(excPath) &&
                    fs::equivalent(p.path().parent_path(), excPath) )
                    {
                    inExcludedPath = true;
                    break;
                    }
                }
            if (p.exists() && p.is_regular_file() &&
                !inExcludedPath &&
                (ext == L".c" || ext == L".cpp" || ext == L".h" || ext == L".hpp"))
                {
                filesToAnalyze.push_back(p.path().string());
                }
            }
        }
    else
        {
        std::wcout << L"You must pass in at least one folder to analyze.";
        return 0;
        }

    i18n_check::cpp_i18n_review cpp;

    size_t currentFileIndex{ 0 };
    for (const auto& file : filesToAnalyze)
        {
        std::wifstream ifs(file);
        std::wstring str((std::istreambuf_iterator<wchar_t>(ifs)),
                          std::istreambuf_iterator<wchar_t>());

        std::wcout << L"Processed " << std::to_wstring(++currentFileIndex) <<
            " of " << std::to_wstring(filesToAnalyze.size()) << " files (" <<
            fs::path(file).filename() << ")\n";
        cpp(str.c_str(), str.length(), fs::path(file).wstring());
        }

    std::wcout << "Reviewing strings...\n";
    cpp.review_localizable_strings();
    std::wcout << "Running diagnostics...\n";
    cpp.run_diagnostics();

    std::wstringstream report;
    if (cpp.get_unsafe_localizable_strings().size())
        {
        report << L"Localizable strings that probably should not be\n" <<
                  L"===================================================================================\n\n";
        }
    for (const auto& val : cpp.get_unsafe_localizable_strings())
        {
        report << L"\"" << val.m_string << L"\"\n\t" <<
            val.m_file_name << L" (line " << val.m_line << L", column " << val.m_column << L")\n\t";
        if (val.m_usage.m_type == i18n_review::string_info::usage_info::usage_type::function)
            { report << L"In function call: " << val.m_usage.m_value << L"\n\n"; }
        else if (val.m_usage.m_type == i18n_review::string_info::usage_info::usage_type::variable)
            { report << L"Assigned to variable: " << val.m_usage.m_value << L"\n\n"; }
        else
            { report << val.m_usage.m_value << L"\n\n"; }                  
        }
    
    if (cpp.get_localizable_strings_in_internal_call().size())
        {
        report << L"Strings being used with internal functions/variables that should not be localizable\n" <<
                  L"===================================================================================\n\n";
        }
    for (const auto& val : cpp.get_localizable_strings_in_internal_call())
        {
        report << L"\"" << val.m_string << L"\"\n\t" <<
            val.m_file_name << L" (line " << val.m_line << L", column " << val.m_column << L")\n\t";
        if (val.m_usage.m_type == i18n_review::string_info::usage_info::usage_type::function)
            { report << L"In non-localizable function call: " << val.m_usage.m_value << L"\n\n"; }
        else if (val.m_usage.m_type == i18n_review::string_info::usage_info::usage_type::variable)
            { report << L"Assigned to non-localizable variable: " << val.m_usage.m_value << L"\n\n"; }
        else
            { report << val.m_usage.m_value << L"\n\n"; }                  
        }

    if (cpp.get_not_available_for_localization_strings().size())
        {
        report << L"Strings not available for translation\n" <<
                  L"===================================================================================\n\n";
        }
    for (const auto& val : cpp.get_not_available_for_localization_strings())
        {
        report << L"\"" << val.m_string << L"\"\n\t" <<
            val.m_file_name << L" (line " << val.m_line << L", column " << val.m_column << L")\n\t";
        if (val.m_usage.m_type == i18n_review::string_info::usage_info::usage_type::function)
            { report << L"In function call: " << val.m_usage.m_value << L"\n\n"; }
        else if (val.m_usage.m_type == i18n_review::string_info::usage_info::usage_type::variable)
            { report << L"Assigned to variable: " << val.m_usage.m_value << L"\n\n"; }
        else
            { report << val.m_usage.m_value << L"\n\n"; }                  
        }

    std::wofstream ofs(L"output.txt");
    ofs << report.str().c_str();

    return 0;
    }
