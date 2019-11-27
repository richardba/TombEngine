#include "newobjects.h"
#include "../Game/Box.h"
#include "../Game/items.h"
#include "../Game/lot.h"
#include "../Game/control.h"
#include "../Game/effects.h"
#include "../Game/draw.h"
#include "../Game/sphere.h"
#include "../Game/collide.h"
#include "../Game/pickup.h"

short SarcophagusBounds[12] = {
	0xFE00, 0x0200, 0xFF9C, 0x0064, 0xFE00, 0x0000,
	0xF8E4, 0x071C, 0xEAAC, 0x1554, 0x0000, 0x0000
};
PHD_VECTOR SarcophagusPosition = { 0x00000000, 0x00000000, 0xFFFFFED4 };

void __cdecl InitialiseSarcophagus(short itemNum)
{

}

void __cdecl SarcophagusCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll)
{
	ITEM_INFO* item = &Items[itemNum];

	if (TrInput & IN_ACTION &&
		item->status != ITEM_ACTIVE &&
		l->currentAnimState == 2 &&
		l->animNumber == 103 &&
		!Lara.gunStatus ||
		Lara.isMoving && (short)Lara.generalPtr == itemNum)
	{
		if (TestLaraPosition(SarcophagusBounds, item, l))
		{
			if (MoveLaraPosition(&SarcophagusPosition, item, l))
			{
				l->animNumber = 439;
				l->currentAnimState = 89;
				l->frameNumber = Anims[l->animNumber].frameBase;
				item->flags |= 0x3E00;

				AddActiveItem(itemNum);
				item->status = ITEM_ACTIVE;

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
		else if (Lara.isMoving)
		{
			if ((short)Lara.generalPtr == itemNum)
			{
				Lara.isMoving = false;
				Lara.gunStatus = LG_NO_ARMS;
			}
		}
	}
	else if (l->animNumber != 439 || l->frameNumber != Anims[439].frameBase + 113)
	{
		ObjectCollision(itemNum, l, coll);
	}
	else
	{
		short linknum;
		for (linknum = Items[Rooms[item->roomNumber].itemNumber].nextItem; linknum != NO_ITEM; linknum = Items[linknum].nextItem)
		{
			ITEM_INFO* currentItem = &Items[linknum];

			if (linknum != itemNum && currentItem->pos.xPos == item->pos.xPos && currentItem->pos.zPos == item->pos.zPos)
			{
				if (Objects[currentItem->objectNumber].collision == PickupCollision)
				{
					PickedUpObject(currentItem->objectNumber);
					currentItem->status = ITEM_ACTIVE;
					currentItem->itemFlags[3] = 1;
				}
			}
		}
	}
}

void __cdecl InitialiseLaraDouble(short itemNum)
{
	ClearItem(itemNum);
}

void __cdecl LaraDoubleControl(short itemNum)
{
	ITEM_INFO* item = &Items[itemNum];

	SoundEffect(SFX_TR4_METAL_SCRAPE_LOOP_1_ID12, &item->pos, 0);

	if (CreatureActive(itemNum))
	{
		if (item->hitStatus)
		{
			LaraItem->hitPoints += item->hitPoints - 1000;
		}

		item->hitPoints = 1000;

		AnimateItem(item);
	}
}