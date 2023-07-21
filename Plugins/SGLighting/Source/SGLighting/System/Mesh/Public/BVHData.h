#pragma once

#include "CoreMinimal.h"
#include "MeshCollecter.h"
#include "UObject/Object.h"
#include "BVHData.generated.h"

/**
 * 
 */
struct FProcMeshTangent;
UCLASS()
class SGLIGHTING_API UBVHData : public UObject
{
	GENERATED_BODY()

public:
	UBVHData();
	~UBVHData();

	void Init(UMeshCollecter* _meshCollecter);

	UMeshCollecter* GetMeshCollecter() const { return MeshCollecter == nullptr? nullptr:MeshCollecter; }
	int32 GetTrangleNum(bool bPrintNum = false, bool bPrintSM = false, bool bStatic = false) const;
	void GetAllMeshInfo(TArray<FVector>& _Vertices, TArray<int32>& _Triangles, TArray<FVector>& _Normals, TArray<FVector2D>& _UVs, TArray<FProcMeshTangent>& _Tangents);
	void GetSceneData();
	void ClearSceneData();
	
private:
	UMeshCollecter* MeshCollecter;

	TArray<FVector> Vertices;
	TArray<int32> Triangles;//组成三角形的顶点ID列表
	TArray<FVector> Normals;
	TArray<FVector2D> UVs;
	TArray<FProcMeshTangent> Tangents;
};
