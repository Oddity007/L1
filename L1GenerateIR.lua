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
	assert(node.type == "natural")
	assert(node.value)
	local destination = state:genid()
	state:output {type = "natural", destination = destination, data = node.value}
	return destination
end

function handlers.string(state, node)
	assert(node.type == "string")
	assert(node.value)
	local destination = state:genid()
	state:output {type = "string", destination = destination, data = node.value}
	return destination
end

function handlers.identifier(state, node, inAssignContext)
	assert(node.type == "identifier")
	assert(node.value)
	local dst = nil
	if inAssignContext then
		dst = state:genid()
		state:bind(node.value, dst)
	else
		dst = state:lookupBinding(node.value)
		if not dst then error("Undefined variable access to " .. node.value) end
	end
	assert(dst)
	return dst
end

function handlers.call(state, node, inAssignContext)
	assert(node.type == "call")
	assert(node.callee)
	assert(node.arguments)
	local callee = state:gen(node.callee, inAssignContext)
	for i = 1, #node.arguments do
		local argument = state:gen(node.arguments[i], inAssignContext)
		local result = state:genid()
		state:output {type = "call", destination = result, callee = callee, argument = argument}
		callee = result
	end
	return callee
end

function handlers.constructorConstraint(state, node, inAssignContext)
	assert(node.type == "constructorConstraint")
	assert(node.expression)
	assert(node.construction)
	local dst = state:genid()
	local exp = state:gen(node.expression, inAssignContext)
	local cons = state:gen(node.construction, inAssignContext)
	state:output {type = "construction", destination = dst, expression = exp, construction = cons}
	return dst
end

function handlers.branch(state, node, inAssignContext)
	assert(node.type == "branch")
	assert(node.condition)
	assert(node.resultIfTrue)
	assert(node.resultIfFalse)
	assert(not inAssignContext)
	local cnd = state:gen(node.condition)
	local rtt = state:gen(node.resultIfTrue)
	local rtf = state:gen(node.resultIfFalse)
	local dst = state:genid()
	state:output {type = "branch", destination = dst, condition = cnd, resultIfTrue = rtt, resultIfFalse = rtf}
	return dst
end

function handlers.option(state, node, inAssignContext)
	assert(node.type == "option")
	assert(node.construction)
	assert(node.defaultConstruction)
	assert(not inAssignContext)
	local a = state:gen(node.construction, false)
	local b = state:gen(node.defaultConstruction, false)
	local dst = state:genid()
	state:output {type = "option", destination = dst, construction = a, defaultConstruction = b}
	return dst
end

function handlers.anonymousFunction(state, node, inAssignContext)
	assert(not inAssignContext)
	assert(node.type == "anonymousFunction")
	assert(node.arguments)
	state:pushBindingBlock()
	local argBindings = {}
	for i = 1, #node.arguments do
		argBindings[i] = state:gen(node.arguments[i], true)
	end
	local result = nil
	if node.isConstructor then
		result = state:genid()
		state:output {type = "construct", source = state:gen(node.source, false), destination = result}
	else
		result = state:gen(node.source, false)
	end
	for i = #node.arguments, 1, -1 do
		local dst = state:genid()
		state:output {type = "closure", destination = dst, argument = argBindings[i], result = result}
		result = dst
	end
	state:popBindingBlock()
	return result
end

function handlers.assignment(state, node, inAssignContext)
	assert(not inAssignContext)
	state:pushBindingBlock()
	local dst = state:gen(node.destination, true)
	local src = state:gen(node.source, false)
	state:output {type = node.isConstructor and "construct" or "overload", source = src, destination = dst}
	local rst = state:gen(node.followingContext, false)
	state:popBindingBlock()
	return rst
end

function handlers.undefined(state, node, inAssignContext)
	local dst = state:genid()
	state:output {type = "undefined", destination = dst}
	return dst
end

function handlers.eval(state, node, inAssignContext)
	return state:gen(node.expression, not inAssignContext)
end

function handlers.list(state, node, inAssignContext)
	local rst = state:genid()
	state:output {type = "list_empty", destination = rst}
	for i = 1, #node.elements do
		local dst = state:genid()
		local val = state:gen(node.elements[i], inAssignContext)
		state:output {type = "list_prepend", destination = dst, head = val, tail = rst}
		rst = dst
	end
	if node.sublist.type ~= "undefined" then
		local dst = state:genid()
		local back = state:gen(node.sublist, inAssignContext)
		state:output {type = "list_merge", destination = dst, front = rst, back = back}
		rst = dst
	end
	return rst
end

local function GenerateRough(rootNode)
	local state = {ops = {}, id = 0, bindingDepth = 0}
	function state:output(op)
		self.ops[#self.ops + 1] = op
	end
	function state:genid()
		local id = self.id
		self.id = id + 1
		return id
	end
	function state:gen(node, inAssignContext)
		return handlers[node.type](state, node, inAssignContext)
	end
	function state:pushBindingBlock()
		self.bindingDepth = self.bindingDepth + 1
		self.bindingBlock = {bindings = {}, previous = self.bindingBlock}
	end
	function state:popBindingBlock()
		self.bindingDepth = self.bindingDepth - 1
		assert(self.bindingDepth >= 0)
		assert(self.bindingBlock)
		for k, _ in pairs(self.bindingBlock.bindings) do
			print("Discarding " .. k)
		end
		self.bindingBlock = self.bindingBlock.previous
	end
	function state:bind(identifier, id)
		assert(id)
		assert(self.bindingBlock)
		assert(not self.bindingBlock.bindings[identifier])
		self.bindingBlock.bindings[identifier] = id
		print("Binding " .. identifier)
	end
	function state:lookupBinding(identifier)
		local b = self.bindingBlock
		print("Looking for " .. identifier)
		while b do
			local id = b.bindings[identifier]
			for k, _ in pairs(b.bindings) do
				print("Found " .. k)
			end
			if id then return id end
			b = b.previous
		end
		error("Problem")
		return nil
	end
	local destination = state:gen(rootNode)
	return destination, state.ops
end

return GenerateRough
