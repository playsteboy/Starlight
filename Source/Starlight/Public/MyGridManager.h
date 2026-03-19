#pragma once
#include "BiomeBounds.h"
#include "BiomeCluster.h"
#include "BiomeType.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MyGridCell.h"
#include "MyGridManager.generated.h"
class UInstancedStaticMeshComponent; class UBoxComponent; class ANavMeshBoundsVolume; class UBiomeData;
UCLASS()
class STARLIGHT_API AMyGridManager : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AMyGridManager();

	UFUNCTION(BlueprintCallable)
	void SpawnAtTile(const FIntPoint& Position, TSubclassOf<AActor> ActorToSpawn);

	FIntPoint GetRandomFreeTile(EBiomeType BiomeType);

	UFUNCTION(BlueprintCallable)
	void OccupyTile(const FIntPoint& Tile, AActor* Actor);

	FIntPoint GetRandomFreeTile();

	UFUNCTION(BlueprintCallable)
	FVector GetTileWorldLocation(const FIntPoint& Tile) const;

	UFUNCTION(BlueprintCallable)
	void ResetGrid();

	void SpawnRandomActors(TSubclassOf<AActor> ActorClass, int32 Count, EBiomeType BiomeType);

	void SpawnRandomActors(TSubclassOf<AActor> ActorClass, int32 Count);

	UFUNCTION(BlueprintCallable)
	void SpawnAllGameplayActors();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid Settings")
	int32 Width;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid Settings")
	int32 Height;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid Settings")
	float CellSize;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UInstancedStaticMeshComponent* CellMesh;

	UPROPERTY(VisibleAnywhere)
	UBoxComponent* WallLeft;

	UPROPERTY(VisibleAnywhere)
	UBoxComponent* WallRight;

	UPROPERTY(VisibleAnywhere)
	UBoxComponent* WallTop;

	UPROPERTY(VisibleAnywhere)
	UBoxComponent* WallBottom;

	UPROPERTY(EditAnywhere, Category = "Spawning")
	TSubclassOf<AActor> LightShardClass;

	UPROPERTY(EditAnywhere, Category = "Spawning")
	int32 LightShardCount;

	bool bGridReady;

	UPROPERTY(EditInstanceOnly, Category = "Navigation")
	ANavMeshBoundsVolume* NavVolume;

	UFUNCTION(BlueprintCallable)
	UBiomeData* GetBiomeData(EBiomeType Type) const;

	UPROPERTY(EditAnywhere, Category = "Biomes")
	TArray<UBiomeData*> Biomes;

	UFUNCTION(BlueprintCallable)
	EBiomeType DetermineBiome(int32 X, int32 Y);

	UPROPERTY(EditAnywhere, Category = "Generation")
	int32 MapSeed;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:

	TMap<FIntPoint, FMyGridCell> GridCells;
	TMap<EBiomeType, TArray<FIntPoint>> BiomeCells;
	TArray<FBiomeCluster> BiomeClusters;
	FVector ComputeBiomeCenter(const TArray<FIntPoint>& Tile);

	void SpawnStarsForTiles();
	FVector2D SeedOffset;
	void GenerateBiomeClusters();

};
