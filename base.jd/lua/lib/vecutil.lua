local M = { }

function M.surroundingPositions8(p)
    -- 123
    -- 4p5
    -- 678
    return {
        jd.Vec3(p.x - 1, p.y - 1, p.z), -- 1
        jd.Vec3(p.x,     p.y - 1, p.z), -- 2
        jd.Vec3(p.x + 1, p.y - 1, p.z), -- 3
        jd.Vec3(p.x - 1, p.y,     p.z), -- 4
        jd.Vec3(p.x + 1, p.y,     p.z), -- 5
        jd.Vec3(p.x - 1, p.y + 1, p.z), -- 6
        jd.Vec3(p.x,     p.y + 1, p.z), -- 7
        jd.Vec3(p.x + 1, p.y + 1, p.z)  -- 8
    }
end

function M.surroundingPositions4(p)
    --  1
    -- 2p3
    --  4
    return {
        jd.Vec3(p.x,     p.y - 1, p.z), -- 1
        jd.Vec3(p.x - 1, p.y,     p.z), -- 2
        jd.Vec3(p.x + 1, p.y,     p.z), -- 3
        jd.Vec3(p.x,     p.y + 1, p.z), -- 4
    }
end

return M
