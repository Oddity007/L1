local simpleSingleCharacterTokens =
{
	["("] = "opening parenthesis",
	[")"] = "closing parenthesis",
	["["] = "opening square bracket",
	["]"] = "closing square bracket",
	["$"] = "eval",
	["%"] = "modulo",
	["+"] = "addition",
	["-"] = "subtraction",
	["*"] = "multiplication",
	[";"] = "terminal",
	[","] = "comma",
}

local reservedCharacters = {["("] = true, [")"] = true, ["["] = true, ["]"] = true, ["$"] = true, ["%"] = true, ["+"] = true, ["-"] = true, ["*"] = true, ["/"] = true, [";"] = true, ["<"] = true, ["="] = true, [">"] = true, [","] = true, ["\""] = true, ["?"] = true, ["."] = true, ["\\"] = true, ["\0"] = true}

local function isDigit(character)
	--[[local digits = {"0" = true, "1" = true, "2" = true, "3" = true, "4" = true, "5" = true, "6" = true, "7" = true, "8" = true, "9" = true}
	return digits[character] or false]]
	if character == nil then return false end
	return character >= "0" and character <= "9"
end

local spaceCharacters = {[" "] = true, ["\t"] = true, ["\r"] = true, ["\n"] = true}

local function Lex(characters, state)
	repeat
		if characters[state.i] == "\0" then
			state.i = state.i + 1
			return "done"
		elseif characters[state.i] == nil then
			state.i = state.i + 1
			return "done"
		elseif spaceCharacters[characters[state.i]] then
			if characters[state.i] == "\n" then state.l = state.l + 1 end
			state.i = state.i + 1
		elseif simpleSingleCharacterTokens[characters[state.i]] then
			state.i = state.i + 1
			return simpleSingleCharacterTokens[characters[state.i - 1]]
		elseif characters[state.i] == "?" then
			state.i = state.i + 1
			if characters[state.i] == "?" then
				state.i = state.i + 1
				return "double question mark"
			else
				return "single question mark"
			end
		elseif characters[state.i] == "." then
			if characters[state.i + 1] == "." and characters[state.i + 2] == "." then
				state.i = state.i + 3
				return "ellipsis"
			else
				return nil, nil, "invalid token"
			end
		elseif characters[state.i] == "/" then
			state.i = state.i + 1
			if characters[state.i] == "/" then
				repeat
					state.i = state.i + 1
				until characters[state.i] == "\n" or characters[state.i] == "\0" or characters[state.i] == nil
				if characters[state.i] == "\n" then state.l = state.l + 1 end
				state.i = state.i + 1
			elseif characters[state.i] == "*" then
				repeat
					state.i = state.i + 1
					if characters[state.i] == "\n" then state.l = state.l + 1 end
				until (characters[state.i] == "*" and characters[state.i + 1] == "/") or characters[state.i] == "\0" or characters[state.i] == nil
				state.i = state.i + 2
			else
				return "division"
			end
		elseif characters[state.i] == "=" then
			state.i = state.i + 1
			if characters[state.i] == "=" then
				state.i = state.i + 1
				return "equality"
			elseif characters[state.i] == ">" then
				state.i = state.i + 1
				return "yield"
			else
				return "assign"
			end
		elseif characters[state.i] == "<" then
			state.i = state.i + 1
			if characters[state.i] == "=" then
				state.i = state.i + 1
				return "lesser than or equal to"
			else
				return "lesser than"
			end
		elseif characters[state.i] == ">" then
			state.i = state.i + 1
			if characters[state.i] == "=" then
				state.i = state.i + 1
				return "greater than or equal to"
			else
				return "greater than"
			end
		elseif characters[state.i] == "\"" then
			local string = ""
			state.i = state.i + 1
			repeat
				if characters[state.i] == "\\" then
					state.i = state.i + 1
					if characters[state.i] == "\\" then
						string = string .. "\\"
					elseif characters[state.i] == "\"" then
						string = string .. "\""
					elseif characters[state.i] == "n" then
						string = string .. "\n"
					elseif characters[state.i] == "r" then
						string = string .. "\r"
					elseif characters[state.i] == "t" then
						string = string .. "\t"
					elseif characters[state.i] == "u" then
						return nil, nil, "not yet implemented"
					else
						return nil, nil, "invalid escape sequence in string"
					end
					state.i = state.i + 1
				else
					if characters[state.i] == "\""  then
						--and characters[state.i - 1] ~= "\\"
						state.i = state.i + 1
						break
					else
						if characters[state.i] == "\n" then state.l = state.l + 1 end
						string = string .. characters[state.i]
						state.i = state.i + 1
					end
				end
			until false
			return "string", string
		elseif isDigit(characters[state.i]) then
			local digits = characters[state.i]
			repeat
				state.i = state.i + 1
				if isDigit(characters[state.i]) then
					digits = digits .. characters[state.i]
				elseif characters[state.i] == "_" then
					--nothing, we skip it
				else
					return "number", digits
				end
			until false
		elseif not isDigit(characters[state.i]) and not reservedCharacters[characters[state.i]] then
			local identifier = ""
			repeat
				identifier = identifier .. characters[state.i]
				state.i = state.i + 1
			until characters[state.i] == nil or isDigit(characters[state.i]) or reservedCharacters[characters[state.i]] or spaceCharacters[characters[state.i]]
			return "identifier", identifier
		end
	until false
end

return function(input)
	local characters = {}
	--for _, v in input:gmatch("[\0-\127\192-\255]+[\128-\191]*") do
	for v in input:gmatch("[%z\1-\127\194-\244][\128-\191]*") do
		--print(v)
		characters[#characters + 1] = v
	end
	local state = {i = 1, l = 1}
	local tokens = {}
	repeat
		local tokenType, tokenData, errorType = Lex(characters, state)
		if errorType then
			return nil, errorType, state.l
		else
			local token = {type = tokenType, data = tokenData, characterIndex = state.i, lineNumber = state.l}
			tokens[#tokens + 1] = token
		end
	until tokenType == "done"
	return tokens
end