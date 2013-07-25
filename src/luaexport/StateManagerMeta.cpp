// Part of the Jade Engine -- Copyright (c) Christian Neumüller 2012--2013
// This file is subject to the terms of the BSD 2-Clause License.
// See LICENSE.txt or http://opensource.org/licenses/BSD-2-Clause

#include "svc/StateManager.hpp"

#include "jdConfig.hpp"
#include "State.hpp" // State class must be defined to make binding compile

#include <luabind/class.hpp>
#include <luabind/iterator_policy.hpp>

static char const libname[] = "StateManager";
#include "ExportThis.hpp"


static void init(LuaVm& vm)
{
    vm.initLib("State");
    LHMODULE [
#       define LHCURCLASS StateManager
        class_<StateManager>("StateManager")
            .def("push", (void(StateManager::*)(State&))(&StateManager::push))
            .def("push", (void(StateManager::*)(std::string const&))(&StateManager::push))
            .def("pushAdditional", (void(StateManager::*)(State&))(&StateManager::pushAdditional))
            .def("pushAdditional", (void(StateManager::*)(std::string const&))(&StateManager::pushAdditional))
            .def("switchTo", (void(StateManager::*)(State&))(&StateManager::switchTo))
            .def("switchTo", (void(StateManager::*)(std::string const&))(&StateManager::switchTo))
            .LHMEMFN(pop)
            .LHMEMFN(clear)
            .LHMEMFN(popRunning)
            .LHPROPG(active)
            .property(LHMEMARGS(running), return_stl_iterator)
            .property(LHMEMARGS(stacked), return_stl_iterator)
            .def("register", &StateManager::registerState)
            .def("state", &StateManager::stateForId)
    ];
}
