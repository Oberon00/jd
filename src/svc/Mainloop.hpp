// Part of the Jade Engine -- Copyright (c) Christian Neumüller 2012--2013
// This file is subject to the terms of the BSD 2-Clause License.
// See LICENSE.txt or http://opensource.org/licenses/BSD-2-Clause

#ifndef MAINLOOP_HPP_INCLUDED
#define MAINLOOP_HPP_INCLUDED MAINLOOP_HPP_INCLUDED

#include <ssig.hpp>

#include <cstdlib>


class MetaComponent;

class Mainloop {
#define CALLBACKS(m) \
        m(started)      \
        m(preFrame)     \
        m(processInput) \
        m(update)       \
        m(interact)     \
        m(preDraw)      \
        m(draw)         \
        m(postDraw)     \
        m(postFrame)

#   define CALLBACK(n) SSIG_DEFINE_MEMBERSIGNAL(n, void())
    CALLBACKS(CALLBACK)
#   undef CALLBACK
#   ifndef MAINLOOP_KEEP_CALLBACKS
#       undef CALLBACKS
#   endif
    SSIG_DEFINE_MEMBERSIGNAL(quitRequested, void(int))
    SSIG_DEFINE_MEMBERSIGNAL(quitting, void(int))
public:
    Mainloop();

    int exec();
    void quit(int exitcode = EXIT_SUCCESS);

private:
    bool m_exitRequested;
    int m_exitcode;
};
#endif
