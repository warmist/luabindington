--Constructor test:
test_obj=first_class:new()
--simple table access tests
test_obj.stuff=100
print("Stuff:"..test_obj.stuff)
collectgarbage()
print("Stuff:"..test_obj.stuff)
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
--casting.
test_obj3=second_class:new()
test_obj3:PrintData("New string")
test_obj3:int_funct(100)
--garbage collect test
test_obj=nil
test_obj2=nil
collectgarbage()
test_obj3:PrintData("New string") --gc error
--funct_that_takes_obj(0) --should be:error- does not have metatable
--funct_that_takes_obj(some_table_w_meta) --should be:error- incorrect metatable (if does not have typename) 
funct_that_takes_obj(test_obj3)