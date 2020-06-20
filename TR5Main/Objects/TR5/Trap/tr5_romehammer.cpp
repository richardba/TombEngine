#include "framework.h"
#include "tr5_romehammer.h"
#include "items.h"
#include "level.h"

void InitialiseRomeHammer(short itemNumber)
{
	ITEM_INFO* item = &Items[itemNumber];
	item->itemFlags[0] = 2;
	item->itemFlags[3] = 250;
}