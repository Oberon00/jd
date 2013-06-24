local M = { }

-- If you can ignore false, then you should prefer the "v or default" syntax
-- over this function: it's faster and more readable.
function M.default(v, default)
    if v ~= nil then
        return v
    end
    return default
end

function M.callopt(f, ...)
    if f then
        return f(...)
    end
end

-- Beware: nil vs. { }
function M.forward(fn, args)
    local targs = type(args)
    if targs == 'table' then
        return fn(table.unpack(args))
    elseif targs == 'nil' then
        return fn()
    end
    return fn(args)
end

function M.id(...)
    return ...
end

function M.rawtostring(o)
    local mt = getmetatable(o)
    if not mt then
        return tostring(o)
    end
    if mt.__tostring and type(o) ~= 'table' then
        error "cannot get raw string representation: "
              "cannot remove __tostring from metatable"
    end
    local tos = mt.__tostring
    mt.__tostring = nil
    s = tostring(o)
    mt.__tostring = tos
    return s
end

function M.fail()
    error "invalid function"
end

function M.failMsg(msg)
    return msg and function()
        error(msg)
    end or M.fail
end

function M.isCallable(f)
    if type(f) == 'function' then
        return true
    end
    local mt = getmetatable(f)
    if mt then
        return mt._call and M.isCallable(mt.__call) or false
    end
    return false
end

function M.inplaceMap(seq, f)
    if type(seq) == 'userdata' and seq.count then
        local i = 1
        while i <= seq.count do
            local v = f(seq:get(i))
            seq:set(i, v)
            if v ~= nil then
                i = i + 1
            end -- if v ~= nil
        end -- while i < seq.count
    else -- if seq is a C++-Container
        for i = 1, #seq do
            seq[i] = f(seq[i])
        end -- for i, v in ipairs(seq)
    end -- if seq is a C++-Container/else
end -- function M.inplaceMap

return M
