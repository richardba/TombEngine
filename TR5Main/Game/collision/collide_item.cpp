#include "framework.h"
#include "Game/collision/collide_item.h"

#include "Game/control/los.h"
#include "Game/collision/collide_room.h"
#include "Game/animation.h"
#include "Game/Lara/lara.h"
#include "Game/items.h"
#include "Game/effects/effects.h"
#include "Game/collision/sphere.h"
#include "Game/room.h"
#include "Specific/setup.h"
#include "Sound/sound.h"
#include "Specific/trmath.h"
#include "Specific/prng.h"
#include "Renderer/Renderer11.h"

using namespace TEN::Math::Random;
using namespace TEN::Renderer;

BOUNDING_BOX GlobalCollisionBounds;
ITEM_INFO* CollidedItems[MAX_COLLIDED_OBJECTS];
MESH_INFO* CollidedMeshes[MAX_COLLIDED_OBJECTS];

void GenericSphereBoxCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll)
{
	ITEM_INFO* item = &g_Level.Items[itemNum];

	if (item->status != ITEM_INVISIBLE)
	{
		if (TestBoundsCollide(item, l, coll->Setup.Radius))
		{
			int collidedBits = TestCollision(item, l);
			if (collidedBits)
			{
				short oldRot = item->pos.yRot;

				item->pos.yRot = 0;
				GetSpheres(item, CreatureSpheres, SPHERES_SPACE_WORLD, Matrix::Identity);
				item->pos.yRot = oldRot;

				int deadlyBits = *((int*)&item->itemFlags[0]);
				SPHERE* sphere = &CreatureSpheres[0];

				if (item->itemFlags[2] != 0)
				{
					collidedBits &= ~1;
				}

				while (collidedBits)
				{
					if (collidedBits & 1)
					{
						GlobalCollisionBounds.X1 = sphere->x - sphere->r - item->pos.xPos;
						GlobalCollisionBounds.X2 = sphere->x + sphere->r - item->pos.xPos;
						GlobalCollisionBounds.Y1 = sphere->y - sphere->r - item->pos.yPos;
						GlobalCollisionBounds.Y2 = sphere->y + sphere->r - item->pos.yPos;
						GlobalCollisionBounds.Z1 = sphere->z - sphere->r - item->pos.zPos;
						GlobalCollisionBounds.Z2 = sphere->z + sphere->r - item->pos.zPos;

						int x = l->pos.xPos;
						int y = l->pos.yPos;
						int z = l->pos.zPos;

						if (ItemPushItem(item, l, coll, deadlyBits & 1, 3) && (deadlyBits & 1))
						{
							l->hitPoints -= item->itemFlags[3];

							int dx = x - l->pos.xPos;
							int dy = y - l->pos.yPos;
							int dz = z - l->pos.zPos;

							if (dx || dy || dz)
							{
								if (TriggerActive(item))
									TriggerLaraBlood();
							}

							if (!coll->Setup.EnableObjectPush)
							{
								l->pos.xPos += dx;
								l->pos.yPos += dy;
								l->pos.zPos += dz;
							}
						}
					}

					collidedBits >>= 1;
					deadlyBits >>= 1;
					sphere++;
				}
			}
		}
	}
}

bool GetCollidedObjects(ITEM_INFO* collidingItem, int radius, bool onlyVisible, ITEM_INFO** collidedItems, MESH_INFO** collidedMeshes, int ignoreLara)
{
	short numRooms;
	short numItems = 0, numMeshes = 0;

	// Collect all the rooms where to check
	auto roomsArray = GetRoomList(collidingItem->roomNumber);

	for (auto i : roomsArray)
	{
		auto room = &g_Level.Rooms[i];

		if (collidedMeshes)
		{
			for (int j = 0; j < room->mesh.size(); j++)
			{
				MESH_INFO* mesh = &room->mesh[j];
				STATIC_INFO* staticMesh = &StaticObjects[mesh->staticNumber];

				if (!(mesh->flags & StaticMeshFlags::SM_VISIBLE))
					continue;

				if (collidingItem->pos.yPos + radius + CLICK(0.5f) < mesh->pos.yPos + staticMesh->collisionBox.Y1)
					continue;

				if (collidingItem->pos.yPos > mesh->pos.yPos + staticMesh->collisionBox.Y2)
					continue;

				auto s = phd_sin(mesh->pos.yRot);
				auto c = phd_cos(mesh->pos.yRot);
				auto rx = (collidingItem->pos.xPos - mesh->pos.xPos) * c - s * (collidingItem->pos.zPos - mesh->pos.zPos);
				auto rz = (collidingItem->pos.zPos - mesh->pos.zPos) * c + s * (collidingItem->pos.xPos - mesh->pos.xPos);

				if (radius + rx + CLICK(0.5f) < staticMesh->collisionBox.X1 || rx - radius - CLICK(0.5f) > staticMesh->collisionBox.X2)
					continue;

				if (radius + rz + CLICK(0.5f) < staticMesh->collisionBox.Z1 || rz - radius - CLICK(0.5f) > staticMesh->collisionBox.Z2)
					continue;

				collidedMeshes[numMeshes++] = mesh;

				if (!radius)
				{
					collidedItems[0] = NULL;
					return true;
				}
			}

			collidedMeshes[numMeshes] = NULL;
		}

		if (collidedItems)
		{
			int itemNumber = room->itemNumber;
			if (itemNumber != NO_ITEM)
			{
				do
				{
					ITEM_INFO* item = &g_Level.Items[itemNumber];

					if ((item == collidingItem)
						|| (item->objectNumber == ID_LARA && ignoreLara)
						|| (item->flags & 0x8000)
						|| (item->meshBits == 0)
						|| (Objects[item->objectNumber].drawRoutine == NULL && item->objectNumber != ID_LARA)
						|| (Objects[item->objectNumber].collision == NULL && item->objectNumber != ID_LARA)
						|| (onlyVisible && item->status == ITEM_INVISIBLE)
						|| (item->objectNumber == ID_BURNING_FLOOR))
					{
						itemNumber = item->nextItem;
						continue;
					}

					/*this is awful*/
					if (item->objectNumber == ID_UPV && item->hitPoints == 1)
					{
						itemNumber = item->nextItem;
						continue;
					}
					if (item->objectNumber == ID_BIGGUN && item->hitPoints == 1)
					{
						itemNumber = item->nextItem;
						continue;
					}
					/*we need a better system*/

					int dx = collidingItem->pos.xPos - item->pos.xPos;
					int dy = collidingItem->pos.yPos - item->pos.yPos;
					int dz = collidingItem->pos.zPos - item->pos.zPos;

					ANIM_FRAME* framePtr = GetBestFrame(item);

					if (dx >= -2048 && dx <= 2048 &&
						dy >= -2048 && dy <= 2048 &&
						dz >= -2048 && dz <= 2048 &&
						collidingItem->pos.yPos + radius + 128 >= item->pos.yPos + framePtr->boundingBox.Y1 &&
						collidingItem->pos.yPos - radius - 128 <= item->pos.yPos + framePtr->boundingBox.Y2)
					{
						float s = phd_sin(item->pos.yRot);
						float c = phd_cos(item->pos.yRot);

						int rx = dx * c - s * dz;
						int rz = dz * c + s * dx;

						if (item->objectNumber == ID_TURN_SWITCH)
						{
							framePtr->boundingBox.X1 = -256;
							framePtr->boundingBox.X2 = 256;
							framePtr->boundingBox.Z1 = -256;
							framePtr->boundingBox.Z1 = 256;
						}

						if (radius + rx + 128 >= framePtr->boundingBox.X1 && rx - radius - 128 <= framePtr->boundingBox.X2)
						{
							if (radius + rz + 128 >= framePtr->boundingBox.Z1 && rz - radius - 128 <= framePtr->boundingBox.Z2)
							{
								collidedItems[numItems++] = item;

								if (!radius)
									return true;
							}
						}
					}

					itemNumber = item->nextItem;

				} while (itemNumber != NO_ITEM);
			}

			collidedItems[numItems] = NULL;
		}
	}

	return (numItems || numMeshes);
}

bool TestWithGlobalCollisionBounds(ITEM_INFO* item, ITEM_INFO* lara, COLL_INFO* coll)
{
	ANIM_FRAME* framePtr = GetBestFrame(lara);

	if (item->pos.yPos + GlobalCollisionBounds.Y2 <= lara->pos.yPos + framePtr->boundingBox.Y1)
		return false;

	if (item->pos.yPos + GlobalCollisionBounds.Y1 >= framePtr->boundingBox.Y2)
		return false;

	float s = phd_sin(item->pos.yRot);
	float c = phd_cos(item->pos.yRot);

	int dx = lara->pos.xPos - item->pos.xPos;
	int dz = lara->pos.zPos - item->pos.zPos;

	int x = c * dx - s * dz;
	int z = c * dz + s * dx;

	if (x < GlobalCollisionBounds.X1 - coll->Setup.Radius ||
		x > GlobalCollisionBounds.X2 + coll->Setup.Radius ||
		z < GlobalCollisionBounds.Z1 - coll->Setup.Radius ||
		z > GlobalCollisionBounds.Z2 + coll->Setup.Radius)
		return false;

	return true;
}

void TestForObjectOnLedge(ITEM_INFO* item, COLL_INFO* coll)
{
	auto bounds = GetBoundsAccurate(item);
	auto height = abs(bounds->Y2 + bounds->Y1);

	for (int i = 0; i < 3; i++)
	{
		auto s = (i != 1) ? phd_sin(coll->Setup.ForwardAngle + ANGLE((i * 90) - 90)) : 0;
		auto c = (i != 1) ? phd_cos(coll->Setup.ForwardAngle + ANGLE((i * 90) - 90)) : 0;

		auto x = item->pos.xPos + (s * (coll->Setup.Radius));
		auto y = item->pos.yPos - height - STEP_SIZE;
		auto z = item->pos.zPos + (c * (coll->Setup.Radius));

		auto origin = Vector3(x, y, z);
		auto mxR = Matrix::CreateFromYawPitchRoll(TO_RAD(coll->Setup.ForwardAngle), 0, 0);
		auto direction = (Matrix::CreateTranslation(Vector3::UnitZ) * mxR).Translation();

		// g_Renderer.addDebugSphere(origin, 16, Vector4::One, RENDERER_DEBUG_PAGE::DIMENSION_STATS);

		for (auto i : GetRoomList(item->roomNumber))
		{
			short itemNumber = g_Level.Rooms[i].itemNumber;
			while (itemNumber != NO_ITEM)
			{
				auto item2 = &g_Level.Items[itemNumber];
				auto obj = &Objects[item2->objectNumber];

				if (obj->isPickup || obj->collision == nullptr || !item2->collidable || item2->status == ITEM_INVISIBLE)
				{
					itemNumber = item2->nextItem;
					continue;
				}

				if (phd_Distance(&item->pos, &item2->pos) < COLLISION_CHECK_DISTANCE)
				{
					auto box = TO_DX_BBOX(item2->pos, GetBoundsAccurate(item2));
					float dist;

					if (box.Intersects(origin, direction, dist) && dist < coll->Setup.Radius * 2)
					{
						coll->HitStatic = true;
						return;
					}
				}

				itemNumber = item2->nextItem;
			}

			for (int j = 0; j < g_Level.Rooms[i].mesh.size(); j++)
			{
				auto mesh = &g_Level.Rooms[i].mesh[j];

				if (!(mesh->flags & StaticMeshFlags::SM_VISIBLE))
					continue;

				if (phd_Distance(&item->pos, &mesh->pos) < COLLISION_CHECK_DISTANCE)
				{
					auto box = TO_DX_BBOX(mesh->pos, &StaticObjects[mesh->staticNumber].collisionBox);
					float dist;

					if (box.Intersects(origin, direction, dist) && dist < coll->Setup.Radius * 2)
					{
						coll->HitStatic = true;
						return;
					}
				}
			}
		}
	}
}

bool TestLaraPosition(OBJECT_COLLISION_BOUNDS* bounds, ITEM_INFO* item, ITEM_INFO* l)
{
	short xRotRel = l->pos.xRot - item->pos.xRot;
	short yRotRel = l->pos.yRot - item->pos.yRot;
	short zRotRel = l->pos.zRot - item->pos.zRot;

	if (xRotRel < bounds->rotX1)
		return false;
	if (xRotRel > bounds->rotX2)
		return false;
	if (yRotRel < bounds->rotY1)
		return false;
	if (yRotRel > bounds->rotY2)
		return false;
	if (zRotRel < bounds->rotZ1)
		return false;
	if (zRotRel > bounds->rotZ2)
		return false;

	int x = l->pos.xPos - item->pos.xPos;
	int y = l->pos.yPos - item->pos.yPos;
	int z = l->pos.zPos - item->pos.zPos;

	Vector3 pos = Vector3(x, y, z);

	Matrix matrix = Matrix::CreateFromYawPitchRoll(
		TO_RAD(item->pos.yRot),
		TO_RAD(item->pos.xRot),
		TO_RAD(item->pos.zRot)
	);

	// This solves once for all the minus sign hack of CreateFromYawPitchRoll.
	// In reality it should be the inverse, but the inverse of a rotation matrix is equal to the transpose 
	// and transposing a matrix is faster.
	// It's the only piece of code that does it, because we want Lara's location relative to the identity frame 
	// of the object we are test against.
	matrix = matrix.Transpose();

	pos = Vector3::Transform(pos, matrix);

	if (pos.x < bounds->boundingBox.X1 || pos.x > bounds->boundingBox.X2 ||
		pos.y < bounds->boundingBox.Y1 || pos.y > bounds->boundingBox.Y2 ||
		pos.z < bounds->boundingBox.Z1 || pos.z > bounds->boundingBox.Z2)
		return false;

	return true;
}

void AlignLaraPosition(PHD_VECTOR* vec, ITEM_INFO* item, ITEM_INFO* l)
{
	l->pos.xRot = item->pos.xRot;
	l->pos.yRot = item->pos.yRot;
	l->pos.zRot = item->pos.zRot;

	Matrix matrix = Matrix::CreateFromYawPitchRoll(
		TO_RAD(item->pos.yRot),
		TO_RAD(item->pos.xRot),
		TO_RAD(item->pos.zRot)
	);

	Vector3 pos = Vector3::Transform(Vector3(vec->x, vec->y, vec->z), matrix);

	l->pos.xPos = item->pos.xPos + pos.x;
	l->pos.yPos = item->pos.yPos + pos.y;
	l->pos.zPos = item->pos.zPos + pos.z;
}

bool MoveLaraPosition(PHD_VECTOR* vec, ITEM_INFO* item, ITEM_INFO* l)
{
	FLOOR_INFO* floor;
	PHD_3DPOS dest;
	int height;
	short roomNumber;

	dest.xRot = item->pos.xRot;
	dest.yRot = item->pos.yRot;
	dest.zRot = item->pos.zRot;

	Vector3 pos = Vector3(vec->x, vec->y, vec->z);

	Matrix matrix = Matrix::CreateFromYawPitchRoll(
		TO_RAD(item->pos.yRot),
		TO_RAD(item->pos.xRot),
		TO_RAD(item->pos.zRot)
	);

	pos = Vector3::Transform(pos, matrix);

	dest.xPos = item->pos.xPos + pos.x;
	dest.yPos = item->pos.yPos + pos.y;
	dest.zPos = item->pos.zPos + pos.z;

	if (item->objectNumber != ID_FLARE_ITEM && item->objectNumber != ID_BURNING_TORCH_ITEM)
		return Move3DPosTo3DPos(&l->pos, &dest, LARA_VELOCITY, ANGLE(2));

	roomNumber = l->roomNumber;
	floor = GetFloor(dest.xPos, dest.yPos, dest.zPos, &roomNumber);
	height = GetFloorHeight(floor, dest.xPos, dest.yPos, dest.zPos);

	if (abs(height - l->pos.yPos) <= CLICK(2))
	{
		if (sqrt(SQUARE(dest.xPos - l->pos.xPos) + SQUARE(dest.yPos - l->pos.yPos) + SQUARE(dest.zPos - l->pos.zPos)) < (STEP_SIZE / 2))
			return true;

		return Move3DPosTo3DPos(&l->pos, &dest, LARA_VELOCITY, ANGLE(2));
	}

	if (Lara.isMoving)
	{
		Lara.isMoving = false;
		Lara.gunStatus = LG_HANDS_FREE;
	}

	return false;
}

static bool ItemCollide(int value, int radius)
{
	return value >= -radius && value <= radius;
}

static bool ItemInRange(int x, int z, int radius)
{
	return (SQUARE(x) + SQUARE(z)) <= SQUARE(radius);
}

bool ItemNearLara(PHD_3DPOS* pos, int radius)
{
	BOUNDING_BOX* bounds;
	GAME_VECTOR target;
	target.x = pos->xPos - LaraItem->pos.xPos;
	target.y = pos->yPos - LaraItem->pos.yPos;
	target.z = pos->zPos - LaraItem->pos.zPos;
	if (!ItemCollide(target.y, ITEM_RADIUS_YMAX))
		return false;
	if (!ItemCollide(target.x, radius) || !ItemCollide(target.z, radius))
		return false;
	if (!ItemInRange(target.x, target.z, radius))
		return false;

	bounds = GetBoundsAccurate(LaraItem);
	if (target.y >= bounds->Y1 && target.y <= (bounds->Y2 + LARA_RAD))
		return true;

	return false;
}

bool ItemNearTarget(PHD_3DPOS* src, ITEM_INFO* target, int radius)
{
	BOUNDING_BOX* bounds;
	PHD_VECTOR pos;
	pos.x = src->xPos - target->pos.xPos;
	pos.y = src->yPos - target->pos.yPos;
	pos.z = src->zPos - target->pos.zPos;
	if (!ItemCollide(pos.y, ITEM_RADIUS_YMAX))
		return false;
	if (!ItemCollide(pos.x, radius) || !ItemCollide(pos.z, radius))
		return false;
	if (!ItemInRange(pos.x, pos.z, radius))
		return false;

	bounds = GetBoundsAccurate(target);
	if (pos.y >= bounds->Y1 && pos.y <= bounds->Y2)
		return true;

	return false;
}

bool Move3DPosTo3DPos(PHD_3DPOS* src, PHD_3DPOS* dest, int velocity, short angAdd)
{
	int x = dest->xPos - src->xPos;
	int y = dest->yPos - src->yPos;
	int z = dest->zPos - src->zPos;
	int distance = sqrt(SQUARE(x) + SQUARE(y) + SQUARE(z));

	if (velocity < distance)
	{
		src->xPos += x * velocity / distance;
		src->yPos += y * velocity / distance;
		src->zPos += z * velocity / distance;
	}
	else
	{
		src->xPos = dest->xPos;
		src->yPos = dest->yPos;
		src->zPos = dest->zPos;
	}

	if (!Lara.isMoving)
	{
		if (Lara.waterStatus != LW_UNDERWATER)
		{
			int angle = mGetAngle(dest->xPos, dest->zPos, src->xPos, src->zPos);
			int direction = (GetQuadrant(angle) - GetQuadrant(dest->yRot)) & 3;

			switch (direction)
			{
			case 0:
				SetAnimation(LaraItem, LA_SIDESTEP_LEFT);
				Lara.gunStatus = LG_HANDS_BUSY;
				break;

			case 1:
				SetAnimation(LaraItem, LA_WALK);
				Lara.gunStatus = LG_HANDS_BUSY;
				break;

			case 2:
				SetAnimation(LaraItem, LA_SIDESTEP_RIGHT);
				Lara.gunStatus = LG_HANDS_BUSY;
				break;

			case 3:
			default:
				SetAnimation(LaraItem, LA_WALK_BACK);
				Lara.gunStatus = LG_HANDS_BUSY;
				break;
			}
		}

		Lara.isMoving = true;
		Lara.moveCount = 0;
	}

	short deltaAngle = dest->xRot - src->xRot;
	if (deltaAngle > angAdd)
		src->xRot += angAdd;
	else if (deltaAngle < -angAdd)
		src->xRot -= angAdd;
	else
		src->xRot = dest->xRot;

	deltaAngle = dest->yRot - src->yRot;
	if (deltaAngle > angAdd)
		src->yRot += angAdd;
	else if (deltaAngle < -angAdd)
		src->yRot -= angAdd;
	else
		src->yRot = dest->yRot;

	deltaAngle = dest->zRot - src->zRot;
	if (deltaAngle > angAdd)
		src->zRot += angAdd;
	else if (deltaAngle < -angAdd)
		src->zRot -= angAdd;
	else
		src->zRot = dest->zRot;

	return (src->xPos == dest->xPos
		&& src->yPos == dest->yPos
		&& src->zPos == dest->zPos
		&& src->xRot == dest->xRot
		&& src->yRot == dest->yRot
		&& src->zRot == dest->zRot);
}

bool TestBoundsCollide(ITEM_INFO* item, ITEM_INFO* l, int radius)
{
	auto bounds = (BOUNDING_BOX*)GetBestFrame(item);
	auto laraBounds = (BOUNDING_BOX*)GetBestFrame(l);

	if (item->pos.yPos + bounds->Y2 <= l->pos.yPos + laraBounds->Y1)
		return false;

	if (item->pos.yPos + bounds->Y1 >= l->pos.yPos + laraBounds->Y2)
		return false;

	auto c = phd_cos(item->pos.yRot);
	auto s = phd_sin(item->pos.yRot);
	int x = l->pos.xPos - item->pos.xPos;
	int z = l->pos.zPos - item->pos.zPos;
	int dx = c * x - s * z;
	int dz = c * z + s * x;

	if (dx >= bounds->X1 - radius &&
		dx <= radius + bounds->X2 &&
		dz >= bounds->Z1 - radius &&
		dz <= radius + bounds->Z2)
	{
		return true;
	}

	return false;
}

bool TestBoundsCollideStatic(ITEM_INFO* item, MESH_INFO* mesh, int radius)
{
	auto bounds = StaticObjects[mesh->staticNumber].collisionBox;

	if (!(bounds.Z2 != 0 || bounds.Z1 != 0 || bounds.X1 != 0 || bounds.X2 != 0 || bounds.Y1 != 0 || bounds.Y2 != 0))
		return false;

	ANIM_FRAME* frame = GetBestFrame(item);
	if (mesh->pos.yPos + bounds.Y2 <= item->pos.yPos + frame->boundingBox.Y1)
		return false;

	if (mesh->pos.yPos + bounds.Y1 >= item->pos.yPos + frame->boundingBox.Y2)
		return false;

	float c, s;
	int x, z, dx, dz;

	c = phd_cos(mesh->pos.yRot);
	s = phd_sin(mesh->pos.yRot);
	x = item->pos.xPos - mesh->pos.xPos;
	z = item->pos.zPos - mesh->pos.zPos;
	dx = c * x - s * z;
	dz = c * z + s * x;

	if (dx <= radius + bounds.X2
		&& dx >= bounds.X1 - radius
		&& dz <= radius + bounds.Z2
		&& dz >= bounds.Z1 - radius)
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool ItemPushItem(ITEM_INFO* item, ITEM_INFO* item2, COLL_INFO* coll, bool spazon, char bigpush) // previously ItemPushLara
{
	// Get item's rotation
	auto c = phd_cos(item->pos.yRot);
	auto s = phd_sin(item->pos.yRot);

	// Get vector from item to Lara
	int dx = item2->pos.xPos - item->pos.xPos;
	int dz = item2->pos.zPos - item->pos.zPos;

	// Rotate Lara vector into item frame
	int rx = c * dx - s * dz;
	int rz = c * dz + s * dx;

	BOUNDING_BOX* bounds;
	if (bigpush & 2)
		bounds = &GlobalCollisionBounds;
	else
		bounds = (BOUNDING_BOX*)GetBestFrame(item);

	int minX = bounds->X1;
	int maxX = bounds->X2;
	int minZ = bounds->Z1;
	int maxZ = bounds->Z2;

	if (bigpush & 1)
	{
		minX -= coll->Setup.Radius;
		maxX += coll->Setup.Radius;
		minZ -= coll->Setup.Radius;
		maxZ += coll->Setup.Radius;
	}

	// Big enemies
	if (abs(dx) > 4608
		|| abs(dz) > 4608
		|| rx <= minX
		|| rx >= maxX
		|| rz <= minZ
		|| rz >= maxZ)
		return false;

	int left = rx - minX;
	int top = maxZ - rz;
	int bottom = rz - minZ;
	int right = maxX - rx;

	if (right <= left && right <= top && right <= bottom)
		rx += right;
	else if (left <= right && left <= top && left <= bottom)
		rx -= left;
	else if (top <= left && top <= right && top <= bottom)
		rz += top;
	else
		rz -= bottom;

	item2->pos.xPos = item->pos.xPos + c * rx + s * rz;
	item2->pos.zPos = item->pos.zPos + c * rz - s * rx;

	auto lara = item2->data.is<LaraInfo*>() ? (LaraInfo*&)item2->data : nullptr;

	if (lara != nullptr && spazon && bounds->Y2 - bounds->Y1 > CLICK(1))
	{
		rx = (bounds->X1 + bounds->X2) / 2;
		rz = (bounds->Z1 + bounds->Z2) / 2;

		dx -= c * rx + s * rz;
		dz -= c * rz - s * rx;

		lara->hitDirection = (item2->pos.yRot - phd_atan(dz, dz) - ANGLE(135)) / 16384;

		if ((!lara->hitFrame) && (!lara->spazEffectCount))
		{
			SoundEffect(SFX_TR4_LARA_INJURY, &item2->pos, 0);
			lara->spazEffectCount = GenerateInt(15, 35);
		}

		if (lara->spazEffectCount)
			lara->spazEffectCount--;

		lara->hitFrame++;
		if (lara->hitFrame > 34)
			lara->hitFrame = 34;
	}

	coll->Setup.BadHeightDown = NO_BAD_POS;
	coll->Setup.BadHeightUp = -STEPUP_HEIGHT;
	coll->Setup.BadCeilingHeight = 0;

	auto facing = coll->Setup.ForwardAngle;
	coll->Setup.ForwardAngle = phd_atan(item2->pos.zPos - coll->Setup.OldPosition.z, item2->pos.xPos - coll->Setup.OldPosition.x);

	GetCollisionInfo(coll, item2);

	coll->Setup.ForwardAngle = facing;

	if (coll->CollisionType == CT_NONE)
	{
		coll->Setup.OldPosition.x = item2->pos.xPos;
		coll->Setup.OldPosition.y = item2->pos.yPos;
		coll->Setup.OldPosition.z = item2->pos.zPos;

		// Commented because causes Lara to jump out of the water if she touches an object on the surface. re: "kayak bug"
		// UpdateItemRoom(item2, -10);
	}
	else
	{
		item2->pos.xPos = coll->Setup.OldPosition.x;
		item2->pos.zPos = coll->Setup.OldPosition.z;
	}

	if (lara != nullptr && lara->moveCount > 15)
	{
		Lara.isMoving = false;
		Lara.gunStatus = LG_HANDS_FREE;
	}

	return true;
}

bool ItemPushStatic(ITEM_INFO* item, MESH_INFO* mesh, COLL_INFO* coll) // previously ItemPushLaraStatic
{
	auto bounds = StaticObjects[mesh->staticNumber].collisionBox;

	auto c = phd_cos(mesh->pos.yRot);
	auto s = phd_sin(mesh->pos.yRot);

	auto dx = item->pos.xPos - mesh->pos.xPos;
	auto dz = item->pos.zPos - mesh->pos.zPos;
	auto rx = c * dx - s * dz;
	auto rz = c * dz + s * dx;
	auto minX = bounds.X1 - coll->Setup.Radius;
	auto maxX = bounds.X2 + coll->Setup.Radius;
	auto minZ = bounds.Z1 - coll->Setup.Radius;
	auto maxZ = bounds.Z2 + coll->Setup.Radius;

	if (abs(dx) > 4608
		|| abs(dz) > 4608
		|| rx <= minX
		|| rx >= maxX
		|| rz <= minZ
		|| rz >= maxZ)
		return false;

	auto left = rx - minX;
	auto top = maxZ - rz;
	auto bottom = rz - minZ;
	auto right = maxX - rx;

	if (right <= left && right <= top && right <= bottom)
		rx += right;
	else if (left <= right && left <= top && left <= bottom)
		rx -= left;
	else if (top <= left && top <= right && top <= bottom)
		rz += top;
	else
		rz -= bottom;

	item->pos.xPos = mesh->pos.xPos + c * rx + s * rz;
	item->pos.zPos = mesh->pos.zPos + c * rz - s * rx;

	coll->Setup.BadHeightDown = NO_BAD_POS;
	coll->Setup.BadHeightUp = -STEPUP_HEIGHT;
	coll->Setup.BadCeilingHeight = 0;

	auto oldFacing = coll->Setup.ForwardAngle;
	coll->Setup.ForwardAngle = phd_atan(item->pos.zPos - coll->Setup.OldPosition.z, item->pos.xPos - coll->Setup.OldPosition.x);

	GetCollisionInfo(coll, item);

	coll->Setup.ForwardAngle = oldFacing;

	if (coll->CollisionType == CT_NONE)
	{
		coll->Setup.OldPosition.x = item->pos.xPos;
		coll->Setup.OldPosition.y = item->pos.yPos;
		coll->Setup.OldPosition.z = item->pos.zPos;

		UpdateItemRoom(item, -10);
	}
	else
	{
		item->pos.xPos = coll->Setup.OldPosition.x;
		item->pos.zPos = coll->Setup.OldPosition.z;
	}

	if (item == LaraItem && Lara.isMoving && Lara.moveCount > 15)
	{
		Lara.isMoving = false;
		Lara.gunStatus = LG_HANDS_FREE;
	}

	return true;
}

void CollideSolidStatics(ITEM_INFO* item, COLL_INFO* coll)
{
	coll->HitTallObject = false;

	for (auto i : GetRoomList(item->roomNumber))
	{
		for (int j = 0; j < g_Level.Rooms[i].mesh.size(); j++)
		{
			auto mesh = &g_Level.Rooms[i].mesh[j];

			// Only process meshes which are visible and solid
			if ((mesh->flags & StaticMeshFlags::SM_VISIBLE) && (mesh->flags & StaticMeshFlags::SM_SOLID))
			{
				if (phd_Distance(&item->pos, &mesh->pos) < COLLISION_CHECK_DISTANCE)
				{
					auto stInfo = StaticObjects[mesh->staticNumber];
					if (CollideSolidBounds(item, stInfo.collisionBox, mesh->pos, coll))
						coll->HitStatic = true;
				}
			}
		}
	}
}

bool CollideSolidBounds(ITEM_INFO* item, BOUNDING_BOX box, PHD_3DPOS pos, COLL_INFO* coll)
{
	bool result = false;

	// Get DX static bounds in global coords
	auto staticBounds = TO_DX_BBOX(pos, &box);

	// Get local TR bounds and DX item bounds in global coords
	auto itemBBox = GetBoundsAccurate(item);
	auto itemBounds = TO_DX_BBOX(item->pos, itemBBox);

	// Extend bounds a bit for visual testing
	itemBounds.Extents = itemBounds.Extents + Vector3(WALL_SIZE);

	// Filter out any further checks if static isn't nearby
	if (!staticBounds.Intersects(itemBounds))
		return false;

	// Bring back extents
	itemBounds.Extents = itemBounds.Extents - Vector3(WALL_SIZE);

	// Draw static bounds
	g_Renderer.addDebugBox(staticBounds, Vector4(1, 0.3, 0, 1), RENDERER_DEBUG_PAGE::LOGIC_STATS);

	// Calculate horizontal item coll bounds according to radius
	BOUNDING_BOX collBox;
	collBox.X1 = -coll->Setup.Radius;
	collBox.X2 = coll->Setup.Radius;
	collBox.Z1 = -coll->Setup.Radius;
	collBox.Z2 = coll->Setup.Radius;

	// Calculate vertical item coll bounds according to either height (land mode) or precise bounds (water mode).
	// Water mode needs special processing because height calc in original engines is inconsistent in such cases.
	if (g_Level.Rooms[item->roomNumber].flags & ENV_FLAG_WATER)
	{
		collBox.Y1 = itemBBox->Y1;
		collBox.Y2 = itemBBox->Y2;
	}
	else
	{
		collBox.Y1 = -coll->Setup.Height;
		collBox.Y2 = 0;
	}

	// Get and test DX item coll bounds
	auto collBounds = TO_DX_BBOX(PHD_3DPOS(item->pos.xPos, item->pos.yPos, item->pos.zPos), &collBox);
	bool intersects = staticBounds.Intersects(collBounds);

	// Draw item coll bounds
	g_Renderer.addDebugBox(collBounds, intersects ? Vector4(1, 0, 0, 1) : Vector4(0, 1, 0, 1), RENDERER_DEBUG_PAGE::LOGIC_STATS);

	// Decompose static bounds into top/bottom plane vertices
	Vector3 corners[8];
	staticBounds.GetCorners(corners);
	Vector3 planeVertices[4][3] =
	{
		{ corners[0], corners[4], corners[1] },
		{ corners[5], corners[4], corners[1] },
		{ corners[3], corners[6], corners[7] },
		{ corners[3], corners[6], corners[2] }
	};

	// Determine collision box vertical dimensions
	auto height = collBox.Y2 - collBox.Y1;
	auto center = item->pos.yPos - height / 2;

	// Do a series of angular tests with 90 degree steps to determine top/bottom collision.

	int closestPlane = -1;
	Ray closestRay;
	auto minDistance = std::numeric_limits<float>::max();

	for (int i = 0; i < 4; i++)
	{
		// Calculate ray direction
		auto mxR = Matrix::CreateFromYawPitchRoll(TO_RAD(item->pos.yRot), TO_RAD(item->pos.xRot + (ANGLE(90 * i))), 0);
		auto mxT = Matrix::CreateTranslation(Vector3::UnitY);
		auto direction = (mxT * mxR).Translation();

		// Make a ray and do ray tests against all decomposed planes
		auto ray = Ray(collBounds.Center, direction);

		// Determine if top/bottom planes are closest ones or not
		for (int p = 0; p < 4; p++)
		{
			// No plane intersection, quickly discard
			float d = 0.0f;
			if (!ray.Intersects(planeVertices[p][0], planeVertices[p][1], planeVertices[p][2], d))
				continue;

			// Process plane intersection only if distance is smaller
			// than already found minimum
			if (d < minDistance)
			{
				closestRay = ray;
				closestPlane = p;
				minDistance = d;
			}
		}
	}

	if (closestPlane != -1) // Top/bottom plane found
	{
		auto bottom = closestPlane >= 2;
		auto yPoint = abs((closestRay.direction * minDistance).y);
		auto distanceToVerticalPlane = height / 2 - yPoint;

		// Correct position according to top/bottom bounds, if collided and plane is nearby
		if (intersects && minDistance < height)
		{
			if (bottom)
			{
				// HACK: additionally subtract 2 from bottom plane, or else false positives may occur.
				item->pos.yPos += distanceToVerticalPlane + 2;
				coll->CollisionType = CT_TOP;
			}
			else
			{
				// Set collision type only if dry room (in water rooms it causes stucking)
				item->pos.yPos -= distanceToVerticalPlane;
				coll->CollisionType = (g_Level.Rooms[item->roomNumber].flags & 1) ? coll->CollisionType : CT_CLAMP;
			}

			result = true;
		}

		if (bottom && coll->Middle.Ceiling < distanceToVerticalPlane)
			coll->Middle.Ceiling = distanceToVerticalPlane;
	}

	// If no actual intersection occured, stop testing.
	if (!intersects)
		return false;

	// Check if bounds still collide after top/bottom position correction
	if (!staticBounds.Intersects(TO_DX_BBOX(PHD_3DPOS(item->pos.xPos, item->pos.yPos, item->pos.zPos), &collBox)))
		return result;

	// Determine identity rotation/distance
	auto distance = Vector3(item->pos.xPos, item->pos.yPos, item->pos.zPos) - Vector3(pos.xPos, pos.yPos, pos.zPos);
	auto c = phd_cos(pos.yRot);
	auto s = phd_sin(pos.yRot);

	// Rotate item to collision bounds identity
	auto x = round(distance.x * c - distance.z * s) + pos.xPos;
	auto y = item->pos.yPos;
	auto z = round(distance.x * s + distance.z * c) + pos.zPos;

	// Determine identity static collision bounds
	auto XMin = pos.xPos + box.X1;
	auto XMax = pos.xPos + box.X2;
	auto YMin = pos.yPos + box.Y1;
	auto YMax = pos.yPos + box.Y2;
	auto ZMin = pos.zPos + box.Z1;
	auto ZMax = pos.zPos + box.Z2;

	// Determine item collision bounds
	auto inXMin = x + collBox.X1;
	auto inXMax = x + collBox.X2;
	auto inYMin = y + collBox.Y1;
	auto inYMax = y + collBox.Y2;
	auto inZMin = z + collBox.Z1;
	auto inZMax = z + collBox.Z2;

	// Don't calculate shifts if not in bounds
	if (inXMax <= XMin || inXMin >= XMax ||
		inYMax <= YMin || inYMin >= YMax ||
		inZMax <= ZMin || inZMin >= ZMax)
		return result;

	// Calculate shifts

	PHD_VECTOR rawShift = {};

	auto shiftLeft = inXMax - XMin;
	auto shiftRight = XMax - inXMin;

	if (shiftLeft < shiftRight)
		rawShift.x = -shiftLeft;
	else
		rawShift.x = shiftRight;

	shiftLeft = inZMax - ZMin;
	shiftRight = ZMax - inZMin;

	if (shiftLeft < shiftRight)
		rawShift.z = -shiftLeft;
	else
		rawShift.z = shiftRight;

	// Rotate previous collision position to identity
	distance = Vector3(coll->Setup.OldPosition.x, coll->Setup.OldPosition.y, coll->Setup.OldPosition.z) - Vector3(pos.xPos, pos.yPos, pos.zPos);
	auto ox = round(distance.x * c - distance.z * s) + pos.xPos;
	auto oz = round(distance.x * s + distance.z * c) + pos.zPos;

	// Calculate collisison type based on identity rotation
	switch (GetQuadrant(coll->Setup.ForwardAngle - pos.yRot))
	{
	case NORTH:
		if (rawShift.x > coll->Setup.Radius || rawShift.x < -coll->Setup.Radius)
		{
			coll->Shift.z = rawShift.z;
			coll->Shift.x = ox - x;
			coll->CollisionType = CT_FRONT;
		}
		else if (rawShift.x > 0 && rawShift.x <= coll->Setup.Radius)
		{
			coll->Shift.x = rawShift.x;
			coll->Shift.z = 0;
			coll->CollisionType = CT_LEFT;
		}
		else if (rawShift.x < 0 && rawShift.x >= -coll->Setup.Radius)
		{
			coll->Shift.x = rawShift.x;
			coll->Shift.z = 0;
			coll->CollisionType = CT_RIGHT;
		}
		break;

	case SOUTH:
		if (rawShift.x > coll->Setup.Radius || rawShift.x < -coll->Setup.Radius)
		{
			coll->Shift.z = rawShift.z;
			coll->Shift.x = ox - x;
			coll->CollisionType = CT_FRONT;
		}
		else if (rawShift.x > 0 && rawShift.x <= coll->Setup.Radius)
		{
			coll->Shift.x = rawShift.x;
			coll->Shift.z = 0;
			coll->CollisionType = CT_RIGHT;
		}
		else if (rawShift.x < 0 && rawShift.x >= -coll->Setup.Radius)
		{
			coll->Shift.x = rawShift.x;
			coll->Shift.z = 0;
			coll->CollisionType = CT_LEFT;
		}
		break;

	case EAST:
		if (rawShift.z > coll->Setup.Radius || rawShift.z < -coll->Setup.Radius)
		{
			coll->Shift.x = rawShift.x;
			coll->Shift.z = oz - z;
			coll->CollisionType = CT_FRONT;
		}
		else if (rawShift.z > 0 && rawShift.z <= coll->Setup.Radius)
		{
			coll->Shift.z = rawShift.z;
			coll->Shift.x = 0;
			coll->CollisionType = CT_RIGHT;
		}
		else if (rawShift.z < 0 && rawShift.z >= -coll->Setup.Radius)
		{
			coll->Shift.z = rawShift.z;
			coll->Shift.x = 0;
			coll->CollisionType = CT_LEFT;
		}
		break;

	case WEST:
		if (rawShift.z > coll->Setup.Radius || rawShift.z < -coll->Setup.Radius)
		{
			coll->Shift.x = rawShift.x;
			coll->Shift.z = oz - z;
			coll->CollisionType = CT_FRONT;
		}
		else if (rawShift.z > 0 && rawShift.z <= coll->Setup.Radius)
		{
			coll->Shift.z = rawShift.z;
			coll->Shift.x = 0;
			coll->CollisionType = CT_LEFT;
		}
		else if (rawShift.z < 0 && rawShift.z >= -coll->Setup.Radius)
		{
			coll->Shift.z = rawShift.z;
			coll->Shift.x = 0;
			coll->CollisionType = CT_RIGHT;
		}
		break;
	}

	// Determine final shifts rotation/distance
	distance = Vector3(x + coll->Shift.x, y, z + coll->Shift.z) - Vector3(pos.xPos, pos.yPos, pos.zPos);
	c = phd_cos(-pos.yRot);
	s = phd_sin(-pos.yRot);

	// Calculate final shifts rotation/distance
	coll->Shift.x = (round(distance.x * c - distance.z * s) + pos.xPos) - item->pos.xPos;
	coll->Shift.z = (round(distance.x * s + distance.z * c) + pos.zPos) - item->pos.zPos;

	if (coll->Shift.x == 0 && coll->Shift.z == 0)
		coll->CollisionType = CT_NONE; // Paranoid

	// Set splat state flag if item is Lara and bounds are taller than Lara's headroom
	if (item == LaraItem && coll->CollisionType == CT_FRONT)
		coll->HitTallObject = (YMin <= inYMin + LARA_HEADROOM);

	return true;
}

void DoProjectileDynamics(short itemNumber, int x, int y, int z, int xv, int yv, int zv) // previously DoProperDetection
{
	int bs, yang;

	ITEM_INFO* item = &g_Level.Items[itemNumber];

	auto oldCollResult = GetCollisionResult(x, y, z, item->roomNumber);
	auto collResult = GetCollisionResult(item);

	if (item->pos.yPos >= collResult.Position.Floor)
	{
		bs = 0;

		if (collResult.Position.Slope && oldCollResult.Position.Floor < collResult.Position.Floor)
		{
			yang = (long)((unsigned short)item->pos.yRot);
			if (collResult.TiltX < 0)
			{
				if (yang >= 0x8000)
					bs = 1;
			}
			else if (collResult.TiltX > 0)
			{
				if (yang <= 0x8000)
					bs = 1;
			}

			if (collResult.TiltZ < 0)
			{
				if (yang >= 0x4000 && yang <= 0xc000)
					bs = 1;
			}
			else if (collResult.TiltZ > 0)
			{
				if (yang <= 0x4000 || yang >= 0xc000)
					bs = 1;
			}
		}

		/* If last position of item was also below this floor height, we've hit a wall, else we've hit a floor */

		if (y > (collResult.Position.Floor + 32) && bs == 0 &&
			(((x / SECTOR(1)) != (item->pos.xPos / SECTOR(1))) ||
			((z / SECTOR(1)) != (item->pos.zPos / SECTOR(1)))))
		{
			// Need to know which direction the wall is.

			long	xs;

			if ((x & (~(WALL_SIZE - 1))) != (item->pos.xPos & (~(WALL_SIZE - 1))) &&	// X crossed boundary?
				(z & (~(WALL_SIZE - 1))) != (item->pos.zPos & (~(WALL_SIZE - 1))))	// Z crossed boundary as well?
			{
				if (abs(x - item->pos.xPos) < abs(z - item->pos.zPos))
					xs = 1;	// X has travelled the shortest, so (maybe) hit first. (Seems to work ok).
				else
					xs = 0;
			}
			else
				xs = 1;

			if ((x & (~(WALL_SIZE - 1))) != (item->pos.xPos & (~(WALL_SIZE - 1))) && xs)	// X crossed boundary?
			{
				if (xv <= 0)	// Hit angle = 0xc000.
					item->pos.yRot = 0x4000 + (0xc000 - item->pos.yRot);
				else			// Hit angle = 0x4000.
					item->pos.yRot = 0xc000 + (0x4000 - item->pos.yRot);
			}
			else		// Z crossed boundary.
				item->pos.yRot = 0x8000 - item->pos.yRot;

			item->speed /= 2;

			/* Put item back in its last position */
			item->pos.xPos = x;
			item->pos.yPos = y;
			item->pos.zPos = z;
		}
		else if (collResult.Position.Slope) 	// Hit a steep slope?
		{
			// Need to know which direction the slope is.

			item->speed -= (item->speed / 4);

			if (collResult.TiltX < 0 && ((abs(collResult.TiltX)) - (abs(collResult.TiltZ)) >= 2))	// Hit angle = 0x4000
			{
				if (((unsigned short)item->pos.yRot) > 0x8000)
				{
					item->pos.yRot = 0x4000 + (0xc000 - (unsigned short)item->pos.yRot - 1);
					if (item->fallspeed > 0)
						item->fallspeed = -(item->fallspeed / 2);
				}
				else
				{
					if (item->speed < 32)
					{
						item->speed -= collResult.TiltX * 2;
						if ((unsigned short)item->pos.yRot > 0x4000 && (unsigned short)item->pos.yRot < 0xc000)
						{
							item->pos.yRot -= 4096;
							if ((unsigned short)item->pos.yRot < 0x4000)
								item->pos.yRot = 0x4000;
						}
						else if ((unsigned short)item->pos.yRot < 0x4000)
						{
							item->pos.yRot += 4096;
							if ((unsigned short)item->pos.yRot > 0x4000)
								item->pos.yRot = 0x4000;
						}
					}

					if (item->fallspeed > 0)
						item->fallspeed = -(item->fallspeed / 2);
					else
						item->fallspeed = 0;
				}
			}
			else if (collResult.TiltX > 0 && ((abs(collResult.TiltX)) - (abs(collResult.TiltZ)) >= 2))	// Hit angle = 0xc000
			{
				if (((unsigned short)item->pos.yRot) < 0x8000)
				{
					item->pos.yRot = 0xc000 + (0x4000 - (unsigned short)item->pos.yRot - 1);
					if (item->fallspeed > 0)
						item->fallspeed = -(item->fallspeed / 2);
				}
				else
				{
					if (item->speed < 32)
					{
						item->speed += collResult.TiltX * 2;
						if ((unsigned short)item->pos.yRot > 0xc000 || (unsigned short)item->pos.yRot < 0x4000)
						{
							item->pos.yRot -= 4096;
							if ((unsigned short)item->pos.yRot < 0xc000)
								item->pos.yRot = 0xc000;
						}
						else if ((unsigned short)item->pos.yRot < 0xc000)
						{
							item->pos.yRot += 4096;
							if ((unsigned short)item->pos.yRot > 0xc000)
								item->pos.yRot = 0xc000;
						}
					}

					if (item->fallspeed > 0)
						item->fallspeed = -(item->fallspeed / 2);
					else
						item->fallspeed = 0;
				}
			}
			else if (collResult.TiltZ < 0 && ((abs(collResult.TiltZ)) - (abs(collResult.TiltX)) >= 2))	// Hit angle = 0
			{
				if (((unsigned short)item->pos.yRot) > 0x4000 && ((unsigned short)item->pos.yRot) < 0xc000)
				{
					item->pos.yRot = (0x8000 - item->pos.yRot - 1);
					if (item->fallspeed > 0)
						item->fallspeed = -(item->fallspeed / 2);
				}
				else
				{
					if (item->speed < 32)
					{
						item->speed -= collResult.TiltZ * 2;

						if ((unsigned short)item->pos.yRot < 0x8000)
						{
							item->pos.yRot -= 4096;
							if ((unsigned short)item->pos.yRot > 0xf000)
								item->pos.yRot = 0;
						}
						else if ((unsigned short)item->pos.yRot >= 0x8000)
						{
							item->pos.yRot += 4096;
							if ((unsigned short)item->pos.yRot < 0x1000)
								item->pos.yRot = 0;
						}
					}

					if (item->fallspeed > 0)
						item->fallspeed = -(item->fallspeed / 2);
					else
						item->fallspeed = 0;
				}
			}
			else if (collResult.TiltZ > 0 && ((abs(collResult.TiltZ)) - (abs(collResult.TiltX)) >= 2))	// Hit angle = 0x8000
			{
				if (((unsigned short)item->pos.yRot) > 0xc000 || ((unsigned short)item->pos.yRot) < 0x4000)
				{
					item->pos.yRot = (0x8000 - item->pos.yRot - 1);
					if (item->fallspeed > 0)
						item->fallspeed = -(item->fallspeed / 2);
				}
				else
				{
					if (item->speed < 32)
					{
						item->speed += collResult.TiltZ * 2;

						if ((unsigned short)item->pos.yRot > 0x8000)
						{
							item->pos.yRot -= 4096;
							if ((unsigned short)item->pos.yRot < 0x8000)
								item->pos.yRot = 0x8000;
						}
						else if ((unsigned short)item->pos.yRot < 0x8000)
						{
							item->pos.yRot += 4096;
							if ((unsigned short)item->pos.yRot > 0x8000)
								item->pos.yRot = 0x8000;
						}
					}

					if (item->fallspeed > 0)
						item->fallspeed = -(item->fallspeed / 2);
					else
						item->fallspeed = 0;
				}
			}
			else if (collResult.TiltX < 0 && collResult.TiltZ < 0)	// Hit angle = 0x2000
			{
				if (((unsigned short)item->pos.yRot) > 0x6000 && ((unsigned short)item->pos.yRot) < 0xe000)
				{
					item->pos.yRot = 0x2000 + (0xa000 - (unsigned short)item->pos.yRot - 1);
					if (item->fallspeed > 0)
						item->fallspeed = -(item->fallspeed / 2);
				}
				else
				{
					if (item->speed < 32)
					{
						item->speed += (-collResult.TiltX) + (-collResult.TiltZ);
						if ((unsigned short)item->pos.yRot > 0x2000 && (unsigned short)item->pos.yRot < 0xa000)
						{
							item->pos.yRot -= 4096;
							if ((unsigned short)item->pos.yRot < 0x2000)
								item->pos.yRot = 0x2000;
						}
						else if ((unsigned short)item->pos.yRot != 0x2000)
						{
							item->pos.yRot += 4096;
							if ((unsigned short)item->pos.yRot > 0x2000)
								item->pos.yRot = 0x2000;
						}
					}

					if (item->fallspeed > 0)
						item->fallspeed = -(item->fallspeed / 2);
					else
						item->fallspeed = 0;
				}
			}
			else if (collResult.TiltX < 0 && collResult.TiltZ > 0)	// Hit angle = 0x6000
			{
				if (((unsigned short)item->pos.yRot) > 0xa000 || ((unsigned short)item->pos.yRot) < 0x2000)
				{
					item->pos.yRot = 0x6000 + (0xe000 - (unsigned short)item->pos.yRot - 1);
					if (item->fallspeed > 0)
						item->fallspeed = -(item->fallspeed / 2);
				}
				else
				{
					if (item->speed < 32)
					{
						item->speed += (-collResult.TiltX) + collResult.TiltZ;
						if ((unsigned short)item->pos.yRot < 0xe000 && (unsigned short)item->pos.yRot > 0x6000)
						{
							item->pos.yRot -= 4096;
							if ((unsigned short)item->pos.yRot < 0x6000)
								item->pos.yRot = 0x6000;
						}
						else if ((unsigned short)item->pos.yRot != 0x6000)
						{
							item->pos.yRot += 4096;
							if ((unsigned short)item->pos.yRot > 0x6000)
								item->pos.yRot = 0x6000;
						}
					}

					if (item->fallspeed > 0)
						item->fallspeed = -(item->fallspeed / 2);
					else
						item->fallspeed = 0;
				}
			}
			else if (collResult.TiltX > 0 && collResult.TiltZ > 0)	// Hit angle = 0xa000
			{
				if (((unsigned short)item->pos.yRot) > 0xe000 || ((unsigned short)item->pos.yRot) < 0x6000)
				{
					item->pos.yRot = 0xa000 + (0x2000 - (unsigned short)item->pos.yRot - 1);
					if (item->fallspeed > 0)
						item->fallspeed = -(item->fallspeed / 2);
				}
				else
				{
					if (item->speed < 32)
					{
						item->speed += collResult.TiltX + collResult.TiltZ;
						if ((unsigned short)item->pos.yRot < 0x2000 || (unsigned short)item->pos.yRot > 0xa000)
						{
							item->pos.yRot -= 4096;
							if ((unsigned short)item->pos.yRot < 0xa000)
								item->pos.yRot = 0xa000;
						}
						else if ((unsigned short)item->pos.yRot != 0xa000)
						{
							item->pos.yRot += 4096;
							if ((unsigned short)item->pos.yRot > 0xa000)
								item->pos.yRot = 0xa000;
						}
					}

					if (item->fallspeed > 0)
						item->fallspeed = -(item->fallspeed / 2);
					else
						item->fallspeed = 0;
				}
			}
			else if (collResult.TiltX > 0 && collResult.TiltZ < 0)	// Hit angle = 0xe000
			{
				if (((unsigned short)item->pos.yRot) > 0x2000 && ((unsigned short)item->pos.yRot) < 0xa000)
				{
					item->pos.yRot = 0xe000 + (0x6000 - (unsigned short)item->pos.yRot - 1);
					if (item->fallspeed > 0)
						item->fallspeed = -(item->fallspeed / 2);
				}
				else
				{
					if (item->speed < 32)
					{
						item->speed += collResult.TiltX + (-collResult.TiltZ);
						if ((unsigned short)item->pos.yRot < 0x6000 || (unsigned short)item->pos.yRot > 0xe000)
						{
							item->pos.yRot -= 4096;
							if ((unsigned short)item->pos.yRot < 0xe000)
								item->pos.yRot = 0xe000;
						}
						else if ((unsigned short)item->pos.yRot != 0xe000)
						{
							item->pos.yRot += 4096;
							if ((unsigned short)item->pos.yRot > 0xe000)
								item->pos.yRot = 0xe000;
						}
					}

					if (item->fallspeed > 0)
						item->fallspeed = -(item->fallspeed / 2);
					else
						item->fallspeed = 0;
				}
			}

			/* Put item back in its last position */
			item->pos.xPos = x;
			item->pos.yPos = y;
			item->pos.zPos = z;
		}
		else
		{
			/* Hit the floor; bounce and slow down */
			if (item->fallspeed > 0)
			{
				if (item->fallspeed > 16)
				{
					if (item->objectNumber == ID_GRENADE)
						item->fallspeed = -(item->fallspeed - (item->fallspeed / 2));
					else
					{
						item->fallspeed = -(item->fallspeed / 2);
						if (item->fallspeed < -100)
							item->fallspeed = -100;
					}
				}
				else
				{
					/* Roll on floor */
					item->fallspeed = 0;
					if (item->objectNumber == ID_GRENADE)
					{
						item->requiredAnimState = 1;
						item->pos.xRot = 0;
						item->speed--;
					}
					else
						item->speed -= 3;

					if (item->speed < 0)
						item->speed = 0;
				}
			}
			item->pos.yPos = collResult.Position.Floor;
		}
	}
	else	// Check for on top of object.
	{
		if (yv >= 0)
		{
			oldCollResult = GetCollisionResult(item->pos.xPos, y, item->pos.zPos, item->roomNumber);
			collResult = GetCollisionResult(item);

			// Bounce off floor.

			// Removed weird OnObject global check from here which didnt make sense because OnObject
			// was always set to 0 by GetHeight() function which was called before the check.
			// Possibly a mistake or unfinished feature by Core? -- Lwmte, 27.08.21

			if (item->pos.yPos >= oldCollResult.Position.Floor)
			{
				/* Hit the floor; bounce and slow down */
				if (item->fallspeed > 0)
				{
					if (item->fallspeed > 16)
					{
						if (item->objectNumber == ID_GRENADE)
							item->fallspeed = -(item->fallspeed - (item->fallspeed / 2));
						else
						{
							item->fallspeed = -(item->fallspeed / 4);
							if (item->fallspeed < -100)
								item->fallspeed = -100;
						}
					}
					else
					{
						/* Roll on floor */
						item->fallspeed = 0;
						if (item->objectNumber == ID_GRENADE)
						{
							item->requiredAnimState = 1;
							item->pos.xRot = 0;
							item->speed--;
						}
						else
							item->speed -= 3;

						if (item->speed < 0)
							item->speed = 0;
					}
				}
				item->pos.yPos = oldCollResult.Position.Floor;
			}
		}
		//		else
		{
			/* Bounce off ceiling */
			collResult = GetCollisionResult(item);

			if (item->pos.yPos < collResult.Position.Ceiling)
			{
				if (y < collResult.Position.Ceiling &&
					(((x / SECTOR(1)) != (item->pos.xPos / SECTOR(1))) ||
					((z / SECTOR(1)) != (item->pos.zPos / SECTOR(1)))))
				{
					// Need to know which direction the wall is.

					if ((x & (~(WALL_SIZE - 1))) != (item->pos.xPos & (~(WALL_SIZE - 1))))	// X crossed boundary?
					{
						if (xv <= 0)	// Hit angle = 0xc000.
							item->pos.yRot = 0x4000 + (0xc000 - item->pos.yRot);
						else			// Hit angle = 0x4000.
							item->pos.yRot = 0xc000 + (0x4000 - item->pos.yRot);
					}
					else		// Z crossed boundary.
					{
						item->pos.yRot = 0x8000 - item->pos.yRot;
					}

					if (item->objectNumber == ID_GRENADE)
						item->speed -= item->speed / 8;
					else
						item->speed /= 2;

					/* Put item back in its last position */
					item->pos.xPos = x;
					item->pos.yPos = y;
					item->pos.zPos = z;
				}
				else
					item->pos.yPos = collResult.Position.Ceiling;

				if (item->fallspeed < 0)
					item->fallspeed = -item->fallspeed;
			}
		}
	}

	collResult = GetCollisionResult(item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber);
	if (collResult.RoomNumber != item->roomNumber)
		ItemNewRoom(itemNumber, collResult.RoomNumber);
}

void DoObjectCollision(ITEM_INFO* l, COLL_INFO* coll) // previously LaraBaddieCollision
{
	ITEM_INFO* item;
	OBJECT_INFO* obj;

	l->hitStatus = false;
	coll->HitStatic = false;

	if (l == LaraItem)
		Lara.hitDirection = -1;

	if (l->hitPoints > 0)
	{
		short* door, numDoors;
		for (auto i : GetRoomList(l->roomNumber))
		{
			short itemNumber = g_Level.Rooms[i].itemNumber;
			while (itemNumber != NO_ITEM)
			{
				item = &g_Level.Items[itemNumber];
				if (item->collidable && item->status != ITEM_INVISIBLE)
				{
					obj = &Objects[item->objectNumber];
					if (obj->collision != nullptr)
					{
						if (phd_Distance(&item->pos, &l->pos) < COLLISION_CHECK_DISTANCE)
							obj->collision(itemNumber, l, coll);
					}
				}
				itemNumber = item->nextItem;
			}

			for (int j = 0; j < g_Level.Rooms[i].mesh.size(); j++)
			{
				MESH_INFO* mesh = &g_Level.Rooms[i].mesh[j];

				// Only process meshes which are visible and non-solid
				if ((mesh->flags & StaticMeshFlags::SM_VISIBLE) && !(mesh->flags & StaticMeshFlags::SM_SOLID))
				{
					if (phd_Distance(&mesh->pos, &l->pos) < COLLISION_CHECK_DISTANCE)
					{
						if (TestBoundsCollideStatic(l, mesh, coll->Setup.Radius))
						{
							coll->HitStatic = true;

							if (coll->Setup.EnableObjectPush)
								ItemPushStatic(l, mesh, coll);
							else
								break;
						}
					}
				}
			}
		}

		if (l == LaraItem && Lara.hitDirection == -1)
			Lara.hitFrame = 0;
	}
}

void AIPickupCollision(short itemNumber, ITEM_INFO* l, COLL_INFO* c)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];
	if (item->objectNumber == ID_SHOOT_SWITCH1 && !(item->meshBits & 1))
		item->status = ITEM_INVISIBLE;
}

void ObjectCollision(short itemNumber, ITEM_INFO* l, COLL_INFO* coll)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];

	if (TestBoundsCollide(item, l, coll->Setup.Radius))
	{
		if (TestCollision(item, l))
		{
			if (coll->Setup.EnableObjectPush)
				ItemPushItem(item, l, coll, false, true);
		}
	}
}

void CreatureCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll)
{
	ITEM_INFO* item = &g_Level.Items[itemNum];
	float c, s;
	int x, z, rx, rz;
	ANIM_FRAME* frame;

	if (item->objectNumber != ID_HITMAN || item->currentAnimState != LS_INSERT_PUZZLE)
	{
		if (TestBoundsCollide(item, l, coll->Setup.Radius))
		{
			if (TestCollision(item, l))
			{
				if (coll->Setup.EnableObjectPush || Lara.waterStatus == LW_UNDERWATER || Lara.waterStatus == LW_SURFACE)
				{
					ItemPushItem(item, l, coll, coll->Setup.EnableSpaz, 0);
				}
				else if (coll->Setup.EnableSpaz)
				{
					x = l->pos.xPos - item->pos.xPos;
					z = l->pos.zPos - item->pos.zPos;
					c = phd_cos(item->pos.yRot);
					s = phd_sin(item->pos.yRot);
					frame = GetBestFrame(item);
					rx = (frame->boundingBox.X1 + frame->boundingBox.X2) / 2;
					rz = (frame->boundingBox.X2 + frame->boundingBox.Z2) / 2;

					if (frame->boundingBox.Y2 - frame->boundingBox.Y1 > STEP_SIZE)
					{
						int angle = (l->pos.yRot - phd_atan(z - c * rx - s * rz, x - c * rx + s * rz) - ANGLE(135)) / 16384;
						Lara.hitDirection = (short)angle;
						// TODO: check if a second Lara.hitFrame++; is required there !
						Lara.hitFrame++;
						if (Lara.hitFrame > 30)
							Lara.hitFrame = 30;
					}
				}
			}
		}
	}
}

void TrapCollision(short itemNumber, ITEM_INFO* l, COLL_INFO* coll)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];

	if (item->status == ITEM_ACTIVE)
	{
		if (!TestBoundsCollide(item, l, coll->Setup.Radius))
			return;

		TestCollision(item, l);
	}
	else if (item->status != ITEM_INVISIBLE)
		ObjectCollision(itemNumber, l, coll);
}