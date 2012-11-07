#include "luaexport/Geometry.hpp"
#include "sfUtil.hpp"
#include "svc/LuaVm.hpp"

namespace lgeo = luaSfGeo;

char const* const lgeo::libname = "SfSystem::Vector";

static char const mmtKey2 = 0;
char const* const lgeo::Traits<LuaVec2>::expName = "Vec2";
char const* const lgeo::Traits<LuaVec2>::mtName  = "jd.Vec2";
void const* const lgeo::Traits<LuaVec2>::mmtKey  = &mmtKey2;

static char const mmtKey3 = 0;
char const* const lgeo::Traits<LuaVec3>::mtName  = "jd.Vec3";
char const* const lgeo::Traits<LuaVec3>::expName = "Vec3";
void const* const lgeo::Traits<LuaVec3>::mmtKey  = &mmtKey3;

static char const mmtKeyR = 0;
char const* const lgeo::Traits<LuaRect>::mtName  = "jd.Rect";
char const* const lgeo::Traits<LuaRect>::expName = "Rect";
void const* const lgeo::Traits<LuaRect>::mmtKey  = &mmtKeyR;

template<typename T>
static int Vec_abs(lua_State* L)
{
    T* v = lgeo::to<T>(L, 1);
    lua_pushnumber(L, jd::math::abs(*v));
    return 1;
}

template<typename T>
static int Vec_isZero(lua_State* L)
{
    T* v = lgeo::to<T>(L, 1);
    lua_pushboolean(L, jd::isZero(*v));
    return 1;
}

template<typename T>
static int Vec_distance(lua_State* L)
{
    T *v1 = lgeo::to<T>(L, 1), *v2 = lgeo::to<T>(L, 2);
    lua_pushnumber(L, jd::distance(*v1, *v2));
    return 1;
}

template<typename T>
static int Vec_manhattanDistance(lua_State* L)
{
    T *v1 = lgeo::to<T>(L, 1), *v2 = lgeo::to<T>(L, 2);
    lua_pushnumber(L, jd::manhattanDistance(*v1, *v2));
    return 1;
}


template<typename T>
static int geo_eq(lua_State* L)
{
    T *v1 = lgeo::to<T>(L, 1),
      *v2 = lgeo::to<T>(L, 2);
    lua_pushboolean(L, *v1 == *v2);
    return 1;
}

template<typename T>
static int Vec_add(lua_State* L)
{
    T *v1 = lgeo::to<T>(L, 1),
      *v2 = lgeo::to<T>(L, 2);
    lgeo::push<T>(L, *v1 + *v2);
    return 1;
}

template<typename T>
static int Vec_sub(lua_State* L)
{
    T *v1 = lgeo::to<T>(L, 1),
      *v2 = lgeo::to<T>(L, 2);
    lgeo::push<T>(L, *v1 - *v2);
    return 1;
}

template<typename T>
static int Vec_unm(lua_State* L)
{
    T* v = lgeo::to<T>(L, 1);
    lgeo::push<T>(L, -*v);
    return 1;
}

template<typename T>
static void checkbinopargs(lua_State* L, T*& v, lua_Number& n)
{
    if (luaL_testudata(L, 1, lgeo::Traits<T>::mtName)) {
        v = static_cast<T*>(lua_touserdata(L, 1));
        n = luaL_checknumber(L, 2);
    } else {
        n = luaL_checknumber(L, 1);
        v = lgeo::to<T>(L, 2);
    }
}

template<typename T>
static int Vec_mul(lua_State* L)
{
    T* v;
    lua_Number n;
    checkbinopargs(L, v, n);
    lgeo::push<T>(L, *v * n);
    return 1;
}

template<typename T>
static int Vec_div(lua_State* L)
{
    T* v;
    lua_Number n;
    checkbinopargs(L, v, n);
    lgeo::push<T>(L, *v / n);
    return 1;
}

static bool geo_attrib(lua_State* L, char const* n, LuaVec2 const* v)
{
    if (n[0] != 0 && n[1] == 0) switch (n[0]) {
        case 'x': lua_pushnumber(L, v->x); return true;
        case 'y': lua_pushnumber(L, v->y); return true;
    }
    return false;
}

static bool geo_attrib(lua_State* L, char const* n, LuaVec3 const* v)
{
    if (n[0] != 0 && n[1] == 0) switch (n[0]) {
        case 'x': lua_pushnumber(L, v->x); return true;
        case 'y': lua_pushnumber(L, v->y); return true;
        case 'z': lua_pushnumber(L, v->z); return true;
    }
    return false;
}

static bool geo_attrib(lua_State* L, char const* n, LuaRect const* v)
{
    static auto const cmp = &std::strcmp;
    if (!cmp(n, "x") || !cmp(n, "left"))
        lua_pushnumber(L, v->left);
    else if (!cmp(n, "y") || !cmp(n, "top"))
        lua_pushnumber(L, v->top);
    else if (!cmp(n, "w") || !cmp(n, "width"))
        lua_pushnumber(L, v->width);
    else if (!cmp(n, "h") || !cmp(n, "height"))
        lua_pushnumber(L, v->height);
    else if (!cmp(n, "right"))
        lua_pushnumber(L, jd::right(*v));
    else if (!cmp(n, "bottom"))
        lua_pushnumber(L, jd::bottom(*v));
    else if (!cmp(n, "right"))
        lua_pushnumber(L, jd::rightIn(*v));
    else if (!cmp(n, "bottom"))
        lua_pushnumber(L, jd::bottomIn(*v));
    else if (!cmp(n, "xy") || !cmp(n, "topLeft") || !cmp(n, "position"))
        lgeo::push(L, jd::topLeft(*v));
    else if (!cmp(n, "size") || !cmp(n, "wh"))
        lgeo::push(L, jd::size(*v));
    else if (!cmp(n, "bottomRight"))
        lgeo::push(L, jd::bottomRight(*v));
    else if (!cmp(n, "bottomRightIn"))
        lgeo::push(L, jd::bottomRightIn(*v));
    else if (!cmp(n, "center"))
        lgeo::push(L, jd::center(*v));
    else
        return false;
    return true;
}

template <typename T>
static int geo_index(lua_State* L)
{
    T* v = lgeo::to<T>(L, 1);
    char const* name = luaL_checkstring(L, 2);

    if (geo_attrib(L, name, v))
        return 1;

    // return mmt[arg#2]
    lua_rawgetp(L, LUA_REGISTRYINDEX, lgeo::Traits<T>::mmtKey);
    lua_insert(L, 2);
    lua_rawget(L, 2);
    return 1;
}

bool geo_setattrib(lua_State* L, char const* n, LuaVec2* v)
{
    if (n[0] != 0 && n[1] == 0) switch (n[0]) {
        case 'x': v->x = luaL_checknumber(L, 3); return true;
        case 'y': v->y = luaL_checknumber(L, 3); return true;
    }
    return false;
}

bool geo_setattrib(lua_State* L, char const* n, LuaVec3* v)
{
    if (n[0] != 0 && n[1] == 0) switch (n[0]) {
        case 'x': v->x = luaL_checknumber(L, 3); return true;
        case 'y': v->y = luaL_checknumber(L, 3); return true;
        case 'z': v->z = luaL_checknumber(L, 3); return true;
    }
    return false;
}

static bool geo_setattrib(lua_State* L, char const* n, LuaRect* v)
{
    static auto const cmp = &std::strcmp;
    if (!cmp(n, "x") || !cmp(n, "left")) {
        v->left = luaL_checknumber(L, 3);
    } else if (!cmp(n, "y") || !cmp(n, "top")) {
        v->top = luaL_checknumber(L, 3);
    } else if (!cmp(n, "w") || !cmp(n, "width")) {
        v->width = luaL_checknumber(L, 3);
    } else if (!cmp(n, "h") || !cmp(n, "height")) {
        v->height = luaL_checknumber(L, 3);
    } else if (!cmp(n, "right")) {
        lua_Number const n = luaL_checknumber(L, 3);
        v->width = n - v->left;
    } else if (!cmp(n, "bottom")) {
        lua_Number const n = luaL_checknumber(L, 3);
        v->height = n - v->top;
    } else if (!cmp(n, "xy") || !cmp(n, "topLeft") || !cmp(n, "position")) {
        LuaVec2 const* nv = lgeo::to<LuaVec2>(L, 3);
        v->left = nv->x;
        v->top  = nv->y;
    } else if (!cmp(n, "size") || !cmp(n, "wh")) {
        LuaVec2 const* nv = lgeo::to<LuaVec2>(L, 3);
        v->width = nv->x;
        v->height = nv->y;
    } else if (!cmp(n, "bottomRight")) {
        LuaVec2 const* nv = lgeo::to<LuaVec2>(L, 3);
        v->width  = nv->x - v->left;
        v->height = nv->y - v->top;
    } else if (!cmp(n, "center")) {
        LuaVec2 const* nv = lgeo::to<LuaVec2>(L, 3);
        LuaVec2 const curCenter = jd::center(*v);
        LuaVec2 const motion = *nv - curCenter;
        v->left += motion.x;
        v->top  += motion.y;
    } else {
        return false;
    }
    return true;
}

template <typename T>
static int geo_newindex(lua_State* L)
{
    T* v = lgeo::to<T>(L, 1);
    char const* name = luaL_checkstring(L, 2);

    if (geo_setattrib(L, name, v))
        return 1;

    return luaL_argerror(L, 2, "cannot create new attributes");
}

template <typename T>
static int geo_create(lua_State* L);

template <>
static int geo_create<LuaVec2>(lua_State* L)
{
    if (lua_istable(L, 1))
        lua_remove(L, 1); // remove table which is called
    if (lua_gettop(L) == 1) {
        LuaVec3* v = lgeo::optTo<LuaVec3>(L, 1);
        if (v)
            lgeo::push(L, jd::vec3to2(*v)); // conversion Vec3 --> Vec2
        else
            lgeo::push(L, *lgeo::to<LuaVec2>(L, 1)); // copy ctor
    } else {
        lua_Number const x = luaL_optnumber(L, 1, 0),
                         y = luaL_optnumber(L, 2, 0);
        lgeo::push<LuaVec2>(L, LuaVec2(x, y));
    }
    return 1;
}

template <>
static int geo_create<LuaVec3>(lua_State* L)
{
    if (lua_istable(L, 1))
        lua_remove(L, 1); // remove table which is called
    if (lua_gettop(L) == 1) { // copy ctor
        lgeo::push(L, *lgeo::to<LuaVec3>(L, 1));
    } else {
        lua_Number const x = luaL_optnumber(L, 1, 0),
                         y = luaL_optnumber(L, 2, 0),
                         z = luaL_optnumber(L, 3, 0);
        lgeo::push<LuaVec3>(L, LuaVec3(x, y, z));
    }
    return 1;
}

template <>
static int geo_create<LuaRect>(lua_State* L)
{
    if (lua_istable(L, 1))
        lua_remove(L, 1); // remove table which is called

    if (lua_gettop(L) == 1) { // 1 arg: copy ctor
        lgeo::push(L, *lgeo::to<LuaRect>(L, 1));
    } else if (lua_gettop(L) == 2) { // 2 args: LuaRect(LuaVec2 p, LuaVec2
        LuaVec2 const *p  = lgeo::to<LuaVec2>(L, 1),
                      *sz = lgeo::to<LuaVec2>(L, 2);
        lgeo::push<LuaRect>(L, LuaRect(*p, *sz));
    } else { // default constructor (0, 0, 0, 0) or (x, y) or (x, y, w, h)
        lua_Number const x = luaL_optnumber(L, 1, 0),
                         y = luaL_optnumber(L, 2, 0),
                         w = luaL_optnumber(L, 3, 0),
                         h = luaL_optnumber(L, 3, 0);
        lgeo::push<LuaRect>(L, LuaRect(x, y, w, h));
    }
    return 1;
}

template <typename T>
static int geo_tostring(lua_State* L);

template <>
static int geo_tostring<LuaVec2>(lua_State* L)
{
    LuaVec2* v = lgeo::to<LuaVec2>(L, 1);
    lua_pushfstring(L, "jd.Vec2(%f,%f)", v->x, v->y);
    return 1;
}

template <>
static int geo_tostring<LuaVec3>(lua_State* L)
{
    LuaVec3* v = lgeo::to<LuaVec3>(L, 1);
    lua_pushfstring(L, "jd.Vec3(%f,%f,%f)", v->x, v->y, v->z);
    return 1;
}

template <>
static int geo_tostring<LuaRect>(lua_State* L)
{
    LuaRect* v = lgeo::to<LuaRect>(L, 1);
    lua_pushfstring(L, "jd.Rect(%f,%f,%f,%f)", v->left, v->top, v->width, v->height);
    return 1;
}

static int Rect_intersection(lua_State* L)
{
    LuaRect const *v1 = lgeo::to<LuaRect>(L, 1),
                  *v2 = lgeo::to<LuaRect>(L, 2);
    LuaRect intersection;
    if (v1->intersects(*v2, intersection)) {
        lgeo::push(L, intersection);
        return 1;
    }
    return 0;
}

static int Rect_contains(lua_State* L)
{
    LuaRect* v = lgeo::to<LuaRect>(L, 1);

    // 1 argument: contains(LuaVec2 p)
    if (lua_gettop(L) == 2) {
        LuaVec2* p = lgeo::to<LuaVec2>(L, 2);
        lua_pushboolean(L, v->contains(*p));
        return 1;
    }

    // 2 arguments (or more: ignored): contains(lua_Number x, lua_Number y)
    lua_Number const x = luaL_checknumber(L, 2),
                     y = luaL_checknumber(L, 3);
    lua_pushboolean(L, v->contains(x, y));
    return 1;
}

static int Rect_outermostPoint(lua_State* L)
{
    LuaRect const* in = lgeo::to<LuaRect>(L, 1);
    LuaVec2 const *d = lgeo::to<LuaVec2>(L, 2);
    if (lua_gettop(L) > 2) {
        if (LuaVec2 const* from = lgeo::optTo<LuaVec2>(L, 3)) {
            lgeo::push(L, jd::outermostPoint(*in, *d, *from));
        } else {
            lgeo::push(L, jd::outermostPoint(
                *in, *d, *lgeo::to<LuaRect>(L, 3)));
        }
    } else {
        lgeo::push(L, jd::outermostPoint(*in, *d, jd::center(*in)));
    }
    return 1;
}

static int Rect_nearestPoint(lua_State* L)
{
    LuaRect const* in = lgeo::to<LuaRect>(L, 1);
    LuaVec2 const *to = lgeo::to<LuaVec2>(L, 2);
    lgeo::push(L, jd::nearestPoint(*in, *to));
    return 1;
}

static int Rect_intersectsLine(lua_State* L)
{
    LuaRect const* r = lgeo::to<LuaRect>(L, 1);
    LuaVec2 const *p1 = lgeo::to<LuaVec2>(L, 2), *p2 = lgeo::to<LuaVec2>(L, 2);
    lua_pushboolean(L, jd::intersection(*p1, *p2, *r));
    return 1;
}

static int Rect_clipLine(lua_State* L)
{
    LuaRect const* r = lgeo::to<LuaRect>(L, 1);
    LuaVec2 p1 = *lgeo::to<LuaVec2>(L, 2), p2 = *lgeo::to<LuaVec2>(L, 3);
    bool const intersection = jd::clipLine(p1, p2, *r);
    if (!intersection)
        return 0;
    lgeo::push(L, p1);
    lgeo::push(L, p2);
    return 2;
}


template <typename T>
void Vec_export(lua_State* L)
{
    int const success = luaL_newmetatable(L, lgeo::Traits<T>::mtName);
    assert(success);
    (void)success;
    static luaL_Reg const fns[] = {
        {"__index", &geo_index<T>},
        {"__newindex", &geo_newindex<T>},
        {"__eq", &geo_eq<T>},
        {"__unm", &Vec_unm<T>},
        {"__add", &Vec_add<T>},
        {"__sub", &Vec_sub<T>},
        {"__mul", &Vec_mul<T>},
        {"__div", &Vec_div<T>},
        {"__len", &Vec_abs<T>},
        {"__tostring", &geo_tostring<T>},
        // no __gc, because LuaVec is trivially destructible
        {nullptr, nullptr}
    };
    luaL_setfuncs(L, fns, 0);
    lua_pop(L, 1);

    static luaL_Reg const mfns[] = {
        {"abs", &Vec_abs<T>},
        {"isZero", &Vec_isZero<T>},
        {"distance", &Vec_distance<T>},
        {"manhattanDistance", &Vec_manhattanDistance<T>},
        {nullptr, nullptr}
    };
    lua_getglobal(L, "jd");
    luaL_newlib(L, mfns);
    lua_pushvalue(L, -1);

    lua_createtable(L, 0, 1);
    lua_pushcfunction(L, &geo_create<T>);
    lua_setfield(L, -2, "__call");
    lua_setmetatable(L, -2);

    lua_setfield(L, -3, lgeo::Traits<T>::expName); // jd[expName] = mmt
    lua_rawsetp(L, LUA_REGISTRYINDEX, lgeo::Traits<T>::mmtKey); // REGISTRY[mmtKey] = mmt
    lua_pop(L, 1);
}

void Rect_export(lua_State* L)
{
    int const success = luaL_newmetatable(L, lgeo::Traits<LuaRect>::mtName);
    assert(success);
    (void)success;
    static luaL_Reg const fns[] = {
        {"__index", &geo_index<LuaRect>},
        {"__newindex", &geo_newindex<LuaRect>},
        {"__eq", &geo_eq<LuaRect>},
        {"__tostring", &geo_tostring<LuaRect>},
        {nullptr, nullptr}
    };
    luaL_setfuncs(L, fns, 0);
    lua_pop(L, 1);

    static luaL_Reg const mfns[] = {
        {"intersection", &Rect_intersection},
        {"contains", &Rect_contains},
        {"outermostPoint", &Rect_outermostPoint},
        {"nearestPoint", &Rect_nearestPoint},
        {"intersectsLine", &Rect_intersectsLine},
        {"clipLine", &Rect_clipLine},
        {nullptr, nullptr}
    };

    lua_getglobal(L, "jd");
    luaL_newlib(L, mfns);
    lua_pushvalue(L, -1);

    lua_createtable(L, 0, 1);
    lua_pushcfunction(L, &geo_create<LuaRect>);
    lua_setfield(L, -2, "__call");
    lua_setmetatable(L, -2);


    lua_setfield(L, -3, lgeo::Traits<LuaRect>::expName); // jd[expName] = mmt
    lua_rawsetp(L, LUA_REGISTRYINDEX, lgeo::Traits<LuaRect>::mmtKey); // REGISTRY[mmtKey] = mmt
    lua_pop(L, 1);
}


void init(LuaVm& vm)
{
    lua_State* L = vm.L();

    Vec_export<LuaVec2>(L);
    Vec_export<LuaVec3>(L);
    Rect_export(L);
}

static bool prepareInit()
{
    LuaVm::registerLib(lgeo::libname, &init);
    return bool();
}

static bool const dummy = prepareInit();
