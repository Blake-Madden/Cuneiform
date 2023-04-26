# i18n-check

Internationalization & localization analysis system for C++ code.

`i18n-check` scans a folder of C++ code and reviews the following issues:
- Strings exposed for translation¹ that probably should not be. This includes (but not limited to):
    - Filenames
    - Strings only containing `printf()` commands
    - Numbers
    - Regular expressions
    - Strings inside of debug functions
    - Formulas
    - Code (used for code generators)
    - Strings that contain URLs or email addresses.
- Strings not available for translation that possibly should be.
- Strings that contain extended ASCII characters that are not encoded.
  ("Danke schön" instead of "Danke sch\U000000F6n".)<br />
  Encoding extended ASCII characters is recommended for
  best portability between compilers.
- Strings with malformed syntax (e.g., malformed HTML tags).
- The use of deprecated text macros (e.g., the `wxT()` macro in wxWidgets).
- Files that contain extended ASCII characters, but are not UTF-8 encoded.<br />
  (It is recommended that files be UTF-8 encoded for portability between compilers.)
- UTF-8 encoded files which start with a BoM/UTF-8 signature.<br />
  It is recommended to save without the file signature for best compiler portability.
- `printf()`-like functions being used to just format an integer to a string.<br />
  It is recommended to use `std::to_string()` to do this instead.
- ID variable² assignment issues:
    - The same value being assigned to different ID variables in the same source file
      (e.g., "wxID_HIGHEST + 1" being assigned to two menu ID constants).
    - Hard-coded numbers being assigned to ID variables.

Code formatting issues can also be checked for, such as:
- Trailing spaces at the end of a line.
- Tabs (instead of spaces).
- Lines longer than 120 characters.
- Spaces missing between "//" and their comments.

¹ Strings are considered translatable if inside of [gettext](https://www.gnu.org/software/gettext/),
[wxWidgets](https://www.wxwidgets.org), or [Qt](https://www.qt.io) l10n functions.
This includes functions and macros such as `gettext()`, `_()`, `tr()`, `translate()`,
`QT_TR_NOOP()`, `wxTRANSLATE()`, etc.<br />
² Variables are determined to be ID variables if they are integral types with the whole word "ID" in their name.

Refer [here](Example.md) for example usage.

[![cppcheck](https://github.com/Blake-Madden/i18n-check/actions/workflows/cppcheck.yml/badge.svg)](https://github.com/Blake-Madden/i18n-check/actions/workflows/cppcheck.yml)
[![doxygen](https://github.com/Blake-Madden/i18n-check/actions/workflows/doxygen.yml/badge.svg)](https://github.com/Blake-Madden/i18n-check/actions/workflows/doxygen.yml)
[![unix build](https://github.com/Blake-Madden/i18n-check/actions/workflows/unix%20build.yml/badge.svg)](https://github.com/Blake-Madden/i18n-check/actions/workflows/unix%20build.yml)
[![unit-tests](https://github.com/Blake-Madden/i18n-check/actions/workflows/unit-tests.yml/badge.svg)](https://github.com/Blake-Madden/i18n-check/actions/workflows/unit-tests.yml)

# Usage

`i18n-check` accepts the following arguments:

```shellscript
[input]: The folder to analyze.

--enable: Which checks to perform. Can be any combination of:
  allI18N:             Perform all i18n checks (the default).
                       This does not include code formatting checks.
  suspectL10NString:   Check for translatable strings that shouldn't be
                       (e.g., numbers, keywords, printf commands).
  suspectL10NUsage:    Check for translatable strings being used in internal contexts
                       (e.g., debugging functions).
  urlInL10NString:     Check for translatable strings that contain URLs or email addresses.
                       It is recommended to dynamically format these into the string so that
                       translators don't have to manage them.
  notL10NAvailable:    Check for strings not exposed for translation.
  deprecatedMacros:    Check for deprecated text macros (e.g., wxT()).
  nonUTF8File:         Check that files containing extended ASCII characters are UTF-8 encoded.
  UTF8FileWithBoM:     Check for UTF-8 encoded files which start with a BoM/UTF-8 signature.
                       It is recommended to save without the file signature for
                       best compiler portability.
  unencodedExtASCII:   Check for strings containing extended ASCII characters that are not encoded.
  printfSingleNumber:  Check for printf()-like functions being used to just format a number.
  dupValAssignedToIds: Check for the same value being assigned to different ID variables.
  numberAssignedToId:  Check for ID variables being assigned a hard-coded number.
                       It may be preferred to assign framework-defined constants to IDs.
  malformedStrings:    Check for malformed syntax in strings (e.g., malformed HTML tags).
  allCodeFormatting:   Check all code formatting issues (see below).
                       These are not enabled by default.
  trailingSpaces:      Check for trailing spaces at the end of each line.
                       This is a code formatting check and is not enabled by default.
  tabs:                Check for tabs.
                       (Spaces are recommended as tabs may appear differently between editors.)
                       This is a code formatting check and is not enabled by default.
  wideLines:           Check for overly long lines.
                       This is a code formatting check and is not enabled by default.
  commentMissingSpace: Check that there is a space at the start of a comment.
                       This is a code formatting check and is not enabled by default.

--disable: Which checks to not perform. (Refer to options available above.)
           This will override any options passed to "--enable".

--log-l10n-allowed: Whether it is acceptable to pass translatable strings to 
                    logging functions. Setting this to false will emit warnings
                    when a translatable string is passed to functions such as
                    wxLogMessage or SDL_Log.
                    (Default is true.)

--punct-l10n-allowed: Whether it is acceptable for punctuation only strings to be
                      translatable. Setting this to true will suppress warnings about
                      strings such as " - " being available for localization.
                      (Default is false.)

--exceptions-l10n-required: Whether to verify that exception messages are available
                            for translation. Setting this to true will emit warnings
                            when untranslatable strings are passed to various exception
                            constructors or functions (e.g., AfxThrowOleDispatchException).
                            (Default is true.)

--min-l10n-wordcount: The minimum number of words that a string must have to be
                      considered translatable. Higher values for this will result in less
                      strings being classified as a notL10NAvailable warning.
                      (Default is 2.)

-i,--ignore: Folders and files to ignore (can be used multiple times).

-o,--output: The output report path. (Can either be a full path, or a file name within
             the current working directory.)

-q,--quiet: Only print errors and the final output.

-v,--verbose: Display debug information.

-h,--help: Print usage.
```

The following example will analyze the folder "c:\src\wxWidgets\src"
(but ignore the subfolders "expat" and "zlib"). It will only check for
suspect translatable strings, and then send the output to "results.txt"
in the current working directory. 

```shellscript
i18n-check C:\src\wxWidgets\src -i expat -i zlib --enable=suspectL10NString -o results.txt
```

This example will only check for `suspectL10NUsage` and `suspectL10NString` and not show
any progress messages.

```shellscript
i18n-check C:\src\wxWidgets\samples -q --enable=suspectL10NUsage,suspectL10NString
```

This example will ignore multiple folders (and files) and output the results to "WDVResults.txt."

```shellscript
i18n-check C:\src\Wisteria-dataviz\src --ignore=import,i18n-check,wxsimplejson,wxStartPage,math,easyexif,debug,utfcpp,CRCpp,base/colors.cpp,base/colors.h -o WDVresults.txt
```

Refer [here](Example.md) for more examples.

# Building

`i18n-check` can be configured with Cmake and then built with your compiler of choice.

On Unix:

```shellscript
cmake ./
make -j4
```

On Windows, "CMakeLists.txt" can be opened and built directly in Visual Studio.

After building, "i18n-check" will then be available in the "bin" folder.

# GitHub Action

You can also create an `i18n-check` GitHub action to make it part of your CI.
For example, create a new workflow called "i18n-check.yml" and enter the following:

```shellscript
name: i18n-check
on: [push]

jobs:
  build:
    name: i18n-check
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v3

      - name: install i18n-check
        run: |
             git clone https://github.com/Blake-Madden/i18n-check.git --recurse-submodules
             cd i18n-check
             cmake ./
             make -j4
             cd ..

      - name: analyze
        run: |
             # Ignore i18n-check's own folder.
             # You can ignore other folders by adding a comma and the folder name
             # after "--ignore=i18n-check".
             ./i18n-check/bin/i18n-check ./ --ignore=i18n-check -q -o i18nresults.txt

      - name: review results
        run: |
             REPORTFILE=./i18nresults.txt
             WARNINGSFILE=./warnings.txt
             if test -f "$REPORTFILE"; then
                cat "$REPORTFILE" > "$WARNINGSFILE"
                # are there any warnings?
                if grep -qP '\[[a-zA-Z0-9]+\]' "$WARNINGSFILE"; then
                    # print the remaining warnings
                    echo Warnings detected:
                    echo ==================
                    cat "$WARNINGSFILE" | grep -P '\[[a-zA-Z0-9]+\]'
                    # fail the job
                    exit 1
                else
                    echo No issues detected
                fi
             else
                echo "$REPORTFILE" not found
             fi
```
