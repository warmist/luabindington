--Constructor test:
test_obj=first_class:new()
--simple table access tests
test_obj.stuff=100 --adds value to (secret) table
print("Stuff:"..test_obj.stuff) --test if sucess (also test if gets work)
collectgarbage() --should not delete table
print("Stuff:"..test_obj.stuff) --test if still there
--function tests:
PrintResult(test_obj:void_funct(15),"void_funct1") --function call tests
PrintResult(test_obj:void_funct(15),"void_funct2")

PrintResult(test_obj:int_funct(15),"int_funct1")
PrintResult(test_obj:int_funct(15),"int_funct2")
--get/set tests
print(test_obj._a) --get
test_obj._a_set=-1 --different function to set
print(test_obj._a) --get again (should be -1)
--passing obj as params/rets
funct_that_takes_obj(test_obj) --cast to ptr test
test_obj2=funct_that_rets_obj() --creates new object from c++ to lua
PrintResult(test_obj2:int_funct(15),"int_funct3") -- test if correct object
--casting.
test_obj3=second_class:new() -- create obj of "second_class"
test_obj3:PrintData("New string") --test if it works
test_obj3:int_funct(100) --cast it to first_class
--garbage collect test
test_obj=nil
test_obj2=nil
collectgarbage() --should destroy everything
test_obj3:PrintData("New string")  --but not this
--funct_that_takes_obj(0) --should be:error- does not have metatable
--funct_that_takes_obj(some_table_w_meta) --should be:error- incorrect metatable (if does not have typename) 
funct_that_takes_obj(test_obj3) --temporary cast happens here.
collectgarbage() --should not collect test_obj3
test_obj3:PrintData("New string") --test if not destroyed