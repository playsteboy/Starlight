// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyAIController.h"
#include "Kismet/GameplayStatics.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "EnemyCharacter.h"
AEnemyAIController::AEnemyAIController()
{
	PrimaryActorTick.bCanEverTick = true;
}
void AEnemyAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    AEnemyCharacter* Enemy = Cast<AEnemyCharacter>(InPawn);
    if (Enemy && Enemy->BehaviorTreeAsset)
    {
        RunBehaviorTree(Enemy->BehaviorTreeAsset);
        APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
        if (GetBlackboardComponent() && PlayerPawn)
        {
            GetBlackboardComponent()->SetValueAsObject("TargetActor", PlayerPawn);
        }
    }
}