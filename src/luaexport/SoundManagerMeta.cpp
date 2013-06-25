// Part of the Jade Engine -- Copyright (c) Christian Neumüller 2012--2013
// This file is subject to the terms of the BSD 2-Clause License.
// See LICENSE.txt or http://opensource.org/licenses/BSD-2-Clause

#include "svc/SoundManager.hpp"

#include "compsys/BasicMetaComponent.hpp"
#include "resources.hpp"
#include "sharedPtrConverter.hpp"

static char const libname[] = "SoundManager";
#include "ExportThis.hpp"


JD_BASIC_COMPONENT_IMPL(SoundManager)

static void init(LuaVm& vm)
{
    vm.initLib("ComponentSystem");

    using namespace sf;
    using namespace luabind;

#   define LHCURCLASS SoundBuffer
    class_<LHCURCLASS, std::shared_ptr<LHCURCLASS>> cSoundBuffer("SoundBuffer");
    cSoundBuffer
        .def(constructor<LHCURCLASS const&>())
        .property("sampleCount", &LHCURCLASS::getSampleCount)
        .property("sampleRate", &LHCURCLASS::getSampleRate)
        .property("channelCount", &LHCURCLASS::getChannelCount)
        .property("duration", &LHCURCLASS::getDuration);
#   undef LHCURCLASS
    addResMngMethods(cSoundBuffer);

    LHMODULE [
        cSoundBuffer,
#       define LHCURCLASS SoundManager
        class_<LHCURCLASS, Component>("SoundManager")
            .def("playSound",
                (void(LHCURCLASS::*)(std::string const&))
                &SoundManager::playSound)
            .def("setBackgroundMusic",
                (void(LHCURCLASS::*)(std::string const&, sf::Time))
                &SoundManager::setBackgroundMusic),
#       undef LHCURCLASS

#       define LHCURCLASS AutoSound
        class_<LHCURCLASS>("Sound")
            .def(constructor<>())
            .def(constructor<LHCURCLASS::Ptr>())
            .def(constructor<LHCURCLASS const&>())
            .LHMEMFN(play)
            .LHMEMFN(pause)
            .LHMEMFN(stop)
            .property("buffer",
                (LHCURCLASS::Ptr(LHCURCLASS::*)())&LHCURCLASS::resource,
                &LHCURCLASS::setResource)
            .property("looped", &LHCURCLASS::getLoop, &LHCURCLASS::setLoop)
            .property("offset",
                &LHCURCLASS::getPlayingOffset, &LHCURCLASS::setPlayingOffset)
            .property("status", &LHCURCLASS::getStatus)
            .enum_("status") [
                value("PAUSED", LHCURCLASS::Paused),
                value("STOPPED", LHCURCLASS::Stopped),
                value("PLAYING", LHCURCLASS::Playing)
            ]
            .property("volume", &LHCURCLASS::getVolume, &LHCURCLASS::setVolume)

#       undef LHCURCLASS

    ];
}
