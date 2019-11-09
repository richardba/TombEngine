#include "switch.h"
#include "door.h"
#include "items.h"
#include "lot.h"
#include "objects.h"
#include "collide.h"
#include "Lara.h"
#include "inventory.h"
#include "draw.h"
#include "sphere.h"

byte SequenceUsed[6];
byte SequenceResults[3][3][3];
byte Sequences[3];
byte CurrentSequence;

__int16 Switch2Bounds[12] =  
{
	0xFC00, 0x0400, 0xFC00, 0x0400, 0xFC00, 0x0200, 0xC720, 0x38E0, 0xC720, 0x38E0,
	0xC720, 0x38E0
};
PHD_VECTOR Switch2Position = { 0, 0, 108 };  
__int16 TurnSwitchBoundsA[12] = // offset 0xA14D8
{
	0x0200, 0x0380, 0x0000, 0x0000, 0xFE00, 0x0000, 0xF8E4, 0x071C, 0xEAAC, 0x1554,
	0xF8E4, 0x071C
};
PHD_VECTOR TurnSwitchPos = { 650, 0, 138 }; // offset 0xA14F0
__int16 TurnSwitchBoundsC[12] = // offset 0xA14FC
{
	0x0200, 0x0380, 0x0000, 0x0000, 0x0000, 0x0200, 0xF8E4, 0x071C, 0xEAAC, 0x1554,
	0xF8E4, 0x071C
};
PHD_VECTOR TurnSwitchPosA = { 650, 0, -138 }; // offset 0xA1514
PHD_VECTOR CogSwitchPos = { 0, 0, -856 }; // offset 0xA1520
__int16 CogSwitchBounds[12] = // offset 0xA152C
{
	0xFE00, 0x0200, 0x0000, 0x0000, 0xFA00, 0xFE00, 0xF8E4, 0x071C, 0xEAAC, 0x1554,
	0xF8E4, 0x071C
};
PHD_VECTOR RailSwitchPos = { 0, 0, -550 }; // offset 0xA1544
__int16 RailSwitchBounds[12] = // offset 0xA1550
{
	0xFF00, 0x0100, 0x0000, 0x0000, 0xFD00, 0xFE00, 0xF8E4, 0x071C, 0xEAAC, 0x1554,
	0xF8E4, 0x071C
};
PHD_VECTOR RailSwitchPos2 = { 0, 0, 550 }; // offset 0xA1568
__int16 RailSwitchBounds2[12] = // offset 0xA1574
{
	0xFF00, 0x0100, 0x0000, 0x0000, 0x0200, 0x0300, 0xF8E4, 0x071C, 0xEAAC, 0x1554,
	0xF8E4, 0x071C
};
__int16 JumpSwitchBounds[12] = // offset 0xA158C
{
	0xFF80, 0x0080, 0xFF00, 0x0100, 0x0180, 0x0200, 0xF8E4, 0x071C, 0xEAAC, 0x1554,
	0xF8E4, 0x071C
};
PHD_VECTOR JumpSwitchPos = { 0, -208, 256 }; // offset 0xA15A4
PHD_VECTOR CrowbarPos = { -89, 0, -328 }; // offset 0xA15B0
__int16 CrowbarBounds[12] = // offset 0xA15BC
{
	0xFF00, 0x0100, 0x0000, 0x0000, 0xFE00, 0xFF00, 0xF8E4, 0x071C, 0xEAAC, 0x1554,
	0xF8E4, 0x071C
};
PHD_VECTOR CrowbarPos2 = { 89, 0, 328 }; // offset 0xA15D4
__int16 CrowbarBounds2[12] = // offset 0xA15E0
{
	0xFF00, 0x0100, 0x0000, 0x0000, 0x0100, 0x0200, 0xF8E4, 0x071C, 0xEAAC, 0x1554,
	0xF8E4, 0x071C
};
__int16 FullBlockSwitchBounds[12] = // offset 0xA15F8
{
	0xFE80, 0x0180, 0x0000, 0x0100, 0x0000, 0x0200, 0xF8E4, 0x071C, 0xEAAC, 0x1554,
	0xF8E4, 0x071C
};
PHD_VECTOR FullBlockSwitchPos = { 0, 0, 0 }; // offset 0xA1610
__int16 PulleyBounds[12] = // offset 0xA161C
{
	0xFF00, 0x0100, 0x0000, 0x0000, 0xFE00, 0x0200, 0xF8E4, 0x071C, 0xEAAC, 0x1554,
	0xF8E4, 0x071C
};
PHD_VECTOR PulleyPos = { 0, 0, -148 }; // offset 0xA1634
PHD_VECTOR CrowDovePos = { 0, 0, -400 }; // offset 0xA1640
__int16 UnderwaterSwitchBounds[12] = // offset 0xA164C
{
	0xFF00, 0x0100, 0xFB00, 0xFE00, 0xFE00, 0x0000, 0xC720, 0x38E0, 0xC720, 0x38E0,
	0xC720, 0x38E0
};
__int16 UnderwaterSwitchBounds2[12] = // offset 0xA1664
{
	0xFF00, 0x0100, 0xFB00, 0xFE00, 0x0000, 0x0200, 0xC720, 0x38E0, 0xC720, 0x38E0,
	0xC720, 0x38E0
};
PHD_VECTOR UnderwaterSwitchPos = { 0, -736, -416 }; // offset 0xA167C
PHD_VECTOR UnderwaterSwitchPos2 = { 0, -736, 416 }; // offset 0xA1688
__int16 SwitchBounds[12] = // offset 0xA1694
{
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xF8E4, 0x071C, 0xEAAC, 0x1554,
	0xF8E4, 0x071C
};
PHD_VECTOR SwitchPos = { 0, 0, 0 }; // offset 0xA16AC

void __cdecl ProcessExplodingSwitchType8(ITEM_INFO* item) 
{
	PHD_VECTOR pos;
	pos.x = 0;
	pos.y = 0;
	pos.z = 0;
	GetJointAbsPosition(item, &pos, 0);
	TestTriggersAtXYZ(pos.x, pos.y, pos.z, item->roomNumber, 1, 0);
	ExplodeItemNode(item, Objects[item->objectNumber].nmeshes - 1, 0, 64);
	item->meshBits |= 1 << ((Objects[item->objectNumber].nmeshes & 0xFF) - 2);
}

void __cdecl CrowDoveSwitchCollision(__int16 itemNum, ITEM_INFO* l, COLL_INFO* coll) 
{
	ITEM_INFO* item = &Items[itemNum];

	if (item->flags & 0x100
		|| !(item->meshBits & 4)
		|| (!(TrInput & IN_ACTION) || Lara.gunStatus || l->currentAnimState != STATE_LARA_STOP || l->animNumber != ANIMATION_LARA_STAY_IDLE || l->gravityStatus)
		&& (!Lara.isMoving || Lara.generalPtr != (void*)itemNum))
	{
		if (l->currentAnimState != STATE_LARA_DOVESWITCH)
			ObjectCollision(itemNum, l, coll);
	}
	else
	{
		__int32 oldYrot = item->pos.yRot;
		item->pos.yRot = l->pos.yRot;
		if (TestLaraPosition(PulleyBounds, item, l))
		{
			if (MoveLaraPosition(&CrowDovePos, item, l))
			{
				l->animNumber = ANIMATION_LARA_DOVESWITCH_TURN;
				l->currentAnimState = STATE_LARA_DOVESWITCH;
				l->frameNumber = Anims[l->animNumber].frameBase;
				
				AddActiveItem(itemNum);

				item->status = ITEM_ACTIVE;
				item->pos.yRot = oldYrot;
				Lara.isMoving = false;
				Lara.headYrot = 0;
				Lara.headXrot = 0;
				Lara.torsoYrot = 0;
				Lara.torsoXrot = 0;
				Lara.gunStatus = LG_HANDS_BUSY;
				Lara.generalPtr = (void*)item;
			}
			else
			{
				Lara.generalPtr = (void*)itemNum;
			}
			item->pos.yRot = oldYrot;
		}
		else
		{
			if (Lara.isMoving && Lara.generalPtr == (void*)itemNum)
			{
				Lara.isMoving = false;
				Lara.gunStatus = LG_NO_ARMS;
			}
			item->pos.yRot = oldYrot;
		}
	}
}

void __cdecl CrowDoveSwitchControl(__int16 itemNumber) 
{
	ITEM_INFO* item = &Items[itemNumber];

	if ((item->meshBits & 2))
	{
		ExplodeItemNode(item, 1, 0, 0x100);
		SoundEffect(SFX_RAVENSWITCH_EXP, &item->pos, 0);
		item->meshBits = 5;
		RemoveActiveItem(itemNumber);
	}
	else
	{
		if (item->currentAnimState == 0)
			item->goalAnimState = 1;

		AnimateItem(item);

		if (item->currentAnimState == 0)
			item->pos.yRot += ANGLE(90);
	}
}

void __cdecl CogSwitchCollision(__int16 itemNum, ITEM_INFO* l, COLL_INFO* coll) 
{
	ITEM_INFO* item = &Items[itemNum];
	DOOR_DATA* door = (DOOR_DATA*)item->data;

	FLOOR_INFO* floor = GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &item->roomNumber);
	GetFloorHeight(floor, item->pos.xPos, item->pos.yPos, item->pos.zPos);
	__int16* trigger = TriggerIndex;
	for (__int32 i = *TriggerIndex; (i & 0x1F) != 4; trigger += 2)
	{
		if (i < 0)
			break;
		i = *(trigger + 2);
	}

	ITEM_INFO* target = &Items[*(trigger + 6) & 0x3FF];

	if (item->status == ITEM_INACTIVE)
	{
		if (!(item->flags & 0x100)
			&& (TrInput & IN_ACTION && 
				!Lara.gunStatus && 
				!(item->status && item->gravityStatus) &&
				l->currentAnimState == STATE_LARA_STOP &&
				l->animNumber == ANIMATION_LARA_STAY_IDLE  
				|| Lara.isMoving && Lara.generalPtr == (void*)itemNum))
		{
			if (TestLaraPosition(CogSwitchBounds, item, l))
			{
				if (MoveLaraPosition(&CogSwitchPos, item, l))
				{
					Lara.isMoving = false;
					Lara.headYrot = 0;
					Lara.headXrot = 0;
					Lara.torsoYrot = 0;
					Lara.torsoXrot = 0;
					Lara.gunStatus = LG_HANDS_BUSY;
					Lara.generalPtr = target;
					l->animNumber = ANIMATION_LARA_COGWHEEL_GRAB;
					l->goalAnimState = STATE_LARA_COGWHEEL;
					l->currentAnimState = STATE_LARA_COGWHEEL;
					l->frameNumber = Anims[l->animNumber].frameBase;

					AddActiveItem(itemNum);
					
					item->goalAnimState = 1;
					item->status = ITEM_ACTIVE;
					if (!door->opened)
					{
						AddActiveItem(target - Items);
						target->itemFlags[2] = target->pos.yPos;
						target->status = ITEM_ACTIVE;
					}
				}
				else
				{
					Lara.generalPtr = (void*)itemNum;
				}
				return;
			}
			if (Lara.isMoving && Lara.generalPtr == (void*)itemNum)
			{
				Lara.isMoving = false;
				Lara.gunStatus = LG_NO_ARMS;
			}
		}

		ObjectCollision(itemNum, l, coll);
	}
}

void __cdecl CogSwitchControl(__int16 itemNum) 
{
	ITEM_INFO* item = &Items[itemNum];
	
	AnimateItem(item);

	if (item->currentAnimState == 1)
	{
		if (item->goalAnimState == 1 && !(TrInput & IN_ACTION))
		{
			LaraItem->goalAnimState = 2;
			item->goalAnimState = 0;
		}

		if (LaraItem->animNumber == ANIMATION_LARA_COGWHEEL_PULL)
		{
			if (LaraItem->frameNumber == (Anims[ANIMATION_LARA_COGWHEEL_PULL].frameBase + 10))
			{
				ITEM_INFO* it = (ITEM_INFO*)Lara.generalPtr;
				it->itemFlags[0] = 40;
				Lara.generalPtr = it;
				SoundEffect(SFX_STONE_SCRAPE_FAST, &it->pos, 0);
			}
		}
	}
	else
	{
		if (item->frameNumber == Anims[item->animNumber].frameEnd)
		{
			item->currentAnimState = 0;
			item->status = ITEM_INACTIVE;
			RemoveActiveItem(itemNum);
			LaraItem->animNumber = ANIMATION_LARA_STAY_SOLID;
			LaraItem->frameNumber = Anims[LaraItem->animNumber].frameBase;
			LaraItem->goalAnimState = STATE_LARA_STOP;
			LaraItem->currentAnimState = STATE_LARA_STOP;
			Lara.gunStatus = LG_NO_ARMS;
		}
	}
}

void __cdecl FullBlockSwitchCollision(__int16 itemNum, ITEM_INFO* l, COLL_INFO* coll) 
{
	ITEM_INFO* item = &Items[itemNum];
	
	if ((!(TrInput & IN_ACTION)
		|| item->status 
		|| item->flags & 0x100
		|| CurrentSequence >= 3u
		|| Lara.gunStatus
		|| l->currentAnimState != STATE_LARA_STOP
		|| l->animNumber != ANIMATION_LARA_STAY_IDLE)
		&& (!Lara.isMoving || Lara.generalPtr != (void*)itemNum))
	{
		ObjectCollision(itemNum, l, coll);
		return;
	}
	
	if (TestLaraPosition(FullBlockSwitchBounds, item, l))
	{
		if (MoveLaraPosition(&FullBlockSwitchPos, item, l))
		{
			if (item->currentAnimState == 1)
			{
				l->currentAnimState = STATE_LARA_SWITCH_DOWN;
				l->animNumber = ANIMATION_LARA_GIANT_BUTTON_PUSH;
				item->goalAnimState = 0;
			}
			l->goalAnimState = STATE_LARA_STOP;
			l->frameNumber = Anims[l->animNumber].frameBase;
			item->status = ITEM_ACTIVE;
			
			AddActiveItem(itemNum);
			AnimateItem(item);

			Lara.isMoving = false;
			Lara.headYrot = 0;
			Lara.headXrot = 0;
			Lara.torsoYrot = 0;
			Lara.torsoXrot = 0;
			Lara.gunStatus = LG_HANDS_BUSY;
		}
		else
		{
			Lara.generalPtr = (void*)itemNum;
		}
	}
	else if (Lara.isMoving && Lara.generalPtr == (void*)itemNum)
	{
		Lara.isMoving = false;
		Lara.gunStatus = LG_NO_ARMS;
	}
}

void __cdecl FullBlockSwitchControl(__int16 itemNumber) 
{
	ITEM_INFO* item = &Items[itemNumber];
	
	if (item->animNumber != Objects[item->objectNumber].animIndex + 2
		|| CurrentSequence >= 3u
		|| item->itemFlags[0])
	{
		if (CurrentSequence >= 4u)
		{
			item->itemFlags[0] = 0;
			item->goalAnimState = 1;
			item->status = ITEM_INACTIVE;
			if (++CurrentSequence >= 7u)
				CurrentSequence = 0;
		}
	}
	else
	{
		item->itemFlags[0] = 1;
		Sequences[CurrentSequence++] = item->triggerFlags;
	}

	AnimateItem(item);
}

void __cdecl CrowbarSwitchCollision(__int16 itemNum, ITEM_INFO* l, COLL_INFO* coll) 
{
	__int32 doSwitch = 0;
	ITEM_INFO* item = &Items[itemNum];

	if ((!(TrInput & IN_ACTION) && InventoryItemChosen != ID_CROWBAR_ITEM
		|| l->currentAnimState != STATE_LARA_STOP
		|| l->animNumber != ANIMATION_LARA_STAY_IDLE
		|| Lara.gunStatus
		|| item->itemFlags[0])
		&& (!Lara.isMoving|| Lara.generalPtr != (void*)itemNum))
	{
		ObjectCollision(itemNum, l, coll);
		return;
	}
	
	if (item->currentAnimState)
	{
		if (item->currentAnimState != 1)
		{
			ObjectCollision(itemNum, l, coll);
			return;
		}

		l->pos.yRot ^= (__int16)ANGLE(180);
		if (TestLaraPosition(CrowbarBounds2, item, l))
		{
			if (Lara.isMoving || InventoryItemChosen == ID_CROWBAR_ITEM)
			{
				if (MoveLaraPosition(&CrowbarPos2, item, l))
				{
					l->animNumber = ANIMATION_LARA_CROWBAR_USE_ON_FLOOR;
					doSwitch = 1;
					l->frameNumber = Anims[l->animNumber].frameBase;
					item->goalAnimState = 0;
				}
				else
				{
					Lara.generalPtr = (void*)itemNum;
				}
				InventoryItemChosen = NO_ITEM;
			}
			else
			{
				doSwitch = -1;
			}
		}
		else if (Lara.isMoving && Lara.generalPtr == (void*)itemNum)
		{
			Lara.isMoving = false;
			Lara.gunStatus = LG_NO_ARMS;
		}
		l->pos.yRot ^= (__int16)ANGLE(180);
	}
	else
	{
		if (!TestLaraPosition(CrowbarBounds, item, l))
		{
			if (Lara.isMoving && Lara.generalPtr == (void*)itemNum)
			{
				Lara.isMoving = false;
				Lara.gunStatus = LG_NO_ARMS;
			}
			ObjectCollision(itemNum, l, coll);
			return;
		}

		if (!(Lara.isMoving && InventoryItemChosen != ID_CROWBAR_ITEM))
		{
			if (Lara.crowbar)
				EnterInventoryItem = ID_CROWBAR_ITEM;
			else
			{
				if (OldPickupPos.x != l->pos.xPos || OldPickupPos.y != l->pos.yPos || OldPickupPos.z != l->pos.zPos)
				{
					OldPickupPos.x = l->pos.xPos;
					OldPickupPos.y = l->pos.yPos;
					OldPickupPos.z = l->pos.zPos;
					SayNo();
				}
			}
			return;
		}

		if (MoveLaraPosition(&CrowbarPos, item, l))
		{
			l->animNumber = ANIMATION_LARA_CROWBAR_USE_ON_FLOOR;
			doSwitch = 1;
			l->frameNumber = Anims[l->animNumber].frameBase;
			item->goalAnimState = 1;
		}
		else
		{
			Lara.generalPtr = (void*)itemNum;
		}

		InventoryItemChosen = NO_ITEM;
	}

	if (!doSwitch)
	{
		ObjectCollision(itemNum, l, coll);
		return;
	}

	if (doSwitch != -1)
	{
		l->goalAnimState = STATE_LARA_SWITCH_DOWN;
		l->currentAnimState = STATE_LARA_SWITCH_DOWN;
		Lara.isMoving = false;
		Lara.headYrot = 0;
		Lara.headXrot = 0;
		Lara.torsoYrot = 0;
		Lara.torsoXrot = 0;
		Lara.gunStatus = LG_HANDS_BUSY;
		item->status = ITEM_ACTIVE;

		AddActiveItem(itemNum);
		AnimateItem(item);

		return;
	}

	if (Lara.crowbar)
		EnterInventoryItem = ID_CROWBAR_ITEM;
	else
	{
		if (OldPickupPos.x != l->pos.xPos || OldPickupPos.y != l->pos.yPos || OldPickupPos.z != l->pos.zPos)
		{
			OldPickupPos.x = l->pos.xPos;
			OldPickupPos.y = l->pos.yPos;
			OldPickupPos.z = l->pos.zPos;
			SayNo();
		}
	}
}

void __cdecl JumpSwitchCollision(__int16 itemNum, ITEM_INFO* l, COLL_INFO* coll) 
{
	ITEM_INFO* item = &Items[itemNum];
	
	if (TrInput & IN_ACTION)
	{
		if (!Lara.gunStatus)
		{
			if (l->currentAnimState == STATE_LARA_REACH || l->currentAnimState == STATE_LARA_JUMP_UP)
			{
				if (l->status || l->gravityStatus)
				{
					if (l->fallspeed > 0 && !item->currentAnimState)
					{
						if (TestLaraPosition(JumpSwitchBounds, item, l))
						{
							AlignLaraPosition(&JumpSwitchPos, item, l);

							l->currentAnimState = STATE_LARA_SWITCH_DOWN;
							l->animNumber = ANIMATION_LARA_JUMPSWITCH;
							l->fallspeed = 0;
							l->frameNumber = Anims[l->animNumber].frameBase;
							l->gravityStatus = false;
							Lara.gunStatus = LG_HANDS_BUSY;
							
							item->goalAnimState = 1;
							item->status = ITEM_ACTIVE;
							
							AddActiveItem(itemNum);
						}
					}
				}
			}
		}
	}
}

void __cdecl RailSwitchCollision(__int16 itemNum, ITEM_INFO* l, COLL_INFO* coll) 
{
	__int32 flag = 0;
	ITEM_INFO* item = &Items[itemNum];

	if ((!(TrInput & IN_ACTION) || l->currentAnimState != STATE_LARA_STOP || l->animNumber != ANIMATION_LARA_STAY_IDLE || Lara.gunStatus)
		&& (!Lara.isMoving || Lara.generalPtr != (void*)itemNum))
	{
		ObjectCollision(itemNum, l, coll);
	}
	
	if (item->currentAnimState)
	{
		if (item->currentAnimState == 1)
		{
			l->pos.yRot ^= (__int16)ANGLE(180);
			if (TestLaraPosition(RailSwitchBounds2, item, l))
			{
				if (MoveLaraPosition(&RailSwitchPos2, item, l))
				{
					item->goalAnimState = 0;
					flag = 1;
				}
				else
				{
					Lara.generalPtr = (void*)itemNum;
				}
			}
			else if (Lara.isMoving && Lara.generalPtr == (void*)itemNum)
			{
				Lara.isMoving = false;
				Lara.gunStatus = LG_NO_ARMS;
			}
			l->pos.yRot ^= (__int16)ANGLE(180);
			if (flag)
			{
				item->goalAnimState = 1;
				l->animNumber = ANIMATION_LARA_LEVERSWITCH_PUSH;
				l->frameNumber = Anims[l->animNumber].frameBase;
				l->goalAnimState = STATE_LARA_LEVERSWITCH_PUSH;
				l->currentAnimState = STATE_LARA_LEVERSWITCH_PUSH;
				Lara.isMoving = false;
				Lara.headYrot = 0;
				Lara.headXrot = 0;
				Lara.torsoYrot = 0;
				Lara.torsoXrot = 0;
				Lara.gunStatus = LG_HANDS_BUSY;

				item->status = ITEM_ACTIVE;
				AddActiveItem(itemNum);
				AnimateItem(item);

				return;
			}
		}
		
		ObjectCollision(itemNum, l, coll);
	}

	if (!TestLaraPosition(RailSwitchBounds, item, l))
	{
		if (Lara.isMoving)
		{
			if (Lara.generalPtr == (void*)itemNum)
			{
				Lara.isMoving = false;
				Lara.gunStatus = LG_NO_ARMS;
			}
		}
		
		ObjectCollision(itemNum, l, coll);
	}

	if (!MoveLaraPosition(&RailSwitchPos, item, l))
	{
		Lara.generalPtr = (void*)itemNum;
		ObjectCollision(itemNum, l, coll);
	}

	item->goalAnimState = 1;
	l->animNumber = ANIMATION_LARA_LEVERSWITCH_PUSH;
	l->frameNumber = Anims[l->animNumber].frameBase;
	l->goalAnimState = STATE_LARA_LEVERSWITCH_PUSH;
	l->currentAnimState = STATE_LARA_LEVERSWITCH_PUSH;
	Lara.isMoving = false;
	Lara.headYrot = 0;
	Lara.headXrot = 0;
	Lara.torsoYrot = 0;
	Lara.torsoXrot = 0;
	Lara.gunStatus = LG_HANDS_BUSY;
	
	item->status = ITEM_ACTIVE;
	AddActiveItem(itemNum);
	AnimateItem(item);
}

void __cdecl TurnSwitchCollision(__int16 itemNum, ITEM_INFO* l, COLL_INFO* coll) 
{
	ITEM_INFO* item = &Items[itemNum];
	__int32 flag = 0;

	if (item->currentAnimState
		&& TrInput & IN_ACTION
		&& l->currentAnimState == STATE_LARA_STOP
		&& l->animNumber == ANIMATION_LARA_STAY_IDLE
		&& !l->gravityStatus
		&& Lara.gunStatus == LG_NO_ARMS
		|| Lara.isMoving && Lara.generalPtr == (void*)itemNum)
	{
		__int16 ItemNos[8];
		if (TestLaraPosition(TurnSwitchBoundsA, item, l))
		{
			if (MoveLaraPosition(&TurnSwitchPosA, item, l))
			{
				l->animNumber = ANIMATION_LARA_ROUND_HANDLE_GRAB_COUNTERCLOCKWISE;
				l->frameNumber = Anims[ANIMATION_LARA_ROUND_HANDLE_GRAB_COUNTERCLOCKWISE].frameBase;
				item->animNumber = Objects[item->objectNumber].animIndex + 4;
				item->frameNumber = Anims[item->animNumber].frameBase;
				item->itemFlags[0] = 1;
				ForcedFixedCamera.x = item->pos.xPos - (1024 * SIN(item->pos.yRot) >> W2V_SHIFT);
				ForcedFixedCamera.z = item->pos.zPos - (1024 * COS(item->pos.yRot) >> W2V_SHIFT);
				Lara.isMoving = 0;
				Lara.headYrot = 0;
				Lara.headXrot = 0;
				Lara.torsoYrot = 0;
				Lara.torsoXrot = 0;
				Lara.gunStatus = LG_HANDS_BUSY;
				l->currentAnimState = ANIMATION_LARA_TRY_HANG_SOLID;

				UseForcedFixedCamera = true;
				ForcedFixedCamera.y = item->pos.yPos - 2048;
				ForcedFixedCamera.roomNumber = item->roomNumber;

				AddActiveItem(itemNum);

				item->status = ITEM_ACTIVE;
				item->itemFlags[1] = 0;

				if (GetSwitchTrigger(item, ItemNos, 0))
				{
					if (!TriggerActive(&Items[ItemNos[0]]))
					{
						Items[ItemNos[0]].animNumber = Objects[Items[ItemNos[0]].objectNumber].animIndex;
						Items[ItemNos[0]].frameNumber = Anims[Items[ItemNos[0]].animNumber].frameBase;
					}
				}
				return;
			}
			Lara.generalPtr = (void*)itemNum;
		}
		else
		{
			l->pos.yRot ^= (__int16)ANGLE(180);
			if (TestLaraPosition(TurnSwitchBoundsC, item, l))
			{
				if (MoveLaraPosition(&TurnSwitchPos, item, l))
				{
					l->animNumber = 319;
					flag = 1;
					l->frameNumber = Anims[319].frameBase;
					item->itemFlags[0] = 2;
					ForcedFixedCamera.x = item->pos.xPos + (1024 * SIN(item->pos.yRot) >> W2V_SHIFT);
					ForcedFixedCamera.z = item->pos.zPos + (1024 * COS(item->pos.yRot) >> W2V_SHIFT);
				}
				else
				{
					Lara.generalPtr = (void*)itemNum;
				}
			}
			else if (Lara.isMoving && Lara.generalPtr == (void*)itemNum)
			{
				Lara.isMoving = FALSE;
				Lara.gunStatus = LG_NO_ARMS;
			}
			l->pos.yRot ^= (__int16)ANGLE(180);
			if (flag)
			{
				Lara.isMoving = 0;
				Lara.headYrot = 0;
				Lara.headXrot = 0;
				Lara.torsoYrot = 0;
				Lara.torsoXrot = 0;
				Lara.gunStatus = LG_HANDS_BUSY;
				l->currentAnimState = ANIMATION_LARA_TRY_HANG_SOLID;
				UseForcedFixedCamera = TRUE;
				ForcedFixedCamera.y = item->pos.yPos - 2048;
				ForcedFixedCamera.roomNumber = item->roomNumber;
				AddActiveItem(itemNum);
				item->status = ITEM_ACTIVE;
				item->itemFlags[1] = 0;
				if (GetSwitchTrigger(item, ItemNos, 0))
				{
					if (!TriggerActive(&Items[ItemNos[0]]))
					{
						Items[ItemNos[0]].animNumber = Objects[Items[ItemNos[0]].objectNumber].animIndex + 4;
						Items[ItemNos[0]].frameNumber = Anims[Items[ItemNos[0]].animNumber].frameBase;
					}
				}
				return;
			}
		}
	}

	if (coll->enableBaddiePush && TestBoundsCollide(item, l, coll->radius))
	{
		GlobalCollisionBounds.X1 = -512;
		GlobalCollisionBounds.X2 = 512;
		GlobalCollisionBounds.Y1 = -512;
		GlobalCollisionBounds.Y2 = 0;
		GlobalCollisionBounds.Z1 = -512;
		GlobalCollisionBounds.Z2 = 512;

		ItemPushLara(item, l, coll, 0, 2);

		GlobalCollisionBounds.X1 = 256;
		GlobalCollisionBounds.X2 = 1024;
		GlobalCollisionBounds.Z1 = -128;
		GlobalCollisionBounds.Z2 = 128;

		ItemPushLara(item, l, coll, 0, 2);
	}
}

void __cdecl TurnSwitchControl(__int16 itemNum) 
{
	ITEM_INFO* l = LaraItem;
	ITEM_INFO* item = &Items[itemNum];

	if (Items[itemNum].itemFlags[0] == 2)
	{
		if (item->animNumber == Objects[ID_TURN_SWITCH].animIndex + 2)
		{
			item->pos.yRot += ANGLE(90);
			if (TrInput & IN_ACTION)
			{
				l->animNumber = ANIMATION_LARA_ROUND_HANDLE_PUSH_RIGHT_BEGIN;
				l->frameNumber = Anims[l->animNumber].frameBase;
				item->animNumber = Objects[item->objectNumber].animIndex + 1;
				item->frameNumber = Anims[item->animNumber].frameBase;
			}
		}

		if (l->animNumber == ANIMATION_LARA_ROUND_HANDLE_PUSH_RIGHT_END && l->frameNumber == Anims[l->animNumber].frameEnd && !item->itemFlags[1])
			item->itemFlags[1] = 1;

		if (l->frameNumber >= Anims[ANIMATION_LARA_ROUND_HANDLE_PUSH_RIGHT_BEGIN].frameBase && 
			l->frameNumber <= Anims[ANIMATION_LARA_ROUND_HANDLE_PUSH_RIGHT_BEGIN].frameBase + 43
			|| 
			l->frameNumber >= Anims[ANIMATION_LARA_ROUND_HANDLE_PUSH_RIGHT_BEGIN].frameBase + 58 &&
			l->frameNumber <= Anims[ANIMATION_LARA_ROUND_HANDLE_PUSH_RIGHT_BEGIN].frameBase + 115)
		{
			SoundEffect(SFX_PUSHABLE_SOUND, &item->pos, 2);
		}
	}
	else
	{
		if (item->animNumber == Objects[ID_TURN_SWITCH].animIndex + 6)
		{
			item->pos.yRot -= ANGLE(90);
			if (TrInput & IN_ACTION)
			{
				l->animNumber = ANIMATION_LARA_ROUND_HANDLE_PUSH_LEFT_BEGIN;
				l->frameNumber = Anims[l->animNumber].frameBase;
				item->animNumber = Objects[item->objectNumber].animIndex + 5;
				item->frameNumber = Anims[item->animNumber].frameBase;
			}
		}

		if (l->animNumber == ANIMATION_LARA_ROUND_HANDLE_PUSH_LEFT_END && l->frameNumber == Anims[ANIMATION_LARA_ROUND_HANDLE_PUSH_LEFT_END].frameEnd && 
			!item->itemFlags[1])
			item->itemFlags[1] = 1;

		if (l->frameNumber >= Anims[ANIMATION_LARA_ROUND_HANDLE_PUSH_LEFT_BEGIN].frameBase &&
			l->frameNumber <= Anims[ANIMATION_LARA_ROUND_HANDLE_PUSH_LEFT_BEGIN].frameBase + 43
			||
			l->frameNumber >= Anims[ANIMATION_LARA_ROUND_HANDLE_PUSH_LEFT_BEGIN].frameBase + 58 &&
			l->frameNumber <= Anims[ANIMATION_LARA_ROUND_HANDLE_PUSH_LEFT_BEGIN].frameBase + 115)
		{
			SoundEffect(SFX_PUSHABLE_SOUND, &item->pos, 2);
		}
	}

	AnimateItem(item);

	if (item->itemFlags[1] == 1)
	{
		l->animNumber = ANIMATION_LARA_STAY_IDLE;
		l->currentAnimState = STATE_LARA_STOP;
		l->frameNumber = Anims[l->animNumber].frameBase;
		item->animNumber = Objects[item->objectNumber].animIndex;
		item->frameNumber = Anims[item->animNumber].frameBase;
		item->status = ITEM_INACTIVE;
		
		RemoveActiveItem(itemNum);

		Lara.gunStatus = LG_NO_ARMS;
		UseForcedFixedCamera = 0;
		item->itemFlags[1] = 2;
	}
}

void __cdecl PulleyCollision(__int16 itemNum, ITEM_INFO* l, COLL_INFO* coll) 
{
	ITEM_INFO* item = &Items[itemNum];
	
	if (item->flags & 0x100
		|| (!(TrInput & IN_ACTION) || Lara.gunStatus || l->currentAnimState != STATE_LARA_STOP || l->animNumber != ANIMATION_LARA_STAY_IDLE || 
			item->status || item->gravityStatus)
		&& (!Lara.isMoving || Lara.generalPtr != (void*)itemNum))
	{
		if (l->currentAnimState != STATE_LARA_PULLEY)
			ObjectCollision(itemNum, l, coll);
	}
	else
	{
		__int16 oldYrot = item->pos.yRot;
		item->pos.yRot = l->pos.yRot;
		if (TestLaraPosition(PulleyBounds, item, l))
		{
			if (MoveLaraPosition(&PulleyPos, item, l))
			{
				l->animNumber = ANIMATION_LARA_PULLEY_GRAB;
				l->currentAnimState = STATE_LARA_PULLEY;
				l->frameNumber = Anims[l->animNumber].frameBase;
				
				AddActiveItem(itemNum);
				
				item->pos.yRot = oldYrot;
				item->status = ITEM_ACTIVE;
				
				Lara.isMoving = false;
				Lara.headYrot = 0;
				Lara.headXrot = 0;
				Lara.torsoYrot = 0;
				Lara.torsoXrot = 0;
				Lara.gunStatus = LG_HANDS_BUSY;
				Lara.generalPtr = item;
			}
			else
			{
				Lara.generalPtr = (void*)itemNum;
			}
			item->pos.yRot = oldYrot;
		}
		else
		{
			if (Lara.isMoving && Lara.generalPtr == (void*)itemNum)
			{
				Lara.isMoving = false;
				Lara.gunStatus = LG_NO_ARMS;
			}
			item->pos.yRot = oldYrot;
		}
	}
}

void __cdecl UnderwaterSwitchCollision(__int16 itemNum, ITEM_INFO* l, COLL_INFO* coll) 
{
	ITEM_INFO* item = &Items[itemNum];
	__int32 flag = 0;

	if (TrInput & IN_ACTION
		&& Lara.waterStatus == LW_UNDERWATER
		&& l->currentAnimState == STATE_LARA_UNDERWATER_STOP
		&& l->animNumber == ANIMATION_LARA_UNDERWATER_IDLE
		&& !Lara.gunStatus
		&& !item->currentAnimState
		|| Lara.isMoving && Lara.generalPtr == (void*)itemNum)
	{
		flag = 0;
		if (TestLaraPosition(UnderwaterSwitchBounds, item, l))
		{
			if (!MoveLaraPosition(&UnderwaterSwitchPos, item, l))
			{
				Lara.generalPtr = (void*)itemNum;
				return ;
			}
		LABEL_17:
			l->currentAnimState = STATE_LARA_SWITCH_DOWN;
			l->animNumber = ANIMATION_LARA_UNDERWATER_PULLEY;
			l->frameNumber = Anims[l->animNumber].frameBase;
			l->fallspeed = 0;
			Lara.isMoving = false;
			Lara.gunStatus = LG_HANDS_BUSY;
			item->goalAnimState = 1;
			item->status = ITEM_ACTIVE;

			AddActiveItem(itemNum);
			
			ForcedFixedCamera.x = item->pos.xPos - 1024 * SIN(item->pos.yRot + ANGLE(90)) >> W2V_SHIFT;
			ForcedFixedCamera.y = item->pos.yPos - 1024;
			ForcedFixedCamera.z = item->pos.zPos - 1024 * COS(item->pos.yRot + ANGLE(90)) >> W2V_SHIFT;
			ForcedFixedCamera.roomNumber = item->roomNumber;

			return;
		}

		l->pos.yRot ^= (__int16)ANGLE(180);
		if (TestLaraPosition(UnderwaterSwitchBounds2, item, l))
		{
			if (MoveLaraPosition(&UnderwaterSwitchPos2, item, l))
				flag = 1;
			else
				Lara.generalPtr = (void*)itemNum;
		}
		l->pos.yRot ^= (__int16)ANGLE(180);

		if (flag)
		{
			l->currentAnimState = STATE_LARA_SWITCH_DOWN;
			l->animNumber = ANIMATION_LARA_UNDERWATER_PULLEY;
			l->frameNumber = Anims[l->animNumber].frameBase;
			l->fallspeed = 0;
			Lara.isMoving = false;
			Lara.gunStatus = LG_HANDS_BUSY;
			item->goalAnimState = 1;
			item->status = ITEM_ACTIVE;

			AddActiveItem(itemNum);

			ForcedFixedCamera.x = item->pos.xPos - 1024 * SIN(item->pos.yRot + ANGLE(90)) >> W2V_SHIFT;
			ForcedFixedCamera.y = item->pos.yPos - 1024;
			ForcedFixedCamera.z = item->pos.zPos - 1024 * COS(item->pos.yRot + ANGLE(90)) >> W2V_SHIFT;
			ForcedFixedCamera.roomNumber = item->roomNumber;
		}
	}
}

void __cdecl SwitchCollision2(__int16 itemNum, ITEM_INFO* l, COLL_INFO* coll) 
{
	ITEM_INFO* item = &Items[itemNum];

	if (TrInput & IN_ACTION)
	{
		if (item->status == ITEM_INACTIVE && Lara.waterStatus == LW_UNDERWATER && !Lara.gunStatus && l->currentAnimState == STATE_LARA_UNDERWATER_STOP)
		{
			if (TestLaraPosition(Switch2Bounds, item, l))
			{
				if (item->currentAnimState == 1 || !item->currentAnimState)
				{
					if (MoveLaraPosition(&Switch2Position, item, l))
					{
						l->fallspeed = 0;
						l->goalAnimState = STATE_LARA_SWITCH_DOWN;

						do
							AnimateLara(l);
						while (l->goalAnimState != STATE_LARA_SWITCH_DOWN);

						l->goalAnimState = STATE_LARA_UNDERWATER_STOP;
						Lara.gunStatus = LG_HANDS_BUSY;
						item->goalAnimState = item->currentAnimState != 1;
						item->status = ITEM_ACTIVE;						
						AddActiveItem(itemNum);
						AnimateItem(item);
					}
				}
			}
		}
	}
}

void __cdecl SwitchCollision(__int16 itemNum, ITEM_INFO* l, COLL_INFO* coll) 
{
	ITEM_INFO* item = &Items[itemNum];
	if (TrInput & IN_ACTION
		&& l->currentAnimState == STATE_LARA_STOP
		&& l->animNumber == ANIMATION_LARA_STAY_IDLE
		&& !Lara.gunStatus
		&& item->status == ITEM_INACTIVE
		&& !(item->flags & 0x100)
		&& item->triggerFlags >= 0
		|| Lara.isMoving && Lara.generalPtr == (void*)itemNum)
	{
		__int16* bounds = GetBoundsAccurate(item);
		
		if (item->triggerFlags == 3 && item->currentAnimState == 1 || item->triggerFlags >= 5 && item->triggerFlags <= 7 && !item->currentAnimState)
			return;

		SwitchBounds[0] = *bounds - 256;
		SwitchBounds[1] = bounds[1] + 256;
		if (item->triggerFlags)
		{
			SwitchBounds[4] = bounds[4] - 512;
			SwitchBounds[5] = bounds[5] + 512;

			if (item->triggerFlags == 3)
			{
				SwitchPos.z = bounds[4] - 256;
			}
			else if (item->triggerFlags == 4)
			{
				SwitchPos.z = bounds[4] - 88;
			}
			else if (item->triggerFlags < 5 || item->triggerFlags > 7)
			{
				if (item->triggerFlags < 8)
					SwitchPos.z = bounds[4] - 128;
				else
					SwitchPos.z = bounds[4] - 96;
			} 
			else
			{
				SwitchPos.z = bounds[4] - 160;
			}
		}
		else
		{
			SwitchBounds[4] = bounds[4] - 200;
			SwitchBounds[5] = bounds[5] + 200;
			SwitchPos.z = bounds[4] - 64;
		}
		
		if (TestLaraPosition(SwitchBounds, item, l))
		{
			if (MoveLaraPosition(&SwitchPos, item, l))
			{
				if (item->currentAnimState == 1)
				{
					if (item->triggerFlags)
					{
						if (item->triggerFlags >= 3)
						{
							if (item->triggerFlags == 4)
							{
								l->currentAnimState = STATE_LARA_SWITCH_UP;
								l->animNumber = ANIMATION_LARA_SWITCH_SMALL_DOWN;
								item->goalAnimState = 0;
							}
							else
							{
								if (item->triggerFlags >= 5 && item->triggerFlags <= 7)
								{
									if (item->triggerFlags == 6)
										DisableLaraControl = true;
									l->currentAnimState = STATE_LARA_SWITCH_DOWN;
									l->animNumber = ANIMATION_LARA_BUTTON_PUSH;
								}
								item->goalAnimState = 0;
							}
						}
						else
						{
							l->animNumber = ANIMATION_LARA_HOLE_GRAB;
							l->currentAnimState = STATE_LARA_HOLE;
							item->goalAnimState = 0;
						}
					}
					else
					{
						l->currentAnimState = STATE_LARA_SWITCH_UP;
						l->animNumber = ANIMATION_LARA_PULL_SWITCH_DOWN;
						item->goalAnimState = 0;
					}
				}
				else
				{
					if (item->triggerFlags)
					{
						if (item->triggerFlags == 3)
						{
							l->currentAnimState = STATE_LARA_SWITCH_DOWN;
							l->animNumber = ANIMATION_LARA_BIG_BUTTON_PUSH;
						}
						else if (item->triggerFlags == 4)
						{
							l->currentAnimState = STATE_LARA_SWITCH_DOWN;
							l->animNumber = ANIMATION_LARA_SWITCH_SMALL_UP;
						}
						else if (item->triggerFlags < 8)
						{
							l->currentAnimState = STATE_LARA_HOLE;
							l->animNumber = ANIMATION_LARA_HOLE_GRAB;
						}
						else
						{
							l->currentAnimState = STATE_LARA_SWITCH_DOWN;
							l->animNumber = ANIMATION_LARA_VALVE_TURN;
						}
					}
					else
					{
						l->currentAnimState = STATE_LARA_SWITCH_DOWN;
						l->animNumber = ANIMATION_LARA_PULL_SWITCH_UP;
					}

					item->goalAnimState = 1;
				}
				l->frameNumber = Anims[l->animNumber].frameBase;
				Lara.isMoving = false;
				Lara.gunStatus = LG_HANDS_BUSY;
				Lara.headYrot = 0;
				Lara.headXrot = 0;
				Lara.torsoYrot = 0;
				Lara.torsoXrot = 0;

				AddActiveItem(itemNum);
				item->status = ITEM_ACTIVE;
				AnimateItem(item);
			}
			else
			{
				Lara.generalPtr = (void*)itemNum;
			}
		}
		else if (Lara.isMoving && Lara.generalPtr == (void*)itemNum)
		{
			Lara.isMoving = false;
			Lara.gunStatus = LG_NO_ARMS;
		}

		return;
	}

	if (l->currentAnimState != STATE_LARA_SWITCH_DOWN && l->currentAnimState != STATE_LARA_SWITCH_UP)
		ObjectCollision(itemNum, l, coll);
}

void __cdecl SwitchControl(__int16 itemNumber) 
{
	ITEM_INFO* item = &Items[itemNumber];
	if (item->objectNumber != ID_AIRLOCK_SWITCH || item->triggerFlags < 8)
	{
		item->flags |= 0x3E00;
		if (!TriggerActive(item) && !(item->flags & 0x100))
		{
			if (item->objectNumber == ID_JUMP_SWITCH)
			{
				item->goalAnimState = 0;
				item->timer = 0;
				AnimateItem(item);
				return;
			}
			item->goalAnimState = 1;
			item->timer = 0;
		}
	}
	/*else
	{
		if (item->animNumber == Objects[ID_AIRLOCK_SWITCH].animIndex + 1 && item->frameNumber == Anims[item->animNumber].frameEnd - 1)
		{
			if (GetSwitchTrigger(&item, &ItemNos, 1) > 0)
			{
				v4 = (__int16*)((char*)& v15 + 2 * v3 + 2);
				v11 = v3;
				do
				{
					v5 = *v4;
					v6 = items;
					v6[v5].flags ^= 0x3E00u;
					v7 = & v6[v5];
					if ((v6[v5].flags & 0x3E00) == 15872 && (*(_BYTE*)(v7 + 5610) & 6) != 2)
					{
						AddActiveItem(*v4);
						v8 = *(_DWORD*)(v7 + 5610);
						LOBYTE(v8) = v8 & 0xFB | 2;
						*(_DWORD*)(v7 + 5610) = v8;
					}
					--v4;
					--v11;
				} while (v11);
			}
		}
		if (item->triggerFlags == 8)
		{
			v15 = 0;
			v14 = 0;
			v13 = 0;
			GetJointAbsPosition(item, & v13, 0);
			LOWORD(v9) = item->roomNumber;
			v12 = v9;
			GetFloor(v13, v14, v15, (__int16*)& v12);
			if ((_WORD)v12 != item->roomNumber)
			{
				ItemNewRoom(itemNum, v12);
				return AnimateItem(item);
			}
		}
	}*/
	
	AnimateItem(item);
}

void __cdecl TestTriggersAtXYZ(__int32 x, __int32 y, __int32 z, __int16 roomNumber, __int32 heavy, __int32 flags) 
{
	GetFloorHeight(GetFloor(x, y, z, &roomNumber), x, y, z);
	TestTriggers(TriggerIndex, heavy, flags);
}

__int32 __cdecl GetKeyTrigger(ITEM_INFO* item) 
{
	FLOOR_INFO* floor = GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &item->roomNumber);
	GetFloorHeight(floor, item->pos.xPos, item->pos.yPos, item->pos.zPos);

	if (TriggerIndex)
	{
		__int16* trigger = TriggerIndex;
		for (__int32 i = *TriggerIndex; (i & 0x1F) != 4; trigger += 2)
		{
			if (i < 0)
				break;
			i = *(trigger + 2);
		}
		if (*trigger & 4)
		{
			for (__int16* j = (trigger + 4); (*j >> 8) & 0x3C || item != &Items[*j & 0x3FF]; j++)
			{
				if (*j & 0x8000)
					return 0;
			}
			return 1;
		}
	}

	return 0;
}

__int32 __cdecl GetSwitchTrigger(ITEM_INFO* item, __int16* itemNos, __int32 AttatchedToSwitch)
{
	FLOOR_INFO* floor = GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &item->roomNumber);
	GetFloorHeight(floor, item->pos.xPos, item->pos.yPos, item->pos.zPos);

	if (TriggerIndex)
	{
		__int16* trigger = TriggerIndex;
		for (__int32 i = *TriggerIndex; (i & 0x1F) != 4; trigger += 2)
		{
			if (i < 0)
				break;
			i = *(trigger + 2);
		}

		if (*trigger & 4)
		{
			__int16* current = itemNos;
			for (__int16* j = (trigger + 4); (*j >> 8) & 0x3C || item != &Items[*j & 0x3FF]; j++, current++)
			{
				*current = *j & 0x3FF;
				if (*j & 0x8000)
					return 0;
			}
			return 1;
		}
	}

	return 0;
}

__int32 __cdecl SwitchTrigger(__int16 itemNum, __int16 timer) 
{
	ITEM_INFO* item = &Items[itemNum];
	if (item->status == ITEM_DEACTIVATED)
	{
		if ((!item->currentAnimState && item->objectNumber != ID_JUMP_SWITCH || item->currentAnimState == 1 && item->objectNumber == ID_JUMP_SWITCH) && timer > 0)
		{
			item->timer = timer;
			item->status = ITEM_ACTIVE;
			if (timer != 1)
				item->timer = 30 * timer;
			return 1;
		}
		if (item->triggerFlags != 6 || item->currentAnimState)
		{
			RemoveActiveItem(itemNum);
			
			item->status = ITEM_INACTIVE;
			if (!item->itemFlags[0] == 0)
				item->flags |= 0x100;
			if (item->currentAnimState != 1)
				return 1;
			if (item->triggerFlags != 5 && item->triggerFlags != 6)
				return 1;
		}
		else
		{
			item->status = ITEM_ACTIVE;
			return 1;
		}
	}
	else if (item->status)
	{
		return (item->flags & 0x100u) >> 8;
	}
	else
	{
		return 0;
	}

	return 0;
}

void Inject_Switch()
{
	INJECT(0x0047FD20, CrowDoveSwitchCollision);
	INJECT(0x0047FC80, CrowDoveSwitchControl);
	INJECT(0x0047F990, CogSwitchCollision);
	INJECT(0x0047F810, CogSwitchControl);
	INJECT(0x0047F610, FullBlockSwitchCollision);
	INJECT(0x0047F520, FullBlockSwitchControl);
	INJECT(0x0047F190, CrowbarSwitchCollision);
	INJECT(0x0047F050, JumpSwitchCollision);
	INJECT(0x0047EE00, RailSwitchCollision);
	INJECT(0x0047E950, TurnSwitchCollision);
	INJECT(0x0047E650, TurnSwitchControl);
	INJECT(0x0047E450, PulleyCollision);
	INJECT(0x0047E220, UnderwaterSwitchCollision);
	INJECT(0x0047E0C0, SwitchCollision2);
	INJECT(0x0047DC70, SwitchCollision);
	INJECT(0x0047D9D0, TestTriggersAtXYZ);
	INJECT(0x0047D8C0, GetKeyTrigger);
	INJECT(0x0047D7B0, GetSwitchTrigger);
	INJECT(0x0047D670, SwitchTrigger);

}