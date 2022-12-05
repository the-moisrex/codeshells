name = "Mohammad"
io.write("Size of string ", #name, "\n");
print("Size of string", #name);


bigNum = 999999999999999999999 + 1
print("Big number: ", type(bigNum))


logString = [[
-----------
I am a very ver long string
that goes on
  for ever
-----------
]]

logString = logString .. "Said: " .. name;
print(logString)

AmICool = true;
print(type(AmICool));
print(type(MadeUpVariable))

print("Math is easy:", (5 * 3))
print("Modulus operator:", (5.2 % 3))

print("Random: ", math.random())
math.randomseed(os.time());
print("Random between 1-10: ", math.random(10))
math.randomseed(1)
print("Random between 50-100: ", math.random(50, 100))
math.randomseed(os.time());


print(string.format("PI: %.10f", math.pi))


age = 13
if age < 16 then
    io.write("You can go to school", "\n")
    local localVar = 10
elseif age <= 16 and age < 18 then
    io.write("You can drive", "\n")
else
    io.write("You can vote", "\n")
end

print(localVar);


print(string.format("not true: %s", tostring(not true)))

-- this is a comment
--
--

canvote = age > 18 and true or false
io.write("can i vote: ", tostring(canvote), "\n")


io.write(string.format("Len: %s", string.len(name)), "\n")
io.write(string.format("Len: %s", #name), "\n")
print("name: ", name)
print("Replace I with me: ", string.gsub(name, "Mo", "No")) -- what is this 1 ?? Is it the index?
io.write("Replace I with me: ", string.gsub(name, "Mo", "No"), "\n")
print("cool")
io.write("Index of d: ", string.find(name, "d"), "\n")
io.write(string.upper(name), "\n")
io.write(string.lower(name), "\n")


i = 1
while i <= 10 do
    io.write(i)
    i = i + 1
    
    if i == 8 then
        break
    end
    -- There's no "continue" statement
end
print()


repeat
    io.write("Enter your guess: ")
    -- guess = io.read()
    guess = 15
until tonumber(guess) == 15
print()


for i = 1, 10, 1 do
    io.write(i)
end
print()


month = {
    "Jan", "Feb", "Match", "April"
}

for key, value in pairs(month) do
    io.write(key, " ", value, "\n")
end


aTable = {}

for i = 0, 10 do
    aTable[i] = i
end

io.write("First: ", aTable[0], "\n")
io.write("First: ", aTable[1], "\n")
io.write("Length: ", #aTable, "\n")
table.insert(aTable, 1, 43434) -- starts from 1
table.insert(aTable, #aTable + 1, 43434)
io.write("Length: ", #aTable, "\n")
print(table.concat(aTable, ", "))
table.remove(aTable, 1)
print(table.concat(aTable, ", "))


aMulTab = {}
for i = 0, 9 do
    aMulTab[i] = {}
    for j = 0, 9 do
        aMulTab[i][j] = tostring(i) .. tostring(j)
    end 
end

for key, value in pairs(aMulTab) do
    print(table.concat(aMulTab[key], ", "))
end


function getSum(one, two)
    return one + two
end

print(string.format("5 + 2 = %d", getSum(5, 2)))


function splitStr(theString)
    stringTable = {}
    local i = 1

    for str in string.gmatch(theString, "[^%s]+") do
        stringTable[i] = str
        i = i + 1
    end
    return stringTable, i -- the total number of words matched
end

strTable, count = splitStr("The tuttle is cute.")

for j = 1, count - 1 do 
    print(string.format("%d: %s", j, stringTable[j]));
end


--- https://dl.git.ir/cdn7/1397/video/learn-in-one-video/Lua%20Tutorial.mp4
-- Time in the video: 32:53
--





