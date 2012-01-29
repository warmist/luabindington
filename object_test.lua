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