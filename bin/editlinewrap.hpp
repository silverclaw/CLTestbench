// This file is part of CLTestbench.

// CLTestbench is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// CLTestbench is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.

// You should have received a copy of the GNU Lesser General Public License
// along with CLTestbench.  If not, see <https://www.gnu.org/licenses/>.

#pragma once
#include <cstdio>

#include "cltb_config.h"
#if CLTB_USE_LIBEDIT
    #include <histedit.h>
#elif CLTB_USE_READLINE
    #include <readline/readline.h>
    #include <readline/history.h>
    #include <signal.h>
#else
    #include <iostream>
#endif

#include <string.h>
#include <string_view>

namespace CLTestbench
{
class EditLine final
{
    static constexpr const char* Prompt = "(bench) ";
#if CLTB_USE_LIBEDIT
    ::EditLine* const mEditLine;
    ::History* const mHistory;

    // Taken from histedit.h, and modified to return a const char*
    typedef const char* (*el_pfunct_t)(EditLine*);
    static const char* prompt(EditLine*) noexcept { return Prompt; }
#elif CLTB_USE_READLINE
    char *mBuffer = nullptr;
    static void interruptHandler(int)
    {
        std::cout << std::endl;
        rl_on_new_line();
        rl_replace_line("", 0);
        rl_redisplay();
    }
#else
    std::string mBuffer;
#endif // CLTESTBENCH_USE_LIBEDIT

    EditLine(EditLine&) = delete;
    EditLine(EditLine&&) = delete;
    EditLine& operator=(EditLine&) = delete;
    EditLine* operator=(EditLine&&) = delete;

  public:
#if CLTB_USE_LIBEDIT
    EditLine() noexcept : mEditLine(el_init("CLTestbench", stdin, stdout, stderr)), mHistory(history_init())
    {
        if (!mEditLine) return;

        el_set(mEditLine, EL_PROMPT, static_cast<el_pfunct_t>(&prompt));
        el_set(mEditLine, EL_SIGNAL, 1);
        if (mHistory) el_set(mEditLine, EL_HIST, &::history, mHistory);
    }
#elif CLTB_USE_READLINE
    EditLine() noexcept
    {
        signal(SIGINT, interruptHandler);
    }
#else
    EditLine() = default;
#endif // CLTESTBENCH_USE_LIBEDIT

    bool isValid() const noexcept
    {
#if CLTB_USE_LIBEDIT
        return (mEditLine != nullptr);
#else // CLTESTBENCH_USE_LIBEDIT
        return true;
#endif // CLTESTBENCH_USE_LIBEDIT
    }

    std::string_view getLine() noexcept
    {
#if CLTB_USE_LIBEDIT
        int read = 0;
        const char* data = el_gets(mEditLine, &read);

        if (read == 0 || data == nullptr)
            // TODO - may want to do something here (and check errno).
            return std::string_view("");

        return std::string_view(data, read);
#elif CLTB_USE_READLINE
        free(mBuffer);
        mBuffer = readline(Prompt);

        if (!mBuffer || mBuffer[0] == '\0') return std::string_view("");
        add_history(mBuffer);
        return std::string_view(mBuffer);
#else
        std::cout << Prompt << std::flush;
        std::getline(std::cin, mBuffer);
        return mBuffer;
#endif // CLTESTBENCH_USE_LIBEDIT
    }

    ~EditLine()
    {
#if CLTB_USE_LIBEDIT
        el_end(mEditLine);
        history_end(mHistory);
#elif CLTB_USE_READLINE
        free(mBuffer);
#endif // CLTESTBENCH_USE_LIBEDIT
    }
};
} // namespace CLTestbench
