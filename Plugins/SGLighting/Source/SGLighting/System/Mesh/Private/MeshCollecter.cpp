#include "SGLighting/System/Mesh/Public/MeshCollecter.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

UMeshCollecter::UMeshCollecter()
{
}

void UMeshCollecter::Init()
{
}

TArray<AActor*> UMeshCollecter::GetAllActors(bool bPrintName)
{
	TArray<AActor*> allActors;
	for (TObjectIterator<AActor> Itr; Itr; ++Itr)
	{
		AActor* act = *Itr;
		if(act != nullptr && IsValidChecked(act) && act->CheckStillInWorld() && !allActors.Contains(act))
		{
			FString actName = act->GetActorLabel(false);
			if(actName != "")
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

TArray<AActor*> UMeshCollecter::GetAllStaticMeshActors(bool bPrintName, bool bStatic)
{
	TArray<AActor*> allActors = GetAllActors(false);
	TArray<AActor*> smActors;
	for (AActor* act : allActors)
	{
		UStaticMeshComponent* smComp = act->GetComponentByClass<UStaticMeshComponent>();
		if(smComp != nullptr)
			if(bStatic)
			{
				if(smComp->Mobility == EComponentMobility::Static)
					smActors.Add(act);
			}
			else
			{
				smActors.Add(act);
			}
	}

	if(bPrintName)
	{
		for (AActor* act : smActors)
		{
			FString actName = UKismetSystemLibrary::GetDisplayName(act);
			UE_LOG(LogTemp, Warning, TEXT("StaticMeshActor: %s"), *actName);
		}
	}
	return smActors;
}

TArray<UStaticMeshComponent*> UMeshCollecter::GetAllStaticMeshComponents(bool bPrintName, bool bStatic)
{
	TArray<AActor*> allActors = GetAllActors(false);
	TArray<UStaticMeshComponent*> smComponents;
	for (AActor* act : allActors)
	{
		UStaticMeshComponent* smComp = act->GetComponentByClass<UStaticMeshComponent>();
		if(smComp != nullptr && smComp->Mobility == EComponentMobility::Static)//判断是否需要静态
			smComponents.Add(smComp);
	}

	if(bPrintName)
	{
		for (UStaticMeshComponent* smComp : smComponents)
		{
			FString actName = UKismetSystemLibrary::GetDisplayName(smComp);
			UE_LOG(LogTemp, Warning, TEXT("StaticMeshComponent: %s"), *actName);
		}
	}
	
	return smComponents;
}

TArray<UStaticMesh*> UMeshCollecter::GetAllStaticMeshesInLevel(bool bPrintName, bool bStatic)
{
	TArray<UStaticMeshComponent*> smComponents = GetAllStaticMeshComponents(false, bStatic);
	TArray<UStaticMesh*> staticMeshes;
	for(UStaticMeshComponent* smComp : smComponents)
	{
		UStaticMesh* mesh = smComp->GetStaticMesh();
		if(mesh != nullptr)
			staticMeshes.Add(mesh);
	}

	if(bPrintName)
	{
		for (UStaticMesh* mesh : staticMeshes)
		{
			FString actName = UKismetSystemLibrary::GetDisplayName(mesh);
			UE_LOG(LogTemp, Warning, TEXT("StaticMeshComponent: %s"), *actName);
		}
	}
	
	return  staticMeshes;
}

