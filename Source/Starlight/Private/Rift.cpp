// Fill out your copyright notice in the Description page of Project Settings.


#include "Rift.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/OverlapResult.h"
#include "CollisionQueryParams.h"
#include "MainCharacter.h"
#include "Components/StaticMeshComponent.h"
#include "MyGridManager.h"
ARift::ARift()
{
	PrimaryActorTick.bCanEverTick = true;
    CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
    RootComponent = CollisionBox;
	RiftMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RiftMesh"));
    RiftMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    RiftMesh->SetCollisionProfileName(TEXT("NoCollision"));
	RiftMesh->SetupAttachment(RootComponent);
	CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    RiftMesh->SetVisibility(false);
}

void ARift::BeginPlay()
{
    Super::BeginPlay();
    CollisionBox->SetCollisionProfileName(TEXT("Trigger"));
    CollisionBox->SetGenerateOverlapEvents(true);
    CollisionBox->OnComponentBeginOverlap.AddDynamic(
        this, &ARift::OnZoneEnter);
	GridManager = Cast<AMyGridManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AMyGridManager::StaticClass()));
}
void ARift::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void ARift::OnZoneEnter(
    UPrimitiveComponent* OverlappedComp,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult& SweepResult)
{
    AMainCharacter* Player = Cast<AMainCharacter>(OtherActor);
    if (!Player) return;

    if (GridManager && Player->GetLightShardCount() == GridManager->LightShardCount)
    {
        
        CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        RiftMesh->SetVisibility(false);

        Player->SetLightShardCount(0);

        GetWorld()->GetTimerManager().SetTimerForNextTick(
            this,
            &ARift::HandleMapGeneration
        );

    }
}
void ARift::HandleMapGeneration()
{
    if (!GridManager) return;

    GridManager->ResetGrid();
    GridManager->SpawnAllGameplayActors();
}
