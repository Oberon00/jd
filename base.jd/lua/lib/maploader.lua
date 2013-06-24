--[[
    module maploader: loads tilemaps, tilesets and populates them with Entities

    # Where does data come from? #

        * the tilemap file itself (tmx) and the associated tilemap file (tsx;
          this is handled transparently by jd, as if the tsx was directly
          embedded in the tmx)
        * the tile database (data.tiles, ie. lua/data/tiles.lua)
        * the entity modules (lua/entity/<objecttype>.lua)

    The tsx contains just the names of the tiles, so that a mapping
    tile-ID <--> tile-name <--> tile-data can be etablished. Other properties
    are ignored.

    Since the tile-ID <--> tile-name mapping can change on a per-tileset basis,
    it is reloaded for every map.

    # What is done? #

     1. loadMap() only: The tilemap is loaded with loadFromFile (see mapFile()
        for details about the name --> filename mapping).
     2. Using the returned information, especially the 'name' properties of the
        tiles in the tileset, the tile-ID <--> tile-name mapping is established
        (see above).
     3. A TileCollisionGroup and a table (name --> entity) containing
        proxy-entities for each tile-ID where the tiledatabase's entry has
        doSetProxy set to true are set up.
     4. Each tile which is on the map and has its doSubstitute property set to
        true, is replaced by the corresponding object by calling its substitute
        property as a function. However, the tile-ID at the map position isn't
        changed. If this is necessary, it's the responsibility of the ctor.

     5. A table of the following form is returned (the changed result arg (#3 of
        loadMap()) or data arg (#1 of initializeMap())):
        {
            map = the jd.Tilemap passed as/contained in arg#1
            [loadmap only:] name = arg#2: string
            tileProxies = {name = proxy: jd.Entity}
            substituteObjects = {sequence: jd.Entity}
            tileCollisionInfo = jd.TileCollideableInfo
            mapObjects = {groupname = {sequence: jd.Entity}}
            objectColliders = {groupname or # = jd.RectCollideableGroup}
            tileMapping = {byName = {name = id}, byId = {id = name}}
        }
--]]


local tabutil = require 'tabutil'
local Signal = (require 'evt').Signal

local M = { }
local DEFAULT_ANIMATION_SPEED = 3

local tiledata = require 'data.tiles'

local function createTile(name, id, map)
    -- some tiles have multiple names (eg. Fount#1, #2, ...), so pass the name
    -- in order to distinguish them.
    return tiledata[name](name, id, map)
end

local function createObject(objectInfo, layerInfo, mapdata, props)
    if objectInfo.type == '' then
        return nil
    end
    local Entity = require('entity.' .. objectInfo.type)
    return Entity.load(objectInfo, layerInfo, mapdata)
end

local function substituteObject(name, id, position, mapdata, props)
    return tiledata[name].substitute(name, id, position, mapdata, props)
end

local function findTileIdMapping(props)
    local result = {byName = { }, byId = { }}
    for id = 1, props.count do
        local tprops = props:get(id)
        local name = tprops:get 'name'
        if name then
            result.byName[name] = id
            result.byId[id] = name
        end -- if name
    end -- for each tprops in props
    return result
end

local function setupAnimations(props, map)
    for id = 1, props.count do
        local tprops = props:get(id)
        local animationLength = tprops:get 'animationLength'
        if animationLength then
            local animationSpeed =
                tonumber(tprops:get 'animationSpeed') or DEFAULT_ANIMATION_SPEED
            map:addAnimation(id, id + animationLength - 1, animationSpeed)
        end -- if name
    end -- for each tprops in props
end

local function setupProxies(tileMapping, collisionInfo, map)
    local proxies = { }
    for tname, tid in pairs(tileMapping.byName) do
        local tile = tiledata[tname]
        if not tile then
            jd.log.w(("No data for tile '%s' (#%i) available")
                :format(tname, tid))
        elseif tile.doSetProxy then
            local entity, proxy = createTile(tname, tid, map)
            proxies[tname] = entity
            collisionInfo:setProxy(tid, proxy)
        end -- if is no placeholder
    end -- for each tileMapping.byName
    return proxies
end

--[[
    Return a table {groupname or # = {sequence: jd.Entity}} for each
    object in each objectgroup in props.objectGroup and add objects which have
    a PositionComponent to the newly created table
    mapdata.objectColliders[groupname or #].
--]]
local function setupObjects(props, mapdata)
    mapdata.objectColliders = { }
    local objects = { }
    local groups = props.objectGroups
    for kv in groups:iter() do
        local groupInfo = kv.value
        local group = { }
        assert(groupInfo.name == kv.key)
        local hasName = groupInfo.name ~= ''
        local groupId = hasName and groupInfo.name or #objects + 1
        objects[groupId] = group
        local collider = jd.RectCollideableGroup()
        mapdata.objectColliders[groupId] = collider
        for i = 1, groupInfo.objects.count do
            local objectInfo = groupInfo.objects:get(i)
            local obj = createObject(objectInfo, groupInfo, mapdata, props)
            if obj then
                group[i] = obj
                local objPos = obj:component 'PositionComponent'
                if objPos then
                    collider:add(objPos)
                end -- if objPos
            end -- if obj
        end -- for each object in layer
    end -- for each layer
    return objects
end

local function substituteObjects(props, mapdata)
    local objects = { }
    local map = mapdata.map
    local tileMapping = mapdata.tileMapping
    local mapsz = map.size
    local i = 1
    for z = 0, mapsz.z - 1 do
        for x = 0, mapsz.x - 1 do
            for y = 0, mapsz.y - 1 do
                local position = jd.Vec3(x, y, z)
                local tid = map:get(position)
                local tname = tileMapping.byId[tid]
                if tname then
                    local tile = tiledata[tname]
                    if tile and tile.doSubstitute then
                        local obj = substituteObject(
                            tname, tid, position, mapdata, props)
                        if obj then -- skip nil
                            objects[i] = obj
                            i = i + 1
                        end -- if obj
                    end -- if map[x, y, z] is placeholder
                end -- if tname
            end -- for y
        end -- for x
    end -- for z
    return objects
end

local OBJT_LINE = jd.mapInfo.Object.LINE
local OBJT_RECT = jd.mapInfo.Object.RECT

local function getRect(tposOrR, map)
    return tposOrR.wh and
        tposOrR or map:localTileRect(jd.Vec2(pos.x, pos.y))
end

--[[
    Finds all lines in conobjects which end or start in the rect arg#1 or
    the rect of the tile at the tile position arg#1.
    Returns a sequence of the lines themselves and a sequence of all the other
    line ends.
--]]
function M.findConnections(tposOrR, map, conobjects)
    local r = getRect(tposOrR, map)
    local points  = { }
    local objects = { }
    for c in conobjects:iter() do
        if c.objectType == OBJT_LINE then
            local cpoints = c.absolutePoints
            local p1 = cpoints:get(1)
            local p2 = cpoints:get(cpoints.count)
            local otherp = r:contains(p1) and p2 or r:contains(p2) and p1
            if otherp then
                points [#points  + 1] = otherp
                objects[#objects + 1] = jd.mapInfo.Object(c) -- copy c
            end -- if otherp
        end -- if c.objectType == OBJT_LINE
    end -- for each c in conobjects
    return objects, points
end

--[[
    Returns a sequence of all rectangle-objects in tagobjects which intersect
    arg#1 (see findConnections()).
--]]
function M.findTagObjects(tposOrR, map, tagobjects)
    local r = getRect(tposOrR, map)
    local objects = { }
    for t in tagobjects:iter() do
        if t.objectType == OBJT_RECT and t.type == '' then
            local tr = jd.Rect(t.position, t.size)
            if tr:intersection(r) then
                objects[#objects + 1] = jd.mapInfo.Object(t) -- copy t
            end -- if tr:intersection(r)
        end -- if t.objectType == OBJT_RECT
    end -- for each t in tagobjects
    return objects
end

function M.mapFile(name)
    return "maps/" .. name .. ".tmx"
end

--[[
    data (arg#1) must be a table which contains the jd.Tilemap to be
    initialized at the key 'map' and the value returned by its loadFromFile()
    method at the key 'props'.
--]]
function M.initializeMap(data)

    local props = data.props
    assert(props)
    local map = data.map
    assert(map)

    local tileCollisionInfo = jd.TileCollideableInfo(map)

    data.map = map
    local tprops = props.tileProperties
    data.tileMapping = findTileIdMapping(tprops)
    setupAnimations(tprops, map)

    data.tileCollisionInfo = tileCollisionInfo
    tabutil.default(data, 'postLoad', Signal())
    data.tileProxies = setupProxies(
        data.tileMapping, tileCollisionInfo, map)
    data.substituteObjects = substituteObjects(props, data)
    data.mapObjects = setupObjects(props, data)

    data.postLoad(data, props)
    data.postLoad = nil

    return data
end

function M.loadMap(map, name, data)
    data = data or { }
    data.props = map:loadFromFile(M.mapFile(name))
    data.name = name
    data.map = map

    return M.initializeMap(result)
end

return M
