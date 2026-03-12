// Fill out your copyright notice in the Description page of Project Settings.


#include "LightShard.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/OverlapResult.h"
#include "Components/BoxComponent.h"
#include "MainCharacter.h"
#include "MyGridManager.h"
#include "Rift.h"
#include "MyGameModeBase.h"
ALightShard::ALightShard() {
	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	RootComponent = CollisionBox;
	LightShardMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LightShardMesh"));
	LightShardMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	LightShardMesh->SetCollisionProfileName(TEXT("NoCollision"));
	LightShardMesh->SetupAttachment(RootComponent);
	CollisionBox->SetGenerateOverlapEvents(true);
	CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionBox->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECR_Overlap);
	
}
void ALightShard::BeginPlay() {
	Super::BeginPlay();
	GridManager = Cast<AMyGridManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AMyGridManager::StaticClass()));
	CollisionBox->OnComponentBeginOverlap.AddDynamic(this, &ALightShard::OverlapBegin);
}

void ALightShard::OverlapBegin(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult
) {
	Super::OverlapBegin(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);
	if (!OtherActor || !OtherActor->IsA(AMainCharacter::StaticClass()))
	{
		return;
	}
	if (OtherActor && (OtherActor != this))
	{
		AMainCharacter* Character = Cast<AMainCharacter>(OtherActor);
		
		if (Character)
		{
			CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			CollisionBox->SetCollisionProfileName(TEXT("NoCollision"));
			LightShardMesh->SetVisibility(false);
			Character->SetLightShardCount(Character->GetLightShardCount()+1);
			if (Character->GetLightShardCount() == GridManager->LightShardCount) {
				Rift = Cast<ARift>(UGameplayStatics::GetActorOfClass(GetWorld(), ARift::StaticClass()));
				if (Rift) {
					Rift->RiftMesh->SetVisibility(true);
					Rift->CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
				}
			}
			else {
				Rift = Cast<ARift>(UGameplayStatics::GetActorOfClass(GetWorld(), ARift::StaticClass()));
				if (Rift) {
					Rift->RiftMesh->SetVisibility(false);
					Rift->CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
				}
				
			}
		}
		else
		{
			// Loggez si quelque chose d'autre touche le shard au spawn
			UE_LOG(LogTemp, Log, TEXT("Shard touché par %s (pas un character)"), *OtherActor->GetName());
		}
	}
}
