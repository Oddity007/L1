local function hexToInt(h, l)
	local t = {A=10, B=11, C=12, D=13, E=14, F=15}
	for i = 0, 9 do
		t[tostring(i)] = i
	end
	return t[h] * 16 + t[l]
end

local function hexToString(h)
	local t = {}
	for i = 1, #h, 2 do
		local ch = h:sub(i, i)
		local cl = h:sub(i + 1, i + 1)
		t[#t + 1] = string.char(hexToInt(ch, cl))
	end
	return table.concat(t, "")
end

local handlers = {}

function handlers.natural(state, node)
	return {"string", node.value}
end

function handlers.string(state, node)
	return {"string", node.value}
end

function handlers.identifier(state, node, inAssignContext)
	if node.value == "5F5F636F6E737472756374" then
		--"__construct"
		local id = state:genid()
		local arg = state:genany(true)
		return {"closure", arg, {"construct", arg, id}}
	elseif node.value == "5F5F696D706F7274" then
		--"__import"
		local arg = state:genany(true)
		return {"closure", arg, {"import", arg}}
	end
	
	local value = state:lookupBinding(node.value)
	if value then return value
	else error("Access to undefined variable: " .. node.value) end
end

function handlers.call(state, node)
	assert(node.type == "call")
	assert(node.callee)
	assert(node.arguments)
	local callee = state:gen(node.callee)
	for i = 1, #node.arguments do
		callee = {"call", callee, state:gen(node.arguments[i])}
	end
	return callee
end

function handlers.metacall(state, node)
	assert(node.callee)
	assert(node.arguments)
	local callee = state:gen(node.callee)
	for i = 1, #node.arguments do
		local argument = node.arguments[i]
		assert(argument.type == "identifier")
		callee = {"metacall", callee, argument.value}
	end
	return callee
end

function handlers.assignment(state, node)
	state:pushBindingBlock()
	assert(node.destination.type == "identifier")
	assert(node.destination.value)
	
	local source = nil
	if node.arguments then
		source = state:gen({type = "anonymousFunction", arguments = node.arguments, source = node.source})
	else
		source = state:gen(node.source)
	end
	
	state:bind(node.destination.value, source)
	
	local rest = state:gen(node.followingContext)
	
	if node.isMeta then
		rest = {"metabind", rest, node.destination.value, source}
	end
	
	state:popBindingBlock()
	return rest
end

function handlers.list(state, node)
	local elements = {}
	for i = 1, #node.elements do
		elements[i] = state:gen(node.elements[i])
	end
	
	local list = nil
	
	if node.sublist.type == "undefined" then
		list = {"list_empty"}
	else
		list = state:gen(node.sublist)
	end
	
	for i = #node.elements, 1, -1 do
		list = {"list_cons", state:gen(node.elements[i]), list}
	end
	
	return list
end

function handlers.anonymousFunction(state, node)
	assert(node.type == "anonymousFunction")
	assert(node.arguments)
	
	state:pushBindingBlock()
	
	local argBindings = {}
	for i = 1, #node.arguments do
		argBindings[i] = state:gen(node.arguments[i])
	end
	
	local result = state:gen(node.source)
	
	for i = #node.arguments, 1, -1 do
		result = {"closure", argBindings[i], result}
	end
	
	state:popBindingBlock()
	return result
end

function handlers.option(state, node)
	assert(node.type == "option")
	assert(node.construction)
	assert(node.defaultConstruction)
	state:pushBindingBlock()
	local a = state:gen(node.construction)
	state:popBindingBlock()
	state:pushBindingBlock()
	local b = state:gen(node.defaultConstruction)
	state:popBindingBlock()
	return {type = "option", a, b}
end

function handlers.inlineConstraint(state, node)
	assert(node.expression)
	assert(node.constraint)
	local exp = state:gen(node.expression)
	local cons = state:gen(node.constraint)
	return {"constrain", exp, cons}
end

function handlers.constraint(state, node)
	assert(node.expression)
	assert(node.constraint)
	assert(node.followingContext)
	
	local exp = state:gen(node.expression)
	local cons = state:gen(node.constraint)
	local src = state:gen(node.followingContext)
	
	return {"call", {"closure", cons, src}, exp}
end

function handlers.undefined(state, node)
	local dst = state:genid()
	return {"undefined"}
end

function handlers.any(state, node)
	assert(node.source)
	assert(node.source.type == "identifier")
	assert(node.source.value)
	local value = state:genany(true)
	state:bind(node.source.value, value)
	return value
end

function handlers.metasymbol(state, node)
	error("Invalid metasymbol occurence")
end

local function Generate(rootNode)
	local state = {id = 0, bindingDepth = 0}
	function state:genid()
		local id = self.id
		self.id = id + 1
		return id
	end
	function state:genany(tagged)
		local id = nil
		if tagged then
			id = state:genid()
		end
		return {"any", id}
	end
	function state:gen(node)
		--print(node.type)
		return handlers[node.type](state, node)
	end
	function state:pushBindingBlock()
		self.bindingDepth = self.bindingDepth + 1
		self.bindingBlock = {bindings = {}, previous = self.bindingBlock}
	end
	function state:popBindingBlock()
		self.bindingDepth = self.bindingDepth - 1
		assert(self.bindingDepth >= 0)
		assert(self.bindingBlock)
		--[[for k, _ in pairs(self.bindingBlock.bindings) do
			print("Discarding " .. k)
		end]]
		self.bindingBlock = self.bindingBlock.previous
	end
	function state:bind(identifier, id)
		assert(id)
		assert(self.bindingBlock)
		--assert(not self.bindingBlock.bindings[identifier])
		self.bindingBlock.bindings[identifier] = id
		--print("Binding " .. identifier)
	end
	function state:lookupBinding(identifier)
		local b = self.bindingBlock
		--print("Looking for " .. identifier)
		while b do
			local id = b.bindings[identifier]
			--[[for k, _ in pairs(b.bindings) do
				print("Found " .. k)
			end]]
			if id then return id end
			b = b.previous
		end
		--error("Problem")
		return nil
	end
	local destination = state:gen(rootNode)
	return destination
end

return Generate
