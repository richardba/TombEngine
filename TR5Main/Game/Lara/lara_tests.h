#pragma once
#include "collide.h"

struct ITEM_INFO;
struct COLL_INFO;

SPLAT_COLL TestLaraWall(ITEM_INFO* item, int front, int right, int down);
bool TestValidLedge(ITEM_INFO* item, COLL_INFO* coll);
bool TestLaraVault(ITEM_INFO* item, COLL_INFO* coll);
bool TestLaraStandUp(COLL_INFO* coll);
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
bool LaraFacingCorner(ITEM_INFO* item, short ang, int dist);

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
bool TestLaraFall(COLL_INFO* coll);
bool TestLaraSlideNew(COLL_INFO* coll);	// Rename to TestLaraSlide() when legacy function of the same name is gone. @Sezz 2021.09.27
bool TestLaraStepLeft(ITEM_INFO* item);
bool TestLaraStepRight(ITEM_INFO* item);
bool TestLaraCrawl(ITEM_INFO* item);
bool TestLaraCrouchRoll(ITEM_INFO* item);
