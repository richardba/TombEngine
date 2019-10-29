#pragma once

#include "..\Global\global.h"

typedef struct QUAD_INFO {
	__int32 velocity;
	__int16 frontRot;
	__int16 rearRot;
	__int32 revs;
	__int32 engineRevs;
	__int16 trackMesh;
	__int32 skidooTurn;
	__int32 leftFallspeed;
	__int32 rightFallspeed;
	__int16 momentumAngle;
	__int16 extraRotation;
	__int32 pitch;
	char flags;
};

typedef struct JEEP_INFO {
	__int16 rot1;
	__int16 rot2;
	__int16 rot3;
	__int16 rot4;
	__int32 velocity;
	__int32 revs;
	__int16 engineRevs;
	__int16 trackMesh;
	__int32 jeepTurn;
	__int32 fallSpeed;
	__int16 momentumAngle;
	__int16 extraRotation;
	__int16 unknown0;
	__int32 pitch;
	__int16 flags;
	__int16 unknown1;
	__int16 unknown2;
};

typedef struct FISH_INFO
{
	__int16	x;
	__int16	y;
	__int16	z;
	__int32	angle;
	__int16	destY;
	__int16	angAdd;
	byte speed;
	byte acc;
	byte swim;
};

typedef struct FISH_LEADER_INFO
{
	__int16	angle;
	byte speed;
	byte on;
	__int16	angleTime;
	__int16	speedTime;
	__int16	xRange, yRange, zRange;
};

void __cdecl ClampRotation(PHD_3DPOS *pos, __int16 angle, __int16 rot);

// TR1 objects
void __cdecl InitialiseWolf(__int16 itemNum);
void __cdecl WolfControl(__int16 itemNum);
void __cdecl BearControl(__int16 itemNum);
void __cdecl ApeControl(__int16 itemNum);

// TR2 objects
void __cdecl BarracudaControl(__int16 itemNum);
void __cdecl SharkControl(__int16 itemNum);
void __cdecl InitialiseSpinningBlade(__int16 itemNum);
void __cdecl SpinningBlade(__int16 itemNum);
void __cdecl InitialiseKillerStatue(__int16 itemNum);
void __cdecl KillerStatueControl(__int16 itemNum);
void __cdecl SpringBoardControl(__int16 itemNum);
void __cdecl RatControl(__int16 itemNum);

// TR3 objects
void __cdecl TigerControl(__int16 itemNum);
void __cdecl InitialiseCobra(__int16 itemNum);
void __cdecl CobraControl(__int16 itemNum);
void __cdecl RaptorControl(__int16 itemNum);
__int32 __cdecl GetWaterSurface(__int32 x, __int32 y, __int32 z, __int16 roomNumber);
void __cdecl ShootHarpoon(ITEM_INFO* frogman, __int32 x, __int32 y, __int32 z, __int16 speed, __int16 yRot, __int16 roomNumber);
void __cdecl HarpoonControl(__int16 itemNum);
void __cdecl ScubaControl(__int16 itemNumber);
void __cdecl InitialiseEagle(__int16 itemNum);
void __cdecl EagleControl(__int16 itemNum);
void __cdecl TribemanAxeControl(__int16 itemNum);
void __cdecl TribesmanShotDart(ITEM_INFO* item);
void __cdecl TribesmanDartsControl(__int16 itemNum);
void __cdecl ControlSpikyWall(__int16 itemNum);
void __cdecl LaraTyrannosaurDeath(ITEM_INFO* item);
void __cdecl TyrannosaurControl(__int16 itemNum);
void __cdecl TriggerFlamethrowerFlame(__int32 x, __int32 y, __int32 z, __int32 xv, __int32 yv, __int32 zv, __int32 fxnum);
void __cdecl TriggerPilotFlame(__int32 itemnum);
__int16 __cdecl TriggerFlameThrower(ITEM_INFO* item, BITE_INFO* bite, __int16 speed);
void __cdecl FlameThrowerControl(__int16 itemNumber);
void __cdecl ControlSpikyCeiling(__int16 itemNumber);
void __cdecl InitialiseMonkey(__int16 itemNumber);
void __cdecl MonkeyControl(__int16 itemNumber);
void __cdecl MPGunControl(__int16 itemNumber);
void InitialiseMPStick(__int16 itemNumber);
void MPStickControl(__int16 itemNumber);
void __cdecl SetupShoal(__int32 shoalNumber);
void __cdecl SetupFish(__int32 leader, ITEM_INFO* item);
void ControlFish(__int16 itemNumber);
bool FishNearLara(PHD_3DPOS* pos, __int32 distance, ITEM_INFO* item);

__int32 __cdecl GetCollisionAnim(ITEM_INFO* item, PHD_VECTOR* p);
__int32 __cdecl TestHeight(ITEM_INFO* item, __int32 dz, __int32 dx, PHD_VECTOR* pos);
__int32 __cdecl DoShift(ITEM_INFO* quad, PHD_VECTOR* pos, PHD_VECTOR* old);
__int32 __cdecl DoDynamics(__int32 height, __int32 fallspeed, __int32 *y);
__int32 __cdecl QuadDynamics(ITEM_INFO* item);
void __cdecl AnimateQuadBike(ITEM_INFO* item, __int32 collide, __int32 dead);
__int32 __cdecl QuadUserControl(ITEM_INFO* item, __int32 height, int* pitch);
void __cdecl InitialiseQuadBike(__int16 itemNumber);
void __cdecl QuadBikeCollision(__int16 itemNumber, ITEM_INFO* l, COLL_INFO* coll);
__int32 __cdecl QuadBikeControl();
void __cdecl TriggerExhaustSmoke(__int32 x, __int32 y, __int32 z, __int16 angle, __int32 speed, __int32 moving);

// TR4 object
void __cdecl InitialiseWildBoar(__int16 itemNum);
void __cdecl WildBoarControl(__int16 itemNum);
void __cdecl InitialiseSmallScorpion(__int16 itemNum);
void __cdecl SmallScorpionControl(__int16 itemNum);
void __cdecl InitialiseBat(__int16 itemNum);
void __cdecl BatControl(__int16 itemNum);
void __cdecl InitialiseBaddy(__int16 itemNum);
void __cdecl BaddyControl(__int16 itemNum);
void __cdecl InitialiseSas(__int16 itemNum);
void __cdecl SasControl(__int16 itemNum);
void __cdecl InitialiseMummy(__int16 itemNum);
void __cdecl MummyControl(__int16 itemNum);
void __cdecl InitialiseSkeleton(__int16 itemNum);
void __cdecl SkeletonControl(__int16 itemNum);
void __cdecl WakeUpSkeleton(ITEM_INFO* item);
void __cdecl FourBladesControl(__int16 itemNum);
void __cdecl BirdBladeControl(__int16 itemNum);
void __cdecl CatwalkBlaldeControl(__int16 itemNum);
void __cdecl PlinthBladeControl(__int16 itemNum);
void __cdecl InitialiseSethBlade(__int16 itemNum);
void __cdecl SethBladeControl(__int16 itemNum);
void __cdecl ChainControl(__int16 itemNum);
void __cdecl PloughControl(__int16 itemNum);
void __cdecl CogControl(__int16 itemNum);
void __cdecl SpikeballControl(__int16 itemNum);
void __cdecl InitialiseKnightTemplar(__int16 itemNum);
void __cdecl KnightTemplarControl(__int16 itemNum);
void __cdecl StargateControl(__int16 itemNum);
void __cdecl StargateCollision(__int16 itemNum, ITEM_INFO* item, COLL_INFO* coll);
void __cdecl InitialiseSlicerDicer(__int16 itemNum);
void __cdecl SlicerDicerControl(__int16 itemNum);
void __cdecl BladeCollision(__int16 itemNum, ITEM_INFO* item, COLL_INFO* coll);
void __cdecl SarcophagusCollision(__int16 itemNum, ITEM_INFO* item, COLL_INFO* coll);
void __cdecl InitialiseScorpion(__int16 itemNum);
void __cdecl ScorpionControl(__int16 itemNum);
void __cdecl InitialiseLaraDouble(__int16 itemNum);
void __cdecl LaraDoubleControl(__int16 itemNum);
void __cdecl InitialiseDemigod(__int16 itemNum);
void __cdecl DemigodControl(__int16 itemNum);
void __cdecl DemigodThrowEnergyAttack(PHD_3DPOS* pos, __int16 roomNumber, __int32 something);
void __cdecl DemigodEnergyAttack(__int16 itemNum);
void __cdecl DemigodHammerAttack(__int32 x, __int32 y, __int32 z, __int32 something);
void __cdecl InitialiseMine(__int16 itemNum);
void __cdecl MineControl(__int16 itemNum);
void __cdecl MineCollision(__int16 itemNum, ITEM_INFO* item, COLL_INFO* coll);
void __cdecl InitialiseSentryGun(__int16 itemNum);
void __cdecl SentryGunControl(__int16 itemNum);
void __cdecl SentryGunThrowFire(ITEM_INFO* item);
void __cdecl InitialiseJeanYves(__int16 itemNum);
void __cdecl JeanYvesControl(__int16 itemNum);
void __cdecl ScalesControl(__int16 itemNum);
void __cdecl ScalesCollision(__int16 itemNum, ITEM_INFO* item, COLL_INFO* coll);
__int32 __cdecl RespawnAhmet(__int16 itemNum);
void __cdecl InitialiseLittleBeetle(__int16 itemNum);
void __cdecl LittleBeetleControl(__int16 itemNum);
void __cdecl InitialiseTroops(__int16 itemNum);
void __cdecl TroopsControl(__int16 itemNum);
void __cdecl InitialiseHarpy(__int16 itemNum);
void __cdecl HarpyControl(__int16 itemNum);
void __cdecl HarpyBubbles(PHD_3DPOS* pos, __int16 roomNumber, __int32 count);
void __cdecl HarpyAttack(ITEM_INFO* item, __int16 itemNum);
void __cdecl HarpySparks1(__int16 itemNum, byte num, __int32 size);
void __cdecl HarpySparks2(__int32 x, __int32 y, __int32 z, __int32 xv, __int32 yv, __int32 zv);
void __cdecl InitialiseGuide(__int16 itemNum);
void __cdecl GuideControl(__int16 itemNum);
void __cdecl InitialiseCrocodile(__int16 itemNum);
void __cdecl CrocodileControl(__int16 itemNum);
void __cdecl InitialiseSphinx(__int16 itemNum);
void __cdecl SphinxControl(__int16 itemNum);
void __cdecl InitialiseBurningFloor(__int16 itemNum);
void __cdecl BurningFloorControl(__int16 itemNum);
void __cdecl InitialiseHorse(__int16 itemNum);
void __cdecl InitialiseHorseman(__int16 itemNum);
void __cdecl HorsemanControl(__int16 itemNum);
void __cdecl HorsemanSparks(PHD_3DPOS* pos, __int32 param1, __int32 num);

void __cdecl InitialiseJeep(__int16 itemNum);
__int32 __cdecl JeepControl();
__int32 __cdecl JeepCheckGetOff();
__int32 __cdecl JeepDynamics(ITEM_INFO* item);
__int32 __cdecl JeepUserControl(ITEM_INFO* item, __int32 height, __int32* pitch);
void __cdecl AnimateJeep(ITEM_INFO* item, __int32 collide, __int32 dead);
void __cdecl JeepExplode(ITEM_INFO* item);
__int32 __cdecl JeepCanGetOff();
__int32 __cdecl DoJeepDynamics(__int32 height, __int32 speed, __int32* y, __int32 flags);
void __cdecl TriggerJeepExhaustSmoke(__int32 x, __int32 y, __int32 z, __int16 angle, __int16 speed, __int32 moving);
__int32 __cdecl GetOnJeep(int itemNumber);
void __cdecl JeepCollision(__int16 itemNumber, ITEM_INFO* l, COLL_INFO* coll);
__int32 __cdecl GetOnJeep(int itemNumber);
__int32 __cdecl GetJeepCollisionAnim(ITEM_INFO* item, PHD_VECTOR* p);
void __cdecl JeepBaddieCollision(ITEM_INFO* jeep);

void __cdecl InitialiseMotorbike(__int16 itemNum);
__int32 __cdecl MotorbikeControl();
__int32 __cdecl MotorbikeCheckGetOff();
__int32 __cdecl MotorbikeDynamics(ITEM_INFO* item);
__int32 __cdecl MotorbikeUserControl(ITEM_INFO* item, __int32 height, __int32* pitch);
void __cdecl AnimateMotorbike(ITEM_INFO* item, __int32 collide, __int32 dead);
void __cdecl MotorbikeExplode(ITEM_INFO* item);
__int32 __cdecl MotorbikeCanGetOff();
__int32 __cdecl DoMotorbikeDynamics(__int32 height, __int32 speed, __int32* y, __int32 flags);
void __cdecl TriggerMotorbikeExhaustSmoke(__int32 x, __int32 y, __int32 z, __int16 angle, __int16 speed, __int32 moving);
__int32 __cdecl GetOnMotorbike(int itemNumber);
void __cdecl MotorbikeCollision(__int16 itemNumber, ITEM_INFO* l, COLL_INFO* coll);
__int32 __cdecl GetOnMotorbike(int itemNumber);
__int32 __cdecl GetMotorbikeCollisionAnim(ITEM_INFO* item, PHD_VECTOR* p);
void __cdecl MotorbikeBaddieCollision(ITEM_INFO* jeep);

void __cdecl BubblesControl(__int16 fxNum);
__int32 __cdecl BubblesShatterFunction(FX_INFO* fx, __int32 param1, __int32 param2);
void __cdecl BubblesEffect1(__int16 fxNum, __int16 xVel, __int16 yVel, __int16 zVel);
void __cdecl BubblesEffect2(__int16 fxNum, __int16 xVel, __int16 yVel, __int16 zVel);
void __cdecl BubblesEffect3(__int16 fxNum, __int16 xVel, __int16 yVel, __int16 zVel);
void __cdecl BubblesEffect4(__int16 fxNum, __int16 xVel, __int16 yVel, __int16 zVel);
