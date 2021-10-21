#pragma once
#include "collide.h"
#include "lara_struct.h"

struct ITEM_INFO;
struct COLL_INFO;

SPLAT_COLL TestLaraWall(ITEM_INFO* item, int front, int right, int down);
bool TestValidLedge(ITEM_INFO* item, COLL_INFO* coll);
bool TestLaraVault(ITEM_INFO* item, COLL_INFO* coll);
bool TestLaraKeepDucked(COLL_INFO* coll);
bool TestLaraSlide(ITEM_INFO* item, COLL_INFO* coll);
bool TestLaraLean(ITEM_INFO* item, COLL_INFO* coll);
bool TestLaraHang(ITEM_INFO* item, COLL_INFO* coll);
bool TestLaraClimbStance(ITEM_INFO* item, COLL_INFO* coll);
bool TestLaraHangOnClimbWall(ITEM_INFO* item, COLL_INFO* coll);
int  TestLaraEdgeCatch(ITEM_INFO* item, COLL_INFO* coll, int* edge);
int  TestLaraHangLeftCorner(ITEM_INFO* item, COLL_INFO* coll);
int  TestLaraHangRightCorner(ITEM_INFO* item, COLL_INFO* coll);
int  TestLaraValidHangPos(ITEM_INFO* item, COLL_INFO* coll);
bool TestHangSwingIn(ITEM_INFO* item, short angle);
bool TestHangFeet(ITEM_INFO* item, short angle);
bool TestLaraHangSideways(ITEM_INFO* item, COLL_INFO* coll, short angle);
bool TestLaraFacingCorner(ITEM_INFO* item, short angle, int dist);
bool TestLaraStandingJump(ITEM_INFO* item, COLL_INFO* coll, short angle);

int  LaraFloorFront(ITEM_INFO* item, short ang, int dist);
int  LaraCeilingFront(ITEM_INFO* item, short ang, int dist, int h);
COLL_RESULT LaraCollisionFront(ITEM_INFO* item, short ang, int dist);
COLL_RESULT LaraCeilingCollisionFront(ITEM_INFO* item, short ang, int dist, int h);
COLL_RESULT LaraCollisionAboveFront(ITEM_INFO* item, short ang, int dist, int h);

bool LaraFallen(ITEM_INFO* item, COLL_INFO* coll);
bool LaraLandedBad(ITEM_INFO* item, COLL_INFO* coll);

void SetCornerAnim(ITEM_INFO* item, COLL_INFO* coll, short rot, short flip);
void SetCornerAnimFeet(ITEM_INFO* item, COLL_INFO* coll, short rot, short flip);

#ifndef NEW_TIGHTROPE
void GetTighRopeFallOff(int Regularity);
#endif // !NEW_TIGHTROPE

// TODO: New, organise later.
bool IsStandingWeapon(LARA_WEAPON_TYPE gunType);
bool TestLaraStep(COLL_INFO* coll);
bool TestLaraStepUp(ITEM_INFO* item, COLL_INFO* coll);
bool TestLaraStepDown(ITEM_INFO* item, COLL_INFO* coll);
bool TestLaraFall(COLL_INFO* coll);
bool TestLaraSlideNew(COLL_INFO* coll);	// Rename to TestLaraSlide() when legacy function of the same name is gone. @Sezz 2021.09.27
bool TestLaraCrawlMoveForward(ITEM_INFO* item, COLL_INFO* coll, int lowerBound, int upperBound);
bool TestLaraCrawlMoveBack(ITEM_INFO* item, COLL_INFO* coll, int lowerBound, int upperBound);
bool TestLaraMoveForward(ITEM_INFO* item, COLL_INFO* coll, int lowerBound, int upperBound);
bool TestLaraMoveBack(ITEM_INFO* item, COLL_INFO* coll, int lowerBound, int upperBound);
bool TestLaraStepLeft(ITEM_INFO* item, COLL_INFO* coll);
bool TestLaraStepRight(ITEM_INFO* item, COLL_INFO* coll);
bool TestLaraCrouchToCrawl(ITEM_INFO* item);
bool TestLaraCrouchRoll(ITEM_INFO* item, COLL_INFO* coll);
bool TestLaraCrawlUpStep(ITEM_INFO* item, COLL_INFO* coll);
bool TestLaraCrawlDownStep(ITEM_INFO* item, COLL_INFO* coll);
bool TestLaraCrawlExitDownStep(ITEM_INFO* item, COLL_INFO* coll);
bool TestLaraCrawlExitJump(ITEM_INFO* item, COLL_INFO* coll);
bool TestLaraCrawlVault(ITEM_INFO* item, COLL_INFO* coll);
bool TestLaraCrawlToHang(ITEM_INFO* item, COLL_INFO* coll);
bool TestLaraDrawWeaponsFromCrawlIdle(ITEM_INFO* item);
