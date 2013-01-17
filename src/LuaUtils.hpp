#ifndef LUA_UTILS_HPP_INCLUDED
#define LUA_UTILS_HPP_INCLUDED LUA_UTILS_HPP_INCLUDED

#include <luabind/error.hpp>

#include <stdexcept>
#include <string>


struct lua_State;
class VFile;

namespace luaU {

// Debugging & error reporting //
class Error: public std::runtime_error {
public:
    explicit Error(luabind::error const& e, std::string const& msg = "");
    Error(lua_State* L, std::string const& msg);
    explicit Error(std::string const& msg): std::runtime_error(msg) { }
};
std::string const errstring(int luaerr);
std::string const dumpvar(lua_State* L, int idx);
std::string const dumpstack(lua_State* L);

// safe variants of standard functions //
bool next(lua_State* L, int idx);

// Loading & execution //

void pcall(lua_State* L, int nargs, int nresults);

// load: pushes the loaded chunk onto the stack, throws exception in case of errors
void load(lua_State* L, std::string const& vfilename, char const* mode = nullptr);

// exec(L, n, m, na, nr) = luaU::load(L, n, m); luaU::pcall(L, na, nr)
void exec(
    lua_State* L,
    std::string const& vfilename, char const* mode = nullptr,
    int nargs = 0, int nresults = 0);

// Dumping //

void dumpFunction(lua_State* L, VFile& f);
void dumpFunction(lua_State* L, std::string const& vfilename);



// exportenum //
// Usage:
//
// static ExportedEnumValue const myenum[] = {
//      {"foo", valueOfFoo},
//      ...
//      {nullptr, 0} // always mark the end with a entry whose name is nullptr
// exportenum(L, myTableIdx, myenum);
// 
// myTableIdx must be the index of a table

struct ExportedEnumValue {
    char const* name;
    int value;
};
void exportenum(lua_State* L, int idx, ExportedEnumValue const* entries);


// StackBalance //

class StackBalance {
public:
    enum Action { pop = 1, pushNil = 2, adjust = pop | pushNil, debug = 4};
    explicit StackBalance(lua_State* L, int diff = 0, Action action = pop);
    ~StackBalance();
private:
    StackBalance& operator= (StackBalance const&);

    lua_State* const m_L;
    int const m_desiredTop;
    Action const m_action;
};

#define LUAU_BALANCED_STACK_A(L, n, a) \
     luaU::StackBalance _luaUstackBalance##__LINE__( \
            L, n, luaU::StackBalance::a)

#ifdef NDEBUG
#   define LUAU_BALANCED_STACK2_DBG(L, n)
#else
#   define LUAU_BALANCED_STACK2_DBG(L, n) LUAU_BALANCED_STACK_A(L, n, debug)
#endif
#define LUAU_BALANCED_STACK_DBG(L) LUAU_BALANCED_STACK2_DBG(L, 0)
#define LUAU_BALANCED_STACK2(L, n) LUAU_BALANCED_STACK_A(L, n, pop)
#define LUAU_BALANCED_STACK(L) LUAU_BALANCED_STACK2(L, 0)


} // namespace luaU


#endif
