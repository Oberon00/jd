--[[
    module evt

    Contains various helper functions and types for connecting events.
--]]

local tabutil = require 'tabutil'
local util = require 'util'
local oo = require 'oo'

local M = { }

do
    local persistedEvts = { } -- Shields connections from garbage collection

    function M.persistConnection(ev)
        persistedEvts[#persistedEvts + 1] = ev
    end -- function M.persistConnection
end -- do (persistConnection)

function M.connectForever(...)
    M.persistConnection(jd.connect(...))
end


--[[
    Functions connectToKeyPress, connectToKeyRelease:
        Arguments:
            key: The keycode (jd.kb.*) at which f is called (depending on the
                 connection-function either if the key is pressed or released)
            f:   A callback which is called with the key event (as emitted by
                 jd) as arg#1 and the string 'keyPressed' or 'keyReleased' as
                 arg#2.
                 nil is valid and will disconnect the callback for the
                 specified key.
        Note: If there is already a function connected to key and the same event
              (keyPressed or keyReleased), it is overriden and a warning is
              printed to the logfile.
--]]
do
    local callbacks = { keyPressed = false, keyReleased = false }
    local function connectToKeyEvent(event, key, f)
        if not callbacks[event] then
            if not f then
                jd.log.w ("Attempt to disconnect a key event"
                          .. " when none was registered.")
                return
            end
            M.connectForever(jd.eventDispatcher, event, function(keyEvt)
                util.callopt(callbacks[event][keyEvt.code], keyEvt, event)
            end)
            callbacks[event] = { }
        elseif f and callbacks[event][key] then
            jd.log.w(("%s event: overriding key %s"):format(
                event, jd.kb.keyName(key)))
        end
        callbacks[event][key] = f
    end -- local function connectToKeyEvent

    function M.connectToKeyPress(key, f)
        connectToKeyEvent('keyPressed', key, f)
    end
    function M.connectToKeyRelease(...)
        connectToKeyEvent('keyReleased', key, f)
    end
end -- do (connectToKeyPress, connectToKeyRelease)

lclass('Table', M)

    function M.Table:__init(evt)
        self.evts = { }
        if (evt) then
            self:add(evt)
        end
    end

    function M.Table:__gc()
        self:disconnect()
    end

    function M.Table:add(evt)
        if type(evt) == 'table' and not evt.disconnect then
            for _, v in pairs(evt) do
                self:add(v)
            end
        else
            assert(evt.disconnect, "evt (arg#1) has no disconnect method")
            self.evts[#self.evts + 1] = evt
        end
    end

    function M.Table:connect(...)
        self:add(jd.connect(...))
    end

    function M.Table:disconnect()
        if not jd then
            return
        end

        for k, v in pairs(self.evts) do
            if getmetatable(v) then
                local connected = v.isConnected
                if type(connected) == "function" then
                    connected = connected()
                end
                if connected then
                    v:disconnect()
                end
            end
            self.evts[k] = nil
        end
        self.evts = { }
    end

local Connection = oo.cppclass('Connection', oo.NIL_ENV, jd.ConnectionBase)

lclass('Signal', M)

    local function erase(self, i)
        local slotCount = #self
        if i == slotCount then
            self[i] = nil
        else
            self[i] = self[slotCount]
            self[slotCount] = nil
        end
    end

    local function disconnectHint(self, f, hint)
        if self[i] == f then
            erase(self, i)
            return true
        end
        return self:disconnect(f)
    end

    --[[
        Call all connected slots with the passed parameters. Note that the order
        in which the slots are called is undefined.
    --]]
    function M.Signal:__call(...)
        for i = 1, #self do
            self[i](...)
        end
    end

    --[[
        Call, collecting the return values of the signals.
        This function will return a table with one table per connected function
        (slot) each obtained by packing the return value of the slot with
        table.pack(): { table.pack(slot1(...)), table.pack(slot2(...)), ... }

        Compared to the __call metamethod, this function is very slow, so you
        should use it only if you really need the return values.
    --]]
    function M.Signal:callR(...)
        local result = { }
        for i = 1, #self do
            result[i] = table.pack(self[i](...))
        end
        return result
    end

    function M.Signal:connect(f)
        local i = #self + 1
        self[i] = f
        return Connection(self, f, i)
    end

    function M.Signal:clear()
        for i = 1, #self do
            self[i] = nil
        end
    end

    --[[
        Returns true if f was disconnected and false otherwise (if f is not
        connected).
    --]]
    function M.Signal:disconnect(f)
        local slotCount = #self
        for i = 1, slotCount do
            if self[i] == f then
                erase(self, i)
                return true
            end
        end
        return false
    end

    function M.Signal.create(...)
        local names = {...}
        local result = { }
        for i = 1, #names do
            result[names[i]] = M.Signal()
        end
        return result
    end

    function M.Signal.createInComponent(component, ...)
        local result = M.Signal.create(...)
        component._signals = result
        return result
    end

function Connection:__init(sig, f, hint)
    jd.ConnectionBase.__init(self)
    assert(sig and f and hint)
    self.sig, self.f, self.hint = sig, f, hint
end

function Connection:getIsConnected()
    return not not self.sig
end

function Connection:disconnect()
    local success = disconnectHint(self.sig, self.f, self.hint)
    assert(success)
    self.sig, self.f, self.hint = nil, nil, nil
end



function jd.callback_connectLuaEvent(component, name, receiver)
    if type(component._signals) == 'table' then
        local sig = component._signals[name]
        if sig then
            return sig:connect(receiver)
        end
    end
    return nil
end

return M
