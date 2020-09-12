#include "framework.h"
#include "lara.h"
#include "lara_collide.h"
#include "input.h"
#include "sound.h"

short OldAngle = 1;

/*this file has all the related functions to sliding*/

/*tests and others*/
int TestLaraSlide(ITEM_INFO* item, COLL_INFO* coll) // (F) (D)
{
	if (abs(coll->tiltX) <= 2 && abs(coll->tiltZ) <= 2)
		return 0;

	short angle = ANGLE(0.0f);
	if (coll->tiltX > 2)
		angle = -ANGLE(90.0f);
	else if (coll->tiltX < -2)
		angle = ANGLE(90.0f);

	if (coll->tiltZ > 2 && coll->tiltZ > abs(coll->tiltX))
		angle = ANGLE(180.0f);
	else if (coll->tiltZ < -2 && -coll->tiltZ > abs(coll->tiltX))
		angle = ANGLE(0.0f);

	short delta = angle - item->pos.yRot;

	ShiftItem(item, coll);

	if (delta < -ANGLE(90.0f) || delta > ANGLE(90.0f))
	{
		if (item->currentAnimState == LS_SLIDE_BACK && OldAngle == angle)
			return 1;

		item->animNumber = LA_SLIDE_BACK_START;
		item->goalAnimState = LS_SLIDE_BACK;
		item->currentAnimState = LS_SLIDE_BACK;
		item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
		item->pos.yRot = angle + ANGLE(180.0f);
		Lara.moveAngle = ANGLE(180);
	}
	else
	{
		if (item->currentAnimState == LS_SLIDE_FORWARD && OldAngle == angle)
			return 1;

		item->animNumber = LA_SLIDE_FORWARD;
		item->goalAnimState = LS_SLIDE_FORWARD;
		item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
		item->currentAnimState = LS_SLIDE_FORWARD;
		item->pos.yRot = angle;
		Lara.moveAngle = 0;
	}

	OldAngle = angle;

	return 1;
}

void lara_slide_slope(ITEM_INFO* item, COLL_INFO* coll)//127BC, 1286C (F)
{
	coll->badPos = NO_BAD_POS;
	coll->badNeg = -512;
	coll->badCeiling = 0;

	coll->facing = Lara.moveAngle;
	GetCollisionInfo(coll, item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber, LARA_HITE);

	if (!LaraHitCeiling(item, coll))
	{
		LaraDeflectEdge(item, coll);

		if (coll->midFloor <= 200)
		{
			TestLaraSlide(item, coll);

			item->pos.yPos += coll->midFloor;

			if (abs(coll->tiltX) <= 2 && abs(coll->tiltZ) <= 2)
			{
				if (TrInput & IN_FORWARD && item->currentAnimState != LS_SLIDE_BACK)
				{
					item->goalAnimState = LS_RUN_FORWARD;
				}
				else
					item->goalAnimState = LS_STOP;
				StopSoundEffect(SFX_LARA_SLIPPING);
			}
		}
		else
		{
			if (item->currentAnimState == LS_SLIDE_FORWARD)
			{
				item->animNumber = LA_FALL_START;
				item->frameNumber = g_Level.Anims[LA_FALL_START].frameBase;

				item->currentAnimState = LS_JUMP_FORWARD;
				item->goalAnimState = LS_JUMP_FORWARD;
			}
			else
			{
				item->animNumber = LA_FALL_BACK;
				item->frameNumber = g_Level.Anims[LA_FALL_BACK].frameBase;

				item->currentAnimState = LS_FALL_BACK;
				item->goalAnimState = LS_FALL_BACK;
			}

			StopSoundEffect(SFX_LARA_SLIPPING);

			item->gravityStatus = true;
			item->fallspeed = 0;
		}
	}
}

void LaraSlideEdgeJump(ITEM_INFO* item, COLL_INFO* coll)//12B18, 12BC8 (F)
{
	ShiftItem(item, coll);

	switch (coll->collType)
	{
	case CT_LEFT:
		item->pos.yRot += ANGLE(5.0f);
		break;

	case CT_RIGHT:
		item->pos.yRot -= ANGLE(5.0f);
		break;

	case CT_TOP:
	case CT_TOP_FRONT:
		if (item->fallspeed <= 0)
			item->fallspeed = 1;
		break;

	case CT_CLAMP:
		item->pos.zPos -= (400 * phd_cos(coll->facing)) >> W2V_SHIFT;
		item->pos.xPos -= (400 * phd_sin(coll->facing)) >> W2V_SHIFT;

		item->speed = 0;

		coll->midFloor = 0;

		if (item->fallspeed <= 0)
			item->fallspeed = 16;

		break;
	}
}
/*end tests and others*/
/*-*/
/*Lara state code*/
void lara_as_slide(ITEM_INFO* item, COLL_INFO* coll)//1A824(<), 1A958(<) (F)
{
	/*state 24*/
	/*collision: lara_col_slide*/
	Camera.targetElevation = -ANGLE(45.0f); // FIXED
	if ((TrInput & IN_JUMP) && !(TrInput & IN_BACK))
		item->goalAnimState = LS_JUMP_FORWARD;
}

void lara_col_slide(ITEM_INFO* item, COLL_INFO* coll)//1C108(<), 1C23C(<) (F)
{
	/*state 24*/
	/*state code: lara_as_slide*/
	Lara.moveAngle = 0;
	lara_slide_slope(item, coll);
}

void lara_as_slideback(ITEM_INFO* item, COLL_INFO* coll)//1A9E0(<), 1AB14(<) (F)
{
	/*state 32*/
	/*collision: lara_col_slideback*/
	if ((TrInput & IN_JUMP) && !(TrInput & IN_FORWARD))
	{
		item->goalAnimState = LS_JUMP_BACK;
	}
}

void lara_col_slideback(ITEM_INFO* item, COLL_INFO* coll)//1C284(<), 1C3B8(<) (F)
{
	/*state 32*/
	/*state code: lara_as_slideback*/
	Lara.moveAngle = ANGLE(180);
	lara_slide_slope(item, coll);
}
/*end Lara state code*/