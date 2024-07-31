// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/ObstacleBase.h"
#include "Character/SkateSimCharacterBase.h"

AObstacleBase::AObstacleBase()
{
	PrimaryActorTick.bCanEverTick = true;

	SetRootComponent(CreateDefaultSubobject<USceneComponent>("Root"));

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>("Mesh");
	Mesh->SetupAttachment(RootComponent);

	FailBox = CreateDefaultSubobject<UBoxComponent>("Fail Collision");
	FailBox->SetupAttachment(RootComponent);
	FailBox->OnComponentBeginOverlap.AddDynamic(this, &AObstacleBase::OnFailBoxBeginOverlap);

	SuccessBox = CreateDefaultSubobject<UBoxComponent>("Success Collision");
	SuccessBox->SetupAttachment(RootComponent);	
	SuccessBox->OnComponentBeginOverlap.AddDynamic(this, &AObstacleBase::OnSuccessBoxBeginOverlap);
	SuccessBox->OnComponentEndOverlap.AddDynamic(this, &AObstacleBase::OnSuccessBoxEndOverlap);

	bFailBoxOverlapped = false;
	bSuccessBoxOverlapped = false;
	bJumpedOver = false;
}

void AObstacleBase::BeginPlay()
{
	Super::BeginPlay();
	
}

void AObstacleBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AObstacleBase::OnFailBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, 
	int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	bFailBoxOverlapped = true;
}

void AObstacleBase::OnSuccessBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, 
	int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	
}

void AObstacleBase::OnSuccessBoxEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, 
	int32 OtherBodyIndex)
{
	ASkateSimCharacterBase* Player = Cast<ASkateSimCharacterBase>(OtherActor);	
	if (!Player) return;

	// Check if Jumped Over And Set This On Player Side
	if (!bFailBoxOverlapped)
	{
		Player->IncrementJumpedOver();
	}
	
	// Check if Jump Failed And Set This On Player Side
	if (bFailBoxOverlapped)
	{
		Player->IncrementJumpFailed();
	}

	// Reset values
	bFailBoxOverlapped = false;
	bSuccessBoxOverlapped = false;

	UE_LOG(LogTemp, Display, TEXT("S %d F %d"), Player->GetJumpedOverCount(), Player->GetJumpFailedCount());
}

