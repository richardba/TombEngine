#include "control.h"

#include "..\Global\global.h"

#include "pickup.h"
#include "spotcam.h"
#include "camera.h"
#include "lara.h"
#include "hair.h"
#include "items.h"
#include "effect2.h"
#include "draw.h"
#include "inventory.h"
#include "gameflow.h"
#include "lot.h"
#include "pickup.h"
#include "draw.h"
#include "healt.h"
#include "savegame.h"
#include "sound.h"
#include "spotcam.h"

#include "..\Specific\roomload.h"
#include "..\Specific\input.h"
#include "..\Specific\init.h"
#include "..\Specific\winmain.h"

#include <process.h>
#include <stdio.h>

extern GameFlow* g_GameFlow;
extern GameScript* g_GameScript;

GAME_STATUS __cdecl ControlPhase(__int32 numFrames, __int32 demoMode)
{
	GameScriptLevel* level = g_GameFlow->GetLevel(CurrentLevel);

	RegeneratePickups();

	if (numFrames > 10)
		numFrames = 10;

	if (TrackCameraInit)
		UseSpotCam = 0;

	SetDebounce = 1;

	for (FramesCount += numFrames; FramesCount > 0; FramesCount -= 2)
	{
		GlobalCounter++;

		UpdateSky();
		
		// Poll the keyboard and update input variables
		if (S_UpdateInput() == -1)
			return GAME_STATUS::GAME_STATUS_NONE;
		
		// Has Lara control been disabled?
		if (DisableLaraControl && false)
		{
			if (CurrentLevel != 0)
				DbInput = 0;
			TrInput &= 0x200;
		}

		// If cutscene has been triggered then clear input
		if (CutSeqTriggered)
			TrInput = 0;

		// Does the player want to enter inventory?
		SetDebounce = 0;
		if (CurrentLevel != 0)
		{
			if ((DbInput & 0x200000 || GlobalEnterInventory != -1) && !CutSeqTriggered && LaraItem->hitPoints > 0)
			{ 
				// Stop all sounds
				INVENTORY_RESULT inventoryResult = g_Inventory->DoInventory();
				switch (inventoryResult)
				{
				case INVENTORY_RESULT::INVENTORY_RESULT_LOAD_GAME:
					return GAME_STATUS::GAME_STATUS_LOAD_GAME;
				case INVENTORY_RESULT::INVENTORY_RESULT_EXIT_TO_TILE:
					return GAME_STATUS::GAME_STATUS_EXIT_TO_TITLE;
				}
			}
		}

		// Has level been completed?
		if (LevelComplete)
			return GAME_STATUS::GAME_STATUS_LEVEL_COMPLETED;

		__int32 oldInput = TrInput;
		 
		// Is Lara dead?
		if (Lara.deathCount > 300 || Lara.deathCount > 60 && TrInput)
		{
			INVENTORY_RESULT inventoryResult = g_Inventory->DoInventory();
			switch (inventoryResult)
			{
			case INVENTORY_RESULT::INVENTORY_RESULT_NEW_GAME:
				return GAME_STATUS::GAME_STATUS_NEW_GAME;
			case INVENTORY_RESULT::INVENTORY_RESULT_LOAD_GAME:
				return GAME_STATUS::GAME_STATUS_LOAD_GAME;
			case INVENTORY_RESULT::INVENTORY_RESULT_EXIT_TO_TILE:
				return GAME_STATUS::GAME_STATUS_EXIT_TO_TITLE;
			}
		}

		if (demoMode && TrInput == -1)
		{
			oldInput = 0;
			TrInput = 0;
		}

		// CLear dynamic lights
		ClearDynamics();
		ClearFires();
		g_Renderer->ClearDynamicLights();

		GotLaraSpheres = false;

		// Update all items
		InItemControlLoop = true;

		__int16 itemNum = NextItemActive;
		while (itemNum != NO_ITEM)
		{
			ITEM_INFO* item = &Items[itemNum];
			__int16 nextItem = item->nextActive;

			//printf("Num: %d Next: %d Death: %d Hitpoints: %d Status: %d\n", itemNum, item->nextActive, item->afterDeath, item->hitPoints, item->status);
			//printf("NextItemActive: %d\n", NextItemActive);

			if (item->afterDeath < 128)
			{
				if (Objects[item->objectNumber].control)
					Objects[item->objectNumber].control(itemNum);
			}
			else
			{
				KillItem(itemNum);
			}

			itemNum = nextItem;
		}

		InItemControlLoop = false;
		KillMoveItems();

		// Update all effects
		InItemControlLoop = true;

		__int16 fxNum = NextFxActive;
		while (fxNum != NO_ITEM)
		{
			__int16 nextFx = Effects[fxNum].nextActive;
			if (Objects[Effects[fxNum].objectNumber].control)
				(*Objects[Effects[fxNum].objectNumber].control)(fxNum);
			fxNum = nextFx;
		}

		InItemControlLoop = false;
		KillMoveEffects();

		// Update some effect timers
		if (SmokeCountL)
			SmokeCountL--;
		if (SmokeCountR)
			SmokeCountR--;
		if (SplashCount)
			SplashCount--;
		if (WeaponDelay)
			WeaponDelay--;
		if (WeaponEnemyTimer)
			WeaponEnemyTimer--;

		// Control Lara
		InItemControlLoop = true;
		Lara.skelebob = NULL;
		LaraControl();
		InItemControlLoop = false;
		KillMoveItems();

		// Update Lara's ponytails
		HairControl(0, 0, 0);
		if (level->LaraType == LARA_DRAW_TYPE::LARA_YOUNG)
			HairControl(0, 1, 0);

		if (UseSpotCam)
		{
			// Draw flyby cameras
			g_Renderer->EnableCinematicBars(true);
			CalculateSpotCameras();
		}
		else
		{
			// Do the standard camera
			g_Renderer->EnableCinematicBars(false);
			CalculateCamera();
		}
		    
		//WTF: what is this? It's used everywhere so it has to stay
		Wibble = (Wibble + 4) & 0xFC;
		
		// Update special effects
		UpdateSparks();
		UpdateFireSparks();
		UpdateSmoke();
		UpdateBlood();
		UpdateBubbles();
		UpdateDebris();
		UpdateGunShells();
		UpdateSplashes();
		UpdateDrips();
		UpdateRats();
		UpdateBats();
		UpdateSpiders();
		UpdateShockwaves();
		UpdateLightning();

		HealtBarTimer--;
	}

	return GAME_STATUS::GAME_STATUS_NONE;
}

unsigned __stdcall GameMain(void*)
{
	DB_Log(2, "GameMain - DLL");
	printf("GameMain\n");

	// We still need legacy matrices because control routines use them
	MatrixPtr = MatrixStack;
	DxMatrixPtr = (byte*)malloc(48 * 40);

	// Initialise legacy memory buffer and game timer
	InitGameMalloc();
	TIME_Init();

	// TODO: deprecated, to remove because now we have LUA
	LoadNewStrings();
	
	// Execute the LUA gameflow and play the game
	g_GameFlow->DoGameflow();

	// End the game and release some resources
	GameClose();

	// Finish the thread
	PostMessageA((HWND)WindowsHandle, 0x10u, 0, 0);
	_endthreadex(1);

	return 1;   
}   

GAME_STATUS __cdecl DoTitle(__int32 index)
{
	DB_Log(2, "DoTitle - DLL");
	printf("DoTitle\n");

	// Load the title level
	S_LoadLevelFile(0);
	
	INVENTORY_RESULT inventoryResult = g_Inventory->DoTitleInventory();
	switch (inventoryResult)
	{
	case INVENTORY_RESULT::INVENTORY_RESULT_NEW_GAME:
		return GAME_STATUS::GAME_STATUS_NEW_GAME;
	case INVENTORY_RESULT::INVENTORY_RESULT_LOAD_GAME:
		return GAME_STATUS::GAME_STATUS_LOAD_GAME;
	case INVENTORY_RESULT::INVENTORY_RESULT_EXIT_GAME:
		return GAME_STATUS::GAME_STATUS_EXIT_GAME;
	}

	return GAME_STATUS::GAME_STATUS_NEW_GAME;
}

GAME_STATUS __cdecl DoLevel(__int32 index, __int32 ambient, bool loadFromSavegame)
{
	CreditsDone = false;
	CanLoad = false;

	// If not loading a savegame, then clear all the infos
	if (!loadFromSavegame)
	{
		Savegame.Level.Timer = 0;
		Savegame.Game.Timer = 0;
		Savegame.Level.Distance = 0;
		Savegame.Game.Distance = 0;
		Savegame.Level.AmmoUsed = 0;
		Savegame.Game.AmmoUsed = 0;
		Savegame.Level.AmmoHits = 0;
		Savegame.Game.AmmoHits = 0;
		Savegame.Level.Kills = 0;
		Savegame.Game.Kills = 0;
	}

	// Load the level
	S_LoadLevelFile(index);

	// Initialise items, effects, lots, camera
	InitialiseFXArray(true);
	//InitialiseLOTarray(true);
	InitialisePickUpDisplay();
	InitialiseCamera();
	SOUND_Stop();

	// Restore the game?
	if (loadFromSavegame)
	{
		char fileName[255];
		ZeroMemory(fileName, 255);
		sprintf(fileName, "savegame.%d", g_GameFlow->SelectedSaveGame);
		SaveGame::Load(fileName);

		gfRequiredStartPos = false;
		gfInitialiseGame = false;
		g_GameFlow->SelectedSaveGame = 0;
	}
	else
	{
		gfRequiredStartPos = false;
		if (gfInitialiseGame)
		{
			GameTimer = 0;
			gfRequiredStartPos = false;
			gfInitialiseGame = false;
		}

		Savegame.Level.Timer = 0;
		if (CurrentLevel == 1)
			Savegame.TLCount = 0;
	}

	// TODO: deprecated?
	GlobalLastInventoryItem = -1;
	DelCutSeqPlayer = 0;
	TitleControlsLockedOut = false;

	// Initialise flyby cameras
	InitSpotCamSequences();
		
	// Play background music
	CurrentAtmosphere = ambient;
	S_CDPlay(CurrentAtmosphere, 1);
	IsAtmospherePlaying = true;

	// Initialise ponytails
	InitialiseHair();

	__int32 nframes = 2;
	GAME_STATUS result = ControlPhase(nframes, 0);
	g_Renderer->FadeIn();

	// The game loop, finally!
	while (true)
	{
		nframes = DrawPhaseGame();
		result = ControlPhase(nframes, 0);

		if (result == GAME_STATUS::GAME_STATUS_EXIT_TO_TITLE ||
			result == GAME_STATUS::GAME_STATUS_LOAD_GAME ||
			result == GAME_STATUS::GAME_STATUS_LEVEL_COMPLETED)
		{
			// Here is the only way for exiting from the loop
			SOUND_Stop();
			S_CDStop();

			return result;
		}

		Sound_UpdateScene();
	}
}

void __cdecl TestTriggers(__int16* data, __int32 heavy, __int32 HeavyFlags)
{
	__int32 flip = -1;
	__int32 flipAvailable = 0;
	__int32 newEffect = -1;
	__int32 switchOff = 0;
	__int32 switchFlag = 0;
	__int16 objectNumber = 0;
	__int32 keyResult = 0;
	__int16 cameraFlags = 0;
	__int16 cameraTimer = 0;
	__int32 spotCamIndex = 0;

	HeavyTriggered = false;

	if (!heavy)
	{
		Lara.canMonkeySwing = false;
		Lara.climbStatus = true;
	}

	if (!data)
		return;

	// Burn Lara
	if ((*data & 0x1F) == LAVA_TYPE)
	{
		if (!heavy && (LaraItem->pos.yPos == LaraItem->floor || Lara.waterStatus))
			LavaBurn(LaraItem);

		if (*data & 0x8000)
			return;

		data++;
	}

	// Lara can climb
	if ((*data & 0x1F) == CLIMB_TYPE)
	{
		if (!heavy)
		{
			__int16 quad = (__int16)(LaraItem->pos.yRot + ANGLE(45)) / ANGLE(90);
			if ((1 << (quad + 8)) & *data)
				Lara.climbStatus = true;
		}

		if (*data & 0x8000)
			return;

		data++;
	}

	// Lara can monkey
	if ((*data & 0x1F) == MONKEY_TYPE)
	{
		if (!heavy)
			Lara.canMonkeySwing = true;

		if (*data & 0x8000)
			return;

		data++;
	}

	// Trigger triggerer
	if ((*data & 0x1F) == TRIGTRIGGER_TYPE)
	{
		if (!(*data & 0x20) || *data & 0x8000)
			return;

		data++;
	}

	__int16 triggerType = (*(data++) >> 8) & 0x3F;
	__int16 flags = *(data++);
	__int16 timer = flags & 0xFF;

	if (Camera.type != HEAVY_CAMERA)
		RefreshCamera(triggerType, data);

	if (heavy)
	{
		switch (triggerType)
		{
		case HEAVY:
		case HEAVYANTITRIGGER:
			break;

		case HEAVYSWITCH:
			if (!HeavyFlags)
				return;

			if (HeavyFlags >= 0)
			{
				flags &= 0x3E00u;
				if (flags != HeavyFlags)
					return;
			}
			else
			{
				flags |= 0x3E00u;
				flags += HeavyFlags;
			}
			break;

		default:
			// Enemies can only activate heavy triggers
			return;
		}
	}

	__int16 value = 0;

	switch (triggerType)
	{
	case TRIGGER_TYPES::SWITCH:
		value = *(data++) & 0x3FF;

		if (flags & 0x100)
			Items[value].itemFlags[0] = 1;

		if (!SwitchTrigger(value, timer))
			return;

		objectNumber = Items[value].objectNumber;
		if (objectNumber >= ID_SWITCH_TYPE1 && objectNumber <= ID_SWITCH_TYPE6 && Items[value].triggerFlags == 5)
			switchFlag = 1;

		switchOff = (Items[value].currentAnimState == 1);

		break;

	case TRIGGER_TYPES::MONKEY:
		if (LaraItem->currentAnimState >= 75 &&
			(LaraItem->currentAnimState <= 79 ||
				LaraItem->currentAnimState == 82 ||
				LaraItem->currentAnimState == 83))
			break;
		return;

	case TRIGGER_TYPES::TIGHTROPE_T:
		if (LaraItem->currentAnimState >= 119 &&
			LaraItem->currentAnimState <= 127 &&
			LaraItem->currentAnimState != 126)
			break;
		return;

	case TRIGGER_TYPES::CRAWLDUCK_T:
		if (LaraItem->currentAnimState == 80 ||
			LaraItem->currentAnimState == 81 ||
			LaraItem->currentAnimState == 84 ||
			LaraItem->currentAnimState == 85 ||
			LaraItem->currentAnimState == 86 ||
			LaraItem->currentAnimState == 71 ||
			LaraItem->currentAnimState == 72 ||
			LaraItem->currentAnimState == 105 ||
			LaraItem->currentAnimState == 106)
			break;
		return;

	case TRIGGER_TYPES::CLIMB_T:
		if (LaraItem->currentAnimState == 10 ||
			LaraItem->currentAnimState == 56 ||
			LaraItem->currentAnimState == 57 ||
			LaraItem->currentAnimState == 58 ||
			LaraItem->currentAnimState == 59 ||
			LaraItem->currentAnimState == 60 ||
			LaraItem->currentAnimState == 61 ||
			LaraItem->currentAnimState == 75)
			break;
		return;

	case TRIGGER_TYPES::PAD:
	case TRIGGER_TYPES::ANTIPAD:
		if (LaraItem->pos.yPos == LaraItem->floor)
			break;
		return;

	case TRIGGER_TYPES::KEY:
		value = *(data++) & 0x3FF;
		keyResult = KeyTrigger(value);
		if (keyResult != -1)
			break;
		return;

	case TRIGGER_TYPES::PICKUP:
		value = *(data++) & 0x3FF;
		if (!PickupTrigger(value))
			return;
		break;

	case TRIGGER_TYPES::COMBAT:
		if (Lara.gunStatus == LG_READY)
			break;
		return;

	case TRIGGER_TYPES::SKELETON_T:
		Lara.skelebob = 2;
		break;

	case TRIGGER_TYPES::HEAVY:
	case TRIGGER_TYPES::DUMMY:
	case TRIGGER_TYPES::HEAVYSWITCH:
	case TRIGGER_TYPES::HEAVYANTITRIGGER:
		return;

	default:
		break;
	}

	__int16 targetType = 0;
	__int16 trigger = 0;

	ITEM_INFO* item = NULL;
	ITEM_INFO* cameraItem = NULL;

	do
	{
		trigger = *(data++);
		value = trigger & 0x3FF;
		targetType = (trigger >> 10) & 0xF;

		switch (targetType)
		{
		case TO_OBJECT:
			item = &Items[value];

			if (keyResult >= 2 ||
				(triggerType == TRIGGER_TYPES::ANTIPAD ||
					triggerType == TRIGGER_TYPES::ANTITRIGGER ||
					triggerType == TRIGGER_TYPES::HEAVYANTITRIGGER) &&
				item->flags & 0x80)
				break;

			if (triggerType == TRIGGER_TYPES::SWITCH)
			{
				if (item->flags & 0x40)
					break;
				if (item->objectNumber == ID_DART_EMITTER && item->active)
					break;
			}

			item->timer = timer;
			if (timer != 1)
				item->timer = 30 * timer;

			if (triggerType == TRIGGER_TYPES::SWITCH ||
				triggerType == TRIGGER_TYPES::HEAVYSWITCH)
			{
				if (HeavyFlags >= 0)
				{
					if (switchFlag)
						item->flags |= (flags & 0x3E00);
					else
						item->flags ^= (flags & 0x3E00);

					if (flags & 0x100)
						item->flags |= 0x40;
				}
				else
				{
					if (((flags ^ item->flags) & 0x3E00) == 0x3E00)
					{
						item->flags ^= (flags & 0x3E00);
						if (flags & 0x100)
							item->flags |= 0x40;
					}
				}
			}
			else if (triggerType == TRIGGER_TYPES::ANTIPAD ||
				triggerType == TRIGGER_TYPES::ANTITRIGGER ||
				triggerType == TRIGGER_TYPES::HEAVYANTITRIGGER)
			{
				if (item->objectNumber == ID_EARTHQUAKE)
				{
					item->itemFlags[0] = 0;
					item->itemFlags[1] = 100;
				}

				item->flags &= ~(0x3E00 | 0x4000);

				if (flags & 0x100)
					item->flags |= 0x80;

				if (item->active && Objects[item->objectNumber].intelligent)
				{
					item->hitPoints = -16384;
					DisableBaddieAI(value);
					KillItem(value);
				}
			}
			else if (flags & 0x3E00)
			{
				item->flags |= flags & 0x3E00;
			}

			if ((item->flags & 0x3E00) & 0x3E00)
			{
				item->flags |= 0x20;

				if (flags & 0x100)
					item->flags |= 1;

				if (!(item->active))
				{
					if (Objects[item->objectNumber].intelligent)
					{
						if (item->status != ITEM_DEACTIVATED)
						{
							if (item->status == ITEM_INVISIBLE)
							{
								item->touchBits = 0;
								if (EnableBaddieAI(value, 0))
								{
									item->status = ITEM_ACTIVE;
									AddActiveItem(value);
								}
								else
								{
									item->status == ITEM_INVISIBLE;
									AddActiveItem(value);
								}
							}
						}
						else
						{
							item->touchBits = 0;
							item->status = ITEM_ACTIVE;
							AddActiveItem(value);
							EnableBaddieAI(value, 1);
						}
					}
					else
					{
						item->touchBits = 0;
						AddActiveItem(value);
						item->status = ITEM_ACTIVE;
						HeavyTriggered = heavy;
					}
				}
			}
			break;

		case TO_CAMERA:
			trigger = *(data++);

			if (keyResult == 1)
				break;

			if (Camera.fixed[value].flags & 0x100)
				break;

			Camera.number = value;

			if (Camera.type == LOOK_CAMERA || Camera.type == COMBAT_CAMERA && !(Camera.fixed[value].flags & 3))
				break;

			if (triggerType == TRIGGER_TYPES::COMBAT)
				break;

			if (triggerType == TRIGGER_TYPES::SWITCH && timer && switchOff)
				break;

			if (Camera.number != Camera.last || triggerType == TRIGGER_TYPES::SWITCH)
			{
				Camera.timer = (trigger & 0xFF) * 30;

				if (trigger & 0x100)
					Camera.fixed[Camera.number].flags |= 0x100;

				Camera.speed = ((trigger & 0x3E00) >> 6) + 1;
				Camera.type = heavy ? HEAVY_CAMERA : FIXED_CAMERA;
			}
			break;

		case TO_FLYBY:
			trigger = *(data++);

			if (keyResult == 1)
				break;

			if (triggerType == TRIGGER_TYPES::ANTIPAD ||
				triggerType == TRIGGER_TYPES::ANTITRIGGER ||
				triggerType == TRIGGER_TYPES::HEAVYANTITRIGGER)
				UseSpotCam = false;
			else
			{
				spotCamIndex = 0;
				if (SpotCamRemap[value] > 0)
				{
					for (__int32 i = 0; i < SpotCamRemap[value]; i++)
					{
						spotCamIndex += CameraCnt[i];
					}
				}

				if (!(SpotCam[spotCamIndex].flags & 0x8000))
				{
					if (trigger & 0x100)
						SpotCam[spotCamIndex].flags |= 0x8000;

					if (!UseSpotCam)
					{
						UseSpotCam = true;
						if (LastSpotCam != value)
							TrackCameraInit = false;
						InitialiseSpotCameras(value);
					}
				}
			}
			break;

		case TO_TARGET:
			cameraItem = &Items[value];
			break;

		case TO_SINK:
			Lara.currentActive = value + 1;
			break;

		case TO_FLIPMAP:
			flipAvailable = true;
			
			if (FlipMap[value] & 0x100)
				break;

			if (triggerType == TRIGGER_TYPES::SWITCH)
				FlipMap[value] ^= (flags & 0x3E00);
			else if (flags & 0x3E00)
				FlipMap[value] |= (flags & 0x3E00);
			
			if ((FlipMap[value] & 0x3E00) == 0x3E00)
			{

				if (flags & 0x100)
					FlipMap[value] |= 0x100;
				if (!FlipStatus)
					flip = value;
			}
			else if (FlipStatus)
				flip = value;
			break;

		case TO_FLIPON:
			flipAvailable = true;
			if ((FlipMap[value] & 0x3E00) == 0x3E00 && !FlipStatus)
				flip = value;
			break;

		case TO_FLIPOFF:
			flipAvailable = true;
			if ((FlipMap[value] & 0x3E00) == 0x3E00 && FlipStatus)
				flip = value;
			break;

		case TO_FLIPEFFECT:
			TriggerTimer = timer;
			newEffect = value;
			break;

		case TO_FINISH:
			gfRequiredStartPos = false;
			LevelComplete = CurrentLevel + 1;
			break;

		case TO_CD:
			PlaySoundTrack(value, flags);
			break;

		case TO_CUTSCENE:
			// TODO: not used for now
			break;

		case TO_LUA_SCRIPT:
			trigger = *(data++);
			g_GameScript->ExecuteTrigger(trigger & 0x7FFF);

			break;

		default: 
			break;
		}

	} while (!(trigger & 0x8000));

	if (cameraItem && (Camera.type == FIXED_CAMERA || Camera.type == HEAVY_CAMERA))
		Camera.item = cameraItem;

	if (flip != -1)
		DoFlipMap(flip);

	if (newEffect != -1 && (flip || !flipAvailable))
	{
		FlipEffect = newEffect;
		FlipTimer = 0;
	}
}

void Inject_Control()
{
	
}