#ifndef LUA_EXPORT_MACROS_HPP_INCLUDED
#define LUA_EXPORT_MACROS_HPP_INCLUDED LUA_EXPORT_MACROS_HPP_INCLUDED

#include <boost/config.hpp> // BOOST_STRINGIZE


#define LHMODULE  using namespace luabind; module(L, ::jd::moduleName)
#define LHARGS(x) BOOST_STRINGIZE(x), &x
#define LHFN(f)   def(LHARGS(f))

#define LHCLASS2(c)      class_<c>(BOOST_STRINGIZE(c))
#define LHCLASS          LHCLASS2(LHCURCLASS)
#define LHMEMARGS2(x, c) BOOST_STRINGIZE(x), &c::x
#define LHMEMARGS(x)     LHMEMARGS2(x, LHCURCLASS)
#define LHMEMFN2(f, c)   def(LHMEMARGS2(f, c))
#define LHMEMFN(f)       LHMEMFN2(f, LHCURCLASS)
#define LHPROPRW2(p, c)  def_readwrite(LHMEMARGS2(p, c))
#define LHPROPRW(p)      LHPROPRW2(p, LHCURCLASS)
#define LHPROPG2(p, c)   property(LHMEMARGS2(p, c))
#define LHPROPG(p)       LHPROPG2(p, LHCURCLASS)

#define LHVIRTSTUB0(f, r)     virtual r f() { return static_cast<r>(call<r>(BOOST_STRINGIZE(f))); }

#define LHISREFVALID2(c) property("valid", &isValidWeakRef<c>)
#define LHISREFVALID     LHISREFVALID2(LHCURCLASS)

#endif
