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
#include "BiomeCluster.h"

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
	if (MapSeed == 0) MapSeed = FMath::RandRange(1, 1000000);
	FMath::SRandInit(MapSeed);
	SeedOffset = FVector2D(FMath::SRand() * 10000.f, FMath::SRand() * 10000.f);

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
	GenerateBiomeClusters();

	SpawnStarsForTiles();
	
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
		const TArray<FIntPoint>* Cells = BiomeCells.Find(Biome->BiomeType);

		if (!Cells)
		{
			continue;
		}

		int32 Count = FMath::RoundToInt(
			(Biome->GetEnemySpawnRate() / 100.f) * Cells->Num()
		);

		SpawnRandomActors(
			Biome->EnemyMeleeAoEClass,
			Count,
			Biome->BiomeType
		);
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
	float SampleX = (X * 0.05f) + SeedOffset.X;
	float SampleY = (Y * 0.05f) + SeedOffset.Y;

	float Noise = FMath::PerlinNoise2D(FVector2D(SampleX, SampleY));

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

void AMyGridManager::SpawnStarsForTiles()
{
	for (const FBiomeCluster& Cluster: BiomeClusters)
	{
		for (const FIntPoint& Tile : Cluster.Tiles) {
			UBiomeData* Biome = GetBiomeData(Cluster.BiomeType);
			if (FMath::FRand() > 0.3f)
			{
				continue;
			}
			if (!Biome || !Biome->StarSystem)
				continue;
			if (!Biome || !Biome->StarSystem || Biome->GetStarDensity() <= 0)
			{
				continue;
			}
			FVector TileCenter = GetTileWorldLocation(Tile);

			UNiagaraComponent* NiagaraComp =
				UNiagaraFunctionLibrary::SpawnSystemAttached(
					Biome->StarSystem,
					RootComponent,
					NAME_None,
					TileCenter,
					FRotator::ZeroRotator,
					EAttachLocation::KeepWorldPosition,
					true
				);
			NiagaraComp->SetAutoDestroy(false);
			NiagaraComp->SetComponentTickEnabled(false);
			NiagaraComp->SetNiagaraVariableVec2(
				TEXT("User.PlaneSize"),
				FVector2D(CellSize, CellSize)
			);

			int StarCount = FMath::RoundToInt(
				CellSize * CellSize * Biome->GetStarDensity()
			);

			NiagaraComp->SetNiagaraVariableInt(
				TEXT("User.SpawnCount"),
				FMath::Max(1, StarCount)
			);
			if(Biome->BiomeType == EBiomeType::Nebula)
			{
				float RandValForColor = FMath::FRand();
				if(RandValForColor < 0.25f)
				{
					NiagaraComp->SetNiagaraVariableLinearColor(
						TEXT("User.NebulaStarColor"),
						FLinearColor(0.44f, 0.f, 1.f, 1.f)
					);
				}
				else if (RandValForColor < 0.5f)
				{
					NiagaraComp->SetNiagaraVariableLinearColor(
						TEXT("User.NebulaStarColor"),
						FLinearColor(0.f, 1.f, 0.67f, 1.f)
					);
				}
				else if (RandValForColor < 0.75f)
				{
					NiagaraComp->SetNiagaraVariableLinearColor(
						TEXT("User.NebulaStarColor"),
						FLinearColor(1.f, 0.f, 0.32f, 1.f)
					);
				}
				else
				{
					NiagaraComp->SetNiagaraVariableLinearColor(
						TEXT("User.NebulaStarColor"),
						FLinearColor(0.f, 1.f, 0.51f, 1.f)
					);
				}
				
			}
		}
	}
}

void AMyGridManager::GenerateBiomeClusters()
{
	BiomeClusters.Empty();
	TSet<FIntPoint> Visited;

	for (const auto& Elem : GridCells)
	{
		FIntPoint Start = Elem.Key;

		if (Visited.Contains(Start))
			continue;

		FBiomeCluster Cluster;
		Cluster.BiomeType = Elem.Value.Biome;

		TQueue<FIntPoint> Queue;
		Queue.Enqueue(Start);
		Visited.Add(Start);

		while (!Queue.IsEmpty())
		{
			FIntPoint Current;
			Queue.Dequeue(Current);

			Cluster.Tiles.Add(Current);

			TArray<FIntPoint> Neighbors =
			{
				FIntPoint(Current.X + 1, Current.Y),
				FIntPoint(Current.X - 1, Current.Y),
				FIntPoint(Current.X, Current.Y + 1),
				FIntPoint(Current.X, Current.Y - 1)
			};

			for (const FIntPoint& N : Neighbors)
			{
				if (!GridCells.Contains(N))
					continue;

				if (Visited.Contains(N))
					continue;

				if (GridCells[N].Biome != Cluster.BiomeType)
					continue;

				Visited.Add(N);
				Queue.Enqueue(N);
			}
		}
		if (Cluster.Tiles.Num() < 10) {
			continue;
		}
		

		Cluster.BiomeBounds.MinX = INT_MAX;
		Cluster.BiomeBounds.MaxX = INT_MIN;
		Cluster.BiomeBounds.MinY = INT_MAX;
		Cluster.BiomeBounds.MaxY = INT_MIN;

		for (const FIntPoint& Tile : Cluster.Tiles)
		{
			Cluster.BiomeBounds.MinX = FMath::Min(Cluster.BiomeBounds.MinX, Tile.X);
			Cluster.BiomeBounds.MaxX = FMath::Max(Cluster.BiomeBounds.MaxX, Tile.X);
			Cluster.BiomeBounds.MinY = FMath::Min(Cluster.BiomeBounds.MinY, Tile.Y);
			Cluster.BiomeBounds.MaxY = FMath::Max(Cluster.BiomeBounds.MaxY, Tile.Y);
		}
		float CenterX = (Cluster.BiomeBounds.MinX + Cluster.BiomeBounds.MaxX) * 0.5f;
		float CenterY = (Cluster.BiomeBounds.MinY + Cluster.BiomeBounds.MaxY) * 0.5f;

		Cluster.Center = GetActorLocation() + FVector(
			(CenterX + 0.5f) * CellSize,
			(CenterY + 0.5f) * CellSize,
			0.f
		);
		BiomeClusters.Add(Cluster);
	}
}