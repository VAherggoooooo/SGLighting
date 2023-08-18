// Fill out your copyright notice in the Description page of Project Settings.


#include "LightmapBakerDebug.h"

#include "Components/ArrowComponent.h"
#include "Kismet/KismetMathLibrary.h"

ALightmapBakerDebug::ALightmapBakerDebug()
{
	PrimaryActorTick.bCanEverTick = false;

	
	arrs.SetNum(SG_NUM);
	for(int i = 0; i < arrs.Num(); i++)
	{
		FName _name = FName(FString::Format(TEXT("Arrow {0}"), { i }));
		arrs[i] = CreateDefaultSubobject<UArrowComponent>(_name);
		arrs[i]->SetupAttachment(Root);
	}

}

void ALightmapBakerDebug::DebugSGDirs()
{
	InitSGs();
	for(int i = 0; i <SG_NUM; i++)
	{
		arrs[i]->SetWorldRotation(UKismetMathLibrary::MakeRotationFromAxes(FVector(OutSGs[i].Axis), FVector::Zero(), FVector::Zero()));
		arrs[i]->SetArrowColor(FLinearColor(abs(OutSGs[i].Axis.X), abs(OutSGs[i].Axis.Y), abs(OutSGs[i].Axis.Z)));
		UE_LOG(LogTemp, Warning, TEXT("Dir%d %s"), i, *(OutSGs[i].Axis.ToString()));
	}
	UE_LOG(LogTemp, Warning, TEXT("sharpness %f"), OutSGs[7].Sharpness);
	
}

void ALightmapBakerDebug::Tick(float DeltaSeconds)
{
	
}

void ALightmapBakerDebug::BeginPlay()
{
	
}
