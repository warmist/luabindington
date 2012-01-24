function PrintTable(tbl)
	for k,v in pairs(tbl) do 
		print(tostring(k)..'->'..tostring(v)) 
	end 
end
function printValue(obj)
	print("printing value:"..obj:DoSth(1))
end
print(string.format('%d %f',sum(1,0.3))) 
testvoid(0,15)
print(string.format('%f',exp('hello')))
print(string.format('%f',sinsin(0,15)))
kk=thingy:new()
print(kk:DoSth(1))
kk:VoidTest(1)
--[=[for k,v in pairs(debug.getregistry()) do
	print(type(k)..' '..type(v)); 
	if type(k) == 'string' then print(k) end
end--]=]
kk:DoLuaTest()
print(type(kk.value))
kk.value=100
print(kk.value)
kk.miss=1337
print(kk.count)