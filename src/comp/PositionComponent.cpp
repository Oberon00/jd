#include "PositionComponent.hpp"

#include "compsys/BasicMetaComponent.hpp"
#include "compsys/Entity.hpp"
#include "luaexport/LuaEventHelpers.hpp"
#include "luaexport/SfBaseTypes.hpp"

static char const libname[] = "PositionComponent";
#include "luaexport/ExportThis.hpp"


JD_BASIC_EVT_COMPONENT_IMPL(PositionComponent)

JD_EVENT_TABLE_BEGIN(PositionComponent)
    JD_EVENT_ENTRY(rectChanged, void, _1, _2)
JD_EVENT_TABLE_END

PositionComponent::PositionComponent(Entity& parent)
{
    parent.add(*this);
}

void PositionComponent::setRect(sf::FloatRect const& r)
{
    auto const oldRect = m_rect;
    m_rect = r;
    m_sig_rectChanged(oldRect, m_rect);
}

sf::Vector2f PositionComponent::position() const
{
    return sf::Vector2f(m_rect.left, m_rect.top);
}

void PositionComponent::setPosition(sf::Vector2f p)
{
    auto const oldRect = m_rect;
    m_rect.left = p.x;
    m_rect.top = p.y;
    m_sig_rectChanged(oldRect, m_rect);
}

void PositionComponent::move(sf::Vector2f d)
{
    if (d == sf::Vector2f())
        return;
    auto const oldRect = m_rect;
    m_rect.left += d.x;
    m_rect.top += d.y;
    m_sig_rectChanged(oldRect, m_rect);
}

sf::Vector2f PositionComponent::size() const
{
    return sf::Vector2f(m_rect.width, m_rect.height);
}

void PositionComponent::setSize(sf::Vector2f sz)
{
    auto const oldRect = m_rect;
    m_rect.width = sz.x;
    m_rect.height = sz.y;
    m_sig_rectChanged(oldRect, m_rect);
}


static void init(LuaVm& vm)
{
    vm.initLib("ComponentSystem");
    LHMODULE [
#define LHCURCLASS PositionComponent
    class_<LHCURCLASS, Component, WeakRef<Component>>("PositionComponent")
        .def(constructor<Entity&>())
        .property("rect", &LHCURCLASS::rect, &LHCURCLASS::setRect)
        .property("position", &LHCURCLASS::position, &LHCURCLASS::setPosition)
        .property("size", &LHCURCLASS::size, &LHCURCLASS::setSize)
        .LHMEMFN(move)
#undef LHCURCLASS
    ];
}