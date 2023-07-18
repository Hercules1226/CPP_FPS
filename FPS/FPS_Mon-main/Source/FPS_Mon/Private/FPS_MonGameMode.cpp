// Copyright Epic Games, Inc. All Rights Reserved.

#include "FPS_MonGameMode.h"
#include "FPS_MonHUD.h"
#include "FPS_MonCharacter.h"
#include "UObject/ConstructorHelpers.h"

AFPS_MonGameMode::AFPS_MonGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPersonCPP/Blueprints/FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

	// use our custom HUD class
	HUDClass = AFPS_MonHUD::StaticClass();
}
