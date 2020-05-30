#include "framework.h"
#include "tr4_spikywall.h"
#include "level.h"
#include "control.h"
#include "sound.h"
#include "lara.h"
#include "items.h"
#include "effect.h"

void ControlSpikyWall(short itemNum)
{
	ITEM_INFO* item = &Items[itemNum];

	/* Move wall */
	if (TriggerActive(item) && item->status != ITEM_DESACTIVATED)
	{
		int x = item->pos.xPos + phd_sin(item->pos.yRot) >> WALL_SHIFT;
		int z = item->pos.zPos + phd_cos(item->pos.yRot) >> WALL_SHIFT;

		short roomNumber = item->roomNumber;
		FLOOR_INFO* floor = GetFloor(x, item->pos.yPos, z, &roomNumber);

		if (GetFloorHeight(floor, x, item->pos.yPos, z) != item->pos.yPos)
		{
			item->status = ITEM_DESACTIVATED;
			StopSoundEffect(SFX_ROLLING_BALL);
		}
		else
		{
			item->pos.xPos = x;
			item->pos.zPos = z;
			if (roomNumber != item->roomNumber)
				ItemNewRoom(itemNum, roomNumber);
			SoundEffect(SFX_ROLLING_BALL, &item->pos, 0);
		}
	}

	if (item->touchBits)
	{
		LaraItem->hitPoints -= 15;
		LaraItem->hitStatus = true;

		DoLotsOfBlood(LaraItem->pos.xPos, LaraItem->pos.yPos - 512, LaraItem->pos.zPos, 4, item->pos.yRot, LaraItem->roomNumber, 3);
		item->touchBits = 0;

		SoundEffect(56, &item->pos, 0);
	}
}