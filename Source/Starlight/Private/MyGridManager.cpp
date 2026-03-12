// Fill out your copyright notice in the Description page of Project Settings.


#include "MyGridManager.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "MyGridCell.h"
#include "Kismet/GameplayStatics.h"
#include "Components/BoxComponent.h"
#include "Math/UnrealMathUtility.h"
#include "Engine/World.h"
#include "MainCharacter.h"
#include "NavMesh/NavMeshBoundsVolume.h"
#include "NavigationSystem.h"
#include "Components/BrushComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "BiomeData.h"
#include "BiomeType.h"
#include "NiagaraFunctionLibrary.h"
#include "BiomeBounds.h"
// Sets default values
AMyGridManager::AMyGridManager()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	CellMesh = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("CellMesh"));
	RootComponent = CellMesh;
	CellMesh->SetMobility(EComponentMobility::Static);
	Width = 10;
	Height = 10;
	CellSize = 100.0f;

	WallLeft = CreateDefaultSubobject<UBoxComponent>(TEXT("WallLeft"));
	WallRight = CreateDefaultSubobject<UBoxComponent>(TEXT("WallRight"));
	WallTop = CreateDefaultSubobject<UBoxComponent>(TEXT("WallTop"));
	WallBottom = CreateDefaultSubobject<UBoxComponent>(TEXT("WallBottom"));

	WallLeft->SetupAttachment(RootComponent);
	WallRight->SetupAttachment(RootComponent);
	WallTop->SetupAttachment(RootComponent);
	WallBottom->SetupAttachment(RootComponent);

	WallLeft->SetCollisionProfileName(TEXT("BlockAll"));
	WallRight->SetCollisionProfileName(TEXT("BlockAll"));
	WallTop->SetCollisionProfileName(TEXT("BlockAll"));
	WallBottom->SetCollisionProfileName(TEXT("BlockAll"));

	WallLeft->SetHiddenInGame(true);
	WallRight->SetHiddenInGame(true);
	WallTop->SetHiddenInGame(true);
	WallBottom->SetHiddenInGame(true);

	bGridReady = false;

	LightShardCount = 3;
}

// Called when the game starts or when spawned
void AMyGridManager::BeginPlay()
{
	Super::BeginPlay();
	CellMesh->ClearInstances();
	GridCells.Empty();

	const FVector GridOrigin = GetActorLocation();
	float TargetScale = CellSize / 100.0f;
	FVector InstanceScale(TargetScale, TargetScale, 0.1f);

	const float HalfTile = CellSize * 0.5f;

	for (int32 X = 0; X < Width; X++)
	{
		for (int32 Y = 0; Y < Height; Y++)
		{

			const FVector LocalPos((X * CellSize) + HalfTile, (Y * CellSize) + HalfTile, 0.f);
			const FVector WorldPos = GridOrigin + LocalPos;
			FTransform InstanceTransform;
			InstanceTransform.SetLocation(LocalPos);
			InstanceTransform.SetScale3D(InstanceScale);

			CellMesh->AddInstance(InstanceTransform);
			FMyGridCell Cell;
			Cell.WorldLocation = WorldPos;
			Cell.bIsOccupied = false;
			EBiomeType Type = DetermineBiome(X, Y);
			Cell.Biome = Type;
			GridCells.Add(FIntPoint(X, Y), Cell);
			BiomeCells.FindOrAdd(Type).Add(FIntPoint(X, Y));
		}
	}
	bGridReady = true;

	const float GridWidthWorld = Width * CellSize;
	const float GridHeightWorld = Height * CellSize;

	const float WallThickness = 50.f;
	const float WallHeight = 300.f;

	const FVector Origin = GetActorLocation();
	WallLeft->SetBoxExtent(FVector(WallThickness, GridHeightWorld * 0.5f, WallHeight));
	WallLeft->SetWorldLocation(Origin + FVector(
		-WallThickness,
		GridHeightWorld * 0.5f,
		WallHeight
	));

	WallRight->SetBoxExtent(FVector(WallThickness, GridHeightWorld * 0.5f, WallHeight));
	WallRight->SetWorldLocation(Origin + FVector(
		GridWidthWorld + WallThickness,
		GridHeightWorld * 0.5f,
		WallHeight
	));

	WallBottom->SetBoxExtent(FVector(GridWidthWorld * 0.5f, WallThickness, WallHeight));
	WallBottom->SetWorldLocation(Origin + FVector(
		GridWidthWorld * 0.5f,
		-WallThickness,
		WallHeight
	));

	WallTop->SetBoxExtent(FVector(GridWidthWorld * 0.5f, WallThickness, WallHeight));
	WallTop->SetWorldLocation(Origin + FVector(
		GridWidthWorld * 0.5f,
		GridHeightWorld + WallThickness,
		WallHeight
	));

	NavVolume = Cast<ANavMeshBoundsVolume>(
		UGameplayStatics::GetActorOfClass(
			GetWorld(),
			ANavMeshBoundsVolume::StaticClass()
		)
	);

	if (NavVolume)
	{

		FVector Center = Origin + FVector(
			GridWidthWorld * 0.5f,
			GridHeightWorld * 0.5f,
			0.f
		);
		NavVolume->GetBrushComponent()->SetMobility(EComponentMobility::Movable);
		NavVolume->SetActorLocation(Center);

		FVector NewScale(
			GridWidthWorld,
			GridHeightWorld,
			100.f
		);

		NavVolume->SetActorScale3D(NewScale);

		NavVolume->GetRootComponent()->UpdateBounds();

		UNavigationSystemV1* NavSys =
			FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());

		if (NavSys)
		{
			NavSys->OnNavigationBoundsUpdated(NavVolume);
			NavSys->Build();
		}
	}
	
}

void AMyGridManager::SpawnAtTile(const FIntPoint& Position, TSubclassOf<AActor> ActorToSpawn)
{
	if (!ActorToSpawn) {
		return;
	}
	if (!GridCells.Contains(Position)) return;

	FMyGridCell& TargetTile = GridCells[Position];

	if (TargetTile.bIsOccupied) {
		return;
	}

	TargetTile.bIsOccupied = true;
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AActor* SpawnedActor = GetWorld()->SpawnActor<AActor>(
		ActorToSpawn.Get(),
		TargetTile.WorldLocation,
		FRotator::ZeroRotator,
		SpawnParams
	);

	if (!SpawnedActor)
	{
		TargetTile.bIsOccupied = false;
		return;
	}

	FVector Origin, BoxExtent;
	SpawnedActor->GetActorBounds(false, Origin, BoxExtent);
	float ActorBottomZ = Origin.Z - BoxExtent.Z;

	float DistancePivotToBottom = SpawnedActor->GetActorLocation().Z - ActorBottomZ;

	FVector FinalLocation = TargetTile.WorldLocation;

	FinalLocation.Z += DistancePivotToBottom;
	SpawnedActor->SetActorLocation(FinalLocation);

	TargetTile.Occupant = SpawnedActor;
}

FIntPoint AMyGridManager::GetRandomFreeTile(EBiomeType BiomeType)
{
	if (!BiomeCells.Contains(BiomeType))
		return FIntPoint(-1, -1);

	const TArray<FIntPoint>& Cells = BiomeCells[BiomeType];

	for (int i = 0; i < 50; i++)
	{
		int Index = FMath::RandRange(0, Cells.Num() - 1);
		FIntPoint Cell = Cells[Index];

		if (!GridCells[Cell].bIsOccupied)
		{
			return Cell;
		}
	}

	return FIntPoint(-1, -1);
}
FIntPoint AMyGridManager::GetRandomFreeTile() {
	TArray<FIntPoint> FreeTiles;
	for (const auto& Elem : GridCells)
	{
		if (!Elem.Value.bIsOccupied)
		{
			FreeTiles.Add(Elem.Key);
		}
	}
	if (FreeTiles.Num() > 0)
	{
		return FreeTiles[FMath::RandRange(0, FreeTiles.Num() - 1)];
	}
	return FIntPoint(-1, -1);
}

FVector AMyGridManager::GetTileWorldLocation(const FIntPoint& Tile) const
{
	if (GridCells.Contains(Tile))
	{
		return GridCells[Tile].WorldLocation;
	}
	return FVector::ZeroVector;
}
void AMyGridManager::OccupyTile(const FIntPoint& Tile, AActor* Actor)
{
	if (GridCells.Contains(Tile))
	{
		GridCells[Tile].bIsOccupied = true;
		GridCells[Tile].Occupant = Actor;
	}
}

void AMyGridManager::ResetGrid()
{
	for (auto& Elem : GridCells)
	{
		if (!Elem.Value.Occupant.IsValid())
		{
			Elem.Value.bIsOccupied = false;
			Elem.Value.Occupant = nullptr;
			continue;
		}

		AActor* CurrentActor = Elem.Value.Occupant.Get();

		if (CurrentActor->IsA(AMainCharacter::StaticClass()))
		{
			Elem.Value.bIsOccupied = true;
			continue;
		}

		CurrentActor->Destroy();

		Elem.Value.Occupant = nullptr;
		Elem.Value.bIsOccupied = false;
	}
}
void AMyGridManager::SpawnRandomActors(TSubclassOf<AActor> ActorClass, int32 Count, EBiomeType BiomeType)
{
	if (!ActorClass) 
	{
		return;
	}

	for (int32 i = 0; i < Count; i++)
	{
		FIntPoint Tile = GetRandomFreeTile(BiomeType);
		if (Tile == FIntPoint(-1, -1))
		{
			continue;
		}
		SpawnAtTile(Tile, ActorClass);
	}
}
void AMyGridManager::SpawnRandomActors(TSubclassOf<AActor> ActorClass, int32 Count)
{
	if (!ActorClass)
	{
		return;
	}

	for (int32 i = 0; i < Count; i++)
	{
		FIntPoint Tile = GetRandomFreeTile();
		if (Tile == FIntPoint(-1, -1))
		{
			continue;
		}
		SpawnAtTile(Tile, ActorClass);
	}
}
void AMyGridManager::SpawnAllGameplayActors()
{
	SpawnRandomActors(LightShardClass, LightShardCount);
	for (UBiomeData* Biome : Biomes)
	{
		SpawnStarsParticlesForBiome(Biome);
		SpawnRandomActors(Biome->EnemyMeleeAoEClass, FMath::RoundToInt((Biome->GetEnemySpawnRate() / 100) * BiomeCells[Biome->BiomeType].Num()), Biome->BiomeType);
	}
}
UBiomeData* AMyGridManager::GetBiomeData(EBiomeType Type) const
{
	for (UBiomeData* Biome : Biomes)
	{
		if (Biome->BiomeType == Type)
		{
			return Biome;
		}
	}

	return nullptr;
}
EBiomeType AMyGridManager::DetermineBiome(int32 X, int32 Y)
{
	float Noise = FMath::PerlinNoise2D(FVector2D(X * 0.05f, Y * 0.05f));

	if (Noise < 0.1f)
		return EBiomeType::Nebula;

	return EBiomeType::Empty;
}

FVector AMyGridManager::ComputeBiomeCenter(const TArray<FIntPoint>& Tiles)
{
	FVector Sum = FVector::ZeroVector;

	for (const FIntPoint& Tile : Tiles)
	{
		Sum += GetTileWorldLocation(Tile);
	}

	return Sum / Tiles.Num();
}

void AMyGridManager::SpawnStarsParticlesForBiome(UBiomeData* Biome) 
{ 
	const TArray<FIntPoint>* CellsPtr = BiomeCells.Find(Biome->BiomeType);

	if (!CellsPtr || CellsPtr->Num() == 0)
	{
		return;
	}

	const TArray<FIntPoint>& Cells = *CellsPtr;

	FVector Center = ComputeBiomeCenter(Cells);

	FBiomeBounds Bounds;

	for (const FIntPoint& Cell : Cells)
	{
		Bounds.MinX = FMath::Min(Bounds.MinX, Cell.X);
		Bounds.MaxX = FMath::Max(Bounds.MaxX, Cell.X);

		Bounds.MinY = FMath::Min(Bounds.MinY, Cell.Y);
		Bounds.MaxY = FMath::Max(Bounds.MaxY, Cell.Y);
	}
	float WidthTiles = Bounds.MaxX - Bounds.MinX + 1; 
	float HeightTiles = Bounds.MaxY - Bounds.MinY + 1; 
	float BiomeWidthWorld = WidthTiles * CellSize; 
	float BiomeHeightWorld = HeightTiles * CellSize;
	UNiagaraSystem* System = Biome->StarSystem; 
	if (System) 
	{ 
		UNiagaraComponent* NiagaraComp = 
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(
				GetWorld(), 
				System, 
				Center
			);
		NiagaraComp->SetNiagaraVariableVec2(TEXT("User.PlaneSize"), FVector2D(BiomeWidthWorld, BiomeHeightWorld)); 
		int StarCount = FMath::RoundToInt((BiomeWidthWorld * BiomeHeightWorld) * Biome->GetStarDensity()); 
		StarCount = FMath::Max(StarCount, 1); 
		NiagaraComp->SetNiagaraVariableInt(TEXT("User.SpawnCount"), StarCount); 
	} 
}
