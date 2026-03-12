// Copyright Epic Games, Inc. All Rights Reserved.

#include "MyGameModeBase.h"
#include "MainCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "MyGridManager.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"
#include "Rift.h"
AMyGameModeBase::AMyGameModeBase()
{
    // set default pawn class to our Blueprinted character
    static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/Starlight/Blueprint/Allies/BP_MainCharacter"));
    if (PlayerPawnBPClass.Class != NULL)
    {
        DefaultPawnClass = PlayerPawnBPClass.Class;
    }
    ConstructorHelpers::FClassFinder<AMyGridManager> MyGridManagerBPClass(TEXT("/Game/Starlight/Blueprint/Grid/BP_GridManager"));
    if (MyGridManagerBPClass.Succeeded())
    {
        MyGridManagerClass = MyGridManagerBPClass.Class;
    }
    bGameFinished = false;

    bPlayerWon = false;
    TimeRemaining = 99.f;
    TotalScore = 0.f;
}
void AMyGameModeBase::BeginPlay()
{
    if (MyGridManagerClass)
    {
        MyGridManager = GetWorld()->SpawnActor<AMyGridManager>(MyGridManagerClass, FVector::ZeroVector, FRotator::ZeroRotator);
    }
    else {
        return;
    }
    Super::BeginPlay();

    TimeRemainingSave = TimeRemaining;
    if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
    {
        RestartPlayer(PC);
    }

}
void AMyGameModeBase::RestartPlayer(AController* NewPlayer)
{
    if (!NewPlayer || !MyGridManager || !MyGridManager->bGridReady)
        return;

    /*TArray<AActor*> FoundPawns;
    UGameplayStatics::GetAllActorsOfClass(
        GetWorld(),
        AMainCharacter::StaticClass(),
        FoundPawns
    );

    for (AActor* Actor : FoundPawns)
    {
        Actor->Destroy();
    }*/
    bGameFinished = false;
    bPlayerWon = false;
    TimeRemaining = TimeRemainingSave;
    TotalScore = TimeRemaining;
	MyGridManager->ResetGrid();
    MyGridManager->SpawnAllGameplayActors();
	APawn* Pawn = HandlePlayerSpawn(NewPlayer);

    if (AMainCharacter* MC = Cast<AMainCharacter>(Pawn))
    {
        MC->SetLightShardCount(0);
        MC->SetHealth(3);
    }
    if (GetWorldTimerManager().IsTimerActive(TimerHandle_Mission))
    {
        GetWorldTimerManager().ClearTimer(TimerHandle_Mission);
    }
    GetWorldTimerManager().SetTimer(
        TimerHandle_Mission,
        this,
        &AMyGameModeBase::UpdateTimer,
        1.0f,
        true
    );
	SpawnRiftAtPlayerLocation();
}



void AMyGameModeBase::FinishGame() {
    return;
}

void AMyGameModeBase::UpdateTimer()
{
    if (bGameFinished)
        return;
    TimeRemaining -= 1.0f;
    TotalScore = TimeRemaining;
    if (TimeRemaining <= 0.0f)
    {
        FinishGame();

    }
}

float AMyGameModeBase::GetTimeRemaining() const
{
    if (GetWorldTimerManager().IsTimerActive(TimerHandle_Mission))
    {
        return TimeRemaining;
    }
    return 0.0f;
}

float AMyGameModeBase::CalculateFinalScore()
{
    return 0.0f;
}
void AMyGameModeBase::SpawnRiftAtPlayerLocation() {
    if (!RiftClass) return;
    PlayerStartLocation = SpawnLocation;
    TArray<AActor*> FoundRifts;
    UGameplayStatics::GetAllActorsOfClass(
        GetWorld(),
        ARift::StaticClass(),
        FoundRifts
    );

    for (AActor* Actor : FoundRifts)
    {
        Actor->Destroy();
    }
    if (RiftClass) {
        ARift* RiftToSpawn = GetWorld()->SpawnActor<ARift>(RiftClass, PlayerStartLocation, FRotator::ZeroRotator);
    }
}

APawn* AMyGameModeBase::HandlePlayerSpawn(AController* Player) {
    FIntPoint PlayerTile = MyGridManager->GetRandomFreeTile();
    SpawnLocation = MyGridManager->GetTileWorldLocation(PlayerTile);

	SpawnLocation.Z += TileHeight/2 + CapsuleHalfHeight;
    APawn* Pawn = SpawnDefaultPawnAtTransform(
        Player,
        FTransform(FRotator::ZeroRotator, SpawnLocation)
    );
    MyGridManager->OccupyTile(PlayerTile, Pawn);
    if (APlayerController* PC = Cast<APlayerController>(Player))
    {
        PC->SetInputMode(FInputModeGameOnly());
        PC->bShowMouseCursor = false;
    }



    Player->Possess(Pawn);
	return Pawn;
}
