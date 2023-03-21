function reloadPackage(path)
    package.loaded[path] = nil
    return require(path)
end

dofile("lua/debuggerscripts.lua")

local json = reloadPackage("lua/json")

function pprint(value)
    print(json.encode(value))
end

function hex(x)
    return string.format("%x", x):upper()
end

function clearConsole()
    GetLuaEngine().MenuItem5.doClick()
end

function getCWD()
    return io.popen "cd":read '*l'
end

function checkSymbol(symbol)
    if not pcall(function() getAddress(symbol) end) then return false end
    local addr = getAddress(symbol)
    if addr == nil then return false end
    local value = readInteger(addr)
    if value == nil then return false end
    return true
end

function tryInjectDll(memrec, dllBaseName)
    dllBaseName = dllBaseName or memrec.Description
    local dllName = dllBaseName .. ".dll"
    local dllPath = getCWD() .. "\\ModDLL\\x64\\Debug\\" .. dllName
    if checkSymbol(dllName) then
        beep()
    else
        injectDLL(dllPath)
    end
    createTimer(1, function() memrec.Active = false end)
end

function reopenProcess(memrec)
    openProcess(getOpenedProcessID())
    createTimer(1, function() memrec.Active = false end)
end
