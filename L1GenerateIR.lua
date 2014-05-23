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

local function intToHex(i)
	return nil
end

local function MatchPattern(state, node, sourceid, isDestinationOfAssignment)
	assert(node)
	assert(sourceid)
	if node.type == "natural" then
		local exp = state:gen(node)
		local dst = state:genid()
		state:output {type = "constrain", destination = dst, expression = exp, constraint = sourceid}
		return dst
	elseif node.type == "string" then
		local exp = state:gen(node)
		local dst = state:genid()
		state:output {type = "constrain", destination = dst, expression = exp, constraint = sourceid}
		return dst
	elseif node.type == "identifier" then
		assert(node.value)
		if isDestinationOfAssignment then
			local dst, isForwardDeclared = state:lookupBinding(node.value)
			if isForwardDeclared then
				state:overload(dst, sourceid)
				return dst
			end
		end
		state:bind(node.value, sourceid)
		return sourceid
	elseif node.type == "eval" then
		assert(node.source)
		local exp = state:gen(node.source)
		local dst = state:genid()
		state:output {type = "constrain", destination = dst, expression = exp, constraint = sourceid}
		return dst
	elseif node.type == "call" then
		local exp = state:gen(node)
		local dst = state:genid()
		state:output {type = "constrain", destination = dst, expression = exp, constraint = sourceid}
		return dst
		--[[local callee = state:genid()
		MatchPattern(state, node.callee, callee)
		for i = 1, #node.arguments do
			local result = state:genid()
			local argument = state:genid()
			MatchPattern(state, node.arguments[i], argument)
			state:output {type = "pattern_match_call", destination = result, callee = callee, argument = argument}
			callee = result
		end
		local dst = state:genid()
		state:output {type = "constrain", destination = dst, expression = callee, constraint = sourceid}
		return dst]]
	elseif node.type == "list" then
		local exp = state:gen(node)
		local dst = state:genid()
		state:output {type = "constrain", destination = dst, expression = exp, constraint = sourceid}
		return dst
		--[[local tail = sourceid
		for i = 1, #node.elements do
			local head = state:genid()
			local newtail = state:genid()
			state:output {type = "list_head", destination = head, source = tail}
			state:output {type = "list_tail", destination = newtail, source = tail}
			MatchPattern(state, node.elements[i], head)
			tail = newtail
		end
		
		if node.sublist.type == "undefined" then
			local emptylistid = state:genid()
			state:output {type = "list_empty", destination = emptylistid}
			local dst = state:genid()
			state:output {type = "constrain", destination = dst, expression = tail, constraint = emptylistid}
		else
			MatchPattern(state, node.sublist, tail)
		end
		return sourceid]]
	elseif node.type == "anonymousFunction" then
		local cons = state:gen(node)
		local dst = state:genid()
		state:output {type = "constrain", destination = dst, expression = sourceid, constraint = cons}
		return dst
	elseif node.type == "inlineConstraint" then
		local exp = MatchPattern(state, node.expression, sourceid)
		local cons = MatchPattern(state, node.constraint, sourceid)
		local dst = state:genid()
		state:output {type = "constrain", destination = dst, expression = exp, construction = cons}
		return dst
	else
		error("NYI")
	end
end

local handlers = {}

function handlers.natural(state, node)
	local destination = state:genid()
	state:output {type = "natural", destination = destination, data = node.value}
	return destination
end

function handlers.string(state, node)
	local destination = state:genid()
	state:output {type = "string", destination = destination, data = node.value}
	return destination
end

function handlers.identifier(state, node, inAssignContext)
	local dst, isForwardDeclared = state:lookupBinding(node.value)
	if not dst then error("Undefined variable access to " .. node.value) end
	assert(dst)
	return dst
end

function handlers.call(state, node, inAssignContext)
	local callee = state:gen(node.callee, inAssignContext)
	for i = 1, #node.arguments do
		local argument = state:gen(node.arguments[i], inAssignContext)
		local result = state:genid()
		state:output {type = "call", destination = result, callee = callee, argument = argument}
		callee = result
	end
	return callee
end

function handlers.metacall(state, node, inAssignContext)
	local callee = state:gen(node.callee, inAssignContext)
	for i = 1, #node.arguments do
		local result = state:genid()
		assert(node.arguments[i].type == "identifier")
		state:output {type = "metacall", destination = result, callee = callee, key = node.arguments[i].value}
		callee = result
	end
	return callee
end

function handlers.constraint(state, node, inAssignContext)
	local exp = state:gen(node.expression, inAssignContext)
	local cons = state:gen(node.constraint, inAssignContext)
	local rst = state:gen(node.followingContext, inAssignContext)
	local dst = state:genid()
	state:output {type = "constrain", destination = dst, expression = exp, constraint = cons}
	return rst
end

function handlers.inlineConstraint(state, node, inAssignContext)
	local exp = state:gen(node.expression, inAssignContext)
	local cons = state:gen(node.constraint, inAssignContext)
	local dst = state:genid()
	state:output {type = "constrain", destination = dst, expression = exp, construction = cons}
	return dst
end

function handlers.option(state, node, inAssignContext)
	local a = state:gen(node.construction, false)
	local b = state:gen(node.defaultConstruction, false)
	local dst = state:genid()
	state:output {type = "option", destination = dst, construction = a, defaultConstruction = b}
	return dst
end

function handlers.anonymousFunction(state, node, inAssignContext)
	state:pushBindingBlock()
	
	local result = nil
	
	if 0 == #node.arguments then
		local dst = state:genid()
		result = state:gen(node.source)
		state:output {type = "delay", destination = dst, result = result}
		return dst
	else
		local argBindings = {}
		for i = 1, #node.arguments do
			argBindings[i] = state:genid()
			assert(node.arguments[i])
			MatchPattern(state, node.arguments[i], argBindings[i])
		end
		
		result = state:gen(node.source)
		
		for i = #node.arguments, 1, -1 do
			local dst = state:genid()
			state:output {type = "closure", destination = dst, argument = argBindings[i], result = result}
			result = dst
		end
	end
	
	state:popBindingBlock()
	return result
end

function handlers.assignment(state, node, inAssignContext)
	assert(not inAssignContext)
	state:pushBindingBlock()
	local src = nil
	if node.arguments and 0 ~= #node.arguments then
		src = state:gen({type = "anonymousFunction", arguments = node.arguments, source = node.source}, false)
	else
		src = state:gen(node.source, false)
	end
	state:popBindingBlock()
	
	state:pushBindingBlock()
	MatchPattern(state, node.destination, src)
	--assert(node.destination.type == "identifier")
	--state:bind(node.destination.value, src)
	local rst = state:gen(node.followingContext, false)
	state:popBindingBlock()
	
	if node.isMeta then
		assert(node.destination.type == "identifier")
		local dst = state:genid()
		state:output {type = "metabind", destination = dst, expression = rst, key = node.destination.value, value = src}
		rst = dst
	end
	
	return rst
end

function handlers.undefined(state, node, inAssignContext)
	local dst = state:genid()
	state:output {type = "undefined", destination = dst}
	return dst
end

function handlers.declare(state, node, inAssignContext)
	assert(node.destination.type == "identifier")
	
	state:pushBindingBlock()
	local dst = state:genid()
	state:output {type = "declare", destination = dst}
	state:bind(node.destination.value, dst, true)
	local rst = state:gen(node.followingContext)
	state:popBindingBlock()
	
	if node.isMeta then
		local newdst = state:genid()
		state:output {type = "metabind", destination = newdst, expression = rst, key = node.destination.value, value = dst}
		rst = newdst
	end
	return rst
end

function handlers.eval(state, node, inAssignContext)
	error("Evals found in non-assignment context")
end

function handlers.any(state, node)
	local dst = state:genid()
	state:output {type = "any", destination = dst}
	state:bind(node.source.value, dst)
	return dst
end

function handlers.construct(state, node)
	local dst = state:genid()
	state:output {type = "construct", destination = dst, source = state:gen(node.source)}
	return dst
end

function handlers.list(state, node, inAssignContext)
	local rst = nil
	if node.sublist.type == "undefined" then
		rst = state:genid()
		state:output {type = "list_empty", destination = rst}
	else
		rst = state:gen(node.sublist, inAssignContext)
	end
	
	for i = 1, #node.elements do
		local dst = state:genid()
		local val = state:gen(node.elements[i], inAssignContext)
		state:output {type = "list_cons", destination = dst, head = val, tail = rst}
		rst = dst
	end
	return rst
end

local function Generate(rootNode)
	local state = {ops = {}, id = 0, overloads = {}}
	function state:output(op)
		self.ops[#self.ops + 1] = op
	end
	function state:genid()
		local id = self.id
		self.id = id + 1
		return id
	end
	function state:gen(node, inAssignContext)
		print(node.type)
		return handlers[node.type](state, node)
	end
	function state:pushBindingBlock()
		self.bindingBlock = {bindings = {}, previous = self.bindingBlock, forwardDeclarations = {}}
	end
	function state:popBindingBlock()
		assert(self.bindingBlock)
		self.bindingBlock = self.bindingBlock.previous
	end
	function state:bind(identifier, id, isForwardDeclared)
		assert(id)
		assert(self.bindingBlock)
		assert(not self.bindingBlock.bindings[identifier])
		self.bindingBlock.bindings[identifier] = id
		self.bindingBlock.forwardDeclarations[identifier] = isForwardDeclared
	end
	function state:lookupBinding(identifier)
		local b = self.bindingBlock
		while b do
			local id = b.bindings[identifier]
			if id then return id, b.forwardDeclarations[identifier] end
			b = b.previous
		end
		error("Access to undefined variable: " .. hexToString(identifier))
	end
	function state:overload(dst, src)
		local overloads = self.overloads[dst]
		if not overloads then overloads = {} end
		overloads[#overloads + 1] = src
		self.overloads[dst] = overloads
	end
	local destination = state:gen(rootNode)
	for dst, overloads in pairs(state.overloads) do
		local previous = state:genid()
		state:output {type = "undefined", destination = previous}
		
		for _, overload in pairs(overloads) do
			local new = state:genid()
			state:output {type = "option", destination = new, construction = overload, defaultConstruction = previous}
			previous = new
		end
		
		state:output {type = "let", destination = dst, source = previous}
	end
	state:output {type = "export", name = "main", source = destination}
	return state.ops
end

return Generate
