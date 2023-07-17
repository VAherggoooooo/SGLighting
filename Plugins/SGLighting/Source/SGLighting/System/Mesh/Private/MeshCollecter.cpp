#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "SGLighting/System/Mesh/Public/MeshCollecter.h"

#include "Engine/StaticMeshActor.h"

UMeshCollecter::UMeshCollecter()
{
}

TArray<AActor*> UMeshCollecter::GetAllActors(bool bPrintName)
{
	TArray<AActor*> allActors;
	for (TObjectIterator<AActor> Itr; Itr; ++Itr)
	{
		AActor* act = Cast<AActor>(*Itr);
		if(act != nullptr && !act->IsPendingKill() && act->CheckStillInWorld())
		{
			allActors.Add(act);
		}
	}
	if(bPrintName)
	{
		for (AActor* act : allActors)
		{
			FString actName = UKismetSystemLibrary::GetDisplayName(act);
			UE_LOG(LogTemp, Warning, TEXT("Actor: %s"), *actName);
		}
	}
	return allActors;
}

TArray<AStaticMeshActor*> UMeshCollecter::GetAllStaticMeshActors(bool bPrintName)
{
	TArray<AActor*> allActors = GetAllActors();
	TArray<AStaticMeshActor*> smActors;
	for (AActor* act : allActors)
	{
		AStaticMeshActor* smAct = Cast<AStaticMeshActor>(act);
		if(smAct != nullptr)
			smActors.Add(smAct);
	}

	if(bPrintName)
	{
		for (AStaticMeshActor* act : smActors)
		{
			FString actName = UKismetSystemLibrary::GetDisplayName(act);
			UE_LOG(LogTemp, Warning, TEXT("StaticMeshActor: %s"), *actName);
		}
	}
	return smActors;
}
