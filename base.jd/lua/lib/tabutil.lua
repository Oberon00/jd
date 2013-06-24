local util = require 'util'

local M = { }

M.DEEPMERGE_MAXDEPTH = 5

local mergeConflict = { }

function mergeConflict.error(dst, t, k)
    error(("tabutil.merge: conflict at key '%s', values '%s', '%s'").format(
          k, dst[k], t[k]))
end

function mergeConflict.t(dst, t, k)
    return  t[k]
end

mergeConflict.src = mergeConflict.t

function mergeConflict.dst(dst, t, k)
    return  dst[k]
end

function mergeConflict.deepmerge(dst, t, k)
    if (type(dst[k]) ~= 'table' or type(t[k]) ~= 'table') then
        mergeConflict.error(dst, t, k)
    end

    local depth = 1
    local function deepmerge2(dst, t, k)
        if (depth > M.DEEPMERGE_MAXDEPTH) then
            error (("deepmerge: DEEPMERGE_MAXDEPTH (%i) exceeded").format(
                M.DEEPMERGE_MAXDEPTH))
        end
        depth = depth + 1
        return M.merge(dst[k], t[k], deepmerge2)
    end

    return M.merge(dst[k], t[k], deepmerge2)
end

--[[
    conflict can be either a string "error", "deepmerge", "dst" or "t" (alias
    "src") or a function, which takes two tables and a key and returns the
    value which the result should contain at that key. The default is "error".
    Returns dst.
--]]
function M.merge(dst, t, conflict)
    conflict = type(conflict) == 'string' and
        mergeConflict[conflict] or conflict or mergeConflict.error
    for k, v in pairs(t) do
        oldv = dst[k]
        if oldv == nil then
            dst[k] = v
        else
            dst[k] = conflict(dst, t, k)
        end
    end
    return dst
end

function M.pop(t, k)
    local result = t[k]
    t[k] = nil
    return result
end

function M.newEntry(t, k, v)
    if t[k] ~= nil then
        error(("name conflict: key '%s' already exists").format(k))
    end
    t[k] = v
end

function M.copyEntry(dst, k, src)
    M.newEntry(dst, k, src[k])
end

function M.default(t, k, default)
    default = util.default(default, { })
    local v = t[k]
    if v == nil then
        t[k] = default
        return default
    end
    return v
end


function M.fromJdTable(cnt)
    local result = { }
    for kv in cnt.iter() do
        result[kv.key] = kv.value
    end
    return result
end

function M.fromJdSequence(cnt)
    local result = { }
    for i = 1, cnt.count do
        result[i] = cnt:get(i)
    end
    return result
end

function M.fromJdContainer(cnt)
    local f, a, b = cnt.iter()
    local kv = f(a, b)
    if kv.key ~= nil and kv.value ~= nil then
        return fromJdTable(cnt)
    end
    return fromJdSequence(cnt)
end

function M.copy(t)
    local result = { }
    for k, v in pairs(t) do
        result[k] = v
    end
    return result
end

function M.deepCopy(t, cpkeys, udatacp, _cp, _done)
    udatacp = udatacp or util.id
    _done = _done or { }
    _cp = _cp or function(x)
        local tx = type(x)
        if tx == 'userdata' then
            if not _done[x] then
                x = udatacp(x)
                _done[x] = true
            end
        elseif tx == 'table' then
            if not _done[x] then
                x = M.deepCopy(x, cpkeys, udatacp, _cp)
                _done[x] = true
            end
        end
        return x
    end
    local result = { }
    setmetatable(result, getmetatable(t))
    for k, v in pairs(t) do
        if cpkeys then
            k = _cp(k)
        end
        result[k] = _cp(v)
    end
    return result
end

function M.clear(t, v)
    for k, _ in pairs(t) do
        t[k] = v
    end
end

M.ERASE_ALL = 0
function M.erase(t, ev, n)
    n = n or 1
    for k, v in pairs(t) do
        if v == ev then
            t[k] = nil
            n = n - 1
            if n == 0 then
                break
            end -- if n == 0
        end -- if v == ev
    end -- for k, v in pairs(t)
end -- function M.erase

function M.listErase(t, ev, n)
    n = n or 1
    for i = 1, #t do
        if t[i] == ev then
            table.remove(t, i)
            n = n - 1
            if n == 0 then
                break
            end -- if n == 0
        end -- if v == ev
    end -- for k, v in pairs(t)
end -- function M.erase

-- Table which silently discards write access.
M.EVER_EMPTY_TABLE = { }
setmetatable(M.EVER_EMPTY_TABLE, {__newindex = function() end})

return M
