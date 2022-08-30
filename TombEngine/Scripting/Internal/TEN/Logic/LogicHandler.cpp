#include "framework.h"
#include "LogicHandler.h"

#include "ScriptAssert.h"
#include "Game/savegame.h"
#include "Sound/sound.h"
#include "ReservedScriptNames.h"
#include "Game/effects/lightning.h"
#include "ScriptUtil.h"
#include "Objects/Moveable/MoveableObject.h"
#include "Vec3/Vec3.h"
#include "FuncNameHolder.h"
using namespace TEN::Effects::Lightning;

/***
Saving data, triggering functions, and callbacks for level-specific scripts.
@tentable Logic 
@pragma nostrip
*/

enum class CallbackPoint
{
	PreControl,
	PostControl,
};

static const std::unordered_map<std::string, CallbackPoint> kCallbackPoints
{
	{"PRECONTROLPHASE", CallbackPoint::PreControl},
	{"POSTCONTROLPHASE", CallbackPoint::PostControl},
};

using LevelFuncsTable = std::pair<sol::table, std::string>;

void SetVariable(sol::table tab, sol::object key, sol::object value)
{
	switch (value.get_type())
	{
	case sol::type::lua_nil:
	case sol::type::boolean:
	case sol::type::number:
	case sol::type::string:
	case sol::type::table:
		switch (key.get_type())
		{
		case sol::type::number:
		case sol::type::string:
			tab.raw_set(key, value);
			break;
		default:
			ScriptAssert(false, "Unsupported key type used for special table. Valid types are string and number.", ErrorMode::Terminate);
			break;
		}
		break;
	default:
		key.push();
		size_t strLen;
		const char* str = luaL_tolstring(tab.lua_state(), -1, &strLen);
		if (str)
		{
			ScriptAssert(false, "Variable " + std::string{ str } + " has an unsupported type.", ErrorMode::Terminate);
			lua_pop(tab.lua_state(), 1);
		}
		else
		{
			ScriptAssert(false, "Variable has an unsupported type.", ErrorMode::Terminate);
		}
		key.pop();
		break;
	}
}

sol::object GetVariable(sol::table tab, sol::object key)
{
	return tab.raw_get<sol::object>(key);
}

LogicHandler::LogicHandler(sol::state* lua, sol::table & parent) : m_handler{ lua }
{
	m_handler.GetState()->set_function("print", &LogicHandler::LogPrint, this);

	sol::table table_logic{ m_handler.GetState()->lua_state(), sol::create };

	parent.set(ScriptReserved_Logic, table_logic);

	table_logic.set_function(ScriptReserved_AddCallback, &LogicHandler::AddCallback, this);
	table_logic.set_function(ScriptReserved_RemoveCallback, &LogicHandler::RemoveCallback, this);

	m_handler.MakeReadOnlyTable(table_logic, ScriptReserved_CallbackPoint, kCallbackPoints);

	//it crashes when here
	// todo fix?
	//FuncNameHolder::Register(table_logic);

	ResetScripts(true);
}

void LogicHandler::ResetGameTables() 
{
	MakeSpecialTable(m_handler.GetState(), ScriptReserved_GameVars, &GetVariable, &SetVariable);
}

/*** Register a function as a callback.
Possible values for CallbackPoint:
	PRECONTROLPHASE -- will be called immediately before OnControlPhase
	POSTCONTROLPHASE -- will be called immediately after OnControlPhase

The order in which two functions with the same CallbackPoint are called is undefined.
i.e. if you register `MyFunc` and `MyFunc2` with `PRECONTROLPHASE`, both will be called before `OnControlPhase`, but there is no guarantee whether `MyFunc` will be called before `MyFunc2`, or vice-versa.



@function AddCallback
@tparam point CallbackPoint When should the callback be called?
@tparam func string The name of the function to be called. Will receive as an argument the time in seconds since the last frame.
@usage
	local MyFunc = function(dt) print(dt) end
	TEN.Logic.AddCallback(TEN.Logic.CallbackPoint.PRECONTROLPHASE, "MyFunc")
*/
void LogicHandler::AddCallback(CallbackPoint point, LevelFunc & fnh)
{
	//sol::object obj = tab["funcName"];
	//sol::type type = obj.get_type();
	//switch(type)
	//{
	//case sol::type::string:
	//	TENLog("fuck it's a string");
	//	break;

	//case sol::type::userdata:
	//	TENLog("fuck it's userdata");
	//	break;

	//case sol::type::table:
	//	TENLog("fuck it's a table");
	//	break;
	//}

	//auto fnh = tab["funcName"];
	//std::string name = tab["funcName"];
	switch(point)
	{
	case CallbackPoint::PreControl:
		m_callbacksPreControl.insert(fnh.m_funcName);
		break;

	case CallbackPoint::PostControl:
		m_callbacksPostControl.insert(fnh.m_funcName);
		break;
	}
}

/*** Deregister a function as a callback.
Will have no effect if the function was not registered as a callback

@function RemoveCallback
@tparam point CallbackPoint The callback point the function was registered with. See @{AddCallback}
@tparam func string The LevelFuncs function to remove.
@usage
	TEN.Logic.RemoveCallback(TEN.Logic.CallbackPoint.PRECONTROLPHASE, "MyFunc")
*/
//todo this has to deal with LevelFuncs subtables too now
void LogicHandler::RemoveCallback(CallbackPoint point, std::string const & name)
{
	switch(point)
	{
	case CallbackPoint::PreControl:
		m_callbacksPreControl.erase(name);
		break;

	case CallbackPoint::PostControl:
		m_callbacksPostControl.erase(name);
		break;
	}
}

	static constexpr char const* strKey = "__internal_name";
void LogicHandler::ResetLevelTables()
{
	//todo does levelFuncs get cleared normally, pre-these changes?
	//NO. the TABLE gets reset but m_levelFuncs does NOT get reset


	MakeSpecialTable(m_handler.GetState(), ScriptReserved_LevelVars, &GetVariable, &SetVariable);
}

sol::object LogicHandler::GetLevelFunc(sol::table tab, std::string const& luaName)
{
	//if (m_levelFuncs.find(luaName) == m_levelFuncs.end())
	//	return sol::lua_nil;

	std::string partName = tab.raw_get<std::string>(strKey);
	//std::string x = m_levelFuncsTables[luaName];
	sol::table theTab = m_levelFuncsTables[partName];
	//TENLog("looking for " + luaName);
	//std::string x = theTab.raw_get<std::string>(luaName);
	//theTab.raw_get_or()
	//TENLog("table has:");
	//for(auto & [key, val] : theTab)
	//{
	//	std::string keyStr = key.as<std::string>();
	//	std::string valStr = val.as<std::string>();
	//	TENLog(fmt::format("{} = {}", keyStr, valStr));
	//}
	//TENLog("");
	std::string x;
	sol::object obj = theTab.raw_get<sol::object>(luaName);
	if (obj.is<std::string>())
	{
		x = obj.as<std::string>();
		//TENLog("found " + luaName + ", it was " + x);

		//what if you are asking for a table?
		if (m_levelFuncsFakeFuncs[x].valid())
			return m_levelFuncsFakeFuncs[x];
		else
			return m_levelFuncsTables[x];
	}
	if (obj.is<sol::table>())
	{
		return obj;
	}
	else
	{
		return sol::nil;
	}
	//return m_levelFuncs.at(luaName);
}

void LogicHandler::CallLevelFunc(std::string name, float dt)
{
	sol::protected_function f = m_levelFuncsActualFuncs[name];
	//for (auto & v : va)
	//{
	//	TENLog(v.as<std::string>());
	//}
	f.call(dt);
}


void LogicHandler::CallLevelFunc(std::string name, sol::variadic_args va)
{
	sol::protected_function f = m_levelFuncsActualFuncs[name];
	//for (auto & v : va)
	//{
	//	TENLog(v.as<std::string>());
	//}
	f.call(va);
}

bool LogicHandler::SetLevelFunc(sol::table tab, std::string const& luaName, sol::object value)
{

	//make m_levelFuncs the 'root' table, make everything else go into "tab" (so maybe all these should act on "tab" and we gotta create original m_levelFuncs ourselves outside of here)
	std::string partName;
	std::string fullName;
	sol::table newTab;
	sol::table newLevelFuncsTab;
	sol::table aTab;
	sol::table meta;
	LevelFunc fnh;
	switch (value.get_type())
	{
	case sol::type::lua_nil:
	case sol::type::function:
			partName = tab.raw_get<std::string>(strKey);
			fullName = partName + "." + luaName;
			aTab = m_levelFuncsTables[partName];
			aTab.raw_set(luaName, fullName);
			m_levelFuncsActualFuncs[fullName] = value;

			fnh.m_funcName = fullName;
			fnh.m_handler = this;

			m_levelFuncsFakeFuncs[fullName] = fnh;

			//newTab = sol::table{ *(m_handler.GetState()), sol::create };
			//newTab[sol::metatable_key] = sol::table{ *(m_handler.GetState()), sol::create };

			//meta = newTab[sol::metatable_key];
			//meta.set_function("__call", &LogicHandler::CallLevelFunc, this);
			//newTab.set("funcName", fullName);
//			m_levelFuncsFakeFuncs[fullName] = newTab;

			//update: don't need to do this actually I don't think...
			//todo make another m_levelFuncsActualFuncs member with the actual func call
		break;
	case sol::type::table:
		//make a copy of the table using
		//tab.raw_set(luaName, MakeSpecialTable(m_handler.GetState(), luaName, &LogicHandler::GetLevelFunc, &LogicHandler::SetLevelFunc, this));
			//todo need to make anonoymous version of MakeSpecialTable
			newTab = sol::table{ *(m_handler.GetState()), sol::create };
			fullName = tab.raw_get<std::string>(strKey) + "." + luaName;
			m_levelFuncsTables[fullName] = newTab;

			partName = tab.raw_get<std::string>(strKey);
			aTab = m_levelFuncsTables[partName];
			aTab.raw_set(luaName, fullName);

			newTab.raw_set(strKey, fullName);

			for (auto& [key, val] : value.as<sol::table>())
			{
				newTab[key] = val;
			}

			newLevelFuncsTab = MakeSpecialTable(m_handler.GetState(), luaName, &LogicHandler::GetLevelFunc, &LogicHandler::SetLevelFunc, this);
			newLevelFuncsTab.raw_set(strKey, fullName);
			tab.raw_set(luaName, newLevelFuncsTab);

			for (auto& [key, val] : value.as<sol::table>())
			{
				newLevelFuncsTab[key] = val;
			}
			//tab.raw_set(luaName, newTab);

		//	assign the copy of the table
		break;
	default:
		std::string error{ "Failed to add " };
		error += luaName + " to LevelFuncs or one of its tables; it must be a function, a table of functions, or nil.";
		return ScriptAssert(false, error);
	}
	return true;
}


void LogicHandler::LogPrint(sol::variadic_args va)
{
	std::string str;
	for (sol::object const & o : va)
	{
		auto strPart = (*m_handler.GetState())["tostring"](o).get<std::string>();
		str += strPart;
		str += "\t";
	}
	TENLog(str, LogLevel::Info, LogConfig::All, true);
}

void LogicHandler::ResetScripts(bool clearGameVars)
{
	FreeLevelScripts();

	//todo are these getting called?
	//load level. walk to trigger that adds here
	//save and reload

	m_callbacksPreControl.clear();
	m_callbacksPostControl.clear();

	auto currentPackage = m_handler.GetState()->get<sol::table>("package");
	auto currentLoaded = currentPackage.get<sol::table>("loaded");

	for(auto & [first, second] : currentLoaded)
		currentLoaded[first] = sol::nil;

	if(clearGameVars)
		ResetGameTables();

	m_handler.ResetGlobals();

	m_handler.GetState()->collect_garbage();
}

void LogicHandler::FreeLevelScripts()
{
	//m_levelFuncs.clear();
	//m_levelFuncs = 
	m_levelFuncs = MakeSpecialTable(m_handler.GetState(), ScriptReserved_LevelFuncs, &LogicHandler::GetLevelFunc, &LogicHandler::SetLevelFunc, this);

	m_levelFuncsTables = sol::table{ *(m_handler.GetState()), sol::create };
	m_levelFuncsActualFuncs = sol::table{ *(m_handler.GetState()), sol::create };
	m_levelFuncsFakeFuncs = sol::table{ *(m_handler.GetState()), sol::create };

	m_levelFuncsTables[ScriptReserved_LevelFuncs] = sol::table{ *(m_handler.GetState()), sol::create };
	m_levelFuncs.raw_set(strKey, ScriptReserved_LevelFuncs);
	ResetLevelTables();
	m_onStart = sol::nil;
	m_onLoad = sol::nil;
	m_onControlPhase = sol::nil;
	m_onSave = sol::nil;
	m_onEnd = sol::nil;
	m_handler.GetState()->collect_garbage();
}

void JumpToLevel(int levelNum)
{
	if (levelNum >= g_GameFlow->GetNumLevels())
		return;

	LevelComplete = levelNum;
}

int GetSecretsCount()
{
	return Statistics.Level.Secrets;
}

void SetSecretsCount(int secretsNum)
{
	if (secretsNum > 255)
		return;
	Statistics.Level.Secrets = secretsNum;
}

void AddOneSecret()
{
	if (Statistics.Level.Secrets >= 255)
		return;
	Statistics.Level.Secrets++;
	PlaySecretTrack();
}

void LogicHandler::SetVariables(std::vector<SavedVar> const & vars)
{
	ResetGameTables();
	ResetLevelTables();

	std::unordered_map<uint32_t, sol::table> solTables;

	for(std::size_t i = 0; i < vars.size(); ++i)
	{
		if (std::holds_alternative<IndexTable>(vars[i]))
		{
			solTables.try_emplace(i, *m_handler.GetState(), sol::create);
			auto indexTab = std::get<IndexTable>(vars[i]);
			for (auto& [first, second] : indexTab)
			{
				// if we're wanting to reference a table, make sure that table exists
				// create it if need be
				if (std::holds_alternative<IndexTable>(vars[second]))
				{
					solTables.try_emplace(second, *m_handler.GetState(), sol::create);
					solTables[i][vars[first]] = solTables[second];
				}
				else if (std::holds_alternative<double>(vars[second]))
				{
					double theNum = std::get<double>(vars[second]);
					// If this is representable as an integer use an integer.
					// This is to ensure something saved as 1 is not loaded as 1.0
					// which would be confusing for the user.
					// todo: should we throw a warning if the user tries to save or load a value
					// outside of these bounds? - squidshire 30/04/2022
					if (std::trunc(theNum) == theNum && theNum <= INT64_MAX && theNum >= INT64_MIN)
					{
						solTables[i][vars[first]] = static_cast<int64_t>(theNum);
					}
					else
					{
						solTables[i][vars[first]] = vars[second];
					}
				}
				else if (std::holds_alternative<Vector3Int>(vars[second]))
				{
					auto theVec = Vec3{ std::get<Vector3Int>(vars[second]) };
					solTables[i][vars[first]] = theVec;
				}
				else if (std::holds_alternative<FuncName>(vars[second]))
				{
					LevelFunc fnh;
					fnh.m_funcName = std::get<FuncName>(vars[second]).name;
					fnh.m_handler = this;
					solTables[i][vars[first]] = fnh;
				}
				else
				{
					solTables[i][vars[first]] = vars[second];
				}
			}
		}
	}
	
	auto rootTable = solTables[0];

	sol::table levelVars = rootTable[ScriptReserved_LevelVars];
	for (auto& [first, second] : levelVars)
	{
		(*m_handler.GetState())[ScriptReserved_LevelVars][first] = second;
	}

	sol::table gameVars = rootTable[ScriptReserved_GameVars];
	for (auto& [first, second] : gameVars)
	{
		(*m_handler.GetState())[ScriptReserved_GameVars][first] = second;
	}
}

void LogicHandler::GetVariables(std::vector<SavedVar> & vars)
{
	sol::table tab{ *m_handler.GetState(), sol::create };
	tab[ScriptReserved_LevelVars] = (*m_handler.GetState())[ScriptReserved_LevelVars];
	tab[ScriptReserved_GameVars] = (*m_handler.GetState())[ScriptReserved_GameVars];

	std::unordered_map<void const*, uint32_t> varsMap;
	std::unordered_map<double, uint32_t> numMap;
	std::unordered_map<bool, uint32_t> boolMap;

	size_t nVars = 0;

	// The following functions will all try to put their values in a map. If it succeeds
	// then the value was not already in the map, so we can put it into the var vector.
	// If it fails, the value is in the map, and thus will also be in the var vector.
	// We then return the value's position in the var vector.

	// The purpose of this is to only store each value once, and to fill our tables with
	// indices to the values rather than copies of the values.
	 auto handleNum = [&](double num)
	{
		auto [first, second] = numMap.insert(std::make_pair(num, nVars));

		if (second)
		{
			vars.push_back(num);
			++nVars;
		}

		return first->second;
	};

	auto handleBool = [&](bool num)
	{
		auto [first, second] = boolMap.insert(std::make_pair(num, nVars));

		if (second)
		{
			vars.push_back(num);
			++nVars;
		}

		return first->second;
	};

	auto handleStr = [&](sol::object const& obj)
	{
		auto str = obj.as<sol::string_view>();
		auto [first, second] = varsMap.insert(std::make_pair(str.data(), nVars));

		if (second)
		{
			vars.push_back(std::string{ str.data() });
			++nVars;
		}

		return first->second;
	};

	auto handleVec3 = [&](Vec3 const & vec)
	{
		auto [first, second] = varsMap.insert(std::make_pair(&vec, nVars));

		if (second)
		{
			vars.push_back(vec);
			++nVars;
		}

		return first->second;
	};

	auto handleFuncName = [&](LevelFunc const& fnh)
	{
		//auto str = obj.as<sol::string_view>();
		auto [first, second] = varsMap.insert(std::make_pair(&fnh, nVars));

		if (second)
		{
			vars.push_back(FuncName{ std::string{ fnh.m_funcName } });
			++nVars;
		}

		return first->second;
	};

	std::function<uint32_t(sol::table const &)> populate = [&](sol::table const & obj) 
	{
		auto [first, second] = varsMap.insert(std::make_pair(obj.pointer(), nVars));

		if(second)
		{
			++nVars;
			auto id = first->second;

			vars.push_back(IndexTable{});
			for (auto& [first, second] : obj)
			{
				uint32_t keyIndex = 0;
				
				// Strings and numbers can be keys AND values
				switch (first.get_type())
				{
				case sol::type::string:
					keyIndex = handleStr(first);
					break;
				case sol::type::number:
					keyIndex = handleNum(first.as<double>());
					break;
				default:
					ScriptAssert(false, "Tried saving an unsupported type as a key");
				}

				auto putInVars = [&vars, id, keyIndex](uint32_t valIndex)
				{
					std::get<IndexTable>(vars[id]).push_back(std::make_pair(keyIndex, valIndex));
				};

				uint32_t valIndex = 0;
				switch (second.get_type())
				{
				case sol::type::table:
					putInVars(populate(second.as<sol::table>()));
					break;
				case sol::type::string:
					putInVars(handleStr(second));
					break;
				case sol::type::number:
					putInVars(handleNum(second.as<double>()));
					break;
				case sol::type::boolean:
					putInVars(handleBool(second.as<bool>()));
					break;
				case sol::type::userdata:
				{
					if(second.is<Vec3>())
						putInVars(handleVec3(second.as<Vec3>()));
					else if(second.is<LevelFunc>())
						putInVars(handleFuncName(second.as<LevelFunc>()));
					else
						ScriptAssert(false, "Tried saving an unsupported userdata as a value");
				}
					break;
				default:
					ScriptAssert(false, "Tried saving an unsupported type as a value");
				}
			}
		}
		return first->second;
	};
	populate(tab);
}

void LogicHandler::GetCallbackStrings(std::vector<std::string>& preControl, std::vector<std::string>& postControl) const
{
	for (auto const& s : m_callbacksPreControl)
		preControl.push_back(s);

	for (auto const& s : m_callbacksPostControl)
		postControl.push_back(s);
}

void LogicHandler::SetCallbackStrings(std::vector<std::string> const & preControl, std::vector<std::string> const & postControl)
{
	for (auto const& s : preControl)
		m_callbacksPreControl.insert(s);

	for (auto const& s : postControl)
		m_callbacksPostControl.insert(s);
}

template <typename R, char const * S, typename mapType>
std::unique_ptr<R> GetByName(std::string const & type, std::string const & name, mapType const & map)
{
	ScriptAssert(map.find(name) != map.end(), std::string{ type + " name not found: " + name }, ErrorMode::Terminate);
	return std::make_unique<R>(map.at(name), false);
}

/*** Special objects
@section specialobjects
*/

/*** A @{Objects.Moveable} representing Lara herself.
@table Lara
*/
void LogicHandler::ResetVariables()
{
	(*m_handler.GetState())["Lara"] = nullptr;
}

void LogicHandler::ExecuteScriptFile(const std::string & luaFilename)
{
	m_handler.ExecuteScript(luaFilename);
}

void LogicHandler::ExecuteFunction(std::string const& name, short idOne, short idTwo) 
{
	sol::protected_function_result r;
	sol::protected_function func = (*m_handler.GetState())["LevelFuncs"][name.c_str()];
	r = func(std::make_unique<Moveable>(idOne), std::make_unique<Moveable>(idTwo));
	if (!r.valid())
	{
		sol::error err = r;
		ScriptAssertF(false, "Could not execute function {}: {}", name, err.what());
	}
}

void LogicHandler::ExecuteFunction(std::string const& name, TEN::Control::Volumes::VolumeTriggerer triggerer, std::string const& arguments)
{
	sol::protected_function_result r;
	sol::protected_function func = (*m_handler.GetState())["LevelFuncs"][name.c_str()];
	if (std::holds_alternative<short>(triggerer))
	{
		r = func(std::make_unique<Moveable>(std::get<short>(triggerer), true), arguments);
	}
	else
	{
		r = func(nullptr, arguments);
	}

	if (!r.valid())
	{
		sol::error err = r;
		ScriptAssertF(false, "Could not execute function {}: {}", name, err.what());
	}
}

static void doCallback(sol::protected_function const & func, std::optional<float> dt = std::nullopt)
{
	auto r = dt.has_value() ? func(dt) : func();

	if (!r.valid())
	{
		sol::error err = r;
		ScriptAssert(false, err.what(), ErrorMode::Terminate);
	}
}

void LogicHandler::OnStart()
{
	if (m_onStart.valid())
		doCallback(m_onStart);
}

void LogicHandler::OnLoad()
{
	if(m_onLoad.valid())
		doCallback(m_onLoad);
}

void LogicHandler::OnControlPhase(float dt)
{
	//todo write that returned values will get ignored
	auto tryCall = [this, dt](std::string const& name)
	{
		auto func = m_handler.GetState()->script("return " + name);

		//todo fail if this is nil
		if (!func.valid())
			ScriptAssertF(false, "Callback {} not valid", name);
		else //todo this is a FuncNameHolder, not a function, so ye
			func.get<LevelFunc>().CallDT(dt);
	};

	for (auto& name : m_callbacksPreControl)
		tryCall(name);

	lua_gc(m_handler.GetState()->lua_state(), LUA_GCCOLLECT, 0);
	if(m_onControlPhase.valid())
		doCallback(m_onControlPhase, dt);

	for (auto& name : m_callbacksPostControl)
		tryCall(name);
}

void LogicHandler::OnSave()
{
	if(m_onSave.valid())
		doCallback(m_onSave);
}

void LogicHandler::OnEnd()
{
	if(m_onEnd.valid())
		doCallback(m_onEnd);
}

/*** Special tables

TombEngine uses the following tables for specific things.

@section levelandgametables
*/

/*** A table with level-specific data which will be saved and loaded.
This is for level-specific information that you want to store in saved games.

For example, you may have a level with a custom puzzle where Lara has
to kill exactly seven enemies to open a door to a secret. You could use
the following line each time an enemy is killed:

	LevelVars.enemiesKilled = LevelVars.enemiesKilled + 1

If the player saves the level after killing three, saves, and then reloads the save
some time later, the values `3` will be put back into `LevelVars.enemiesKilled.`

__This table is emptied when a level is finished.__ If the player needs to be able
to return to the level (like in the Karnak and Alexandria levels in *The Last Revelation*),
you will need to use the @{GameVars} table, below.
@table LevelVars
*/

/*** A table with game data which will be saved and loaded.
This is for information not specific to any level, but which concerns your whole
levelset or game, that you want to store in saved games.

For example, you may wish to have a final boss say a specific voice line based on
a choice the player made in a previous level. In the level with the choice, you could
write:

	GameVars.playerSnoopedInDrawers = true

And in the script file for the level with the boss, you could write:

	if GameVars.playerSnoopedInDrawers then
		PlayAudioTrack("how_dare_you.wav")
	end

Unlike @{LevelVars}, this table will remain intact for the entirety of the game.
@table GameVars
*/

/*** A table with level-specific functions.

This serves two purposes: it holds the level callbacks (listed below) as well as
any trigger functions you might have specified. For example, if you give a trigger
a Lua name of "my_trigger" in Tomb Editor, you will have to implement it as a member
of this table:

	LevelFuncs.my_trigger = function() 
		-- implementation goes here
	end

The following are the level callbacks. They are optional; if your level has no special
behaviour for a particular scenario, you do not need to implement the function. For
example, if your level does not need any special initialisation when it is loaded,
you can just leave out `LevelFuncs.OnStart`.

__The order of loading is as follows:__

1. The level data itself is loaded.
2. The level script itself is run (i.e. any code you put outside the `LevelFuncs` callbacks is executed).
3. Save data is loaded, if saving from a saved game (will empty `LevelVars` and `GameVars` and repopulate them with what they contained when the game was saved).
4. If loading from a save, `OnLoaded` will be called. Otherwise, `OnStart` will be called.
5. The control loop, in which `OnControlPhase` will be called once per frame, begins.

@tfield function OnStart Will be called when a level is entered by completing a previous level or by selecting it in the menu. Will not be called when loaded from a saved game.
@tfield function OnLoad Will be called when a saved game is loaded
@tfield function(float) OnControlPhase Will be called during the game's update loop,
and provides the delta time (a float representing game time since last call) via its argument.
@tfield function OnSave Will be called when the player saves the game
@tfield function OnEnd Will be called when leaving a level. This includes finishing it, exiting to the menu, or loading a save in a different level. 
@table LevelFuncs
*/

void LogicHandler::InitCallbacks()
{
	auto assignCB = [this](sol::protected_function& func, std::string const & luaFunc) {
		std::string fullName = "LevelFuncs." + luaFunc;

		sol::object theData = (*m_handler.GetState())["LevelFuncs"][luaFunc];

		std::string err{ "Level's script does not define callback " + fullName};
		if (!ScriptAssert(theData.valid(), err)) {
			ScriptWarn("Defaulting to no " + fullName + " behaviour.");
			return;
		}
		LevelFunc fnh = (*m_handler.GetState())["LevelFuncs"][luaFunc];

		func = m_levelFuncsActualFuncs[fnh.m_funcName];

		if(func.get_type() == sol::type::function)
		{
			TENLog("function");
		}
		if(func.get_type() == sol::type::userdata)
		{
			TENLog("userdata");
		}
		if (!ScriptAssert(func.valid(), err)) {
			ScriptWarn("Defaulting to no " + fullName + " behaviour.");
		}
	};

	assignCB(m_onStart, "OnStart");
	assignCB(m_onLoad, "OnLoad");
	assignCB(m_onControlPhase, "OnControlPhase");
	assignCB(m_onSave, "OnSave");
	assignCB(m_onEnd, "OnEnd");
}
