#include "framework.h"
#include "tr5_genslot.h"
#include "items.h"
#include "level.h"
#include "control.h"
#include "laramisc.h"
#include "effect2.h"
#include "sphere.h"
#include "draw.h"
#include "effect.h"
#include "lara.h"

void GenSlot1Control(short itemNumber)
{
	ITEM_INFO* item = &Items[itemNumber];

	if (TriggerActive(item) && !item->triggerFlags)
	{
		int df = item->frameNumber - Anims[item->animNumber].frameBase;

		if (df == 10 || df == 11)
		{
			GetLaraDeadlyBounds();

			DeadlyBounds[0] -= 350;
			DeadlyBounds[1] += 350;
			DeadlyBounds[4] -= 350;
			DeadlyBounds[5] += 350;

			bool found = false;
			for (int i = 0; i < 6; i++)
			{
				PHD_VECTOR pos;
				pos.x = 0;
				pos.y = -350;
				pos.z = 0;

				GetJointAbsPosition(item, &pos, i + 1);

				if (pos.x > DeadlyBounds[0]
					&& pos.x < DeadlyBounds[1]
					&& pos.y > DeadlyBounds[2]
					&& pos.y < DeadlyBounds[3]
					&& pos.z > DeadlyBounds[4]
					&& pos.z < DeadlyBounds[5])
				{
					found = true;
				}
			}

			if (found)
			{
				for (int i = 0; i < 8; i++)
				{
					PHD_VECTOR pos;
					pos.x = 0;
					pos.y = 0;
					pos.z = 0;

					GetLaraJointPosition(&pos, i + 7);

					int x = pos.x + (GetRandomControl() & 0xFF) - 128;
					int y = pos.y + (GetRandomControl() & 0xFF) - 128;
					int z = pos.z + (GetRandomControl() & 0xFF) - 128;

					DoBloodSplat(x, y, z, 1, -1, LaraItem->roomNumber);
				}

				LaraItem->hitPoints = 0;
			}
		}

		AnimateItem(item);
	}
}

void InitialiseGenSlot3(short itemNumber)
{
	ITEM_INFO* item = &Items[itemNumber];
	if (CurrentLevel != 7)
		item->meshBits = item->triggerFlags;
}

void InitialiseGenSlot4(short itemNumber)
{
	/*ITEM_INFO* item = &Items[itemNumber];

	HIWORD(v1) = HIWORD(items);
	item = &items[itemNumber];
	LOWORD(v1) = item->pos.yRot;
	v3 = item->pos.xPos;
	v4 = 2 * ((v1 >> 3) & 0x1FFE);
	v5 = 5 * *(__int16*)((char*)rcossin_tbl + v4);
	v6 = item->pos.zPos;
	v7 = v6 + (10240 * *(__int16*)((char*)& rcossin_tbl[1] + v4) >> 14);
	item->item_flags[2] = 1;
	BYTE1(v4) = v6 >> 9;
	LOBYTE(v4) = v3 >> 9;
	item->item_flags[0] = v4;
	LOBYTE(v6) = (item->pos.xPos + (v5 << 11 >> 14)) >> 9;
	BYTE1(v6) = v7 >> 9;

	item->itemFlags[1] = item->pos.xPos + 2560 * phd_sin(item->pos.yRot) >> W2V_SHIFT;
	item->itemFlags[3] = 0;
	item->triggerFlags = 0;*/
}