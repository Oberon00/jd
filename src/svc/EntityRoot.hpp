#ifndef ENTITY_ROOT_HPP_INCLUDED
#define ENTITY_ROOT_HPP_INCLUDED ENTITY_ROOT_HPP_INCLUDED

#include "compsys/Component.hpp"
#include "compsys/EntityCollection.hpp"

// Inherits from EnableWeakRefFromThis<EntityCollection> and
// EnableWeakRefFromThis<Component>. (Only) As long as no WeakRefs
// are obtained, that should not be a problem.
class EntityRoot: public EntityCollection, public Component
{
    JD_COMPONENT
public:
    EntityRoot(): EntityCollection("EntityRoot") { }
};

#endif