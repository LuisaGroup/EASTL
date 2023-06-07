includes("packages/mimalloc/build_proj.lua")
target("eastl")
_config_project({
	project_kind = "shared",
	batch_size = 64,
	no_rtti = true
})
on_load(function(target)
	local function rela(p)
		return path.relative(path.absolute(p, os.scriptdir()), os.projectdir())
	end
	target:add("includedirs", rela("include"), rela("packages/EABase/include/Common"), {
		public = true
	})
	
	target:add("defines", "EA_PRAGMA_ONCE_SUPPORTED", "EASTL_ASSERT_ENABLED=0", "EA_HAVE_CPP11_CONTAINERS", "EA_HAVE_CPP11_ATOMIC",
					"EA_HAVE_CPP11_CONDITION_VARIABLE", "EA_HAVE_CPP11_MUTEX", "EA_HAVE_CPP11_THREAD", "EA_HAVE_CPP11_FUTURE",
					"EA_HAVE_CPP11_TYPE_TRAITS", "EA_HAVE_CPP11_TUPLES", "EA_HAVE_CPP11_REGEX", "EA_HAVE_CPP11_RANDOM",
					"EA_HAVE_CPP11_CHRONO", "EA_HAVE_CPP11_SCOPED_ALLOCATOR", "EA_HAVE_CPP11_INITIALIZER_LIST",
					"EA_HAVE_CPP11_SYSTEM_ERROR", "EA_HAVE_CPP11_TYPEINDEX", "EASTL_STD_ITERATOR_CATEGORY_ENABLED",
					"EASTL_STD_TYPE_TRAITS_AVAILABLE", "EASTL_MOVE_SEMANTICS_ENABLED", "EASTL_VARIADIC_TEMPLATES_ENABLED",
					"EASTL_VARIABLE_TEMPLATES_ENABLED", "EASTL_INLINE_VARIABLE_ENABLED", "EASTL_HAVE_CPP11_TYPE_TRAITS",
					"EASTL_INLINE_NAMESPACES_ENABLED", "EASTL_ALLOCATOR_EXPLICIT_ENABLED", "EA_DLL", "EASTL_USER_DEFINED_ALLOCATOR", {
						public = true
					})
	target:add("defines", "LC_EASTL_EXPORT=1", "EASTL_PROJECT=1")
end)
add_headerfiles("include/**.h")
add_headerfiles("packages/EABase/include/Common/**.h")
add_files("source/*.cpp")
if enable_mimalloc then
	add_defines("EASTL_MIMALLOC_ENABLED=1")
	includes("packages/mimalloc")
	add_deps("mimalloc")
end
target_end()
