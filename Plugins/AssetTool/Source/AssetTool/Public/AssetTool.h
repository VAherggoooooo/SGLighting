// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FAssetToolModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	TSharedRef<FExtender> OnExtendContentBrowserAssetSelectionMenu(const TArray<FAssetData>& Assets);

	/**
	 * @brief 添加主menu
	 * @param MenuBuilder 
	 */
	virtual void AddToMenuEntry(FMenuBuilder& MenuBuilder);

	/**
	 * @brief 添加次级menu
	 * @param MenuBuilder 
	 */
	virtual void AddSubMenue(FMenuBuilder& MenuBuilder);

	
	void ExportToPNG(int32 OutputSize = 128);
	UTexture2D* GetObjThumbnail(UObject *InObject, FString OutputPath,int32 OutputSize = 128);
	FString SelectFilePath();
	bool OpenExplor(const FString& Title, FString& InOutLastPath, FString& OutOpenFilenames);

	//导出不同分辨率
	void Export128(){ExportToPNG(128);}
	void Export256(){ExportToPNG(256);}
	void Export512(){ExportToPNG(512);}
	void Export1024(){ExportToPNG(1024);}
	void Export2048(){ExportToPNG(2048);}
	void Export4096(){ExportToPNG(4096);}
private:
	FDelegateHandle ContentBrowserExtenderDelegateHandle;
};




