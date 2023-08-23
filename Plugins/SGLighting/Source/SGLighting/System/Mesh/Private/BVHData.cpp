
#include "SGLighting/System/Mesh/Public/BVHData.h"
#include "KismetProceduralMeshLibrary.h"
#include "ProceduralMeshComponent.h"
#include "Engine/StaticMeshActor.h"

UBVHData::UBVHData()
{

}

UBVHData::~UBVHData()
{
	this->MeshCollecter = nullptr;
	ClearSceneData();
}

void UBVHData::Init(UMeshCollecter* _meshCollecter)
{
	if(_meshCollecter == nullptr)
	{
		_meshCollecter = NewObject<UMeshCollecter>();
	}
	this->MeshCollecter = _meshCollecter;
}

int32 UBVHData::GetTrangleNum(bool bPrintNum, bool bPrintSM, bool bStatic) const
{
	if(MeshCollecter == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("No Mesh Collector"));
		return 0;
	}

	TArray<UStaticMesh*> staticMeshes = MeshCollecter->GetAllStaticMeshesInLevel(bPrintSM, bStatic);
	int32 trangleNum = 0;
	for (UStaticMesh* mesh : staticMeshes)
	{
		trangleNum = mesh->GetNumTriangles(mesh->GetMinLODIdx());
	}

	if(bPrintNum)
	{
		UE_LOG(LogTemp, Warning, TEXT("TrangleNum: %d"), trangleNum);
	}
	
	return trangleNum;
}

void UBVHData::GetAllMeshInfo(TArray<FVector>& _Vertices, TArray<FVector3f>& _Position, TArray<int32>& _VertexIDs, TArray<FVector>& _Normals, TArray<FVector2D>& _UVs, TArray<FVector2D>& _UVs2, TArray<FProcMeshTangent>& _Tangents, TArray<FMeshTriangle>& _Triangles)
{
	if(MeshCollecter == nullptr) return;
	TArray<AActor*> staticMesheActors = MeshCollecter->GetAllStaticMeshActors(false, true);
	TArray<UStaticMesh*> staticMeshes;
	for(AActor* aa : staticMesheActors)
	{
		AStaticMeshActor* sma = Cast<AStaticMeshActor>(aa);
		TObjectPtr<UStaticMesh> mm = sma->GetStaticMeshComponent()->GetStaticMesh();
		if(mm != nullptr)
		{
			UE_LOG(LogTemp, Warning, TEXT("mesh: %s"), *(mm->GetName()));
			staticMeshes.Add(mm);
		}
	}

	if(staticMeshes.Num() <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("NULL MESH"));
		return;
	}
	
	for (UStaticMesh* mesh : staticMeshes)
	{
		int curSectionIDOffset = 0;
		for(int sectionID = 0; sectionID < mesh->GetNumSections(mesh->GetMinLODIdx()); sectionID++)//mesh->GetNumSections(mesh->GetMinLODIdx())
		{
			TArray<FVector> v;
			TArray<FVector3f> p;
			TArray<int32> tri;
			TArray<FVector> n;
			TArray<FVector2D> uv;
			TArray<FVector2D> uv2;
			TArray<FProcMeshTangent> tan;

			GetSectionFromStaticMesh(mesh, mesh->GetMinLODIdx(), sectionID, v, tri, n, uv2, tan);
			
			for(int ti = 0; ti < tri.Num(); ti++)
			{
				tri[ti] += curSectionIDOffset;
			}
	
			_Vertices.Append(v);//顶点的坐标
			curSectionIDOffset = _Vertices.Num();
			
			_VertexIDs.Append(tri);//构成三角形的id序号
			_Normals.Append(n);
			_UVs.Append(uv);
			_UVs2.Append(uv2);
			_Position.Append(v);
			_Tangents.Append(tan);
		}
	}

	TArray<FMeshTriangle> trian;
	for(int id = 0; id < _VertexIDs.Num(); id += 3)
	{
		FVector3f A =  _Position[_VertexIDs[id]];
		FVector3f B =  _Position[_VertexIDs[id + 1]];
		FVector3f C =  _Position[_VertexIDs[id + 2]];
		FVector3f NormalA = FVector3f(_Normals[_VertexIDs[id]]);
		FVector3f NormalB = FVector3f(_Normals[_VertexIDs[id + 1]]);
		FVector3f NormalC = FVector3f(_Normals[_VertexIDs[id + 2]]);
		FVector2f UV_A = FVector2f(_UVs2[_VertexIDs[id]]);
		FVector2f UV_B = FVector2f(_UVs2[_VertexIDs[id + 1]]);
		FVector2f UV_C = FVector2f(_UVs2[_VertexIDs[id + 2]]);
		trian.Add(FMeshTriangle(A,B,C, NormalA, NormalB, NormalC, UV_A, UV_B, UV_C));
	}
	_Triangles.Append(trian);
}

void UBVHData::GetSceneData()
{
	ClearSceneData();
	GetAllMeshInfo(VerticeIDs,VerticePositions, TriangleVertexIDs, Normals, UVs, UVs2, Tangents, Triangles);

	//UE_LOG(LogTemp, Warning, TEXT("vNum: %d"), VerticeIDs.Num());
	//UE_LOG(LogTemp, Warning, TEXT("posNum: %d"), VerticePositions.Num());
	//UE_LOG(LogTemp, Warning, TEXT("triNum: %d"), TriangleVertexIDs.Num());
	
	// for (int32 i = 0; i < VerticeIDs.Num(); i++)
	// 	UE_LOG(LogTemp, Warning, TEXT("vid: %s"), *(VerticeIDs[i].ToString()));
	// UE_LOG(LogTemp, Warning, TEXT("======================================================"));
	
	// for (int32 i = 0; i < TriangleVertexIDs.Num(); i++)
	// 	UE_LOG(LogTemp, Warning, TEXT("triID: %d   pos: %s"), TriangleVertexIDs[i], *(VerticePositions[TriangleVertexIDs[i]].ToString()));
	// UE_LOG(LogTemp, Warning, TEXT("======================================================"));
	//
	// for (int32 i = 0; i < Triangles.Num(); i++)
	// 	UE_LOG(LogTemp, Warning, TEXT("trianPos: %s,  %s,  %s"), *(Triangles[i].A.ToString()), *(Triangles[i].B.ToString()), *(Triangles[i].C.ToString()));

	// for (int32 i = 0; i < VerticePositions.Num(); i++)
	// 	UE_LOG(LogTemp, Warning, TEXT("pos: %s"), *(VerticePositions[i].ToString()));
	// UE_LOG(LogTemp, Warning, TEXT("======================================================"));
}

void UBVHData::ClearSceneData()
{
	VerticeIDs.Empty();
	VerticePositions.Empty();
	TriangleVertexIDs.Empty();
	Normals.Empty();
	UVs.Empty();
	UVs2.Empty();
	Tangents.Empty();
}

void UBVHData::GetSectionFromStaticMesh(UStaticMesh* InMesh, int32 LODIndex, int32 SectionIndex,
	TArray<FVector>& _Vertices, TArray<int32>& _Triangles, TArray<FVector>& _Normals, TArray<FVector2D>& _UVs,
	TArray<FProcMeshTangent>& _Tangents)
{
	if(	InMesh != nullptr )
	{
		if (InMesh->GetRenderData() != nullptr && InMesh->GetRenderData()->LODResources.IsValidIndex(LODIndex))
		{
			const FStaticMeshLODResources& LOD = InMesh->GetRenderData()->LODResources[LODIndex];
			if (LOD.Sections.IsValidIndex(SectionIndex))
			{
				// Empty output buffers
				_Vertices.Reset();
				_Triangles.Reset();
				_Normals.Reset();
				_UVs.Reset();
				_Tangents.Reset();

				// Map from vert buffer for whole mesh to vert buffer for section of interest
				TMap<int32, int32> MeshToSectionVertMap;

				const FStaticMeshSection& Section = LOD.Sections[SectionIndex];
				const uint32 OnePastLastIndex = Section.FirstIndex + Section.NumTriangles * 3;
				FIndexArrayView Indices = LOD.IndexBuffer.GetArrayView();

				// Iterate over section index buffer, copying verts as needed
				for (uint32 i = Section.FirstIndex; i < OnePastLastIndex; i++)
				{
					uint32 MeshVertIndex = Indices[i];

					// See if we have this vert already in our section vert buffer, and copy vert in if not 
					int32 SectionVertIndex = GetNewIndexForOldVertIndex(MeshVertIndex, MeshToSectionVertMap, LOD.VertexBuffers, _Vertices, _Normals, _UVs, _Tangents);

					// Add to index buffer
					_Triangles.Add(SectionVertIndex);
				}
			}
		}
	}
}

int32 UBVHData::GetNewIndexForOldVertIndex(int32 MeshVertIndex, TMap<int32, int32>& MeshToSectionVertMap,
	const FStaticMeshVertexBuffers& VertexBuffers, TArray<FVector>& _Vertices, TArray<FVector>& _Normals,
	TArray<FVector2D>& _UVs, TArray<FProcMeshTangent>& _Tangents)
{
	int32* NewIndexPtr = MeshToSectionVertMap.Find(MeshVertIndex);
	if (NewIndexPtr != nullptr)
	{
		return *NewIndexPtr;
	}
	else
	{
		// Copy position
		int32 SectionVertIndex = _Vertices.Add((FVector)VertexBuffers.PositionVertexBuffer.VertexPosition(MeshVertIndex));

		// Copy normal
		_Normals.Add(FVector4(VertexBuffers.StaticMeshVertexBuffer.VertexTangentZ(MeshVertIndex)));
		check(_Normals.Num() == _Vertices.Num());

		// Copy UVs
		_UVs.Add(FVector2D(VertexBuffers.StaticMeshVertexBuffer.GetVertexUV(MeshVertIndex, 1)));
		check(_UVs.Num() == _Vertices.Num());

		// Copy tangents
		FVector4 TangentX = (FVector4)VertexBuffers.StaticMeshVertexBuffer.VertexTangentX(MeshVertIndex);
		FProcMeshTangent NewTangent(TangentX, TangentX.W < 0.f);
		_Tangents.Add(NewTangent);
		check(_Tangents.Num() == _Vertices.Num());

		MeshToSectionVertMap.Add(MeshVertIndex, SectionVertIndex);

		return SectionVertIndex;
	}
}

