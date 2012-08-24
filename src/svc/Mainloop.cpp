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
        m_sig_preFrame();
        m_sig_processInput();
        m_sig_update();
        m_sig_interact();
        m_sig_preDraw();
        m_sig_draw();
        m_sig_postDraw();
        m_sig_postFrame();
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
