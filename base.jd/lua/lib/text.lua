local M = { }

M.defaultFont = jd.Font.request(jd.conf.misc.defaultFont)
M.defaultLayer = jd.drawService:layer(3)

function M.create(s, p, layer, font)
    if type(font) == 'string' then
        font = jd.Font.request(font)
    else
        font = font or M.defaultFont
    end
    layer = layer or M.defaultLayer.group
    p = p or jd.Vec2()
    local text = jd.Text(layer)
    text.font = font
    text.string = s
    text.position = p
    return text
end

local function calculateCenterP(t, layer)
    layer = layer or M.defaultLayer
    local r = t.bounds
    local d = r.position - t.position
    r.center = layer.view.center
    return r.position - d
end

function M.center(t, layer)
    t.position = calculateCenterP(t, layer)
end

function M.centerX(t, layer)
    t.position = jd.Vec2(calculateCenterP(t, layer).x, t.position.y)
end

function M.centerY(t, layer)
    t.position = jd.Vec2(t.position.x, calculateCenterP(t, layer).y)
end


return M
