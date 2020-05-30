#include "framework.h"
#include "tr2_killerstatue.h"
#include "level.h"
#include "setup.h"
#include "control.h"
#include "lara.h"
#include "effect.h"

void InitialiseKillerStatue(short itemNumber)
{
	ITEM_INFO* item = &Items[itemNumber];

	item->animNumber = Objects[item->objectNumber].animIndex + 3;
	item->frameNumber = Anims[item->animNumber].frameBase;
	item->currentAnimState = 1;
}

void KillerStatueControl(short itemNumber)
{
	ITEM_INFO* item;
	int x, y, z;
	short d;

	item = &Items[itemNumber];

	if (TriggerActive(item) && item->currentAnimState == 1)
		item->goalAnimState = 2;
	else
		item->goalAnimState = 1;

	if ((item->touchBits & 0x80) && item->currentAnimState == 2)
	{
		LaraItem->hitStatus = 1;
		LaraItem->hitPoints -= 20;

		int x = LaraItem->pos.xPos + (GetRandomControl() - 16384) / 256;
		int z = LaraItem->pos.zPos + (GetRandomControl() - 16384) / 256;
		int y = LaraItem->pos.yPos - GetRandomControl() / 44;
		int d = (GetRandomControl() - 16384) / 8 + LaraItem->pos.yRot;
		DoBloodSplat(x, y, z, LaraItem->speed, d, LaraItem->roomNumber);
	}

	AnimateItem(item);
}