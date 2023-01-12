function prints(value)
    print(value)
    return false
end

function beeps()
    beep()
    return false
end

math.randomseed(os.time())
function maybe(probability)
    return math.random() < probability
end

function scanRegisters(query)
    local registers = {
        RAX = RAX, RBX = RBX, RCX = RCX, RDX = RDX,
        RSI = RSI, RDI = RDI, RBP = RBP, RSP = RSP,
        R8 = R8, R9 = R9, R10 = R10, R11 = R11,
        R12 = R12, R13 = R13, R14 = R14, R15 = R15
    }
    for name, value in pairs(registers) do
        if value == query then
            print("Found value in register " .. name)
            return true
        end
    end
    return false
end

function scanStack(query, maxOffset, stride)
    for offset = 0, maxOffset, stride do
        local value = readQword(ESP + offset)
        if value == query then
            print("Found value at offset " .. hex(offset))
            return true
        end
    end
    return false
end
