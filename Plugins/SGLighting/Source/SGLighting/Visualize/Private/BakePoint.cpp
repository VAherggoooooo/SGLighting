// Fill out your copyright notice in the Description page of Project Settings.


#include "SGLighting/Visualize/Public/BakePoint.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Kismet/KismetMathLibrary.h"


// Sets default values
ABakePoint::ABakePoint()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("root"));
	Root->SetupAttachment(RootComponent);
	SetRootComponent(Root);

	StaticMeskComp = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("Static Mesh"));
	StaticMeskComp->SetupAttachment(Root);

	UStaticMesh* Mesh = LoadObject<UStaticMesh>(NULL, TEXT("/SGLighting/Mesh/lightpoint.lightpoint"), NULL, LOAD_None, NULL);
	StaticMeskComp->SetStaticMesh(Mesh);

	UMaterialInterface* MeshMat = LoadObject<UMaterialInterface>(NULL, TEXT("/SGLighting/Material/Color_Inst.Color_Inst"), NULL, LOAD_None, NULL);
	StaticMeskComp->SetMaterial(0, MeshMat);
}


void ABakePoint::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	for(FVector trans : TransformList)
	{
		StaticMeskComp->AddInstance(UKismetMathLibrary::Conv_VectorToTransform(trans), true);
	}

	SetActorLocation(FVector(0,0,0));
}

// Called when the game starts or when spawned
void ABakePoint::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ABakePoint::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

