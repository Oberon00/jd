import string
import sys
from os.path import exists

HEADER_TEMPLATE = string.Template(
"""#ifndef $includeGuard
#define $includeGuard $includeGuard

#include "compsys/Component.hpp"
#include <string>

class $compName: public Component {
    JD_COMPONENT

public:
    $compName();
    ~$compName();

private:

};

#endif

""")

IMPLEMENTATION_TEMPLATE = string.Template(
"""#include "$compName.hpp"

$compName::$compName()
{
}

$compName::~$compName()
{
}

""")

META_TEMPLATE = string.Template(
"""#include "$headerPath"
#include "compsys/BasicMetaComponent.hpp"

static char const libname[] = "$compName";
#include "ExportThis.hpp"

JD_SINGLETON_COMPONENT_IMPL($compName)

static void init(LuaVm& vm)
{
    vm.initLib("ComponentSystem");
    LHMODULE [
#       define LHCURCLASS $compName
        class_<LHCURCLASS, Component>("$compName")

#       undef LHCURCLASS
    ];
}

""")


def createComponent(name, pathPrefix = "comp/"):

    headerPath = pathPrefix + name + ".hpp"
    implPath   = pathPrefix + name + ".cpp"
    metaPath   = "luaexport/" + name + "Meta.cpp"

    print "COMP_HEADERS      +=", headerPath
    print "COMP_SOURCES      +=", implPath
    print "LUAEXPORT_SOURCES +=", metaPath

    if (exists(headerPath) or exists(implPath) or exists(metaPath)):
        print "One or more files already exist. Aborting."
        sys.exit(1)

    tmplArgs = {
        "headerPath":   headerPath,
        "compName":     name,
        "includeGuard": name.upper() + "_HPP_INCLUDED"
    }

    def writeout(fname, tmpl):
        with open(fname, "w") as f:
            f.write(tmpl.substitute(tmplArgs))

    writeout(headerPath, HEADER_TEMPLATE)
    writeout(implPath, IMPLEMENTATION_TEMPLATE)
    writeout(metaPath, META_TEMPLATE)


if __name__ == "__main__":
    compPath = (sys.argv[2] if len(sys.argv) > 2 else "comp") + "/"

    createComponent(
        sys.argv[1],
        compPath)
