#ifndef MAINLOOP_HPP_INCLUDED
#define MAINLOOP_HPP_INCLUDED MAINLOOP_HPP_INCLUDED

namespace sf { class RenderWindow; }

#include "compsys/Component.hpp"
#include "ssig.hpp"

#include <cstdlib>


class MetaComponent;

class Mainloop: public Component {
    JD_COMPONENT

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