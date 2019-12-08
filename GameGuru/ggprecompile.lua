function ggprecompile(infile)
	if infile == nil then return end
	local p = loadfile(infile)
	if p == nil then return end -- properly not a lua file, keep original.
	local outfile = infile
	local f = io.open(outfile, "wb")
	f:write(string.dump(p))
	f:close()
end

