local get_settings = nil
if global_settings ~= nil then
	get_settings = function(name, default_value)
		local v = global_settings[name]
		if v ~= nil then
			return v
		end
		return default_value
	end
else
	get_settings = function(name, default_value)
		return default_value
	end
end
function ConfigProject(config)
	target(get_settings("project_name", config.project_name))
	set_kind(get_settings("project_kind", config.project_kind))
	local langs = get_settings("languages", nil)
	if langs ~= nil then
		set_languages(langs)
	else
		set_languages("clagest", "cxx20")
	end
	local enable_unitybuild = get_settings("enable_unitybuild", true)
	if enable_unitybuild then
		local unityBuildBatch = (config.batch_size)
		if (unityBuildBatch ~= nil) and (unityBuildBatch > 1) then
			add_rules("c.unity_build", {
				batchsize = unityBuildBatch
			})
			add_rules("c++.unity_build", {
				batchsize = unityBuildBatch
			})
		end
	end
	local enable_exception = get_settings("enable_exception", true) and (config.enable_exception == true)
	if enable_exception then
		set_exceptions("cxx", "objc")
	else
		set_exceptions("no-cxx", "no-objc")
	end
	if is_plat("windows") then
		local win_runtime = get_settings("win_runtime", nil)
		if win_runtime ~= nil then
			set_runtimes(win_runtime)
		else
			if is_mode("debug") then
				set_runtimes("MDd")
			else
				set_runtimes("MD")
			end
		end
	end
	local events = get_settings("events", nil)
	if events ~= nil then
		for i, k in ipairs(events) do
			k()
		end
	end
end
function GetAddDefines()
	return get_settings("override_add_defines", add_defines)
end
function GetAddIncludeDirs()
	return get_settings("override_add_includedirs", add_includedirs)
end
function GetRTTIUsage()
	return get_settings("enable_rtti", true)
end

--[[
global_settings = {
    languages = {},
    project_name = "",
    project_kind = "",
    enable_unitybuild = true,
	enable_exception = false,
    override_add_defines = add_defines,
    override_add_includedirs = add_includedirs,
    win_runtime = "MD",
    events = {function() end},
}
]]
