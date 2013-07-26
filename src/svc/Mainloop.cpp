// Part of the Jade Engine -- Copyright (c) Christian Neumüller 2012--2013
// This file is subject to the terms of the BSD 2-Clause License.
// See LICENSE.txt or http://opensource.org/licenses/BSD-2-Clause

#define MAINLOOP_KEEP_CALLBACKS
#include "Mainloop.hpp"

Mainloop::Mainloop():
    m_exitRequested(false),
    m_exitcode(EXIT_FAILURE)
{
}

int Mainloop::exec()
{
    m_sig_started();
    while (!m_exitRequested) {
#define EMIT(s) m_sig_##s();
        CALLBACKS(EMIT)
#undef EMIT
    }
    m_sig_quitting(m_exitcode);
    return m_exitcode;
}

void Mainloop::quit(int exitcode)
{
    m_exitRequested = true;
    m_exitcode = exitcode;
    m_sig_quitRequested(exitcode);
}
