// Part of the Jade Engine -- Copyright (c) Christian Neumüller 2012--2013
// This file is subject to the terms of the BSD 2-Clause License.
// See LICENSE.txt or http://opensource.org/licenses/BSD-2-Clause

#include "State.hpp"

static char const libname[] = "State";
#include "ExportThis.hpp"


namespace {

#define STATE_METHODS(m) \
    m(initialize) \
    m(prepare)    \
    m(resume)     \
    m(pause)      \
    m(stop)       \
    m(cleanup)

class wrap_State: public State, public luabind::wrap_base {
public:
#   define WRAP(n) LHVIRTSTUB0(n, void)
    STATE_METHODS(WRAP)
#   undef WRAP
    static void nop(State*) { }
};

} // anonymous namespace

static void init(LuaVm& vm)
{
    LHMODULE [
        class_<State, wrap_State>("State")
            .def(constructor<>())
#           define DECLARE(n) .def(#n, &State::n, &wrap_State::nop)
            STATE_METHODS(DECLARE)
#           undef DECLARE
    ];
}

#undef STATE_METHODS // unnecessary
