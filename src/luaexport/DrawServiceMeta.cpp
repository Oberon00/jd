// Part of the Jade Engine -- Copyright (c) Christian Neumüller 2012--2013
// This file is subject to the terms of the BSD 2-Clause License.
// See LICENSE.txt or http://opensource.org/licenses/BSD-2-Clause

#include "svc/DrawService.hpp"

static char const libname[] = "DrawService";
#include "ExportThis.hpp"


static DrawService::Layer& getLayer(DrawService& ds, std::size_t n)
{
    if (--n >= ds.layerCount())
        throw "layer index out of bounds";
    return ds.layer(n);
}

static void init(LuaVm& vm)
{
    vm.initLib("SfGraphics");
    LHMODULE [
#       define LHCURCLASS DrawService::Layer
        class_<LHCURCLASS>("DrawLayer")
            .def_readonly(LHMEMARGS(group))
            .LHPROPRW(view),
#       undef LHCURCLASS
#       define LHCURCLASS DrawService
        LHCLASS
            .LHMEMFN(resetLayerViews)
            .def("layer", &getLayer)
            .LHPROPG(layerCount)
            .property("backgroundColor",
                &DrawService::backgroundColor,
                &DrawService::setBackgroundColor)
#       undef LHCURCLASS
    ];
}
