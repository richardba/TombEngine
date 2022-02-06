#include "frameworkandsol.h"
#include "ReservedScriptNames.h"
#include "Lara/lara.h"
#include "ObjectsHandler.h"
#include "ReservedScriptNames.h"
#include "ObjectIDs.h"

/***
Scripts that will be run on game startup.
@tentable Objects 
@pragma nostrip
*/

ObjectsHandler::ObjectsHandler(sol::state* lua, sol::table & parent) : LuaHandler{ lua }
{
	m_table_objects = sol::table{ m_lua->lua_state(), sol::create };
	parent.set(ScriptReserved_Objects, m_table_objects);

	/***
	Get a moveable by its name.
	@function GetMoveableByName
	@tparam string name the unique name of the Moveable as set in, or generated by, Tomb Editor
	@treturn Moveable a non-owning Moveable referencing the item.
	*/
	m_table_objects.set_function(ScriptReserved_GetMoveableByName, &ObjectsHandler::GetByName<Moveable, ScriptReserved_Moveable>, this);

	/***
	Get a Static by its name.
	@function GetStaticByName
	@tparam string name the unique name of the mesh as set in, or generated by, Tomb Editor
	@treturn Static a non-owning Static referencing the mesh.
	*/
	m_table_objects.set_function(ScriptReserved_GetStaticByName, &ObjectsHandler::GetByName<Static, ScriptReserved_Static>, this);

	/***
	Get a CameraInfo by its name.
	@function GetCameraByName
	@tparam string name the unique name of the camera as set in, or generated by, Tomb Editor
	@treturn CameraInfo a non-owning CameraInfo referencing the camera.
	*/
	m_table_objects.set_function(ScriptReserved_GetCameraByName, &ObjectsHandler::GetByName<Camera, ScriptReserved_Camera>, this);

	/***
	Get a SinkInfo by its name.
	@function GetSinkByName
	@tparam string name the unique name of the sink as set in, or generated by, Tomb Editor
	@treturn SinkInfo a non-owning SinkInfo referencing the sink.
	*/
	m_table_objects.set_function(ScriptReserved_GetSinkByName, &ObjectsHandler::GetByName<GameScriptSinkInfo, ScriptReserved_SinkInfo>, this);

	/***
	Get a SoundSourceInfo by its name.
	@function GetSoundSourceByName
	@tparam string name the unique name of the sink as set in, or generated by, Tomb Editor
	@treturn SoundSourceInfo a non-owning SoundSourceInfo referencing the sink.
	*/
	m_table_objects.set_function(ScriptReserved_GetSoundSourceByName, &ObjectsHandler::GetByName<GameScriptSoundSourceInfo, ScriptReserved_SoundSourceInfo>, this);

	Moveable::Register(m_table_objects);
	Moveable::SetNameCallbacks(
		[this](auto && ... param) { return AddName(std::forward<decltype(param)>(param)...); },
		[this](auto && ... param) { return RemoveName(std::forward<decltype(param)>(param)...); }
	);

	Static::Register(m_table_objects);
	Static::SetNameCallbacks(
		[this](auto && ... param) { return AddName(std::forward<decltype(param)>(param)...); },
		[this](auto && ... param) { return RemoveName(std::forward<decltype(param)>(param)...); }
	);

	Camera::Register(m_table_objects);
	Camera::SetNameCallbacks(
		[this](auto && ... param) { return AddName(std::forward<decltype(param)>(param)...); },
		[this](auto && ... param) { return RemoveName(std::forward<decltype(param)>(param)...); }
	);

	GameScriptSinkInfo::Register(m_lua);
	GameScriptSinkInfo::SetNameCallbacks(
		[this](auto && ... param) { return AddName(std::forward<decltype(param)>(param)...); },
		[this](auto && ... param) { return RemoveName(std::forward<decltype(param)>(param)...); }
	);

	AIObject::Register(m_table_objects);
	AIObject::SetNameCallbacks(
		[this](auto && ... param) { return AddName(std::forward<decltype(param)>(param)...); },
		[this](auto && ... param) { return RemoveName(std::forward<decltype(param)>(param)...); }
	);

	GameScriptSoundSourceInfo::Register(m_lua);
	GameScriptSoundSourceInfo::SetNameCallbacks(
		[this](auto && ... param) { return AddName(std::forward<decltype(param)>(param)...); },
		[this](auto && ... param) { return RemoveName(std::forward<decltype(param)>(param)...); }
	);

	MakeReadOnlyTable(ScriptReserved_ObjID, kObjIDs);
}

void ObjectsHandler::AssignLara()
{
	m_table_objects.set("Lara", Moveable(Lara.itemNumber, false));
}

