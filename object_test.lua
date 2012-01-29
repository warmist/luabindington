--Constructor test:
test_obj=first_class:new()
--function tests:
PrintResult(test_obj:void_funct(15),"void_funct1")
PrintResult(test_obj:void_funct(15),"void_funct2")

PrintResult(test_obj:int_funct(15),"int_funct1")
PrintResult(test_obj:int_funct(15),"int_funct2")
--get/set tests
print(test_obj._a)
test_obj._a_set=-1
print(test_obj._a)
--passing obj as params/rets
funct_that_takes_obj(test_obj)
test_obj2=funct_that_rets_obj()
PrintResult(test_obj2:int_funct(15),"int_funct3")
test_obj=nil
test_obj2=nil
PrintTable((debug.getregistry()[first_class]))
for i=0,5 do 
	funct_that_rets_obj()
end
for k,v in pairs(debug.getregistry()[first_class]) do
	PrintTable(getmetatable(debug.getregistry()[first_class][k].__udata))
	--debug.getregistry()[first_class][k]=nil
end
collectgarbage()
print("After gc")
for k,v in pairs(debug.getregistry()[first_class]) do
	PrintTable(getmetatable(debug.getregistry()[first_class][k].__udata))
	--debug.getregistry()[first_class][k]=nil
end
