function PrintTable(tbl)
	for k,v in pairs(tbl) do 
		print(tostring(k)..'->'..tostring(v)) 
	end 
end
function PrintResult(val,name)
	print(string.format("%s result is type=%s and is %s",name,type(val),tostring(val)))
end
function Error(msg)
	print("ERROR:"..msg)
	print(debug.traceback())
	return msg
end