#include "framework.h"
#include "Game/items.h"

#include "Game/control/control.h"
#include "Game/collision/floordata.h"
#include "Game/effects/effects.h"
#include "Game/Lara/lara.h"
#include "ScriptInterfaceGame.h"
#include "Specific/setup.h"
#include "Specific/level.h"
#include "Objects/ScriptInterfaceObjectsHandler.h"

using namespace TEN::Floordata;

void ClearItem(short itemNumber)
{
	auto* item = &g_Level.Items[itemNumber];
	auto* room = &g_Level.Rooms[item->RoomNumber];

	item->Collidable = true;
	item->Data = nullptr;
	item->StartPose = item->Pose;
}

void KillItem(short const itemNumber)
{
	if (InItemControlLoop)
	{
		ItemNewRooms[2 * ItemNewRoomNo] = itemNumber | 0x8000;
		ItemNewRoomNo++;
	}
	else// if (NextItemActive != NO_ITEM)
	{
		auto* item = &g_Level.Items[itemNumber];

		DetatchSpark(itemNumber, SP_ITEM);

		item->Active = false;

		if (NextItemActive == itemNumber)
			NextItemActive = item->NextActive;
		else
		{
			short linkNumber;
			for (linkNumber = NextItemActive; linkNumber != NO_ITEM; linkNumber = g_Level.Items[linkNumber].NextActive)
			{
				if (g_Level.Items[linkNumber].NextActive == itemNumber)
				{
					g_Level.Items[linkNumber].NextActive = item->NextActive;
					break;
				}
			}
		}

		if (item->RoomNumber != NO_ROOM)
		{
			if (g_Level.Rooms[item->RoomNumber].itemNumber == itemNumber)
				g_Level.Rooms[item->RoomNumber].itemNumber = item->NextItem;
			else
			{
				short linkNumber;
				for (linkNumber = g_Level.Rooms[item->RoomNumber].itemNumber; linkNumber != NO_ITEM; linkNumber = g_Level.Items[linkNumber].NextItem)
				{
					if (g_Level.Items[linkNumber].NextItem == itemNumber)
					{
						g_Level.Items[linkNumber].NextItem = item->NextItem;
						break;
					}
				}
			}
		}

		if (item == Lara.TargetEntity)
			Lara.TargetEntity = NULL;

		if (Objects[item->ObjectNumber].floor != nullptr)
			UpdateBridgeItem(itemNumber, true);

		if (itemNumber >= g_Level.NumItems)
		{
			item->NextItem = NextItemFree;
			NextItemFree = itemNumber;
		}
		else
		{
			item->Flags |= IFLAG_KILLED;
		}

		g_GameScriptEntities->NotifyKilled(item);
		g_GameScriptEntities->TryRemoveColliding(itemNumber, true);
		if (!item->luaCallbackOnKilledName.empty())
		{
			g_GameScript->ExecuteFunction(item->luaCallbackOnKilledName, itemNumber);
		}

		item->LuaName.clear();
		item->luaCallbackOnKilledName.clear();
		item->luaCallbackOnHitName.clear();
		item->luaCallbackOnCollidedWithObjectName.clear();
		item->luaCallbackOnCollidedWithRoomName.clear();
	}
}

void RemoveAllItemsInRoom(short roomNumber, short objectNumber)
{
	auto* room = &g_Level.Rooms[roomNumber];

	short currentItemNumber = room->itemNumber;
	while (currentItemNumber != NO_ITEM)
	{
		auto* item = &g_Level.Items[currentItemNumber];

		if (item->ObjectNumber == objectNumber)
		{
			RemoveActiveItem(currentItemNumber);
			item->Status = ITEM_NOT_ACTIVE;
			item->Flags &= 0xC1;
		}

		currentItemNumber = item->NextItem;
	}
}

void AddActiveItem(short itemNumber)
{
	auto* item = &g_Level.Items[itemNumber];

	item->Flags |= 0x20;

	if (Objects[item->ObjectNumber].control == NULL)
	{
		item->Status = ITEM_NOT_ACTIVE;
		return;
	}

	if (!item->Active)
	{
		item->Active = true;
		item->NextActive = NextItemActive;
		NextItemActive = itemNumber;
	}
}

void ItemNewRoom(short itemNumber, short roomNumber)
{
	if (InItemControlLoop)
	{
		ItemNewRooms[2 * ItemNewRoomNo] = itemNumber;
		ItemNewRooms[2 * ItemNewRoomNo + 1] = roomNumber;
		ItemNewRoomNo++;
	}
	else
	{
		auto* item = &g_Level.Items[itemNumber];

		if (item->RoomNumber != NO_ROOM)
		{
			auto* room = &g_Level.Rooms[item->RoomNumber];

			if (room->itemNumber == itemNumber)
				room->itemNumber = item->NextItem;
			else
			{
				for (short linkNumber = room->itemNumber; linkNumber != -1; linkNumber = g_Level.Items[linkNumber].NextItem)
				{
					if (g_Level.Items[linkNumber].NextItem == itemNumber)
					{
						g_Level.Items[linkNumber].NextItem = item->NextItem;
						break;
					}
				}
			}
		}

		item->RoomNumber = roomNumber;
		item->NextItem = g_Level.Rooms[roomNumber].itemNumber;
		g_Level.Rooms[roomNumber].itemNumber = itemNumber;
	}
}

void EffectNewRoom(short fxNumber, short roomNumber)
{
	if (InItemControlLoop)
	{
		ItemNewRooms[2 * ItemNewRoomNo] = fxNumber;
		ItemNewRooms[2 * ItemNewRoomNo + 1] = roomNumber;
		ItemNewRoomNo++;
	}
	else
	{
		auto* fx = &EffectList[fxNumber];
		auto* room = &g_Level.Rooms[fx->roomNumber];

		if (room->fxNumber == fxNumber)
			room->fxNumber = fx->nextFx;
		else
		{
			for (short linkNumber = room->fxNumber; linkNumber != -1; linkNumber = EffectList[linkNumber].nextFx)
			{
				if (EffectList[linkNumber].nextFx == fxNumber)
				{
					EffectList[linkNumber].nextFx = fx->nextFx;
					break;
				}
			}
		}

		fx->roomNumber = roomNumber;
		fx->nextFx = g_Level.Rooms[roomNumber].fxNumber;
		g_Level.Rooms[roomNumber].fxNumber = fxNumber;
	}
}

void KillEffect(short fxNumber)
{
	if (InItemControlLoop)
	{
		ItemNewRooms[2 * ItemNewRoomNo] = fxNumber | 0x8000;
		ItemNewRoomNo++;
	}
	else
	{
		auto* fx = &EffectList[fxNumber];
		DetatchSpark(fxNumber, SP_FX);

		if (NextFxActive == fxNumber)
			NextFxActive = fx->nextActive;
		else
		{
			for (short linknum = NextFxActive; linknum != NO_ITEM; linknum = EffectList[linknum].nextActive)
			{
				if (EffectList[linknum].nextActive == fxNumber)
				{
					EffectList[linknum].nextActive = fx->nextActive;
					break;
				}
			}
		}

		if (g_Level.Rooms[fx->roomNumber].fxNumber == fxNumber)
			g_Level.Rooms[fx->roomNumber].fxNumber = fx->nextFx;
		else
		{
			for (short linknum = g_Level.Rooms[fx->roomNumber].fxNumber; linknum != NO_ITEM; linknum = EffectList[linknum].nextFx)
			{
				if (EffectList[linknum].nextFx == fxNumber)
				{
					EffectList[linknum].nextFx = fx->nextFx;
					break;
				}
			}
		}

		fx->nextFx = NextFxFree;
		NextFxFree = fxNumber;
	}
}

short CreateNewEffect(short roomNum) 
{
	short fxNumber = NextFxFree;

	if (NextFxFree != NO_ITEM)
	{
		auto* fx = &EffectList[NextFxFree];
		NextFxFree = fx->nextFx;

		auto* room = &g_Level.Rooms[roomNum];
		fx->roomNumber = roomNum;
		fx->nextFx = room->fxNumber;
		room->fxNumber = fxNumber;
		fx->nextActive = NextFxActive;
		NextFxActive = fxNumber;
		fx->shade = GRAY555;
	}

	return fxNumber;
}

void InitialiseFXArray(int allocateMemory)
{
	NextFxActive = NO_ITEM;
	NextFxFree = 0;

	for (int i = 0; i < NUM_EFFECTS; i++)
	{
		auto* fx = &EffectList[i];
		fx->nextFx = i + 1;
	}

	EffectList[NUM_EFFECTS - 1].nextFx = NO_ITEM;
}

void RemoveDrawnItem(short itemNumber) 
{
	auto* item = &g_Level.Items[itemNumber];

	if (g_Level.Rooms[item->RoomNumber].itemNumber == itemNumber)
		g_Level.Rooms[item->RoomNumber].itemNumber = item->NextItem;
	else
	{
		for (short linkNumber = g_Level.Rooms[item->RoomNumber].itemNumber; linkNumber != NO_ITEM; linkNumber = g_Level.Items[linkNumber].NextItem)
		{
			if (g_Level.Items[linkNumber].NextItem == itemNumber)
			{
				g_Level.Items[linkNumber].NextItem = item->NextItem;
				break;
			}
		}
	}
}

void RemoveActiveItem(short itemNumber) 
{
	auto& item = g_Level.Items[itemNumber];

	if (g_Level.Items[itemNumber].Active)
	{
		g_Level.Items[itemNumber].Active = false;

		if (NextItemActive == itemNumber)
			NextItemActive = g_Level.Items[itemNumber].NextActive;
		else
		{
			for (short linkNumber = NextItemActive; linkNumber != NO_ITEM; linkNumber = g_Level.Items[linkNumber].NextActive)
			{
				if (g_Level.Items[linkNumber].NextActive == itemNumber)
				{
					g_Level.Items[linkNumber].NextActive = g_Level.Items[itemNumber].NextActive;
					break;
				}
			}
		}

		g_GameScriptEntities->NotifyKilled(&item);
		if (!item.luaCallbackOnKilledName.empty())
		{
			g_GameScript->ExecuteFunction(item.luaCallbackOnKilledName, itemNumber);
		}
	}
}

void InitialiseItem(short itemNumber) 
{
	auto* item = &g_Level.Items[itemNumber];

	item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex;
	item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;

	item->Animation.RequiredState = 0;
	item->Animation.TargetState = g_Level.Anims[item->Animation.AnimNumber].ActiveState;
	item->Animation.ActiveState = g_Level.Anims[item->Animation.AnimNumber].ActiveState;

	item->Pose.Orientation.z = 0;
	item->Pose.Orientation.x = 0;

	item->Animation.VerticalVelocity = 0;
	item->Animation.Velocity = 0;

	item->ItemFlags[3] = 0;
	item->ItemFlags[2] = 0;
	item->ItemFlags[1] = 0;
	item->ItemFlags[0] = 0;

	item->Active = false;
	item->Status = ITEM_NOT_ACTIVE;
	item->Animation.Airborne = false;
	item->HitStatus = false;
	item->Collidable = true;
	item->LookedAt = false;

	item->Timer = 0;

	item->HitPoints = Objects[item->ObjectNumber].HitPoints;

	if (item->ObjectNumber == ID_HK_ITEM ||
		item->ObjectNumber == ID_HK_AMMO_ITEM ||
		item->ObjectNumber == ID_CROSSBOW_ITEM ||
		item->ObjectNumber == ID_REVOLVER_ITEM)
	{
		item->MeshBits = 1;
	}
	else
		item->MeshBits = -1;

	item->TouchBits = 0;
	item->AfterDeath = 0;
	item->SwapMeshFlags = 0;

	if (item->Flags & IFLAG_INVISIBLE)
	{
		item->Flags &= ~IFLAG_INVISIBLE;
		item->Status = ITEM_INVISIBLE;
	}
	else if (Objects[item->ObjectNumber].intelligent)
		item->Status = ITEM_INVISIBLE;

	if ((item->Flags & IFLAG_ACTIVATION_MASK) == IFLAG_ACTIVATION_MASK)
	{
		item->Flags &= ~IFLAG_ACTIVATION_MASK;
		item->Flags |= IFLAG_REVERSE;
		AddActiveItem(itemNumber);
		item->Status = ITEM_ACTIVE;
	}

	auto* room = &g_Level.Rooms[item->RoomNumber];

	item->NextItem = room->itemNumber;
	room->itemNumber = itemNumber;

	FloorInfo* floor = GetSector(room, item->Pose.Position.x - room->x, item->Pose.Position.z - room->z);
	item->Floor = floor->FloorHeight(item->Pose.Position.x, item->Pose.Position.z);
	item->BoxNumber = floor->Box;

	if (Objects[item->ObjectNumber].nmeshes > 0)
	{
		item->Animation.Mutator.resize(Objects[item->ObjectNumber].nmeshes);
		for (int i = 0; i < item->Animation.Mutator.size(); i++)
			item->Animation.Mutator[i] = {};
	}
	else
		item->Animation.Mutator.clear();

	if (Objects[item->ObjectNumber].initialise != NULL)
		Objects[item->ObjectNumber].initialise(itemNumber);
}

short CreateItem()
{
	short itemNumber = 0;

	if (NextItemFree == -1) return NO_ITEM;

	itemNumber = NextItemFree;
	g_Level.Items[NextItemFree].Flags = 0;
	NextItemFree = g_Level.Items[NextItemFree].NextItem;

	return itemNumber;
}

void InitialiseItemArray(int totalItem)
{
	auto* item = &g_Level.Items[g_Level.NumItems];

	NextItemActive = NO_ITEM;
	NextItemFree = g_Level.NumItems;

	if (g_Level.NumItems + 1 < totalItem)
	{
		for(int i = g_Level.NumItems + 1; i < totalItem; i++, item++)
		{
			item->NextItem = i;
			item->Active = false;
			item->Data = nullptr;
		}
	}

	item->NextItem = NO_ITEM;
}

short SpawnItem(ItemInfo* item, GAME_OBJECT_ID objectNumber)
{
	short itemNumber = CreateItem();
	if (itemNumber != NO_ITEM)
	{
		auto* spawn = &g_Level.Items[itemNumber];

		spawn->ObjectNumber = objectNumber;
		spawn->RoomNumber = item->RoomNumber;
		memcpy(&spawn->Pose, &item->Pose, sizeof(PHD_3DPOS));

		InitialiseItem(itemNumber);

		spawn->Status = ITEM_NOT_ACTIVE;
		spawn->Shade = 0x4210;
	}

	return itemNumber;
}

int GlobalItemReplace(short search, GAME_OBJECT_ID replace)
{
	int changed = 0;
	for (int i = 0; i < g_Level.Rooms.size(); i++)
	{
		auto* room = &g_Level.Rooms[i];

		for (short itemNumber = room->itemNumber; itemNumber != NO_ITEM; itemNumber = g_Level.Items[itemNumber].NextItem)
		{
			if (g_Level.Items[itemNumber].ObjectNumber == search)
			{
				g_Level.Items[itemNumber].ObjectNumber = replace;
				changed++;
			}
		}
	}

	return changed;
}

// Offset values may be used to account for the quirk of room traversal only being able to occur at portals.
// Note: may not work for dynamic items because of FindItem.
void UpdateItemRoom(ItemInfo* item, int height, int xOffset, int zOffset)
{
	float s = phd_sin(item->Pose.Orientation.y);
	float c = phd_cos(item->Pose.Orientation.y);

	int x = item->Pose.Position.x + roundf(c * xOffset + s * zOffset);
	int y = height + item->Pose.Position.y;
	int z = item->Pose.Position.z + roundf(-s * xOffset + c * zOffset);

	item->Location = GetRoom(item->Location, x, y, z);
	item->Floor = GetFloorHeight(item->Location, x, z).value_or(NO_HEIGHT);

	if (item->RoomNumber != item->Location.roomNumber)
		ItemNewRoom(FindItem(item), item->Location.roomNumber);
}

std::vector<int> FindAllItems(short objectNumber)
{
	std::vector<int> itemList;

	for (int i = 0; i < g_Level.NumItems; i++)
	{
		if (g_Level.Items[i].ObjectNumber == objectNumber)
			itemList.push_back(i);
	}

	return itemList;
}

ItemInfo* FindItem(int objectNumber)
{
	for (int i = 0; i < g_Level.NumItems; i++)
	{
		auto* item = &g_Level.Items[i];

		if (item->ObjectNumber == objectNumber)
			return item;
	}

	return 0;
}

int FindItem(ItemInfo* item)
{
	if (item == LaraItem)
		return Lara.ItemNumber;

	for (int i = 0; i < g_Level.NumItems; i++)
		if (item == &g_Level.Items[i])
			return i;

	return -1;
}
