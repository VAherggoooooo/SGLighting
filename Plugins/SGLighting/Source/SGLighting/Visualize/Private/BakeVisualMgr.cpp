#include "SGLighting/Visualize/Public/BakeVisualMgr.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"


ABakeVisualMgr::ABakeVisualMgr()
{
	PrimaryActorTick.bCanEverTick = false;
	SetActorHiddenInGame(true);

	//为什么不用for循环? 因为我试了就是不能
	FSoftObjectPath PositinRT_SoftRef("TextureRenderTarget2D'/SGLighting/Textures/DataRTs/VisualPosition.VisualPosition'");
	Position_RT = Cast<UTextureRenderTarget2D>(PositinRT_SoftRef.TryLoad());

	FSoftObjectPath NormalRT_SoftRef("TextureRenderTarget2D'/SGLighting/Textures/DataRTs/NormalRT.NormalRT'");
	Normal_RT = Cast<UTextureRenderTarget2D>(NormalRT_SoftRef.TryLoad());

	FSoftObjectPath TangentRT_SoftRef("TextureRenderTarget2D'/SGLighting/Textures/DataRTs/TangentRT.TangentRT'");
	Tangent_RT = Cast<UTextureRenderTarget2D>(TangentRT_SoftRef.TryLoad());

	
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("root"));
	Root->SetupAttachment(RootComponent);
	SetRootComponent(Root);

	StaticMeskComp = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("Static Mesh"));
	StaticMeskComp->SetupAttachment(Root);

	UStaticMesh* Mesh = LoadObject<UStaticMesh>(NULL, TEXT("/SGLighting/Mesh/lightpoint.lightpoint"), NULL, LOAD_None, NULL);
	StaticMeskComp->SetStaticMesh(Mesh);

	UMaterialInterface* MeshMat = LoadObject<UMaterialInterface>(NULL, TEXT("/SGLighting/Material/Debug/Color_GPU_Inst.Color_GPU_Inst"), NULL, LOAD_None, NULL);
	StaticMeskComp->SetMaterial(0, MeshMat);
}

void ABakeVisualMgr::ShowBakePoints()
{
	TArray<AActor*> Actors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ALightmapBaker::StaticClass(), Actors);
	if(Actors.Num() <= 0)
	{
		UE_LOG(LogTemp, Error, TEXT("No Lightmap Baker!"));
		return;
	}
	
	LightmapBaker = static_cast<ALightmapBaker*>(Actors[0]);
	LightmapBaker->UseRDGDraw(this, Position_RT, Normal_RT, Tangent_RT);

	
}

void ABakeVisualMgr::BeginPlay()
{
	Super::BeginPlay();
	
}

void ABakeVisualMgr::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

