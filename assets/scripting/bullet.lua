--bullet

local bullet = {}
function bullet.Update(obj, deltaTime)
    local jumpSpeed = 10
    --when shooting bullet
    if Input_isKeyHeld("SPACE") then
        local x, y = getPosition(obj)
        y = y + jumpSpeed * deltaTime
        setPosition(obj, x, y)
        print(string.format("space pressed, x: %.2f, y: %.2f", x, y))
        SendInputEvent("SPACE", x, y)
    end
end
return bullet