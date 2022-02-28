#include "framework.h"
#include "Objects/TR3/Vehicles/kayak.h"

#include "Game/animation.h"
#include "Game/camera.h"
#include "Game/collision/collide_item.h"
#include "Game/collision/collide_room.h"
#include "Game/control/control.h"
#include "Game/effects/effects.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_flare.h"
#include "Game/Lara/lara_helpers.h"
#include "Objects/TR3/Vehicles/kayak_info.h"
#include "Specific/level.h"
#include "Specific/input.h"
#include "Specific/setup.h"

using std::vector;

#define KAYAK_COLLIDE			CLICK(0.25f)
#define DISMOUNT_DISTANCE 		CLICK(3) // TODO: Find accurate distance.
#define KAYAK_TO_ENTITY_RADIUS	CLICK(1)

#define MAX_VELOCITY				0x380000
#define KAYAK_FRICTION				0x8000
#define KAYAK_ROTATE_FRICTION		0x50000
#define KAYAK_DEFLECT_ROTATION		0x80000
#define KAYAK_FORWARD_VELOCITY		0x180000
#define KAYAK_FORWARD_ROTATION		0x800000
#define KAYAK_LEFT_RIGHT_VELOCITY	0x100000
#define KAYAK_LEFT_RIGHT_ROTATION	0xc00000
#define KAYAK_MAX_LEFT_RIGHT		0xc00000
#define KAYAK_TURN_ROTATION			0x200000
#define KAYAK_MAX_TURN				0x1000000
#define KAYAK_TURN_BRAKE			0x8000
#define KAYAK_HARD_ROTATION			0x1000000
#define KAYAK_MAX_STAT				0x1000000

#define HIT_BACK	1
#define HIT_FRONT	2
#define HIT_LEFT	3
#define HIT_RIGHT	4

#define KAYAK_MOUNT_LEFT_FRAME	GetFrameNumber(KAYAK_ANIM_MOUNT_RIGHT, 0)
#define KAYAK_IDLE_FRAME		GetFrameNumber(KAYAK_ANIM_IDLE, 0)
#define KAYAK_MOUNT_RIGHT_FRAME	GetFrameNumber(KAYAK_ANIM_MOUNT_LEFT, 0)

#define KAYAK_DRAW_SHIFT	32
#define LARA_LEG_BITS		((1 << LM_HIPS) | (1 << LM_LTHIGH) | (1 << LM_LSHIN) | (1 << LM_LFOOT) | (1 << LM_RTHIGH) | (1 << LM_RSHIN) | (1 << LM_RFOOT))
#define NUM_WAKE_SPRITES	32
#define WAKE_SIZE 			32
#define WAKE_VELOCITY 		4
#define KAYAK_X				128
#define KAYAK_Z				128
#define KAYAK_MAX_KICK		-80
#define KAYAK_MIN_BOUNCE	((MAX_VELOCITY / 2) / 256)

#define KAYAK_IN_FORWARD	IN_FORWARD
#define KAYAK_IN_BACK		IN_BACK
#define KAYAK_IN_LEFT		IN_LEFT
#define KAYAK_IN_RIGHT		IN_RIGHT
#define KAYAK_IN_HOLD_LEFT	IN_LSTEP
#define KAYAK_IN_HOLD_RIGHT	IN_RSTEP
#define KAYAK_IN_DISMOUNT	(IN_JUMP | IN_ROLL)

enum KayakState
{
	KAYAK_STATE_BACK = 0,
	KAYAK_STATE_IDLE = 1,
	KAYAK_STATE_TURN_LEFT = 2,
	KAYAK_STATE_TURN_RIGHT = 3,
	KAYAK_STATE_MOUNT_LEFT = 4,
	KAYAK_STATE_IDLE_DEATH = 5,
	KAYAK_STATE_FORWARD = 6,
	KAYAK_STATE_CAPSIZE_RECOVER = 7,	// Unused.
	KAYAK_STATE_CAPSIZE_DEATH = 8,		// Unused.
	KAYAK_STATE_DISMOUNT = 9,
	KAYAK_STATE_HOLD_LEFT = 10,
	KAYAK_STATE_HOLD_RIGHT = 11,
	KAYAK_STATE_MOUNT_RIGHT = 12,
	KAYAK_STATE_DISMOUNT_LEFT = 13,
	KAYAK_STATE_DISMOUNT_RIGHT = 14,
};

enum KayakAnim
{
	KAYAK_ANIM_PADDLE_BACK_END = 0,
	KAYAK_ANIM_PADDLE_BACK_START = 1,
	KAYAK_ANIM_PADDLE_BACK = 2,
	KAYAK_ANIM_MOUNT_RIGHT = 3,
	KAYAK_ANIM_GET_PADDLE = 4,
	KAYAK_ANIM_IDLE_DEATH = 5,
	KAYAK_ANIM_CAPSIZE_DEATH = 6,			// Unused.
	KAYAK_ANIM_PADDLE_FORWARD_END = 7,		// Unused.
	KAYAK_ANIM_PADDLE_FORWARD = 8,			// Unused.
	KAYAK_ANIM_PADDLE_FORWARD_START = 9,	// Unused.
	KAYAK_ANIM_HIT_BACK = 10,
	KAYAK_ANIM_HIT_FRONT = 11,
	KAYAK_ANIM_HIT_RIGHT = 12,
	KAYAK_ANIM_CAPSIZE_LEFT = 13,			// Unused.
	KAYAK_ANIM_DISMOUNT_START = 14,
	KAYAK_ANIM_PADDLE_LEFT = 15,
	KAYAK_ANIM_IDLE = 16,
	KAYAK_ANIM_PADDLE_RIGHT = 17,
	KAYAK_ANIM_CAPSIZE_STRUGGLE = 18,		// Unused.
	KAYAK_ANIM_CAPSIZE_RECOVER_LEFT = 19,	// Unused.
	KAYAK_ANIM_HOLD_PADDLE_LEFT_START = 20,
	KAYAK_ANIM_HOLD_PADDLE_LEFT_END = 21,
	KAYAK_ANIM_HOLD_PADDLE_RIGHT_START = 22,
	KAYAK_ANIM_HOLD_PADDLE_RIGHT_END = 23,
	KAYAK_ANIM_DISMOUNT_LEFT = 24,
	KAYAK_ANIM_OVERBOARD_DEATH = 25,
	KAYAK_ANIM_HOLD_PADDLE_LEFT = 26,
	KAYAK_ANIM_HOLD_PADDLE_RIGHT = 27,
	KAYAK_ANIM_MOUNT_LEFT = 28,
	KAYAK_ANIM_HIT_LEFT = 29,
	KAYAK_ANIM_CAPSIZE_RIGHT = 30,			// Unused.
	KAYAK_ANIM_CAPSIZE_RECOVER_RIGHT = 31,	// Unused.
	KAYAK_ANIM_DISMOUNT_RIGHT = 32
};

enum class KayakMountType
{
	None,
	Left,
	Right
};

struct WAKE_PTS 
{
	int 	x[2];
	int 	y;
	int		z[2];
	short	xvel[2];
	short	zvel[2];
	byte 	life;
	byte	pad[3];
};

WAKE_PTS WakePts[NUM_WAKE_SPRITES][2];
byte CurrentStartWake = 0;
byte WakeShade = 0;

void InitialiseKayak(short itemNumber)
{
	auto* kayakItem = &g_Level.Items[itemNumber];
	kayakItem->Data = KayakInfo();
	auto* kayak = (KayakInfo*)kayakItem->Data;

	kayak->TurnRate = 0;
	kayak->Velocity = 0;
	kayak->FrontVerticalVelocity = 0;
	kayak->LeftVerticalVelocity = 0;
	kayak->RightVerticalVelocity = 0;
	kayak->LeftRightCount = 0;
	kayak->OldPos = kayakItem->Position;
	kayak->Flags = 0;


	for (int i = 0; i < NUM_WAKE_SPRITES; i++)
	{
		WakePts[i][0].life = 0;
		WakePts[i][1].life = 0;
	}
}

void KayakDraw(ITEM_INFO* kayakItem)
{
	DrawAnimatingItem(kayakItem);
}

void KayakDoWake(ITEM_INFO* kayakItem, int xOffset, int zOffset, short rotate)
{
	if (WakePts[CurrentStartWake][rotate].life)
		return;

	float s = phd_sin(kayakItem->Position.yRot);
	float c = phd_cos(kayakItem->Position.yRot);

	int x = kayakItem->Position.xPos + zOffset * s + xOffset * c;
	int z = kayakItem->Position.zPos + zOffset * c - xOffset * s;

	int probedRoomNum = GetCollisionResult(x, kayakItem->Position.yPos, z, kayakItem->RoomNumber).RoomNumber;
	int waterHeight = GetWaterHeight(x, kayakItem->Position.yPos, z, probedRoomNum);

	if (waterHeight != NO_HEIGHT)
	{
		short angle1, angle2;
		if (kayakItem->Velocity < 0)
		{
			if (!rotate)
			{
				angle1 = kayakItem->Position.yRot - ANGLE(10.0f);
				angle2 = kayakItem->Position.yRot - ANGLE(30.0f);
			}
			else
			{
				angle1 = kayakItem->Position.yRot + ANGLE(10.0f);
				angle2 = kayakItem->Position.yRot + ANGLE(30.0f);
			}
		}
		else
		{
			if (!rotate)
			{
				angle1 = kayakItem->Position.yRot - ANGLE(170.0f);
				angle2 = kayakItem->Position.yRot - ANGLE(150.0f);
			}
			else
			{
				angle1 = kayakItem->Position.yRot + ANGLE(170.0f);
				angle2 = kayakItem->Position.yRot + ANGLE(150.0f);
			}
		}

		int xv[2], zv[2];
		xv[0] = WAKE_VELOCITY * phd_sin(angle1);
		zv[0] = WAKE_VELOCITY * phd_cos(angle1);
		xv[1] = (WAKE_VELOCITY + 2) * phd_sin(angle2);
		zv[1] = (WAKE_VELOCITY + 2) * phd_cos(angle2);

		WakePts[CurrentStartWake][rotate].y = kayakItem->Position.yPos + KAYAK_DRAW_SHIFT;
		WakePts[CurrentStartWake][rotate].life = 0x40;

		for (int i = 0; i < 2; i++)
		{
			WakePts[CurrentStartWake][rotate].x[i] = x;
			WakePts[CurrentStartWake][rotate].z[i] = z;
			WakePts[CurrentStartWake][rotate].xvel[i] = xv[i];
			WakePts[CurrentStartWake][rotate].zvel[i] = zv[i];
		}

		if (rotate == 1)
		{
			CurrentStartWake++;
			CurrentStartWake &= (NUM_WAKE_SPRITES - 1);
		}
	}
}

void KayakDoRipple(ITEM_INFO* kayakItem, int xOffset, int zOffset)
{
	float s = phd_sin(kayakItem->Position.yRot);
	float c = phd_cos(kayakItem->Position.yRot);

	int x = kayakItem->Position.xPos + zOffset * s + xOffset * c;
	int z = kayakItem->Position.zPos + zOffset * c - xOffset * s;

	int probedRoomNum = GetCollisionResult(x, kayakItem->Position.yPos, z, kayakItem->RoomNumber).RoomNumber;
	int waterHeight = GetWaterHeight(x, kayakItem->Position.yPos, z, probedRoomNum);

	if (waterHeight != NO_HEIGHT)
		SetupRipple(x, kayakItem->Position.yPos, z, -2 - (GetRandomControl() & 1), 0, Objects[ID_KAYAK_PADDLE_TRAIL_SPRITE].meshIndex,TO_RAD(kayakItem->Position.yRot));
}

void KayakUpdateWakeFX()
{
	for (int i = 0; i < 2; i++)
	{
		for (int j = 0; j < NUM_WAKE_SPRITES; j++)
		{
			if (WakePts[j][i].life)
			{
				WakePts[j][i].life--;
				WakePts[j][i].x[0] += WakePts[j][i].xvel[0];
				WakePts[j][i].z[0] += WakePts[j][i].zvel[0];
				WakePts[j][i].x[1] += WakePts[j][i].xvel[1];
				WakePts[j][i].z[1] += WakePts[j][i].zvel[1];
			}
		}
	}
}

KayakMountType KayakGetMountType(ITEM_INFO* laraItem, short itemNumber)
{
	auto* lara = GetLaraInfo(laraItem);
	auto* kayakItem = &g_Level.Items[itemNumber];

	if (!(TrInput & IN_ACTION) ||
		lara->Control.HandStatus != HandStatus::Free ||
		laraItem->Airborne)
	{
		return KayakMountType::None;
	}

	int distance = pow(laraItem->Position.xPos - kayakItem->Position.xPos, 2) + pow(laraItem->Position.zPos - kayakItem->Position.zPos, 2);
	if (distance > pow(360, 2))
		return KayakMountType::None;

	auto probe = GetCollisionResult(kayakItem);
	if (probe.Position.Floor > -32000)
	{
		short angle = phd_atan(kayakItem->Position.zPos - laraItem->Position.zPos, kayakItem->Position.xPos - laraItem->Position.xPos);
		angle -= kayakItem->Position.yRot;

		int deltaAngle = laraItem->Position.yRot - kayakItem->Position.yRot;
		if (angle > -ANGLE(45.0f) && angle < ANGLE(135.0f))
		{
			deltaAngle = laraItem->Position.yRot - kayakItem->Position.yRot;
			if (deltaAngle > ANGLE(45.0f) && deltaAngle < ANGLE(135.0f))
				return KayakMountType::Left;
		}
		else
		{
			deltaAngle = laraItem->Position.yRot - kayakItem->Position.yRot;
			if (deltaAngle > ANGLE(225.0f) && deltaAngle < ANGLE(315.0f))
				return KayakMountType::Right;
		}
	}

	return KayakMountType::None;
}

int KayakGetCollisionAnim(ITEM_INFO* kayakItem, int xDiff, int zDiff)
{
	xDiff = kayakItem->Position.xPos - xDiff;
	zDiff = kayakItem->Position.zPos - zDiff;

	if ((xDiff) || (zDiff))
	{
		float s = phd_sin(kayakItem->Position.yRot);
		float c = phd_cos(kayakItem->Position.yRot);

		int front = zDiff * c + xDiff * s;
		int side = -zDiff * s + xDiff * c;

		if (abs(front) > abs(side))
		{
			if (front > 0)
				return HIT_BACK;
			else
				return HIT_FRONT;
		}
		else
		{
			if (side > 0)
				return HIT_LEFT;
			else
				return HIT_RIGHT;
		}
	}

	return 0;
}

int KayakDoDynamics(int height, int verticalVelocity, int* y)
{
	if (height > * y)
	{
		*y += verticalVelocity;

		if (*y > height)
		{
			*y = height;
			verticalVelocity = 0;
		}
		else
			verticalVelocity += GRAVITY;
	}
	else
	{
		int kick = (height - *y) * 4;

		if (kick < KAYAK_MAX_KICK)
			kick = KAYAK_MAX_KICK;

		verticalVelocity += (kick - verticalVelocity) / 8;

		if (*y > height)
			*y = height;
	}

	return verticalVelocity;
}

void KayakDoCurrent(ITEM_INFO* laraItem, ITEM_INFO* kayakItem)
{
	auto* lara = GetLaraInfo(laraItem);
	auto* room = &g_Level.Rooms[kayakItem->RoomNumber];

	if (!lara->Control.WaterCurrentActive)
	{
		int absVelocity = abs(lara->ExtraVelocity.x);
		int shift;

		if (absVelocity > 16)
			shift = 4;
		else if (absVelocity > 8)
			shift = 3;
		else
			shift = 2;

		lara->ExtraVelocity.x -= lara->ExtraVelocity.x >> shift;

		if (abs(lara->ExtraVelocity.x) < 4)
			lara->ExtraVelocity.x = 0;

		absVelocity = abs(lara->ExtraVelocity.z);
		if (absVelocity > 16)
			shift = 4;
		else if (absVelocity > 8)
			shift = 3;
		else
			shift = 2;

		lara->ExtraVelocity.z -= lara->ExtraVelocity.z >> shift;
		if (abs(lara->ExtraVelocity.z) < 4)
			lara->ExtraVelocity.z = 0;

		if (lara->ExtraVelocity.x == 0 && lara->ExtraVelocity.z == 0)
			return;
	}
	else
	{
		int sinkval = lara->Control.WaterCurrentActive - 1;
		
		PHD_VECTOR target;
		target.x = g_Level.Sinks[sinkval].x;
		target.y = g_Level.Sinks[sinkval].y;
		target.z = g_Level.Sinks[sinkval].z;
		
		int angle = (((mGetAngle(target.x, target.z, laraItem->Position.xPos, laraItem->Position.zPos) - ANGLE(90))) / 16) & 4095;

		int dx = target.x - laraItem->Position.xPos;
		int dz = target.z - laraItem->Position.zPos;

		int speed = g_Level.Sinks[sinkval].strength;
		dx = phd_sin(angle * 16) * speed * 1024;
		dz = phd_cos(angle * 16) * speed * 1024;

		lara->ExtraVelocity.x += (dx - lara->ExtraVelocity.x) / 16;
		lara->ExtraVelocity.z += (dz - lara->ExtraVelocity.z) / 16;
	}

	kayakItem->Position.xPos += lara->ExtraVelocity.x / 256;
	kayakItem->Position.zPos += lara->ExtraVelocity.z / 256;

	lara->Control.WaterCurrentActive = 0;
}

int KayakTestHeight(ITEM_INFO* kayakItem, int x, int z, PHD_VECTOR* pos)
{
	Matrix world =
		Matrix::CreateFromYawPitchRoll(TO_RAD(kayakItem->Position.yRot), TO_RAD(kayakItem->Position.xRot), TO_RAD(kayakItem->Position.zRot)) *
		Matrix::CreateTranslation(kayakItem->Position.xPos, kayakItem->Position.yPos, kayakItem->Position.zPos);

	Vector3 vec = Vector3(x, 0, z);
	vec = Vector3::Transform(vec, world);
	
	pos->x = vec.x;
	pos->y = vec.y;
	pos->z = vec.z;

	auto probe = GetCollisionResult(pos->x, pos->y, pos->z, kayakItem->RoomNumber);
	int probedRoomNum = probe.RoomNumber;

	int height = GetWaterHeight(pos->x, pos->y, pos->z, probedRoomNum);
	if (height == NO_HEIGHT)
	{
		height = probe.Position.Floor;
		if (height == NO_HEIGHT)
			return height;
	}

	return (height - 5);
}

bool KayakCanGetOut(ITEM_INFO* kayakItem, int dir)
{
	PHD_VECTOR pos;
	int height = KayakTestHeight(kayakItem, (dir < 0) ? -DISMOUNT_DISTANCE : DISMOUNT_DISTANCE, 0, &pos);

	if ((kayakItem->Position.yPos - height) > 0)
		return false;

	return true;
}

int KayakDoShift(ITEM_INFO* kayakItem, PHD_VECTOR* pos, PHD_VECTOR* old)
{
	int x = pos->x / SECTOR(1);
	int z = pos->z / SECTOR(1);

	int xOld = old->x / SECTOR(1);
	int zOld = old->z / SECTOR(1);

	int xShift = pos->x & (SECTOR(1) - 1);
	int zShift = pos->z & (SECTOR(1) - 1);

	if (x == xOld)
	{
		old->x = 0;

		if (z == zOld)
		{
			kayakItem->Position.zPos += old->z - pos->z;
			kayakItem->Position.xPos += old->x - pos->x;
		}
		else if (z > zOld)
		{
			kayakItem->Position.zPos -= zShift + 1;
			return (pos->x - kayakItem->Position.xPos);
		}
		else
		{
			kayakItem->Position.zPos += SECTOR(1) - zShift;
			return (kayakItem->Position.xPos - pos->x);
		}
	}
	else if (z == zOld)
	{
		old->z = 0;

		if (x > xOld)
		{
			kayakItem->Position.xPos -= xShift + 1;
			return (kayakItem->Position.zPos - pos->z);
		}

		else
		{
			kayakItem->Position.xPos += SECTOR(1) - xShift;
			return (pos->z - kayakItem->Position.zPos);
		}
	}
	else
	{
		x = 0;
		z = 0;

		auto probe = GetCollisionResult(old->x, pos->y, pos->z, kayakItem->RoomNumber);
		if (probe.Position.Floor < (old->y - CLICK(1)))
		{
			if (pos->z > old->z)
				z = -zShift - 1;
			else
				z = SECTOR(1) - zShift;
		}

		probe = GetCollisionResult(pos->x, pos->y, old->z, kayakItem->RoomNumber);
		if (probe.Position.Floor < (old->y - CLICK(1)))
		{
			if (pos->x > old->x)
				x = -xShift - 1;
			else
				x = SECTOR(1) - xShift;
		}

		if (x && z)
		{
			kayakItem->Position.xPos += x;
			kayakItem->Position.zPos += z;
		}
		else if (x)
		{
			kayakItem->Position.xPos += x;

			if (x > 0)
				return (pos->z - kayakItem->Position.zPos);
			else
				return (kayakItem->Position.zPos - pos->z);
		}
		else if (z)
		{
			kayakItem->Position.zPos += z;

			if (z > 0)
				return (kayakItem->Position.xPos - pos->x);
			else
				return (pos->x - kayakItem->Position.xPos);
		}
		else
		{
			kayakItem->Position.xPos += (old->x - pos->x);
			kayakItem->Position.zPos += (old->z - pos->z);
		}
	}

	return 0;
}

void KayakToBackground(ITEM_INFO* laraItem, ITEM_INFO* kayakItem)
{
	auto* kayak = (KayakInfo*)kayakItem->Data;

	kayak->OldPos = kayakItem->Position;

	PHD_VECTOR oldPos[9];
	int height[8];
	height[0] = KayakTestHeight(kayakItem, 0, 1024, &oldPos[0]);
	height[1] = KayakTestHeight(kayakItem, -96, 512, &oldPos[1]);
	height[2] = KayakTestHeight(kayakItem, 96, 512, &oldPos[2]);
	height[3] = KayakTestHeight(kayakItem, -128, 128, &oldPos[3]);
	height[4] = KayakTestHeight(kayakItem, 128, 128, &oldPos[4]);
	height[5] = KayakTestHeight(kayakItem, -128, -320, &oldPos[5]);
	height[6] = KayakTestHeight(kayakItem, 128, -320, &oldPos[6]);
	height[7] = KayakTestHeight(kayakItem, 0, -640, &oldPos[7]);

	for (int i = 0; i < 8; i++)
	{
		if (oldPos[i].y > height[i])
			oldPos[i].y = height[i];
	}

	oldPos[8].x = kayakItem->Position.xPos;
	oldPos[8].y = kayakItem->Position.yPos;
	oldPos[8].z = kayakItem->Position.zPos;
 
	PHD_VECTOR frontPos, leftPos, rightPos;
	int frontHeight = KayakTestHeight(kayakItem, 0, 1024, &frontPos);
	int leftHeight = KayakTestHeight(kayakItem, -KAYAK_X, KAYAK_Z, &leftPos);
	int rightHeight = KayakTestHeight(kayakItem, KAYAK_X, KAYAK_Z, &rightPos);

	kayakItem->Position.yRot += kayak->TurnRate / (USHRT_MAX + 1);
	kayakItem->Position.xPos += kayakItem->Velocity * phd_sin(kayakItem->Position.yRot);
	kayakItem->Position.zPos += kayakItem->Velocity * phd_cos(kayakItem->Position.yRot);

	KayakDoCurrent(laraItem,kayakItem);

	kayak->LeftVerticalVelocity = KayakDoDynamics(leftHeight, kayak->LeftVerticalVelocity, &leftPos.y);
	kayak->RightVerticalVelocity = KayakDoDynamics(rightHeight, kayak->RightVerticalVelocity, &rightPos.y);
	kayak->FrontVerticalVelocity = KayakDoDynamics(frontHeight, kayak->FrontVerticalVelocity, &frontPos.y);

	kayakItem->VerticalVelocity = KayakDoDynamics(kayak->WaterHeight, kayakItem->VerticalVelocity, &kayakItem->Position.yPos);

	int height2 = (leftPos.y + rightPos.y) / 2;
	int x = phd_atan(1024, kayakItem->Position.yPos - frontPos.y);
	int z = phd_atan(KAYAK_X, height2 - leftPos.y);

	kayakItem->Position.xRot = x;
	kayakItem->Position.zRot = z;

	int xOld = kayakItem->Position.xPos;
	int zOld = kayakItem->Position.zPos;

	int rot = 0;
	PHD_VECTOR pos;

	if ((height2 = KayakTestHeight(kayakItem, 0, -CLICK(2.5f), &pos)) < (oldPos[7].y - KAYAK_COLLIDE))
		rot = KayakDoShift(kayakItem, &pos, &oldPos[7]);

	if ((height2 = KayakTestHeight(kayakItem, CLICK(0.5f), -CLICK(1.25f), &pos)) < (oldPos[6].y - KAYAK_COLLIDE))
		rot += KayakDoShift(kayakItem, &pos, &oldPos[6]);

	if ((height2 = KayakTestHeight(kayakItem, -CLICK(0.5f), -CLICK(1.25f), &pos)) < (oldPos[5].y - KAYAK_COLLIDE))
		rot += KayakDoShift(kayakItem, &pos, &oldPos[5]);

	if ((height2 = KayakTestHeight(kayakItem, CLICK(0.5f), CLICK(0.5f), &pos)) < (oldPos[4].y - KAYAK_COLLIDE))
		rot += KayakDoShift(kayakItem, &pos, &oldPos[4]);

	if ((height2 = KayakTestHeight(kayakItem, -CLICK(0.5f), CLICK(0.5f), &pos)) < (oldPos[3].y - KAYAK_COLLIDE))
		rot += KayakDoShift(kayakItem, &pos, &oldPos[3]);

	if ((height2 = KayakTestHeight(kayakItem, 96, CLICK(2), &pos)) < (oldPos[2].y - KAYAK_COLLIDE))
		rot += KayakDoShift(kayakItem, &pos, &oldPos[2]);

	if ((height2 = KayakTestHeight(kayakItem, -96, CLICK(2), &pos)) < (oldPos[1].y - KAYAK_COLLIDE))
		rot += KayakDoShift(kayakItem, &pos, &oldPos[1]);

	if ((height2 = KayakTestHeight(kayakItem, 0, CLICK(4), &pos)) < (oldPos[0].y - KAYAK_COLLIDE))
		rot += KayakDoShift(kayakItem, &pos, &oldPos[0]);

	kayakItem->Position.yRot += rot;

	auto probe = GetCollisionResult(kayakItem);
	int probedRoomNum = probe.RoomNumber;

	height2 = GetWaterHeight(kayakItem->Position.xPos, kayakItem->Position.yPos, kayakItem->Position.zPos, probedRoomNum);
	if (height2 == NO_HEIGHT)
		height2 = probe.Position.Floor;

	if (height2 < (kayakItem->Position.yPos - KAYAK_COLLIDE))
		KayakDoShift(kayakItem, (PHD_VECTOR*)&kayakItem->Position, &oldPos[8]);

	probe = GetCollisionResult(kayakItem);
	probedRoomNum = probe.RoomNumber;

	height2 = GetWaterHeight(kayakItem->Position.xPos, kayakItem->Position.yPos, kayakItem->Position.zPos, probedRoomNum);
	if (height2 == NO_HEIGHT)
		height2 = probe.Position.Floor;

	if (height2 == NO_HEIGHT)
	{
		GAME_VECTOR kayakPos;
		kayakPos.x = kayak->OldPos.xPos;
		kayakPos.y = kayak->OldPos.yPos;
		kayakPos.z = kayak->OldPos.zPos;
		kayakPos.roomNumber = kayakItem->RoomNumber;

		CameraCollisionBounds(&kayakPos, 256, 0);
		{
			kayakItem->Position.xPos = kayakPos.x;
			kayakItem->Position.yPos = kayakPos.y;
			kayakItem->Position.zPos = kayakPos.z;
			kayakItem->RoomNumber = kayakPos.roomNumber;
		}
	}

	int collide = KayakGetCollisionAnim(kayakItem, xOld, zOld);

	int slip = 0; // Remnant?
	if (slip || collide)
	{
		int newVelocity;

		newVelocity = (kayakItem->Position.zPos - oldPos[8].z) * phd_cos(kayakItem->Position.yRot) + (kayakItem->Position.xPos - oldPos[8].x) * phd_sin(kayakItem->Position.yRot);
		newVelocity *= 256;

		if (slip)
		{
			if (kayak->Velocity <= MAX_VELOCITY)
				kayak->Velocity = newVelocity;
		}
		else
		{
			if (kayak->Velocity > 0 && newVelocity < kayak->Velocity)
				kayak->Velocity = newVelocity;

			else if (kayak->Velocity < 0 && newVelocity > kayak->Velocity)
				kayak->Velocity = newVelocity;
		}

		if (kayak->Velocity < -MAX_VELOCITY)
			kayak->Velocity = -MAX_VELOCITY;
	}
}

void KayakUserInput(ITEM_INFO* laraItem, ITEM_INFO* kayakItem)
{
	auto* lara = GetLaraInfo(laraItem);
	auto* kayak = (KayakInfo*)kayakItem->Data;

	if (laraItem->HitPoints <= 0 &&
		laraItem->ActiveState != KAYAK_STATE_IDLE_DEATH)
	{
		laraItem->AnimNumber = Objects[ID_KAYAK_LARA_ANIMS].animIndex + KAYAK_ANIM_IDLE_DEATH;
		laraItem->FrameNumber = g_Level.Anims[laraItem->AnimNumber].frameBase;
		laraItem->ActiveState = laraItem->TargetState = KAYAK_STATE_IDLE_DEATH;
	}

	int frame = laraItem->FrameNumber - g_Level.Anims[laraItem->AnimNumber].frameBase;

	switch (laraItem->ActiveState)
	{
	case KAYAK_STATE_IDLE:
		if (TrInput & KAYAK_IN_DISMOUNT &&
			!lara->Control.WaterCurrentActive &&
			!lara->ExtraVelocity.x && !lara->ExtraVelocity.z)
		{
			if (TrInput & KAYAK_IN_LEFT && KayakCanGetOut(kayakItem, -1))
			{
				laraItem->TargetState = KAYAK_STATE_DISMOUNT;
				laraItem->RequiredState = KAYAK_STATE_DISMOUNT_LEFT;
			}
			else if (TrInput & KAYAK_IN_RIGHT && KayakCanGetOut(kayakItem, 1))
			{
				laraItem->TargetState = KAYAK_STATE_DISMOUNT;
				laraItem->RequiredState = KAYAK_STATE_DISMOUNT_RIGHT;
			}
		}
		else if (TrInput & KAYAK_IN_FORWARD)
		{
			laraItem->TargetState = KAYAK_STATE_TURN_RIGHT;
			kayak->Turn = false;
			kayak->Forward = true;
		}
		else if (TrInput & KAYAK_IN_BACK)
			laraItem->TargetState = KAYAK_STATE_BACK;
		else if (TrInput & KAYAK_IN_LEFT)
		{
			laraItem->TargetState = KAYAK_STATE_TURN_LEFT;

			if (kayak->Velocity)
				kayak->Turn = false;
			else
				kayak->Turn = true;

			kayak->Forward = false;
		}

		else if (TrInput & KAYAK_IN_RIGHT)
		{
			laraItem->TargetState = KAYAK_STATE_TURN_RIGHT;

			if (kayak->Velocity)
				kayak->Turn = false;
			else
				kayak->Turn = true;

			kayak->Forward = false;
		}
		else if (TrInput & KAYAK_IN_HOLD_LEFT &&
			(kayak->Velocity ||
				lara->ExtraVelocity.x ||
				lara->ExtraVelocity.z))
		{
			laraItem->TargetState = KAYAK_STATE_HOLD_LEFT;
		}
		else if (TrInput & KAYAK_IN_HOLD_RIGHT &&
			(kayak->Velocity ||
				lara->ExtraVelocity.x ||
				lara->ExtraVelocity.z))
		{
			laraItem->TargetState = KAYAK_STATE_HOLD_RIGHT;
		}

		break;
		
	case KAYAK_STATE_TURN_LEFT:
		if (kayak->Forward)
		{
			if (!frame)
				kayak->LeftRightCount = 0;

			if (frame == 2 && !(kayak->LeftRightCount & 0x80))
				kayak->LeftRightCount++;

			else if (frame > 2)
				kayak->LeftRightCount &= ~0x80;

			if (TrInput & KAYAK_IN_FORWARD)
			{
				if (TrInput & KAYAK_IN_LEFT)
				{
					if ((kayak->LeftRightCount & ~0x80) >= 2)
						laraItem->TargetState = KAYAK_STATE_TURN_RIGHT;
				}
				else
					laraItem->TargetState = KAYAK_STATE_TURN_RIGHT;
			}
			else
				laraItem->TargetState = KAYAK_STATE_IDLE;
		}
		else if (!(TrInput & KAYAK_IN_LEFT))
			laraItem->TargetState = KAYAK_STATE_IDLE;

		if (frame == 7)
		{
			if (kayak->Forward)
			{
				kayak->TurnRate -= KAYAK_FORWARD_ROTATION;
				if (kayak->TurnRate < -KAYAK_MAX_TURN)
					kayak->TurnRate = -KAYAK_MAX_TURN;

				kayak->Velocity += KAYAK_FORWARD_VELOCITY;
			}
			else if (kayak->Turn)
			{
				kayak->TurnRate -= KAYAK_HARD_ROTATION;
				if (kayak->TurnRate < -KAYAK_MAX_STAT)
					kayak->TurnRate = -KAYAK_MAX_STAT;
			}
			else
			{
				kayak->TurnRate -= KAYAK_LEFT_RIGHT_ROTATION;
				if (kayak->TurnRate < -KAYAK_MAX_LEFT_RIGHT)
					kayak->TurnRate = -KAYAK_MAX_LEFT_RIGHT;

				kayak->Velocity += KAYAK_LEFT_RIGHT_VELOCITY;
			}
		}

		if (frame > 6 && frame < 24 && frame & 1)
			KayakDoRipple(kayakItem, -CLICK(1.5f), -CLICK(0.25f));

		break;
		
	case KAYAK_STATE_TURN_RIGHT:	
		if (kayak->Forward)
		{
			if (!frame)
				kayak->LeftRightCount = 0;

			if (frame == 2 && !(kayak->LeftRightCount & 0x80))
				kayak->LeftRightCount++;

			else if (frame > 2)
				kayak->LeftRightCount &= ~0x80;

			if (TrInput & KAYAK_IN_FORWARD)
			{
				if (TrInput & KAYAK_IN_RIGHT)
				{
					if ((kayak->LeftRightCount & ~0x80) >= 2)
						laraItem->TargetState = KAYAK_STATE_TURN_LEFT;
				}
				else
					laraItem->TargetState = KAYAK_STATE_TURN_LEFT;
			}
			else
				laraItem->TargetState = KAYAK_STATE_IDLE;
		}

		else if (!(TrInput & KAYAK_IN_RIGHT))
			laraItem->TargetState = KAYAK_STATE_IDLE;

		if (frame == 7)
		{
			if (kayak->Forward)
			{
				kayak->TurnRate += KAYAK_FORWARD_ROTATION;
				if (kayak->TurnRate > KAYAK_MAX_TURN)
					kayak->TurnRate = KAYAK_MAX_TURN;

				kayak->Velocity += KAYAK_FORWARD_VELOCITY;
			}
			else if (kayak->Turn)
			{
				kayak->TurnRate += KAYAK_HARD_ROTATION;
				if (kayak->TurnRate > KAYAK_MAX_STAT)
					kayak->TurnRate = KAYAK_MAX_STAT;
			}
			else
			{
				kayak->TurnRate += KAYAK_LEFT_RIGHT_ROTATION;
				if (kayak->TurnRate > KAYAK_MAX_LEFT_RIGHT)
					kayak->TurnRate = KAYAK_MAX_LEFT_RIGHT;

				kayak->Velocity += KAYAK_LEFT_RIGHT_VELOCITY;
			}
		}

		if (frame > 6 && frame < 24 && frame & 1)
			KayakDoRipple(kayakItem, CLICK(1.5f), -CLICK(0.25f));

		break;
		
	case KAYAK_STATE_BACK:
		if (!(TrInput & KAYAK_IN_BACK))
			laraItem->TargetState = KAYAK_STATE_IDLE;

		if ((laraItem->AnimNumber - Objects[ID_KAYAK_LARA_ANIMS].animIndex) == KAYAK_ANIM_PADDLE_BACK)
		{
			if (frame == 8)
			{
				kayak->TurnRate += KAYAK_FORWARD_ROTATION;
				kayak->Velocity -= KAYAK_FORWARD_VELOCITY;
			}

			if (frame == 31)
			{
				kayak->TurnRate -= KAYAK_FORWARD_ROTATION;
				kayak->Velocity -= KAYAK_FORWARD_VELOCITY;
			}

			if (frame < 15 && frame & 1)
				KayakDoRipple(kayakItem, CLICK(1.5f), -CLICK(0.5f));

			else if (frame >= 20 && frame <= 34 && frame & 1)
				KayakDoRipple(kayakItem, -CLICK(1.5f), -CLICK(0.5f));
		}

		break;
		
	case KAYAK_STATE_HOLD_LEFT:
		if (!(TrInput & KAYAK_IN_HOLD_LEFT) ||
			(!kayak->Velocity &&
				!lara->ExtraVelocity.x &&
				!lara->ExtraVelocity.z))
		{
			laraItem->TargetState = KAYAK_STATE_IDLE;
		}
		else if ((laraItem->AnimNumber - Objects[ID_KAYAK_LARA_ANIMS].animIndex) == KAYAK_ANIM_HOLD_PADDLE_LEFT)
		{
			if (kayak->Velocity >= 0)
			{
				kayak->TurnRate -= KAYAK_TURN_ROTATION;
				if (kayak->TurnRate < -KAYAK_MAX_TURN)
					kayak->TurnRate = -KAYAK_MAX_TURN;

				kayak->Velocity += -KAYAK_TURN_BRAKE;
				if (kayak->Velocity < 0)
					kayak->Velocity = 0;
			}

			if (kayak->Velocity < 0)
			{
				kayak->TurnRate += KAYAK_TURN_ROTATION;

				kayak->Velocity += KAYAK_TURN_BRAKE;
				if (kayak->Velocity > 0)
					kayak->Velocity = 0;
			}

			if (!(Wibble & 3))
				KayakDoRipple(kayakItem, -CLICK(1), -CLICK(1));
		}

		break;
		
	case KAYAK_STATE_HOLD_RIGHT:
		if (!(TrInput & KAYAK_IN_HOLD_RIGHT) ||
			(!kayak->Velocity &&
				!lara->ExtraVelocity.x &&
				!lara->ExtraVelocity.z))
		{
			laraItem->TargetState = KAYAK_STATE_IDLE;
		}
		else if ((laraItem->AnimNumber - Objects[ID_KAYAK_LARA_ANIMS].animIndex) == KAYAK_ANIM_HOLD_PADDLE_RIGHT)
		{
			if (kayak->Velocity >= 0)
			{
				kayak->TurnRate += KAYAK_TURN_ROTATION;
				if (kayak->TurnRate > KAYAK_MAX_TURN)
					kayak->TurnRate = KAYAK_MAX_TURN;

				kayak->Velocity += -KAYAK_TURN_BRAKE;
				if (kayak->Velocity < 0)
					kayak->Velocity = 0;
			}

			if (kayak->Velocity < 0)
			{
				kayak->TurnRate -= KAYAK_TURN_ROTATION;

				kayak->Velocity += KAYAK_TURN_BRAKE;
				if (kayak->Velocity > 0)
					kayak->Velocity = 0;
			}

			if (!(Wibble & 3))
				KayakDoRipple(kayakItem, CLICK(1), -CLICK(1));
		}

		break;
		
	case KAYAK_STATE_MOUNT_LEFT:
		if (laraItem->AnimNumber == Objects[ID_KAYAK_LARA_ANIMS].animIndex + KAYAK_ANIM_GET_PADDLE &&
			frame == 24 &&
			!(kayak->Flags & 0x80))
		{
			kayak->Flags |= 0x80;
			lara->meshPtrs[LM_RHAND] = Objects[ID_KAYAK_LARA_ANIMS].meshIndex + LM_RHAND;
			laraItem->MeshBits &= ~LARA_LEG_BITS;
		}

		break;
		
	case KAYAK_STATE_DISMOUNT:
		if (laraItem->AnimNumber == Objects[ID_KAYAK_LARA_ANIMS].animIndex + KAYAK_ANIM_DISMOUNT_START &&
			frame == 27 &&
			kayak->Flags & 0x80)
		{
			kayak->Flags &= ~0x80;
			lara->meshPtrs[LM_RHAND] = Objects[ID_LARA_SKIN].meshIndex + LM_RHAND;
			laraItem->MeshBits |= LARA_LEG_BITS;
		}

		laraItem->TargetState = laraItem->RequiredState;
		break;
		
	case KAYAK_STATE_DISMOUNT_LEFT:
		if (laraItem->AnimNumber == Objects[ID_KAYAK_LARA_ANIMS].animIndex + KAYAK_ANIM_DISMOUNT_LEFT &&
			frame == 83)
		{
			PHD_VECTOR vec = { 0, 350, 500 };
			GetLaraJointPosition(&vec, LM_HIPS);

			SetAnimation(laraItem, LA_JUMP_FORWARD);
			laraItem->Position.xPos = vec.x;
			laraItem->Position.yPos = vec.y;
			laraItem->Position.zPos = vec.z;
			laraItem->Position.xRot = 0;
			laraItem->Position.yRot = kayakItem->Position.yRot - ANGLE(90.0f);
			laraItem->Position.zRot = 0;
			laraItem->Velocity = 40;
			laraItem->VerticalVelocity = -50;
			laraItem->Airborne = true;
			lara->Control.HandStatus = HandStatus::Free;
			lara->Vehicle = NO_ITEM;
			kayak->LeftRightCount = 0;
		}

		break;
		
	case KAYAK_STATE_DISMOUNT_RIGHT:
		if (laraItem->AnimNumber == Objects[ID_KAYAK_LARA_ANIMS].animIndex + KAYAK_ANIM_DISMOUNT_RIGHT &&
			frame == 83)
		{
			PHD_VECTOR vec = { 0, 350, 500 };
			GetLaraJointPosition(&vec, LM_HIPS);

			SetAnimation(laraItem, LA_JUMP_FORWARD);
			laraItem->Position.xPos = vec.x;
			laraItem->Position.yPos = vec.y;
			laraItem->Position.zPos = vec.z;
			laraItem->Position.xRot = 0;
			laraItem->Position.yRot = kayakItem->Position.yRot + ANGLE(90.0f);
			laraItem->Position.zRot = 0;
			laraItem->Velocity = 40;
			laraItem->VerticalVelocity = -50;
			laraItem->Airborne = true;
			lara->Control.HandStatus = HandStatus::Free;
			lara->Vehicle = NO_ITEM;
			kayak->LeftRightCount = 0;
		}
	}

	if (kayak->Velocity > 0)
	{
		kayak->Velocity -= KAYAK_FRICTION;
		if (kayak->Velocity < 0)
			kayak->Velocity = 0;
	}
	else if (kayak->Velocity < 0)
	{
		kayak->Velocity += KAYAK_FRICTION;
		if (kayak->Velocity > 0)
			kayak->Velocity = 0;
	}

	if (kayak->Velocity > MAX_VELOCITY)
		kayak->Velocity = MAX_VELOCITY;
	else if (kayak->Velocity < -MAX_VELOCITY)
		kayak->Velocity = -MAX_VELOCITY;

	kayakItem->Velocity = (kayak->Velocity / (USHRT_MAX + 1));
	;
	if (kayak->TurnRate >= 0)
	{
		kayak->TurnRate -= KAYAK_ROTATE_FRICTION;
		if (kayak->TurnRate < 0)
			kayak->TurnRate = 0;
	}
	else if (kayak->TurnRate < 0)
	{
		kayak->TurnRate += KAYAK_ROTATE_FRICTION;
		if (kayak->TurnRate > 0)
			kayak->TurnRate = 0;
	}
}

void KayakToItemCollision(ITEM_INFO* laraItem, ITEM_INFO* kayakItem)
{
	short roomsToCheck[128];
	short numRoomsToCheck = 0;
	roomsToCheck[numRoomsToCheck++] = kayakItem->RoomNumber;

	auto* room = &g_Level.Rooms[kayakItem->RoomNumber];
	for (int i = 0; i < room->doors.size(); i++)
		roomsToCheck[numRoomsToCheck++] = room->doors[i].room;

	for (int i = 0; i < numRoomsToCheck; i++)
	{
		short itemNum = g_Level.Rooms[roomsToCheck[i]].itemNumber;

		while (itemNum != NO_ITEM)
		{
			auto* item = &g_Level.Items[itemNum];
			short nextItem = item->NextItem;

			if (item->Collidable && item->Status != ITEM_INVISIBLE)
			{
				auto* object = &Objects[item->ObjectNumber];

				if (object->collision &&
					(item->ObjectNumber == ID_TEETH_SPIKES ||
						item->ObjectNumber == ID_DARTS &&
						item->ActiveState != 1))
				{
					int x = kayakItem->Position.xPos - item->Position.xPos;
					int y = kayakItem->Position.yPos - item->Position.yPos;
					int z = kayakItem->Position.zPos - item->Position.zPos;

					if (x > -2048 && x < 2048 &&
						y > -2048 && y < 2048 &&
						z > -2048 && z < 2048)
					{
						if (TestBoundsCollide(item, kayakItem, KAYAK_TO_ENTITY_RADIUS))
						{
							DoLotsOfBlood(laraItem->Position.xPos, laraItem->Position.yPos - STEP_SIZE, laraItem->Position.zPos, kayakItem->Velocity, kayakItem->Position.yRot, laraItem->RoomNumber, 3);
							laraItem->HitPoints -= 5;
						}
					}
				}
			}

			itemNum = nextItem;
		}
	}
}

void KayakLaraRapidsDrown(ITEM_INFO* laraItem)
{
	auto* lara = GetLaraInfo(laraItem);

	laraItem->AnimNumber = Objects[ID_KAYAK_LARA_ANIMS].animIndex + KAYAK_ANIM_OVERBOARD_DEATH;
	laraItem->FrameNumber = g_Level.Anims[laraItem->AnimNumber].frameBase;
	laraItem->ActiveState = 12; // TODO
	laraItem->TargetState = 12;
	laraItem->HitPoints = 0;
	laraItem->Velocity = 0;
	laraItem->VerticalVelocity = 0;
	laraItem->Airborne = false;

	AnimateItem(laraItem);

	lara->ExtraAnim = 1;
	lara->Control.HandStatus = HandStatus::Busy;
	lara->Control.WeaponControl.GunType = WEAPON_NONE;
	lara->hitDirection = -1;
}

void KayakCollision(short itemNumber, ITEM_INFO* laraItem, COLL_INFO* coll)
{
	auto* lara = GetLaraInfo(laraItem);
	auto* kayakItem = &g_Level.Items[itemNumber];
	auto* kayak = (KayakInfo*)kayakItem->Data;

	if (laraItem->HitPoints < 0 || lara->Vehicle != NO_ITEM)
		return;

	KayakMountType mountType = KayakGetMountType(laraItem, itemNumber);
	if (mountType != KayakMountType::None)
	{
		lara->Vehicle = itemNumber;

		if (lara->Control.WeaponControl.GunType == WEAPON_FLARE)
		{
			CreateFlare(laraItem, ID_FLARE_ITEM, 0);
			UndrawFlareMeshes(laraItem);
			lara->Flare.ControlLeft = 0;
			lara->Control.WeaponControl.RequestGunType = lara->Control.WeaponControl.GunType = WEAPON_NONE;
		}

		if (mountType == KayakMountType::Right)
			laraItem->AnimNumber = Objects[ID_KAYAK_LARA_ANIMS].animIndex + KAYAK_ANIM_MOUNT_RIGHT;
		else if (mountType == KayakMountType::Left)
			laraItem->AnimNumber = Objects[ID_KAYAK_LARA_ANIMS].animIndex + KAYAK_ANIM_MOUNT_LEFT;

		laraItem->FrameNumber = g_Level.Anims[laraItem->AnimNumber].frameBase;
		laraItem->ActiveState = laraItem->TargetState = KAYAK_STATE_MOUNT_LEFT;
		laraItem->Position.xPos = kayakItem->Position.xPos;
		laraItem->Position.yPos = kayakItem->Position.yPos;
		laraItem->Position.zPos = kayakItem->Position.zPos;
		laraItem->Position.xRot = 0;
		laraItem->Position.yRot = kayakItem->Position.yRot;
		laraItem->Position.zRot = 0;
		laraItem->Velocity = 0;
		laraItem->VerticalVelocity = 0;
		laraItem->Airborne = false;
		lara->Control.WaterStatus = WaterStatus::Dry;

		if (laraItem->RoomNumber != kayakItem->RoomNumber)
			ItemNewRoom(lara->ItemNumber, kayakItem->RoomNumber);

		AnimateItem(laraItem);

		kayak->WaterHeight = kayakItem->Position.yPos;
		kayak->Flags = 0;
	}
	else
	{
		coll->Setup.EnableObjectPush = true;
		ObjectCollision(itemNumber, laraItem, coll);
	}
}

bool KayakControl(ITEM_INFO* laraItem)
{
	auto* lara = GetLaraInfo(laraItem);
	auto* kayakItem = &g_Level.Items[lara->Vehicle];
	auto* kayak = (KayakInfo*)kayakItem->Data;

	if (TrInput & IN_LOOK)
		LookUpDown(laraItem);

	int ofs = kayakItem->VerticalVelocity;

	KayakUserInput(laraItem, kayakItem);
	KayakToBackground(laraItem, kayakItem);
	TestTriggers(kayakItem, false);

	auto probe = GetCollisionResult(kayakItem);
	int water = GetWaterHeight(kayakItem->Position.xPos, kayakItem->Position.yPos, kayakItem->Position.zPos, probe.RoomNumber);
	kayak->WaterHeight = water;

	if (kayak->WaterHeight == NO_HEIGHT)
	{
		water = probe.Position.Floor;
		kayak->WaterHeight = water;
		kayak->TrueWater = false;
	}
	else
	{
		kayak->WaterHeight -= 5;
		kayak->TrueWater = true;
	}

	if ((ofs - kayakItem->VerticalVelocity) > 128 &&
		kayakItem->VerticalVelocity == 0 &&
		water != NO_HEIGHT)
	{
		int damage = ofs - kayakItem->VerticalVelocity;
		if (damage > 160)
			laraItem->HitPoints -= (damage - 160) * 8;
	}

	if (lara->Vehicle != NO_ITEM)
	{
		if (kayakItem->RoomNumber != probe.RoomNumber)
		{
			ItemNewRoom(lara->Vehicle, probe.RoomNumber);
			ItemNewRoom(lara->ItemNumber, probe.RoomNumber);
		}

		laraItem->Position.xPos = kayakItem->Position.xPos;
		laraItem->Position.yPos = kayakItem->Position.yPos;
		laraItem->Position.zPos = kayakItem->Position.zPos;
		laraItem->Position.xRot = kayakItem->Position.xRot;
		laraItem->Position.yRot = kayakItem->Position.yRot;
		laraItem->Position.zRot = kayakItem->Position.zRot / 2;

		AnimateItem(laraItem);

		kayakItem->AnimNumber = Objects[ID_KAYAK].animIndex + (laraItem->AnimNumber - Objects[ID_KAYAK_LARA_ANIMS].animIndex);
		kayakItem->FrameNumber = g_Level.Anims[kayakItem->AnimNumber].frameBase + (laraItem->FrameNumber - g_Level.Anims[laraItem->AnimNumber].frameBase);

		Camera.targetElevation = -ANGLE(30.0f);
		Camera.targetDistance = CLICK(8);
	}

	if (!(Wibble & 15) && kayak->TrueWater)
	{
		KayakDoWake(kayakItem, -CLICK(0.5f), 0, 0);
		KayakDoWake(kayakItem, CLICK(0.5f), 0, 1);
	}

	if (Wibble & 7)
	{
		if (!kayak->TrueWater && kayakItem->VerticalVelocity < 20)
		{
			PHD_VECTOR dest;
			char cnt = 0;
			short MistZPos[10] = { 900, 750, 600, 450, 300, 150, 0,  -150, -300, -450 };
			short MistXPos[10] = { 32,  96,  170, 220, 300, 400, 400, 300,  200,  64 };

			cnt ^= 1;

			for (int i = cnt; i < 10; i += 2)
			{
				if (GetRandomControl() & 1)
					dest.x = (MistXPos[i] / 2);
				else
					dest.x = -(MistXPos[i] / 2);
				dest.y = 50;
				dest.z = MistZPos[i];
			}
		}
	}

	if (!kayakItem->Velocity &&
		!lara->ExtraVelocity.x &&
		!lara->ExtraVelocity.z)
	{
		if (WakeShade)
			WakeShade--;
	}
	else
	{
		if (WakeShade < 16)
			WakeShade++;
	}

	KayakUpdateWakeFX();
	KayakToItemCollision(laraItem, kayakItem);

	return (lara->Vehicle != NO_ITEM) ? true : false;
}
