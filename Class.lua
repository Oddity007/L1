--Based on http://www.rtsoft.com/forums/showthread.php?1466-A-cleaner-easier-class-idiom-in-Lua

return function(superclass)
	local class = superclass and superclass() or {}
	rawset(class, "__instanceMetatable__", {__index = class})
	rawset(class, "__super__", superclass)
	rawset(class, "__class__", class)
	local classMetatable = {__index = superclass}
	function classMetatable.__call(class, ...)
		local instance = setmetatable({}, class.__instanceMetatable__)
		local init = class.__init__
		if init then
			init(instance, ...)
		end
		return instance
	end
	return setmetatable(class, classMetatable)
end