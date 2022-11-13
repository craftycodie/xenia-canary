/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2022 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

// General Set

ADD_MULTI_CHOICE_INPUT_SETTING(
    int, "Game Language",
    "Choose the language for launched games (does not apply to the UI)",
    user_language, "Ganeral Settings", "General",
    MULTI_CHOICE_OPTIONS(
        OPTION("English", 1), OPTION("Japanese", 2), OPTION("German", 3),
        OPTION("French", 4), OPTION("Spanish", 5), OPTION("Italian", 6),
        OPTION("Korean", 7), OPTION("Chinese (Traditional)", 8),
        OPTION("Portuguese", 9), OPTION("Polish", 11), OPTION("Russian", 12),
        OPTION("Slovenian", 13), OPTION("Turkish", 14), OPTION("Norwegian", 15),
        OPTION("Dutch", 16), OPTION("Chinese (Simplified)", 17)));