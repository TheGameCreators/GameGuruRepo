function ggprecompile(infile)
	if infile == nil then return end
	local p = loadfile(infile)
	local outfile = infile
	local f = io.open(outfile, "wb")
	f:write(string.dump(p))
	f:close()
end

