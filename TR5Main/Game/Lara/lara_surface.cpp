#include "framework.h"
#include "Game/Lara/lara_surface.h"

#include "Game/camera.h"
#include "Game/collision/collide_room.h"
#include "Game/control/control.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_collide.h"
#include "Game/Lara/lara_helpers.h"
#include "Game/Lara/lara_swim.h"
#include "Game/Lara/lara_tests.h"
#include "Specific/level.h"
#include "Specific/input.h"

// -----------------------------
// WATER SURFACE TREAD
// Control & Collision Functions
// -----------------------------

// State:		LS_ONWATER_DIVE (35)
// Collision:	lara_col_surface_dive()
void lara_as_surface_dive(ITEM_INFO* item, CollisionInfo* coll)
{
	if (TrInput & IN_FORWARD)
		item->Position.xRot -= ANGLE(1.0f);
}

// State:		LS_ONWATER_DIVE (35)
// Control:		lara_as_surface_dive()
void lara_col_surface_dive(ITEM_INFO* item, CollisionInfo* coll)
{
	LaraSwimCollision(item, coll);
}

// State:		LS_ONWATER_IDLE (33)
// Collision:	lara_col_surface_idle()
void lara_as_surface_idle(ITEM_INFO* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	item->Animation.VerticalVelocity -= 4;
	if (item->Animation.VerticalVelocity < 0)
		item->Animation.VerticalVelocity = 0;

	if (item->HitPoints <= 0)
	{
		item->Animation.TargetState = LS_WATER_DEATH;
		return;
	}

	if (TrInput & IN_LOOK)
	{
		LookUpDown(item);
		return;
	}

	if (TrInput & IN_LEFT)
	{
		lara->Control.TurnRate -= LARA_TURN_RATE * 1.25f;
		if (lara->Control.TurnRate < -LARA_MED_TURN_MAX)
			lara->Control.TurnRate = -LARA_MED_TURN_MAX;
	}
	else if (TrInput & IN_RIGHT)
	{
		lara->Control.TurnRate += LARA_TURN_RATE * 1.25f;
		if (lara->Control.TurnRate > LARA_MED_TURN_MAX)
			lara->Control.TurnRate = LARA_MED_TURN_MAX;
	}

	if (DbInput & IN_JUMP)
	{
		SetLaraSwimDiveAnimation(item);
		return;
	}
	
	if (TrInput & IN_ROLL || (TrInput & IN_FORWARD && TrInput & IN_BACK))
	{
		item->Animation.TargetState = LS_ROLL_FORWARD;
		return;
	}

	if (TrInput & IN_FORWARD)
	{
		item->Animation.TargetState = LS_ONWATER_FORWARD;
		return;
	}
	else if (TrInput & IN_BACK)
	{
		item->Animation.TargetState = LS_ONWATER_BACK;
		return;
	}
	
	if (TrInput & IN_LSTEP)
	{
		item->Animation.TargetState = LS_ONWATER_LEFT;
		return;
	}
	else if (TrInput & IN_RSTEP)
	{
		item->Animation.TargetState = LS_ONWATER_RIGHT;
		return;
	}

	item->Animation.TargetState = LS_ONWATER_IDLE;
}

// State:		LS_ONWATER_IDLE (33)
// Control:		lara_as_surface_idle()
void lara_col_surface_idle(ITEM_INFO* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	lara->Control.MoveAngle = item->Position.yRot;
	LaraSurfaceCollision(item, coll);
}

// State:		LS_ONWATER_FORWARD (34)
// Collision:	lara_col_surface_swim_forward()
void lara_as_surface_swim_forward(ITEM_INFO* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	if (item->HitPoints <= 0)
	{
		item->Animation.TargetState = LS_WATER_DEATH;
		return;
	}
	
	if (TrInput & IN_LEFT)
	{
		lara->Control.TurnRate -= LARA_TURN_RATE * 1.25f;
		if (lara->Control.TurnRate < -LARA_MED_TURN_MAX)
			lara->Control.TurnRate = -LARA_MED_TURN_MAX;
	}
	else if (TrInput & IN_RIGHT)
	{
		lara->Control.TurnRate += LARA_TURN_RATE * 1.25f;
		if (lara->Control.TurnRate > LARA_MED_TURN_MAX)
			lara->Control.TurnRate = LARA_MED_TURN_MAX;
	}

	if (!(TrInput & IN_FORWARD))
		item->Animation.TargetState = LS_ONWATER_IDLE;

	if (DbInput & IN_JUMP)
		SetLaraSwimDiveAnimation(item);

	item->Animation.VerticalVelocity += 8;
	if (item->Animation.VerticalVelocity > 60)
		item->Animation.VerticalVelocity = 60;
}

// State:		LS_ONWATER_FORWARD (34)
// Control:		lara_as_surface_swim_forward()
void lara_col_surface_swim_forward(ITEM_INFO* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	coll->Setup.UpperFloorBound = -STEPUP_HEIGHT;
	lara->Control.MoveAngle = item->Position.yRot;
	LaraSurfaceCollision(item, coll);
	TestLaraWaterClimbOut(item, coll);
	TestLaraLadderClimbOut(item, coll);
}

// State:		LS_ONWATER_LEFT (48)
// Collision:	lara_col_surface_swim_left()
void lara_as_surface_swim_left(ITEM_INFO* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	if (item->HitPoints <= 0)
	{
		item->Animation.TargetState = LS_WATER_DEATH;
		return;
	}

	if (TrInput & IN_LEFT)
	{
		lara->Control.TurnRate -= LARA_TURN_RATE * 1.25f;
		if (lara->Control.TurnRate < -LARA_SLOW_MED_TURN_MAX)
			lara->Control.TurnRate = -LARA_SLOW_MED_TURN_MAX;
	}
	else if (TrInput & IN_RIGHT)
	{
		lara->Control.TurnRate += LARA_TURN_RATE * 1.25f;
		if (lara->Control.TurnRate > LARA_SLOW_MED_TURN_MAX)
			lara->Control.TurnRate = LARA_SLOW_MED_TURN_MAX;
	}

	if (!(TrInput & IN_LSTEP))
		item->Animation.TargetState = LS_ONWATER_IDLE;

	if (DbInput & IN_JUMP)
		SetLaraSwimDiveAnimation(item);

	item->Animation.VerticalVelocity += 8;
	if (item->Animation.VerticalVelocity > 60)
		item->Animation.VerticalVelocity = 60;
}

// State:		LS_ONWATER_LEFT (48)
// Control:		lara_as_surface_swim_left()
void lara_col_surface_swim_left(ITEM_INFO* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	lara->Control.MoveAngle = item->Position.yRot - ANGLE(90.0f);
	LaraSurfaceCollision(item, coll);
}

// State:		LS_ONWATER_RIGHT (49)
// Collision:	lara_col_surface_swim_right()
void lara_as_surface_swim_right(ITEM_INFO* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	if (item->HitPoints <= 0)
	{
		item->Animation.TargetState = LS_WATER_DEATH;
		return;
	}

	if (TrInput & IN_LEFT)
	{
		lara->Control.TurnRate -= LARA_TURN_RATE * 1.25f;
		if (lara->Control.TurnRate < -LARA_SLOW_MED_TURN_MAX)
			lara->Control.TurnRate = -LARA_SLOW_MED_TURN_MAX;
	}
	else if (TrInput & IN_RIGHT)
	{
		lara->Control.TurnRate += LARA_TURN_RATE * 1.25f;
		if (lara->Control.TurnRate > LARA_SLOW_MED_TURN_MAX)
			lara->Control.TurnRate = LARA_SLOW_MED_TURN_MAX;
	}

	if (!(TrInput & IN_RSTEP))
		item->Animation.TargetState = LS_ONWATER_IDLE;

	if (DbInput & IN_JUMP)
		SetLaraSwimDiveAnimation(item);

	item->Animation.VerticalVelocity += 8;
	if (item->Animation.VerticalVelocity > 60)
		item->Animation.VerticalVelocity = 60;
}

// State:		LS_ONWATER_RIGHT (49)
// Conrol:		lara_as_surface_swim_right()
void lara_col_surface_swim_right(ITEM_INFO* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	lara->Control.MoveAngle = item->Position.yRot + ANGLE(90.0f);
	LaraSurfaceCollision(item, coll);
}

// State:		LS_ONWATER_BACK (47)
// Collision:	lara_col_surface_swim_back()
void lara_as_surface_swim_back(ITEM_INFO* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	if (item->HitPoints <= 0)
	{
		item->Animation.TargetState = LS_WATER_DEATH;
		return;
	}

	if (TrInput & IN_LEFT)
	{
		lara->Control.TurnRate -= LARA_TURN_RATE * 1.25f;
		if (lara->Control.TurnRate < -LARA_SLOW_MED_TURN_MAX)
			lara->Control.TurnRate = -LARA_SLOW_MED_TURN_MAX;
	}
	else if (TrInput & IN_RIGHT)
	{
		lara->Control.TurnRate += LARA_TURN_RATE * 1.25f;
		if (lara->Control.TurnRate > LARA_SLOW_MED_TURN_MAX)
			lara->Control.TurnRate = LARA_SLOW_MED_TURN_MAX;
	}

	if (DbInput & IN_JUMP)
		SetLaraSwimDiveAnimation(item);

	if (!(TrInput & IN_BACK))
		item->Animation.TargetState = LS_ONWATER_IDLE;

	item->Animation.VerticalVelocity += 8;
	if (item->Animation.VerticalVelocity > 60)
		item->Animation.VerticalVelocity = 60;
}

// State:		LS_ONWATER_BACK (47)
// Control:		lara_as_surface_swim_back()
void lara_col_surface_swim_back(ITEM_INFO* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	lara->Control.MoveAngle = item->Position.yRot + ANGLE(180.0f);
	LaraSurfaceCollision(item, coll);
}

// State:		LS_ONWATER_EXIT (55)
// Collision:	lara_default_col()
void lara_as_surface_climb_out(ITEM_INFO* item, CollisionInfo* coll)
{
	coll->Setup.EnableObjectPush = false;
	coll->Setup.EnableSpasm = false;
	Camera.flags = CF_FOLLOW_CENTER;
	Camera.laraNode = LM_HIPS;	// Forces the camera to follow Lara instead of snapping.
}
