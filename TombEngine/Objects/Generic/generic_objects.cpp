#include "framework.h"
#include "Objects/Generic/generic_objects.h"
#include "Objects/Generic/Object/objects.h"

#include "Game/pickup/pickup.h"
#include "Game/collision/collide_item.h"

// Objects
#include "Objects/Generic/Object/generic_trapdoor.h"
#include "Objects/Generic/Object/generic_bridge.h"
#include "Objects/Generic/Object/burning_torch.h"
#include "Objects/Generic/Object/polerope.h"
#include "Objects/Generic/Object/rope.h"

// Switches
#include "Objects/Generic/Switches/cog_switch.h"
#include "Objects/Generic/Switches/rail_switch.h"
#include "Objects/Generic/Switches/jump_switch.h"
#include "Objects/Generic/Switches/generic_switch.h"
#include "Objects/Generic/Switches/crowbar_switch.h"
#include "Objects/Generic/Switches/underwater_switch.h"
#include "Objects/Generic/Switches/pulley_switch.h"
#include "Objects/Generic/Switches/fullblock_switch.h"
#include "Objects/Generic/Switches/turn_switch.h"
#include "Objects/Generic/Switches/AirlockSwitch.h"

// Doors
#include "Objects/Generic/Doors/generic_doors.h"
#include "Objects/Generic/Doors/double_doors.h"
#include "Objects/Generic/Doors/pushpull_kick_door.h"
#include "Objects/Generic/Doors/sequence_door.h"
#include "Objects/Generic/Doors/steel_door.h"
#include "Objects/Generic/Doors/underwater_door.h"

// Traps
#include "Objects/Generic/Traps/dart_emitter.h"

/// Necessary import
#include "Specific/setup.h"

using namespace TEN::Entities::Switches;
using namespace TEN::Entities::Doors;
using namespace TEN::Entities::Traps;
using namespace TEN::Entities::Generic;

static void StartObject(ObjectInfo* object)
{
	for (int objectNumber = ID_TRAPDOOR1; objectNumber <= ID_TRAPDOOR3; objectNumber++)
	{
		object = &Objects[objectNumber];
		if (object->loaded)
		{
			object->Initialize = InitializeTrapDoor;
			object->collision = TrapDoorCollision;
			object->control = TrapDoorControl;
			object->floorBorder = TrapDoorFloorBorder;
			object->ceilingBorder = TrapDoorCeilingBorder;
			object->floor = TrapDoorFloor;
			object->ceiling = TrapDoorCeiling;
			object->SetupHitEffect(true);
		}
	}

	for (int objectNumber = ID_FLOOR_TRAPDOOR1; objectNumber <= ID_FLOOR_TRAPDOOR2; objectNumber++)
	{
		object = &Objects[objectNumber];
		if (object->loaded)
		{
			object->Initialize = InitializeTrapDoor;
			object->collision = FloorTrapDoorCollision;
			object->control = TrapDoorControl;
			object->floorBorder = TrapDoorFloorBorder;
			object->ceilingBorder = TrapDoorCeilingBorder;
			object->floor = TrapDoorFloor;
			object->ceiling = TrapDoorCeiling;
			object->SetupHitEffect(true);
		}
	}

	for (int objectNumber = ID_CEILING_TRAPDOOR1; objectNumber <= ID_CEILING_TRAPDOOR2; objectNumber++)
	{
		object = &Objects[objectNumber];
		if (object->loaded)
		{
			object->Initialize = InitializeTrapDoor;
			object->collision = CeilingTrapDoorCollision;
			object->control = TrapDoorControl;
			object->floorBorder = TrapDoorFloorBorder;
			object->ceilingBorder = TrapDoorCeilingBorder;
			object->floor = TrapDoorFloor;
			object->ceiling = TrapDoorCeiling;
			object->SetupHitEffect(true);
		}
	}

	object = &Objects[ID_BRIDGE_FLAT];
	if (object->loaded)
	{
		object->Initialize = InitializeBridge;
		object->floor = BridgeFloor<0>;
		object->ceiling = BridgeCeiling<0>;
		object->floorBorder = BridgeFloorBorder<0>;
		object->ceilingBorder = BridgeCeilingBorder<0>;
	}

	object = &Objects[ID_BRIDGE_TILT1];
	if (object->loaded)
	{
		object->Initialize = InitializeBridge;
		object->floor = BridgeFloor<1>;
		object->ceiling = BridgeCeiling<1>;
		object->floorBorder = BridgeFloorBorder<1>;
		object->ceilingBorder = BridgeCeilingBorder<1>;
	}

	object = &Objects[ID_BRIDGE_TILT2];
	if (object->loaded)
	{
		object->Initialize = InitializeBridge;
		object->floor = BridgeFloor<2>;
		object->ceiling = BridgeCeiling<2>;
		object->floorBorder = BridgeFloorBorder<2>;
		object->ceilingBorder = BridgeCeilingBorder<2>;
	}

	object = &Objects[ID_BRIDGE_TILT3];
	if (object->loaded)
	{
		object->Initialize = InitializeBridge;
		object->floor = BridgeFloor<3>;
		object->ceiling = BridgeCeiling<3>;
		object->floorBorder = BridgeFloorBorder<3>;
		object->ceilingBorder = BridgeCeilingBorder<3>;
	}

	object = &Objects[ID_BRIDGE_TILT4];
	if (object->loaded)
	{
		object->Initialize = InitializeBridge;
		object->floor = BridgeFloor<4>;
		object->ceiling = BridgeCeiling<4>;
		object->floorBorder = BridgeFloorBorder<4>;
		object->ceilingBorder = BridgeCeilingBorder<4>;
	}
}

void StartSwitches(ObjectInfo* object)
{
	object = &Objects[ID_COG_SWITCH];
	if (object->loaded)
	{
		object->collision = CogSwitchCollision;
		object->control = CogSwitchControl;
		object->SetupHitEffect(true);
	}

	object = &Objects[ID_LEVER_SWITCH];
	if (object->loaded)
	{
		object->collision = RailSwitchCollision;
		object->control = SwitchControl;
		object->SetupHitEffect(true);
	}

	object = &Objects[ID_JUMP_SWITCH];
	if (object->loaded)
	{
		object->collision = JumpSwitchCollision;
		object->control = SwitchControl;
		object->SetupHitEffect(true);
	}

	for (int objectNumber = ID_SWITCH_TYPE1; objectNumber <= ID_SWITCH_TYPE16; objectNumber++)
	{
		object = &Objects[objectNumber];
		if (object->loaded)
		{
			object->collision = SwitchCollision;
			object->control = SwitchControl;
			object->SetupHitEffect(true);
		}
	}

	object = &Objects[ID_AIRLOCK_SWITCH];
	if (object->loaded)
	{
		object->collision = AirlockSwitchCollision;
		object->control = SwitchControl;
		object->SetupHitEffect(true);
	}

	object = &Objects[ID_CROWBAR_SWITCH];
	if (object->loaded)
	{
		object->collision = CrowbarSwitchCollision;
		object->control = SwitchControl;
		object->SetupHitEffect(true);
	}

	object = &Objects[ID_MINECART_SWITCH];
	if (object->loaded)
	{
		object->Initialize = InitializeAnimating;
		object->control = AnimatingControl;
		object->collision = ObjectCollision;
		object->SetupHitEffect(true);
		object->shadowType = ShadowMode::All;
	}

	for (int objectNumber = ID_UNDERWATER_SWITCH1; objectNumber <= ID_UNDERWATER_SWITCH4; objectNumber++)
	{
		object = &Objects[objectNumber];
		if (object->loaded)
		{
			object->control = SwitchControl;
			object->collision = UnderwaterSwitchCollision;
		}
	}

	object = &Objects[ID_PULLEY];
	if (object->loaded)
	{
		object->Initialize = InitializePulleySwitch;
		object->control = SwitchControl;
		object->collision = PulleySwitchCollision;
	}

	object = &Objects[ID_TURN_SWITCH];
	if (object->loaded)
	{
		object->control = TurnSwitchControl;
		object->collision = TurnSwitchCollision;
		object->SetupHitEffect(true);
	}

	object = &Objects[ID_SEQUENCE_SWITCH1];
	if (object->loaded)
	{
		object->collision = FullBlockSwitchCollision;
		object->control = FullBlockSwitchControl;
		object->SetupHitEffect(true);
	}

	object = &Objects[ID_SEQUENCE_SWITCH2];
	if (object->loaded)
	{
		object->collision = FullBlockSwitchCollision;
		object->control = FullBlockSwitchControl;
		object->SetupHitEffect(true);
	}

	object = &Objects[ID_SEQUENCE_SWITCH3];
	if (object->loaded)
	{
		object->collision = FullBlockSwitchCollision;
		object->control = FullBlockSwitchControl;
		object->SetupHitEffect(true);
	}
}

void StartDoors(ObjectInfo* object)
{
	for (int objectNumber = ID_DOOR_TYPE1; objectNumber <= ID_DOOR_TYPE30; objectNumber++)
	{
		object = &Objects[objectNumber];
		if (object->loaded)
		{
			object->Initialize = InitializeDoor;
			object->control = DoorControl;
			object->collision = DoorCollision;
			object->SetupHitEffect(true);
		}
	}

	object = &Objects[ID_LIFT_DOORS1];
	if (object->loaded)
	{
		object->Initialize = InitializeDoor;
		object->control = DoorControl;
		object->SetupHitEffect(true);
	}

	object = &Objects[ID_LIFT_DOORS2];
	if (object->loaded)
	{
		object->Initialize = InitializeDoor;
		object->control = DoorControl;
		object->SetupHitEffect(true);
	}

	object = &Objects[ID_SEQUENCE_DOOR1];
	if (object->loaded)
	{
		object->Initialize = InitializeDoor;
		object->collision = DoorCollision;
		object->control = SequenceDoorControl;
		object->SetupHitEffect(true);
	}

	for (int i = ID_DOUBLE_DOORS1; i <= ID_DOUBLE_DOORS4; i++)
	{
		object = &Objects[i];
		if (object->loaded)
		{
			object->Initialize = InitializeDoor;
			object->collision = DoubleDoorCollision;
			object->control = PushPullKickDoorControl;
			object->SetupHitEffect(true);
		}
	}

	for (int i = ID_UNDERWATER_DOOR1; i <= ID_UNDERWATER_DOOR4; i++)
	{
		object = &Objects[i];
		if (object->loaded)
		{
			object->Initialize = InitializeDoor;
			object->collision = UnderwaterDoorCollision;
			object->control = PushPullKickDoorControl;
			object->SetupHitEffect(true);
		}
	}

	for (int objectNum = ID_PUSHPULL_DOOR1; objectNum <= ID_KICK_DOOR4; objectNum++)
	{
		object = &Objects[objectNum];
		if (object->loaded)
		{
			object->Initialize = InitializeDoor;
			object->collision = PushPullKickDoorCollision;
			object->control = PushPullKickDoorControl;
			object->SetupHitEffect(true);
		}
	}

	object = &Objects[ID_STEEL_DOOR];
	if (object->loaded)
	{
		object->Initialize = InitializeSteelDoor;
		object->collision = SteelDoorCollision;
		object->SetupHitEffect(true);
	}
}

void StartTraps(ObjectInfo* object)
{
	object = &Objects[ID_DARTS];
	if (object->loaded)
	{
		object->collision = ObjectCollision;
		object->control = DartControl;
		object->shadowType = ShadowMode::All;
	}

	object = &Objects[ID_DART_EMITTER];
	if (object->loaded)
	{
		object->control = DartEmitterControl;
		object->drawRoutine = nullptr;
		object->usingDrawAnimatingItem = false;
	}

	object = &Objects[ID_HOMING_DART_EMITTER];
	if (object->loaded)
	{
		object->control = DartEmitterControl;
		object->drawRoutine = nullptr;
		object->usingDrawAnimatingItem = false;
	}

	object = &Objects[ID_ROPE];
	if (object->loaded)
	{
		object->Initialize = InitializeRope;
		object->control = RopeControl;
		object->collision = RopeCollision;
		object->drawRoutine = nullptr;
		object->usingDrawAnimatingItem = false;
	}

	object = &Objects[ID_POLEROPE];
	if (object->loaded)
	{
		object->collision = PoleCollision;
		object->SetupHitEffect(true);
	}

	object = &Objects[ID_BURNING_TORCH_ITEM];
	if (object->loaded)
	{
		object->control = TorchControl;
		object->collision = PickupCollision;
		object->usingDrawAnimatingItem = true;
		object->isPickup = true;
	}
}

void InitializeGenericObjects()
{
	ObjectInfo* objToInit = nullptr;
	StartTraps(objToInit);
	StartObject(objToInit);
	StartSwitches(objToInit);
	StartDoors(objToInit);
}
