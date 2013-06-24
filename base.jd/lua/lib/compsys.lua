--[[
    module compsys

    Pseudo keywords: The function M.component is also exported directly to _G.
    You can prevent this behavior by setting COMPSYS_NO_KEYWORDS to true.
--]]

local tabutil = require 'tabutil'
local evt = require 'evt'
local util = require 'util'
local oo = require 'oo' -- cppclass

local M = { }

--[[
    Pseudo keyword.

    Creates the component in the current environment or the environment
    specified. Also overrides __init to do the right things. Define _init
    (with 1 underscore) if you want to do something in the constructor (Most
    initialization code will go to initComponent anyway). __init will also
    create an evt.Table named self.evts, which it automatically disconnects
    in cleanupComponent(). If you want to do something yourself there, define
    cleanup() instead.
    If env is _G then this is a just a shortcut for
        class(name)(jd.Component)
        jd.registerComponent(name)
        function name:__init(p, ...) <...>; self._init and self:_init(...) end
--]]
function M.component(name, env)
    local classobj = oo.cppclass(name, env, jd.Component)
    jd.registerComponent(name)

    function classobj:__init(parent, ...)
        jd.Component.__init(self, parent, name)
        self:_bindLuaPart()
        self.evts = evt.Table()
        util.callopt(self.init, self, ...)
    end

    function classobj:cleanupComponent()
        if getmetatable(self) then
            self.evts:disconnect()
        end
        util.callopt(self.cleanup, getmetatable(self) and self or nil)
    end

    return classobj
end


local function createComponent(constructor, parent, args)
    local targs = type(args)
    if targs == 'nil' then
        return constructor(parent)
    elseif targs == 'table' then
        return constructor(parent, table.unpack(args))
    else
        return constructor(parent, args)
    end
end

--[[
    Returns an entity constructor. This constructor is a function which, when
    called, returns a entity containing all components specified in components.

    The entity constructor has two formal arguments: parent and [initargs].
        parent:     The jd.EntityCollection to which the entity should be
                    added.
        [initargs]: A table containing component constructors as keys and
                    arguments as values. The arguments will be passed to the
                    constructor of the entity instead of the arguments
                    specified in the components table, if any. The constructors
                    do not have to be in components; they will be called, and
                    thus the components added, anyway.

                    Note that initargs will be modified in an undefined way.


    Arguments:

        components: a table containg either component constructors as values
                    and numbers as keys, eg. a sequence like
                        {jd.PositionComponent, jd.TilePositionComponent}
                    or component constructors as *keys* and arguments for the
                    constructors as values either as a table, which will be
                    unpacked when calling it, or as a single argument, e.g.:
                        {jd.PositionComponent = jd.Vec2(42, 7),
                         FooComponent = {"bar", jd.Texture.request "baz"}}
                    To call a constructor with a single table wrap the table in
                    another table, eg. {{1, 2, 3}}.
                    Both forms can be mixed and in both forms the parent enity
                    is added as first argument.

        [setupfn]:  A function to be called after the entity is constructed and
                    components are added, but before finish() is called. If
                    nil, components._setupfn will be used instead. The first
                    argument to seutpfn will be the created entity, followed by
                    any additional arguments passed to M.entity
--]]
function M.entity(components, setupfn)
    return function(initargs, ...) -- component constructor
        local entity = jd.Entity()

        -- Create all "default" components for this entity:
        for component, args in pairs(components) do
            -- If no key was specified in the components-table, it will be a
            -- number. Ignore it.
            if type(component) == 'number' then
                component = args
                args = nil
            end
            -- any component created here is removed from initargs
            args = util.default(tabutil.pop(initargs, component), args)
            createComponent(component, entity, args)
        end -- for ... in pairs(components)

        -- If any non-default components remain, create them now
        for component, args in pairs(initargs) do
            createComponent(component, entity, args)
        end

        util.callopt(setupfn, entity, ...)
        entity:finish()
        return entity
    end -- return function ...
end -- function M.entity(...)

-- Entity which consists of a single compoenent.
function M.singletonEntity(Component)
    return function(...)
        local entity = jd.Entity()
        local component = Component(entity, ...)
        entity:finish()
        return entity, component
    end
end

if not COMPSYS_NO_KEYWORDS then
    tabutil.copyEntry(_G, 'component', M)
end

return M
