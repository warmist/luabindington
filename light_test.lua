print("===============================================")
for k,v in pairs(types) do
	print(k)
end
print("Light test start!")
local ret=Fibo(10)
for k,v in ipairs(ret) do
	print(k,v)
end
print("light object wrapper:")
local lobj=getStruct()

print(lobj.a)
print(lobj.b)
print(lobj.c)
print(lobj.name)
lobj.a=100
print(lobj.a)