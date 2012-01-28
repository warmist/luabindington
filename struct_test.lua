print("Testing struct casts")
arg_is_struct({a=1,b=337,c="yay"});
ret=ret_is_struct()
for k,v in pairs(ret) do
	print(k.." "..v)
end
arg_is_complex({a=999,b={a=13,b=37,c="yay three"}})