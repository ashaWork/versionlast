--player(leave)

--for jumping
local isJumping = false
local velocityY = 0
local gravity = -9.8
local jumpForce = 5.0
local floorY = 0.0  

local player = {}
function player.Update(obj, deltaTime)
    local speed = 5
    local x, y = getPosition(obj)
    --for going up
    if Input_isKeyHeld("W") then
        y = y + speed * deltaTime
        setPosition(obj, x, y)
        print(string.format("input W, x: %.2f, y: %.2f", x, y))
        SendInputEvent("W", x, y)
    end
    --for going down
    if Input_isKeyHeld("S") then
        y = y - speed * deltaTime
        setPosition(obj, x, y)
        print(string.format("input S, x: %.2f, y: %.2f", x, y))
        SendInputEvent("S", x, y)
    end
    --for going left
    if Input_isKeyHeld("A") then
        x = x - speed * deltaTime
        setPosition(obj, x, y)
        print(string.format("input A, x: %.2f, y: %.2f", x, y))
        SendInputEvent("A", x, y)
    end
    --for going right
    if Input_isKeyHeld("D") then
        x = x + speed * deltaTime
        setPosition(obj, x, y)
        print(string.format("input D, x: %.2f, y: %.2f", x, y))
        SendInputEvent("D", x, y)
    end
    --for jumping
    if Input_isKeyHeld("B") and not isJumping then
        isJumping = true
        velocityY = jumpForce
        print(string.format("input B, x: %.2f, y: %.2f", x, y))
        SendInputEvent("B", x, y)
    end
    --check for landing
    if isJumping then
        velocityY = velocityY + gravity * deltaTime
        y = y + velocityY * deltaTime
        -- stop at floor
        if y <= floorY then
            y = floorY
            velocityY = 0
            isJumping = false
            print("Landed!")
        end
        setPosition(obj, x, y)
    end
end
return player