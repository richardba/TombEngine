#include "framework.h"
#include "Game/control/control.h"

#include <process.h>
#include "Game/collision/collide_room.h"
#include "Game/pickup/pickup.h"
#include "Game/camera.h"
#include "Game/Lara/lara.h"
#include "Game/items.h"
#include "Game/control/flipeffect.h"
#include "Game/gui.h"
#include "Game/control/volume.h"
#include "Game/control/lot.h"
#include "Game/health.h"
#include "Game/savegame.h"
#include "Game/room.h"
#include "Game/effects/hair.h"
#include "Game/effects/effects.h"
#include "Game/effects/tomb4fx.h"
#include "Game/effects/debris.h"
#include "Game/effects/footprint.h"
#include "Game/effects/smoke.h"
#include "Game/effects/spark.h"
#include "Game/effects/explosion.h"
#include "Game/effects/drip.h"
#include "Game/effects/weather.h"
#include "Game/effects/lightning.h"
#include "Game/spotcam.h"
#include "Game/effects/simple_particle.h"
#include "Game/collision/sphere.h"
#include "Game/Lara/lara_one_gun.h"
#include "Game/Lara/lara_cheat.h"
#include "Game/Lara/lara_helpers.h"
#include "Objects/Effects/tr4_locusts.h"
#include "Objects/Generic/Object/objects.h"
#include "Objects/Generic/Object/rope.h"
#include "Objects/Generic/Switches/generic_switch.h"
#include "Objects/TR4/Entity/tr4_beetle_swarm.h"
#include "Objects/TR5/Emitter/tr5_rats_emitter.h"
#include "Objects/TR5/Emitter/tr5_bats_emitter.h"
#include "Objects/TR5/Emitter/tr5_spider_emitter.h"
#include "Sound/sound.h"
#include "Specific/clock.h"
#include "Specific/input.h"
#include "Specific/level.h"
#include "Specific/setup.h"
#include "Specific/prng.h"
#include "Specific/winmain.h"
#include "Scripting/Include/Flow/ScriptInterfaceFlowHandler.h"
#include "Scripting/Include/ScriptInterfaceGame.h"
#include "Scripting/Include/Objects/ScriptInterfaceObjectsHandler.h"
#include "Scripting/Include/Strings/ScriptInterfaceStringsHandler.h"

using std::vector;
using std::unordered_map;
using std::string;

using namespace TEN::Effects;
using namespace TEN::Effects::Footprints;
using namespace TEN::Effects::Explosion;
using namespace TEN::Effects::Spark;
using namespace TEN::Effects::Smoke;
using namespace TEN::Effects::Drip;
using namespace TEN::Effects::Lightning;
using namespace TEN::Effects::Environment;
using namespace TEN::Entities::Generic;
using namespace TEN::Entities::Switches;
using namespace TEN::Entities::TR4;
using namespace TEN::Renderer;
using namespace TEN::Math::Random;
using namespace TEN::Floordata;
using namespace TEN::Input;

int GameTimer       = 0;
int GlobalCounter   = 0;
int Wibble          = 0;

bool InitialiseGame;
bool DoTheGame;
bool JustLoaded;
bool ThreadEnded;

int RequiredStartPos;
int CurrentLevel;
int LevelComplete;

int SystemNameHash = 0;

bool  InItemControlLoop;
short ItemNewRoomNo;
short ItemNewRooms[MAX_ROOMS];
short NextItemActive;
short NextItemFree;
short NextFxActive;
short NextFxFree;

int DrawPhase()
{
	g_Renderer.Draw();
	Camera.numberFrames = g_Renderer.SyncRenderer();
	return Camera.numberFrames;
}

GameStatus ControlPhase(int numFrames, int demoMode)
{
	ScriptInterfaceLevel* level = g_GameFlow->GetLevel(CurrentLevel);

	RegeneratePickups();

	if (numFrames > 10)
		numFrames = 10;

	if (TrackCameraInit)
		UseSpotCam = false;

	g_GameStringsHandler->ProcessDisplayStrings(DELTA_TIME);
	
	static int framesCount = 0;
	for (framesCount += numFrames; framesCount > 0; framesCount -= 2)
	{
		GlobalCounter++;

		// This might not be the exact amount of time that has passed, but giving it a
		// value of 1/30 keeps it in lock-step with the rest of the game logic,
		// which assumes 30 iterations per second.
		g_GameScript->OnControlPhase(DELTA_TIME);

		// Poll the keyboard and update input variables
		if (CurrentLevel != 0)
			UpdateInputActions();

		// Has Lara control been disabled?
		if (Lara.Control.Locked || CurrentLevel == 0)
		{
			if (CurrentLevel != 0)
				DbInput = 0;
			TrInput &= IN_LOOK;
		}

		if (CurrentLevel != 0)
		{
			// Does the player want to enter inventory?
			if (IsClicked(In::Save) && LaraItem->HitPoints > 0 &&
				g_Gui.GetInventoryMode() != InventoryMode::Save)
			{
				StopAllSounds();
				StopRumble();

				g_Gui.SetInventoryMode(InventoryMode::Save);

				if (g_Gui.CallInventory(LaraItem, false))
					return GameStatus::SaveGame;
			}
			else if (IsClicked(In::Load) &&
				g_Gui.GetInventoryMode() != InventoryMode::Load)
			{
				StopAllSounds();
				StopRumble();

				g_Gui.SetInventoryMode(InventoryMode::Load);

				if (g_Gui.CallInventory(LaraItem, false))
					return GameStatus::LoadGame;
			}
			else if (IsClicked(In::Pause) && LaraItem->HitPoints > 0 &&
				g_Gui.GetInventoryMode() != InventoryMode::Pause)
			{
				StopAllSounds();
				StopRumble();

				g_Renderer.DumpGameScene();
				g_Gui.SetInventoryMode(InventoryMode::Pause);
				g_Gui.SetMenuToDisplay(Menu::Pause);
				g_Gui.SetSelectedOption(0);
			}
			else if ((IsClicked(In::Option) || g_Gui.GetEnterInventory() != NO_ITEM) &&
				LaraItem->HitPoints > 0 && !BinocularOn)
			{
				StopAllSounds();
				StopRumble();

				if (g_Gui.CallInventory(LaraItem, true))
					return GameStatus::LoadGame;
			}
		}

		while (g_Gui.GetInventoryMode() == InventoryMode::Pause)
		{
			g_Gui.DrawInventory();
			g_Renderer.SyncRenderer();

			if (g_Gui.DoPauseMenu() == InventoryResult::ExitToTitle)
				return GameStatus::ExitToTitle;
		}

		// Has level been completed?
		if (CurrentLevel != 0 && LevelComplete)
			return GameStatus::LevelComplete;

		// Is Lara dead?
		if (CurrentLevel != 0 &&
			(Lara.Control.Count.Death > 300 || Lara.Control.Count.Death > 60 && !NoInput()))
		{
			return GameStatus::ExitToTitle; // Maybe do game over menu like some PSX versions have??
		}

		// TODO: Adapt to new input. What is the -1? @Sezz 2022.07.13
		if (demoMode && TrInput == -1)
			TrInput = 0;

		// Handle lasersight and binocular
		if (CurrentLevel != 0)
			HandleOptics(LaraItem);

		// Update all items
		InItemControlLoop = true;

		short itemNumber = NextItemActive;
		while (itemNumber != NO_ITEM)
		{
			auto* item = &g_Level.Items[itemNumber];
			short nextItem = item->NextActive;

			if (item->AfterDeath <= 128)
			{
				if (Objects[item->ObjectNumber].control)
					Objects[item->ObjectNumber].control(itemNumber);

				TEN::Control::Volumes::TestVolumes(itemNumber);

				if (item->AfterDeath > 0 && item->AfterDeath < 128 && !(Wibble & 3))
					item->AfterDeath++;
				if (item->AfterDeath == 128)
					KillItem(itemNumber);
			}
			else
				KillItem(itemNumber);

			itemNumber = nextItem;
		}

		InItemControlLoop = false;
		KillMoveItems();

		// Update all effects
		InItemControlLoop = true;

		short fxNumber = NextFxActive;
		while (fxNumber != NO_ITEM)
		{
			short nextFx = EffectList[fxNumber].nextActive;
			auto* fx = &EffectList[fxNumber];
			if (Objects[fx->objectNumber].control)
				Objects[fx->objectNumber].control(fxNumber);

			fxNumber = nextFx;
		}

		InItemControlLoop = false;
		KillMoveEffects();

		if (CurrentLevel != 0)
		{
			g_GameScriptEntities->TestCollidingObjects();
			// Control Lara
			InItemControlLoop = true;
			LaraControl(LaraItem, &LaraCollision);
			InItemControlLoop = false;
			KillMoveItems();

			g_Renderer.UpdateLaraAnimations(true);

			if (g_Gui.GetInventoryItemChosen() != NO_ITEM)
			{
				SayNo();
				g_Gui.SetInventoryItemChosen(NO_ITEM);
			}

			LaraCheatyBits(LaraItem);
			TriggerLaraDrips(LaraItem);

			// Update Lara's ponytails
			HairControl(LaraItem, level->GetLaraType() == LaraType::Young);
		}

		if (UseSpotCam)
		{
			// Draw flyby cameras
			//if (CurrentLevel != 0)
			//	g_Renderer->EnableCinematicBars(true);
			CalculateSpotCameras();
		}
		else
		{
			// Do the standard camera
			//g_Renderer->EnableCinematicBars(false);
			TrackCameraInit = false;
			CalculateCamera();
		}

		// Update oscillator seed
		Wibble = (Wibble + WIBBLE_SPEED) & WIBBLE_MAX;

		// Smash shatters and clear stopper flags under them
		UpdateShatters();

		// Update weather
		Weather.Update();

		// Update special FX
		UpdateSparks();
		UpdateFireSparks();
		UpdateSmoke();
		UpdateBlood();
		UpdateBubbles();
		UpdateDebris();
		UpdateGunShells();
		UpdateFootprints();
		UpdateSplashes();
		UpdateLightning();
		UpdateDrips();
		UpdateRats();
		UpdateBats();
		UpdateSpiders();
		UpdateSparkParticles();
		UpdateSmokeParticles();
		updateSimpleParticles();
		UpdateDripParticles();
		UpdateExplosionParticles();
		UpdateShockwaves();
		UpdateBeetleSwarm();
		UpdateLocusts();
		AnimateWaterfalls();

		// Rumble screen (like in submarine level of TRC)
		if (level->Rumble)
			RumbleScreen();

		PlaySoundSources();
		DoFlipEffect(FlipEffect, LaraItem);

		UpdateFadeScreenAndCinematicBars();

		// Clear savegame loaded flag
		JustLoaded = false;

		// Update timers
		HealthBarTimer--;
		GameTimer++;
	}

	return GameStatus::None;
}

unsigned CALLBACK GameMain(void *)
{
	try 
	{
		TENLog("Starting GameMain...", LogLevel::Info);

		TimeInit();

		if (g_GameFlow->IntroImagePath.empty())
			throw TENScriptException("Intro image path is not set.");

		// Do a fixed time title image
		g_Renderer.RenderTitleImage();

		// Execute the LUA gameflow and play the game
		g_GameFlow->DoFlow();

		DoTheGame = false;

		// Finish the thread
		PostMessage(WindowsHandle, WM_CLOSE, NULL, NULL);
		EndThread();
	}
	catch (TENScriptException const& e) 
	{
		std::string msg = std::string{ "An unrecoverable error occurred in " } + __func__ + ": " + e.what();
		TENLog(msg, LogLevel::Error, LogConfig::All);
		throw;
	}

	return true;
}

GameStatus DoTitle(int index, std::string const& ambient)
{
	TENLog("DoTitle", LogLevel::Info);

	// Load the title. Exit game if unsuccessful.
	if (!LoadLevelFile(index))
		return GameStatus::ExitGame;

	InventoryResult inventoryResult;

	if (g_GameFlow->TitleType == TITLE_TYPE::FLYBY)
	{
		// Initialise items, effects, lots, camera
		InitialiseFXArray(true);
		InitialisePickupDisplay();
		InitialiseCamera();

		g_GameScript->ResetScripts(true);

		// Run the level script
		ScriptInterfaceLevel* level = g_GameFlow->GetLevel(index);

		if (!level->ScriptFileName.empty())
		{
			g_GameScript->ExecuteScriptFile(level->ScriptFileName);
			g_GameScript->InitCallbacks();
			g_GameStringsHandler->SetCallbackDrawString([](std::string const key, D3DCOLOR col, int x, int y, int flags)
			{
				g_Renderer.DrawString(float(x)/float(g_Configuration.Width) * REFERENCE_RES_WIDTH, float(y)/float(g_Configuration.Height) * REFERENCE_RES_HEIGHT, key.c_str(), col, flags);
			});
		}

		RequiredStartPos = false;
		if (InitialiseGame)
		{
			GameTimer = 0;
			RequiredStartPos = false;
			InitialiseGame = false;
		}

		Statistics.Level.Timer = 0;

		// Initialise flyby cameras
		InitSpotCamSequences();
		InitialiseSpotCam(0);
		UseSpotCam = true;

		// Play background music
		// MERGE: PlaySoundTrack(index);

		// Initialise menu
		g_Gui.SetMenuToDisplay(Menu::Title);
		g_Gui.SetSelectedOption(0);

		// Initialise ponytails
		InitialiseHair();

		InitialiseItemBoxData();

		g_GameScript->OnStart();

		SetScreenFadeIn(FADE_SCREEN_SPEED);

		ControlPhase(2, 0);

		int frames = 0;
		auto status = InventoryResult::None;

		while (status == InventoryResult::None && DoTheGame)
		{
			g_Renderer.RenderTitle();

			UpdateInputActions();

			status = g_Gui.TitleOptions();

			if (status != InventoryResult::None)
				break;

			Camera.numberFrames = g_Renderer.SyncRenderer();
			frames = Camera.numberFrames;
			ControlPhase(frames, 0);
		}

		inventoryResult = status;
	}
	else
		inventoryResult = g_Gui.TitleOptions();

	StopSoundTracks();

	g_GameScript->OnEnd();
	g_GameScript->FreeLevelScripts();
	g_GameScriptEntities->FreeEntities();

	switch (inventoryResult)
	{
	case InventoryResult::NewGame:
		return GameStatus::NewGame;

	case InventoryResult::LoadGame:
		return GameStatus::LoadGame;

	case InventoryResult::ExitGame:
		return GameStatus::ExitGame;
	}

	return GameStatus::NewGame;
}

GameStatus DoLevel(int index, std::string const& ambient, bool loadFromSavegame)
{
	// Load the level and fall back to title, if load was unsuccessful
	if (!LoadLevelFile(index))
		return GameStatus::ExitToTitle;

	// Initialise items, effects, lots, camera
	InitialiseFXArray(true);
	InitialisePickupDisplay();
	InitialiseCamera();

	g_GameScript->ResetScripts(loadFromSavegame);

	// Run the level script
	ScriptInterfaceLevel* level = g_GameFlow->GetLevel(index);

	if (!level->ScriptFileName.empty())
	{
		g_GameScript->ExecuteScriptFile(level->ScriptFileName);
		g_GameScript->InitCallbacks();
		g_GameStringsHandler->SetCallbackDrawString([](std::string const key, D3DCOLOR col, int x, int y, int flags)
		{
			g_Renderer.DrawString(float(x)/float(g_Configuration.Width) * REFERENCE_RES_WIDTH, float(y)/float(g_Configuration.Height) * REFERENCE_RES_HEIGHT, key.c_str(), col, flags);
		});
	}

	// Play default background music
	PlaySoundTrack(ambient, SoundTrackType::BGM);

	// Restore the game?
	if (loadFromSavegame)
	{
		SaveGame::Load(g_GameFlow->SelectedSaveGame);

		Camera.pos.x = LaraItem->Pose.Position.x + 256;
		Camera.pos.y = LaraItem->Pose.Position.y + 256;
		Camera.pos.z = LaraItem->Pose.Position.z + 256;

		Camera.target.x = LaraItem->Pose.Position.x;
		Camera.target.y = LaraItem->Pose.Position.y;
		Camera.target.z = LaraItem->Pose.Position.z;

		int x = Lara.Control.Weapon.WeaponItem;

		RequiredStartPos = false;
		InitialiseGame = false;
		g_GameFlow->SelectedSaveGame = 0;
	}
	else
	{
		// If not loading a savegame, then clear all the infos
		Statistics.Level = {};
		RequiredStartPos = false;

		if (InitialiseGame)
		{
			// Clear all game infos as well
			Statistics.Game = {};
			GameTimer = 0;
			InitialiseGame = false;
		}
	}

	g_Gui.SetInventoryItemChosen(NO_ITEM);
	g_Gui.SetEnterInventory(NO_ITEM);

	// Initialise flyby cameras
	InitSpotCamSequences();

	// Initialise ponytails
	InitialiseHair();

	InitialiseItemBoxData();

	if (loadFromSavegame)
		g_GameScript->OnLoad();
	else
		g_GameScript->OnStart();

	int nFrames = 2;

	// First control phase
	g_Renderer.ResetAnimations();
	GameStatus result = ControlPhase(nFrames, 0);

	// Fade in screen
	SetScreenFadeIn(FADE_SCREEN_SPEED);

	// The game loop, finally!
	while (DoTheGame)
	{
		result = ControlPhase(nFrames, 0);

		if (result == GameStatus::ExitToTitle ||
			result == GameStatus::LoadGame ||
			result == GameStatus::LevelComplete)
		{
			g_GameScript->OnEnd();
			g_GameScript->FreeLevelScripts();
			g_GameScriptEntities->FreeEntities();

			// Here is the only way for exiting from the loop
			StopAllSounds();
			StopSoundTracks();
			StopRumble();

			return result;
		}

		nFrames = DrawPhase();
		Sound_UpdateScene();
	}

	g_GameScript->ResetScripts(true);
	return GameStatus::ExitGame;
}

void UpdateShatters()
{
	if (!SmashedMeshCount)
		return;

	do
	{
		SmashedMeshCount--;

		FloorInfo* floor = GetFloor(
			SmashedMesh[SmashedMeshCount]->pos.Position.x,
			SmashedMesh[SmashedMeshCount]->pos.Position.y,
			SmashedMesh[SmashedMeshCount]->pos.Position.z,
			&SmashedMeshRoom[SmashedMeshCount]);

		TestTriggers(SmashedMesh[SmashedMeshCount]->pos.Position.x,
			SmashedMesh[SmashedMeshCount]->pos.Position.y,
			SmashedMesh[SmashedMeshCount]->pos.Position.z,
			SmashedMeshRoom[SmashedMeshCount], true);

		floor->Stopper = false;
		SmashedMesh[SmashedMeshCount] = 0;
	} while (SmashedMeshCount != 0);
}

void KillMoveItems()
{
	if (ItemNewRoomNo > 0)
	{
		for (int i = 0; i < ItemNewRoomNo; i++)
		{
			short itemNumber = ItemNewRooms[2 * i];
			if (itemNumber >= 0)
				ItemNewRoom(itemNumber, ItemNewRooms[2 * i + 1]);
			else
				KillItem(itemNumber & 0x7FFF);
		}
	}

	ItemNewRoomNo = 0;
}

void KillMoveEffects()
{
	if (ItemNewRoomNo > 0)
	{
		for (int i = 0; i < ItemNewRoomNo; i++)
		{
			short itemNumber = ItemNewRooms[2 * i];
			if (itemNumber >= 0)
				EffectNewRoom(itemNumber, ItemNewRooms[2 * i + 1]);
			else
				KillEffect(itemNumber & 0x7FFF);
		}
	}

	ItemNewRoomNo = 0;
}

int GetRandomControl()
{
	return GenerateInt();
}

int GetRandomDraw()
{
	return GenerateInt();
}

bool ExplodeItemNode(ItemInfo *item, int node, int noXZVel, int bits)
{
	if (item->MeshBits & (1 << node))
	{
		int number = bits;
		if (item->ObjectNumber == ID_SHOOT_SWITCH1 && (CurrentLevel == 4 || CurrentLevel == 7)) // TODO: remove hardcoded think !
			SoundEffect(SFX_TR5_SMASH_METAL, &item->Pose);
		else if (number == BODY_EXPLODE)
			number = -64;

		GetSpheres(item, CreatureSpheres, SPHERES_SPACE_WORLD | SPHERES_SPACE_BONE_ORIGIN, Matrix::Identity);
		ShatterItem.yRot = item->Pose.Orientation.y;
		ShatterItem.bit = 1 << node;
		ShatterItem.meshIndex = Objects[item->ObjectNumber].meshIndex + node;
		ShatterItem.sphere.x = CreatureSpheres[node].x;
		ShatterItem.sphere.y = CreatureSpheres[node].y;
		ShatterItem.sphere.z = CreatureSpheres[node].z;
		ShatterItem.flags = item->ObjectNumber == ID_CROSSBOW_BOLT ? 0x400 : 0;
		ShatterImpactData.impactDirection = Vector3(0, -1.0f, 0);
		ShatterImpactData.impactLocation = {(float)ShatterItem.sphere.x, (float)ShatterItem.sphere.y, (float)ShatterItem.sphere.z};
		ShatterObject(&ShatterItem, NULL, number, item->RoomNumber, noXZVel);
		item->MeshBits &= ~ShatterItem.bit;

		return true;
	}

	return false;
}

void CleanUp()
{
	// Reset oscillator seed
	Wibble = 0;

	// Needs to be cleared or otherwise controls will lockup if user will exit to title
	// while playing flyby with locked controls
	Lara.Control.Locked = false;

	// Weather.Clear resets lightning and wind parameters so user won't see prev weather in new level
	Weather.Clear();

	// Needs to be cleared because otherwise a list of active creatures from previous level
	// will spill into new level
	ActiveCreatures.clear();

	// Clear ropes
	Ropes.clear();

	// Clear camera data
	ClearSpotCamSequences();
	ClearCinematicBars();

	// Clear all kinds of particles
	DisableSmokeParticles();
	DisableDripParticles();
	DisableBubbles();
	DisableDebris();

	// Clear soundtrack masks
	ClearSoundTrackMasks();
}
