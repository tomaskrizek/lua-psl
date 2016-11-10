describe("psl", function()
	local psl_lib = require "psl"
	it("__tostring works", function()
		local psl = psl_lib.builtin()
		local str = tostring(psl)
		assert.same("psl_ctx_t*: 0x", str:match("^.-0x"))
	end)
	it("has working psl_lib.get_version()", function()
		assert.same("string", type(psl_lib.get_version()))
	end);
	(psl_lib.VERSION_NUMBER and psl_lib.VERSION_NUMBER >= 0x000b00 and it or pending)("has working psl_lib.check_version_number()", function()
		local version = psl_lib.check_version_number()
		assert.same("number", type(version))
		assert.same(false, psl_lib.check_version_number(version - 1))
	end);
	(psl_lib.VERSION_NUMBER and psl_lib.VERSION_NUMBER >= 0x000c00 and it or pending)("has psl_lib.TYPE field", function()
		assert.same("table", type(psl_lib.TYPE))
		assert.same("number", type(psl_lib.TYPE.ANY))
	end)
	describe("psl_lib.str_to_utf8lower()", function()
		it("works", function()
			assert.same("foo.com", psl_lib.str_to_utf8lower("foo.com"))
			assert.same("foo.com", psl_lib.str_to_utf8lower("FOO.com"))
		end)
		it("throws error on invalid arguments", function()
			assert.has.errors(function() psl_lib.str_to_utf8lower() end)
			assert.has.errors(function() psl_lib.str_to_utf8lower(nil) end)
			assert.has.errors(function() psl_lib.str_to_utf8lower({}) end)
			assert.has.errors(function() psl_lib.str_to_utf8lower("foo", {}) end)
			assert.has.errors(function() psl_lib.str_to_utf8lower("foo", nil, {}) end)
		end)
	end)
end)
