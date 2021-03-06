Title:  Jade Engine README
Author: Christian Neumüller

JadeEngine
==========

**JadeEngine** (or **jd** for short) is a game Engine written in *C++* for
*[Lua][]* with 2D graphics, audio, virtual file system (ZIP) and *TMX*
([Tiled Map Editor][]'s format) support.


[Lua]: http://www.lua.org/
[Tiled Map Editor]: http://www.mapeditor.org/



Building
--------

### Ingredients ###

The following libraries are needed:

- **[SFML][]** in the latest (git) version, available at
  <http://github.com/LaurentGomila/SFML>.
- **[Boost][]** in version 1.53 (later versions may/should also work).
- **[Lua][]** in the latest version of the 5.2.x branch. You may want to apply
  the patches listed at <http://www.lua.org/bugs.html>. Please compile as C
  (jd was not tested with the C++ build of Lua which throws exceptinons).
- **[Luabind][]** in a 0.9 version patched by me:
  <https://github.com/Oberon00/luabind>
- **[zlib][]** (tested with version 1.2.7).
- **[PhysFS][]** (tested with version 2.0.3).
- **[ssig][]**: My own library for signals (Boost.Signal is too slow).

The only compiler throughoutly tested is **[MSVC 11][]**, but once upon a time the
source code and `CMakeLists.txt` were adjusted to also work with g++ 4.7.2.
The build system used is **[CMake][]** in a recent 2.8.x version.

[SFML]: http:/sfml-dev.org/
[Boost]: http://www.boost.org/
[Luabind]: http://www.rasterbar.com/products/luabind.html
[zlib]: http://www.zlib.net/
[PhysFS]: http://icculus.org/physfs/
[ssig]: https://github.com/Oberon00/ssig
[MSVC 11]: http://www.microsoft.com/visualstudio/eng/products/visual-studio-express-for-windows-desktop#product-express-desktop
[CMake]: http://www.cmake.org/


### Setup ###

Especially on Windows, make sure to set any environment variables neccessary to
let CMake find the librarys are set correctly: `SFML_ROOT`, `BOOST_ROOT`,
`LUA_DIR`, `LUABIND_DIR`, `PHYSFSDIR`, `SSIG_DIR`. zlib does not look at any
environment variables but only the `ZLIB_ROOT` CMake variable: See the second
option in the [`Find*.cmake`](#setcmakevar) subsection for how to set it.

As an alternative to setting the variables you can also install the libraries
to the (CMake) standard locations or add the containing directories to
`CMAKE_PREFIX_PATH`.


#### `Find*.cmake` ####
The `CMakeLists.txt` for jd call `find_package()` with some libraries where
the corresponding `Find*.cmake` is not built in:

* `FindLua52.cmake` and `FindLuabind.cmake` are contained in my Luabind fork
   linked above.
* `FindSFML.cmake` and `Findssig.cmake` are contained in their own libraries.

To make CMake find them you have two options:

* Copy the files to your CMake installation's module directory. This is the
  directory where e.g. the `AddFileDependencies.cmake` module is located. It
  usually lies in `<prefix>/share/cmake-2.8/Modules`, where `<prefix>` is the
  installation directory on Windows (usually
  `C:\Program Files (x86)\CMake 2.8`) and usually simply `/usr` on Linux.
* <a name=setcmakevar></a>Specify the directories where the files are located
  as a semicolon `;` separated list in the `CMAKE_MODULE_PATH` CMake cache
  variable: *Add entry*, type string in the Windows GUI;
  or `-DCMAKE_MODULE_PATH=<path-list>` on the command line.

### Actual build ###

What's left to do is a standard CMake build. The following contains nothing
special, so if you are familiar with CMake, you can just skip the rest.

#### Unix-like ####

Navigate to the Jade root directory (with the jd and base.jd subfolders) in
your shell, then execute the following commands.

    mkdir build # Name basically arbitrary, but build is already in .gitignore
    cd build    #
    cmake .. # You may need to add the module path modifications here.
    make
    sudo make install # Optional.
    
#### Visual Studio ####

Use the Visual Studio command prompt as your shell and do as in Unix, with the
following modifications: You may need to add `-G "Visual Studio 11"` to the
cmake command line. Then use `msbuild ALL_BUILD.vcxproj` instead of `make` and
`msbuild INSTALL.vcxproj` instead of `sudo make install` if you want to
install the Jade Engine. As in Unix, you will need administrative rights for
that, but because Windows has no `sudo` equivalent, you may need to e.g.
launch a new VS command prompt as administrator.


> Jade Engine -- Copyright (c) Christian Neumüller 2012--2013
> This file is subject to the terms of the BSD 2-Clause License.
> See LICENSE.txt or http://opensource.org/licenses/BSD-2-Clause.
