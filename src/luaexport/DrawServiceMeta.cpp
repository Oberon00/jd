#include "svc/DrawService.hpp"

#include "compsys/BasicMetaComponent.hpp"

static char const libname[] = "DrawService";
#include "ExportThis.hpp"


JD_BASIC_COMPONENT_IMPL(DrawService)

static DrawService::Layer& getLayer(DrawService& ds, std::size_t n)
{
    if (--n >= ds.layerCount())
        throw "layer index out of bounds";
    return ds.layer(n);
}

static void init(LuaVm& vm)
{
    vm.initLib("ComponentSystem");
    vm.initLib("SfGraphics");
    LHMODULE [
#       define LHCURCLASS DrawService::Layer
        class_<LHCURCLASS>("DrawLayer")
            .def_readonly(LHMEMARGS(group))
            .LHPROPRW(view),
#       undef LHCURCLASS
        class_<DrawService, Component>("DrawService")
            .def("resetLayerViews", &DrawService::resetLayerViews)
            .def("layer", &getLayer)
            .property("layerCount", &DrawService::layerCount)
            .property("backgroundColor",
                &DrawService::backgroundColor,
                &DrawService::setBackgroundColor)
    ];
}
