local M = { }

jd.createDirectory("lua/data")
function M.store(id, ...)
    jd.writeString("lua/data/" .. id .. ".lua", jd.serialize(...))
end

function M.unload(id)
    package.loaded['data.' .. id] = nil
end

jd.createDirectory("pst")
function M.zstore(id, ...)
    jd.writeString("pst/" .. id, jd.compress(jd.serialize(...)))
end

function M.zload(id)
    local chunk, err = load(jd.uncompress(jd.readString("pst/" .. id)))
    if not chunk then
        error("persistence.zload failed: " .. err)
    end
    return chunk()
end

return M
