parameters = {
   {
      ["%%STEPS%%"]   = 1,
      ["%%LENGTH%%"]  = 2,
      ["%%ILENGTH%%"] = 2,
      ["%%OMEGA%%"]   = 2,
      ["%%IOMEGA%%"]  = 2,
      ["%%PRIME%%"]   = 3,
   },
   {
      ["%%STEPS%%"]   = 2,
      ["%%LENGTH%%"]  = 4,
      ["%%ILENGTH%%"] = 4,
      ["%%OMEGA%%"]   = 2,
      ["%%IOMEGA%%"]  = 3,
      ["%%PRIME%%"]   = 5,
   },
   {
      ["%%STEPS%%"]   = 3,
      ["%%LENGTH%%"]  = 8,
      ["%%ILENGTH%%"] = 36,
      ["%%OMEGA%%"]   = 14,
      ["%%IOMEGA%%"]  = 3,
      ["%%PRIME%%"]   = 41,
   },
   {
      ["%%STEPS%%"]   = 4,
      ["%%LENGTH%%"]  = 16,
      ["%%ILENGTH%%"] = 16,
      ["%%OMEGA%%"]   = 3,
      ["%%IOMEGA%%"]  = 6,
      ["%%PRIME%%"]   = 17,
   },
   {
      ["%%STEPS%%"]   = 5,
      ["%%LENGTH%%"]  = 32,
      ["%%ILENGTH%%"] = 94,
      ["%%OMEGA%%"]   = 30,
      ["%%IOMEGA%%"]  = 55,
      ["%%PRIME%%"]   = 97,
   },
   {
      ["%%STEPS%%"]   = 6,
      ["%%LENGTH%%"]  = 64,
      ["%%ILENGTH%%"] = 190,
      ["%%OMEGA%%"]   = 87,
      ["%%IOMEGA%%"]  = 71,
      ["%%PRIME%%"]   = 193,
   },
   {
      ["%%STEPS%%"]   = 7,
      ["%%LENGTH%%"]  = 128,
      ["%%ILENGTH%%"] = 636,
      ["%%OMEGA%%"]   = 199,
      ["%%IOMEGA%%"]  = 335,
      ["%%PRIME%%"]   = 641,
   },
   {
      ["%%STEPS%%"]   = 8,
      ["%%LENGTH%%"]  = 256,
      ["%%ILENGTH%%"] = 256,
      ["%%OMEGA%%"]   = 74,
      ["%%IOMEGA%%"]  = 66,
      ["%%PRIME%%"]   = 257,
   },
   {
      ["%%STEPS%%"]   = 9,
      ["%%LENGTH%%"]  = 512,
      ["%%ILENGTH%%"] = 7666,
      ["%%OMEGA%%"]   = 1704,
      ["%%IOMEGA%%"]  = 4431,
      ["%%PRIME%%"]   = 7681,
   },
   {
      ["%%STEPS%%"]   = 10,
      ["%%LENGTH%%"]  = 1024,
      ["%%ILENGTH%%"] = 13300,
      ["%%OMEGA%%"]   = 4085,
      ["%%IOMEGA%%"]  = 6430,
      ["%%PRIME%%"]   = 13313,
   },
   {
      ["%%STEPS%%"]   = 11,
      ["%%LENGTH%%"]  = 2048,
      ["%%ILENGTH%%"] = 18424,
      ["%%OMEGA%%"]   = 5062,
      ["%%IOMEGA%%"]  = 9770,
      ["%%PRIME%%"]   = 18433,
   },
   {
      ["%%STEPS%%"]   = 12,
      ["%%LENGTH%%"]  = 4096,
      ["%%ILENGTH%%"] = 12286,
      ["%%OMEGA%%"]   = 2015,
      ["%%IOMEGA%%"]  = 494,
      ["%%PRIME%%"]   = 12289,
   },
   {
      ["%%STEPS%%"]   = 13,
      ["%%LENGTH%%"]  = 8192,
      ["%%ILENGTH%%"] = 40956,
      ["%%OMEGA%%"]   = 2821,
      ["%%IOMEGA%%"]  = 20299,
      ["%%PRIME%%"]   = 40961,
   },
   {
      ["%%STEPS%%"]   = 14,
      ["%%LENGTH%%"]  = 16384,
      ["%%ILENGTH%%"] = 114682,
      ["%%OMEGA%%"]   = 43,
      ["%%IOMEGA%%"]  = 42675,
      ["%%PRIME%%"]   = 114689,
   },
   {
      ["%%STEPS%%"]   = 15,
      ["%%LENGTH%%"]  = 32768,
      ["%%ILENGTH%%"] = 163836,
      ["%%OMEGA%%"]   = 31285,
      ["%%IOMEGA%%"]  = 110575,
      ["%%PRIME%%"]   = 163841,
   },
   {
      ["%%STEPS%%"]   = 16,
      ["%%LENGTH%%"]  = 65536,
      ["%%ILENGTH%%"] = 65536,
      ["%%OMEGA%%"]   = 7659,
      ["%%IOMEGA%%"]  = 22573,
      ["%%PRIME%%"]   = 65537,
   },
   {
      ["%%STEPS%%"]   = 17,
      ["%%LENGTH%%"]  = 131072,
      ["%%ILENGTH%%"] = 1179640,
      ["%%OMEGA%%"]   = 93583,
      ["%%IOMEGA%%"]  = 425255,
      ["%%PRIME%%"]   = 1179649,
   }
}


h = io.open("autocorr.c.in", "r")
pattern = h:read("*all")
h:close()

for _, param in pairs (parameters) do
   code = string.gsub(pattern, "%%%%%u+%%%%", param)
   name = "autocorr_" .. tostring(param["%%STEPS%%"]) .. ".c"
   h = io.open(name, "w")
   h:write (code)
   h:close()
end

h = io.open("generated.h.in", "r")
pattern = h:read("*all")
h:close()

decls = ""
for _, param in pairs (parameters) do
   decls = decls .. string.format ("void autocorr_%i (const uint64_t *src, uint64_t *dst);\n",
                                   param["%%LENGTH%%"])
end

code = string.gsub(pattern, "%%%%%u+%%%%",
                   { ["%%DECLS%%"] = decls, ["%%LENGTH%%"] = #parameters })

h = io.open("generated.h", "w")
h:write(code)
h:close()

h = io.open("params.c.in", "r")
pattern = h:read("*all")
h:close()

data  = ""
for _, param in pairs (parameters) do
   data = data .. string.format ("    { %i, %i, %i, %i, %i, %s },\n",
                                 param["%%LENGTH%%"], param["%%ILENGTH%%"],
                                 param["%%OMEGA%%"], param["%%IOMEGA%%"],
                                 param["%%PRIME%%"], "autocorr_" .. param["%%LENGTH%%"])
end

code = string.gsub(pattern, "%%%%%u+%%%%", { ["%%DATA%%"] = data })

h = io.open("params.c", "w")
h:write(code)
h:close()
