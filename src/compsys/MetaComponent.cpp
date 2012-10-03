#include "compsys/MetaComponent.hpp"
#include "compsys/Component.hpp"
#include "ComponentRegistry.hpp"
#include <unordered_map>
#include "LuaUtils.hpp"
#include "svc/ServiceLocator.hpp"
#include "svc/LuaVm.hpp"
#include "Logfile.hpp"
#include <boost/format.hpp>

static char const libname[] = "ComponentSystem";
#include "luaexport/ExportThis.hpp"
#include <luabind/class_info.hpp>
#include <luabind/raw_policy.hpp>
#include <luabind/adopt_policy.hpp>
#include <luabind/operator.hpp>
#include <ssig.hpp>
#include "Entity.hpp"

namespace {

static void registerMetaComponent(lua_State* L, std::string const& name)
{
    static std::vector<std::unique_ptr<LuaMetaComponent>> components;
    components.push_back(std::unique_ptr<LuaMetaComponent>(new LuaMetaComponent(L, name)));
}

static std::string const& Component_componentName(Component const& c)
{
    return c.metaComponent().name();
}

luabind::object Component_parent(Component const& c, lua_State* L)
{
    if (c.parent())
        return luabind::object(L, c.parent()->ref());
    else
        return luabind::object();
}

static std::ostream& operator<< (std::ostream& os, Component const& c)
{
    return os << "jd.Component (" << c.metaComponent().name() << " @" << &c << ')';
}

static ConnectionBase* connectEvent(
    lua_State* L,
    Component& sender,
    std::string const& eventname,
    luabind::object const& receiver)
{
    receiver.push(L);
    ConnectionBase* result = sender.metaComponent().connectEvent(
        L, &sender, eventname);
    lua_pop(L, 1);
    if (!result)
        throw luaU::Error("Event \"" + eventname + "\" not found!");
    return result;
}

class wrap_Component: public Component, public luabind::wrap_base {
public:
    wrap_Component(lua_State* L, Entity& parent, std::string const& classname)
    {
        m_metaComponent = ComponentRegistry::get(L)[classname];
        if (!m_metaComponent)
            throw InvalidMetaComponentName(classname);

        parent.add(*this);
        assert(this->parent() == &parent);
    }

	void bindLuaPart()
	{
		luabind::wrapped_self_t& wrapper = luabind::detail::wrap_access::ref(*this);
		if (wrapper.m_strong_ref.is_valid()) {
			LOG_W("Double call to wrap_Component::bindLuaPart");
			return;
		}
		wrapper.get(wrapper.state());
		wrapper.m_strong_ref.set(wrapper.state());
	}

    virtual MetaComponent const& metaComponent() const { return *m_metaComponent; }

    virtual void initComponent()
	{
		luabind::wrapped_self_t& wrapper = luabind::detail::wrap_access::ref(*this);
		if (!wrapper.m_strong_ref.is_valid()) {
			LOG_W("wrap_Component::bindLuaPart was not called yet! Calling it now.");
			bindLuaPart();
		}
		call<void>("initComponent");
	}
    virtual void cleanupComponent()
	{
		if (ServiceLocator::luaVm().L()) {
			call<void>("cleanupComponent");
		} else {
			LOG_W(boost::format("wrap_Component::cleanupComponent: cannot dispatch to Lua, because"
			     " the lua_State is closing. (this=%1%; name=%2%)") % this % m_metaComponent->name());
		}
	}

    static void nop(Component*) { /* NOP */ }

private:
    MetaComponent const* m_metaComponent;
};

class wrap_ConnectionBase: public ConnectionBase, public luabind::wrap_base {
public:
    virtual void disconnect() { call<void>("disconnect"); }
    virtual bool isConnected() const { return call<bool>("isConnected"); }
};

} // anonymous namespace

static bool operator==(Component const& lhs, Component const* rhs)
    { return &lhs == rhs; }


static void init(LuaVm& vm)
{
    LUAU_BALANCED_STACK_DBG(vm.L());

    LHMODULE [
        // Component class
        // Note: metaComponent() is left out and implemented in wrap_Component
        class_<Component, wrap_Component, WeakRef<Component>>("Component")
            .def(constructor<lua_State*, Entity&, std::string const&>())
            .def("initComponent", &Component::initComponent, &wrap_Component::nop)
            .def("cleanupComponent", &Component::cleanupComponent, &wrap_Component::nop)
            .property("parent", &Component_parent)
            .property("componentName", &Component_componentName)
			.def("_bindLuaPart", &wrap_Component::bindLuaPart)
            .def(const_self == other<Component*>())
            .def(tostring(const_self))
            .LHISREFVALID2(Component),
        def("registerComponent", &registerMetaComponent),
        def("connect", &connectEvent, adopt(result)),
        class_<ConnectionBase, wrap_ConnectionBase>("ConnectionBase")
            .def(constructor<>())
            .def("disconnect", &ConnectionBase::disconnect)
            .def("getIsConnected", &ConnectionBase::isConnected)
            .property("isConnected", &ConnectionBase::isConnected)
    ];

    lua_getglobal(vm.L(), "jd");
    luabind::object(vm.L(), static_cast<Component*>(nullptr)).push(vm.L());
    lua_setfield(vm.L(), -2, "NIL_COMPONENT");
    lua_pop(vm.L(), 1);
}


bool operator== (MetaComponent const& lhs, MetaComponent const& rhs)
{
    if (&lhs == &rhs) {
        assert(&lhs.name() == &rhs.name());
        return true;
    }
    assert(&lhs.name() != &rhs.name());
    assert(lhs.name() != rhs.name());
    return false;
}

bool operator!= (MetaComponent const& lhs, MetaComponent const& rhs)
{
    return !(lhs == rhs);
}

bool operator<  (MetaComponent const& lhs, MetaComponent const& rhs)
{
    return lhs.name() < rhs.name();
}

///////////////////////////////////////////////////////

LuaMetaComponent::LuaMetaComponent(lua_State* L, std::string const& name):
    m_L(L),
    m_name(name)
{
    ComponentRegistry::get(L).registerComponent(this);
}

std::string const& LuaMetaComponent::name() const
{
    return m_name;
}

Component* LuaMetaComponent::create() const
{
    using namespace luabind;
    return call_function<Component*>(m_L, m_name.c_str())[adopt(result)];
}

void LuaMetaComponent::castUp(lua_State* L, Component* c) const
{
    assert(dynamic_cast<wrap_Component*>(c));
	luabind::object o(L, c->ref<wrap_Component>());
    o.push(L);
}

ConnectionBase* LuaMetaComponent::connectEvent(Component* c, std::string const& name) const
{
    using namespace luabind;
    object receiver(from_stack(m_L, -1));
    object callback = globals(m_L)[jd::moduleName]["callback_connectLuaEvent"];
    if (type(callback) != LUA_TFUNCTION)
        throw std::runtime_error("no callback for event connection defined (must be a plain function)");
    return object_cast<ConnectionBase*>(callback(c, name, receiver)[adopt(result)]);
}
