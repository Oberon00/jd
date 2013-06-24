--[[
    module oo (object orientation)
--]]

local util = require 'util'

local M = { }

local function construct(cls, ...)
    local self = { }
    setmetatable(self, cls)
    return self, util.callopt(self.__init, self, ...)
end

local id = debug.id or util.rawtostring

function M.objectToString(obj)
    local name = getmetatable(obj).clsName
    if not name then
        return "Object: " .. id(obj)
    end
    return name .. ": " .. id(obj)
end

function M.classToString(cls)
    return "class: " .. cls.clsName or id(cls)
end

-- class([name, [env]], [super])
-- class([super])
function M.class(name, env, super)
    if name and not super and type(name) ~= 'string' then
        super, name = name, nil
    end
    local cls = {super = super, clsName = name, __tostring = M.objectToString}
    cls.__index = cls
    setmetatable(cls, {
        __index = super,
        __call = construct,
        __tostring = M.classToString
    })
    if name and env then
        env[name] = cls
    end
    return cls
end

function M.isInstance(obj, cls, orDerived)
    local mt = getmetatable(cls)
    if not mt then
        return false
    end
    if mt == cls or mt.clsName == cls then
        return true
    end
    return orDerived and M.isInstance(obj, cls.super)
end

M.lclass = M.class -- alias for consistence with pseudo keyword

M.mustOverride = util.failMsg "must override this method"

-- cppclass(name, [env], super)
function M.cppclass(name, env, super)
    if type(env) == 'userdata' then
        super, env = env, _ENV
    elseif not env then
        env = _ENV
    end
    assert(
        type(super) == 'userdata',
        "Only use cppclass for deriving from C++ classes! Use lclass instead.")
    assert(env[name] == nil, "oo.cppclass: name conflict")
    local oldval = _G[name]
    class(name)(super) -- create the actual class
    local classobj = _G[name]
    if env ~= _G then
        env[name], _G[name] = classobj, oldval
    end
    function classobj:__init()
        super.__init(self)
    end
    return classobj
end

if not OO_NO_KEYWORDS then
    (require 'tabutil').copyEntry(_G, 'lclass', M)
end

return M
