// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "ObstacleBase.generated.h"

UCLASS()
class SKATESIM_API AObstacleBase : public AActor
{
	GENERATED_BODY()
	
public:	
	AObstacleBase();

protected:
	virtual void BeginPlay() override;

    UFUNCTION()
    void OnFailBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, 
	int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);

    UFUNCTION()
    void OnSuccessBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, 
	int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);

    UFUNCTION()
    void OnSuccessBoxEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, 
	int32 OtherBodyIndex);

public:	
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, Category = "Obstacle")
	TObjectPtr<UStaticMeshComponent> Mesh;	

	UPROPERTY(EditAnywhere, Category = "Obstacle")
	TObjectPtr<UBoxComponent> FailBox;

	UPROPERTY(EditAnywhere, Category = "Obstacle")
	TObjectPtr<UBoxComponent> SuccessBox;	

private:
	bool bFailBoxOverlapped;
	bool bSuccessBoxOverlapped;
	bool bJumpedOver;
};
