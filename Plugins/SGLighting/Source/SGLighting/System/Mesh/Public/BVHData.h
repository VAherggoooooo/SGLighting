#pragma once

#include "CoreMinimal.h"
#include "MeshCollecter.h"
#include "UObject/Object.h"
#include "BVHData.generated.h"

/**
 * 
 */

struct FMeshTriangle
{
	//A -> B -> C
	//face direction: cross(B-A, C-B)
	FVector3f A;
	FVector3f B;
	FVector3f C;

	FMeshTriangle(){A = FVector3f(); B = FVector3f(); C = FVector3f();}
	FMeshTriangle(FVector3f _A, FVector3f _B, FVector3f _C)
	{
		A = _A;
		B = _B;
		C = _C;
	}
};

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
	void GetAllMeshInfo(
		TArray<FVector>& _Vertices,
		TArray<FVector3f>& _Position,
		TArray<int32>& _VertexIDs,
		TArray<FVector>& _Normals,
		TArray<FVector2D>& _UVs,
		TArray<FVector2D>& _UVs2,
		TArray<FProcMeshTangent>& _Tangents,
		TArray<FMeshTriangle>& _Triangles);
	void GetSceneData();
	void ClearSceneData();
	
private:
	UMeshCollecter* MeshCollecter;

public:
	TArray<FVector> VerticeIDs;
	TArray<FVector3f>VerticePositions;
	TArray<int32> TriangleVertexIDs;//组成三角形的顶点ID列表
	TArray<FVector> Normals;
	TArray<FVector2D> UVs, UVs2;
	TArray<FProcMeshTangent> Tangents;
	TArray<FMeshTriangle> Triangles;//三角形列表
};
