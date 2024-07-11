#pragma once

#include <sol/sol.hpp>

struct ScriptComponent
{
	sol::function Funct;
	ScriptComponent(sol::function Funct = sol::lua_nil)
	{
		this->Funct = Funct;
	}
};