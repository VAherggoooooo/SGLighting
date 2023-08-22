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
	FVector3f NormalA;
	FVector3f NormalB;
	FVector3f NormalC;
	FVector2f UV_A;
	FVector2f UV_B;
	FVector2f UV_C;

	FMeshTriangle()
	{
		A = FVector3f(); B = FVector3f(); C = FVector3f();
		NormalA = FVector3f(); NormalB = FVector3f(); NormalC = FVector3f();
		UV_A = FVector2f(); UV_B = FVector2f(); UV_C = FVector2f(); 
	}
	FMeshTriangle(
		FVector3f _A, FVector3f _B, FVector3f _C,
		FVector3f _NormalA, FVector3f _NormalB, FVector3f _NormalC
		,FVector2f _UV_A, FVector2f _UV_B, FVector2f _UV_C
		)
	{
		A = _A;
		B = _B;
		C = _C;
		NormalA = _NormalA;
		NormalB = _NormalB;
		NormalC = _NormalC;
		UV_A = _UV_A;
		UV_B = _UV_B;
		UV_C = _UV_C;
	}
};

struct FMainLight
{
	FVector3f LightDir;
	FVector3f LightColor;
	float LightIntensity;

	FMainLight(){LightDir = FVector3f(); LightColor = FVector3f(); LightIntensity = 1;}
	FMainLight(FVector3f _lightDir, FVector3f _lightColor, float _lightIntensity)
	{
		LightDir = _lightDir;
		LightColor = _lightColor;
		LightIntensity = _lightIntensity;
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
