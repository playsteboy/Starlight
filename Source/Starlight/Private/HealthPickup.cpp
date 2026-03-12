// Fill out your copyright notice in the Description page of Project Settings.


#include "HealthPickup.h"
#include "MainCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Components/BoxComponent.h"
#include "Engine/OverlapResult.h"

AHealthPickup::AHealthPickup()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	RootComponent = CollisionBox;
	PickupMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LightShardMesh"));
	PickupMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PickupMesh->SetCollisionProfileName(TEXT("NoCollision"));
	PickupMesh->SetupAttachment(RootComponent);
	CollisionBox->SetGenerateOverlapEvents(true);
	CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionBox->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECR_Overlap);

	HealthAmount = 1;
}
void AHealthPickup::OverlapBegin(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult
)
{
	Super::OverlapBegin(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);
	if (AMainCharacter* MC = Cast<AMainCharacter>(OtherActor))
	{
		MC->SetHealth(MC->GetHealth() + HealthAmount);
		CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		CollisionBox->SetCollisionProfileName(TEXT("NoCollision"));
		PickupMesh->SetVisibility(false);
	}
}

