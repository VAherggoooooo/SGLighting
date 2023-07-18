#pragma once

#include "CoreMinimal.h"
#include "MeshCollecter.h"
#include "UObject/Object.h"
#include "BVHData.generated.h"

/**
 * 
 */
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
	void GetTrangles();
private:
	UMeshCollecter* MeshCollecter;
	//TArray<>
};
