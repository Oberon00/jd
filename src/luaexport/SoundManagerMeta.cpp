#include "svc/SoundManager.hpp"

#include "compsys/BasicMetaComponent.hpp"

static char const libname[] = "SoundManager";
#include "ExportThis.hpp"


JD_BASIC_COMPONENT_IMPL(SoundManager)

static void init(LuaVm& vm)
{
    vm.initLib("ComponentSystem");
    LHMODULE [
#       define LHCURCLASS SoundManager
        class_<LHCURCLASS, Component>("SoundManager")
            .def("playSound",
                (void(LHCURCLASS::*)(std::string const&))
                &SoundManager::playSound)
            .def("setBackgroundMusic",
                (void(LHCURCLASS::*)(std::string const&, sf::Time))
                &SoundManager::setBackgroundMusic)
#       undef LHCURCLASS
    ];
}

