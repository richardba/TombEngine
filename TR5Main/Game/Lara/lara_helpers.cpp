#include "framework.h"
#include "collide.h"
#include "control/control.h"
#include "input.h"
#include "items.h"
#include "level.h"
#include "lara.h"
#include "lara_tests.h"
#include "lara_collide.h"
#include "setup.h"

// -----------------------------
// HELPER FUNCTIONS
// For State Control & Collision
// -----------------------------

// TODO: Some states can't make the most of this function due to missing step up/down animations.
// Try implementing leg IK as a substitute to make step animations obsolete. @Sezz 2021.10.09
void DoLaraStep(ITEM_INFO* item, COLL_INFO* coll)
{
	if (TestLaraStepUp(item, coll) &&
		!TestLaraSwamp(item))
	{
		item->goalAnimState = LS_STEP_UP;
		if (GetChange(item, &g_Level.Anims[item->animNumber]))
		{
			item->pos.yPos += coll->Middle.Floor;

			return;
		}
	}
	else if (TestLaraStepDown(item, coll) &&
		!TestLaraSwamp(item))
	{
		item->goalAnimState = LS_STEP_DOWN;
		if (GetChange(item, &g_Level.Anims[item->animNumber]))
		{
			item->pos.yPos += coll->Middle.Floor;

			return;
		}
	}

	// Height difference is below threshold for step dispatch OR step animation doesn't exist; translate Lara to new floor height.
	// TODO: This approach might cause underirable artefacts where an object pushes Lara rapidly up/down a slope or a platform rapidly ascends/descends.
	int threshold = STEP_SIZE / 8;
	int rate = 50;
	int sign = std::copysign(1, coll->Middle.Floor);
	if (coll->Middle.Floor != NO_HEIGHT)
	{
		if (TestLaraSwamp(item) &&
			coll->Middle.Floor > 0)
		{
			item->pos.yPos += SWAMP_GRAVITY;
		}
		else if (abs(coll->Middle.Floor) > (STEPUP_HEIGHT / 2))			// Outer range.
			item->pos.yPos += rate * sign;
		else if (abs(coll->Middle.Floor) <= (STEPUP_HEIGHT / 2) &&		// Inner range.
			abs(coll->Middle.Floor) >= threshold)
		{
			item->pos.yPos += std::max((int)abs(coll->Middle.Floor / 2.75), threshold) * sign;
		}
		else
			item->pos.yPos += coll->Middle.Floor;
	}
}

void DoLaraCrawlVault(ITEM_INFO* item, COLL_INFO* coll)
{
	if (TestLaraCrawlExitDownStep(item, coll))
	{
		if (TrInput & IN_DUCK && TestLaraCrawlDownStep(item, coll))
			item->goalAnimState = LS_STEP_DOWN;
		else [[likely]]
			item->goalAnimState = LS_CRAWL_EXIT_DOWN_STEP;

		return;
	}

	if (TestLaraCrawlExitJump(item, coll))
	{
		if (TrInput & IN_WALK)
			item->goalAnimState = LS_CRAWL_EXIT_FLIP;
		else [[likely]]
			item->goalAnimState = LS_CRAWL_EXIT_JUMP;

		return;
	}

	if (TestLaraCrawlUpStep(item, coll))
	{
		item->goalAnimState = LS_STEP_UP;

		return;
	}

	if (TestLaraCrawlDownStep(item, coll))
	{
		item->goalAnimState = LS_STEP_DOWN;

		return;
	}
}

void DoLaraCrawlToHangSnap(ITEM_INFO* item, COLL_INFO* coll)
{
	coll->Setup.ForwardAngle += ANGLE(180.0f);
	GetCollisionInfo(coll, item);
	SnapItemToLedge(item, coll);
	MoveItem(item, coll->Setup.ForwardAngle, -LARA_RAD_CRAWL);
	item->pos.yRot += ANGLE(180.0f);
	LaraResetGravityStatus(item, coll);
}

// TODO: Make lean rate proportional to the turn rate, allowing for nicer aesthetics with future analog stick input.
void DoLaraLean(ITEM_INFO* item, COLL_INFO* coll, int maxAngle, short rate)
{
	int sign = copysign(1, maxAngle);

	if (coll->CollisionType == CT_LEFT || coll->CollisionType == CT_RIGHT)
		item->pos.zRot += std::min(rate, (short)(abs((maxAngle * 3) / 5 - item->pos.zRot) / 3)) * sign;
	else
		item->pos.zRot += std::min(rate, (short)(abs(maxAngle - item->pos.zRot) / 3)) * sign;
}

void DoLaraCrawlFlex(ITEM_INFO* item, COLL_INFO* coll, short maxAngle, short rate)
{
	LaraInfo*& info = item->data;

	int sign = copysign(1, maxAngle);
	rate = copysign(rate, maxAngle);

	info->torsoZrot += std::min(abs(rate), abs(maxAngle - info->torsoZrot) / 6) * sign;

	if (!(TrInput & IN_LOOK) &&
		item->currentAnimState != LS_CRAWL_BACK)
	{
		info->headZrot = info->torsoZrot / 2;
		info->headYrot = info->headZrot;
	}
}

void ResetLaraFlex(ITEM_INFO* item, short rate)
{
	LaraInfo*& info = item->data;

	// Reset head.
	if (abs(info->headXrot) > ANGLE(0.1f))
		info->headXrot += info->headXrot / -rate;
	else
		info->headXrot = 0;

	if (abs(info->headYrot) > ANGLE(0.1f))
		info->headYrot += info->headYrot / -rate;
	else
		info->headYrot = 0;

	if (abs(info->headZrot) > ANGLE(0.1f))
		info->headZrot += info->headZrot / -rate;
	else
		info->headZrot = 0;

	// Reset torso.
	if (abs(info->torsoXrot) > ANGLE(0.1f))
		info->torsoXrot += info->torsoXrot / -rate;
	else
		info->torsoXrot = 0;

	if (abs(info->torsoYrot) > ANGLE(0.1f))
		info->torsoYrot += info->torsoYrot / -rate;
	else
		info->torsoYrot = 0;

	if (abs(info->torsoZrot) > ANGLE(0.1f))
		info->torsoZrot += info->torsoZrot / -rate;
	else
		info->torsoZrot = 0;
}

void SetLaraFallState(ITEM_INFO* item)
{
	SetAnimation(item, LA_FALL_START);
	item->fallspeed = 0;
	item->gravityStatus = true;
}

void SetLaraFallBackState(ITEM_INFO* item)
{
	SetAnimation(item, LA_FALL_BACK);
	item->fallspeed = 0;
	item->gravityStatus = true;
}
