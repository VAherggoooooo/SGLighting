// Copyright Epic Games, Inc. All Rights Reserved.

#include "SGLighting.h"
#include "SGLightingStyle.h"
#include "SGLightingCommands.h"
#include "LevelEditorActions.h"
#include "GameFramework/WorldSettings.h"
#include "SGLighting/System/Mesh/Public/MeshCollecter.h"
#include "Misc/MessageDialog.h"
#include "ToolMenus.h"
#include "SGLighting/System/Mesh/Public/LevelManager.h"

static const FName SGLightingTabName("SGLighting");

#define LOCTEXT_NAMESPACE "FSGLightingModule"

void FSGLightingModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	FSGLightingStyle::Initialize();
	FSGLightingStyle::ReloadTextures();

	FSGLightingCommands::Register();
	
	BakeSGLightingCommands = MakeShareable(new FUICommandList);
	CleanGLightingCommands = MakeShareable(new FUICommandList);

	BakeSGLightingCommands->MapAction(
		FSGLightingCommands::Get().BakeSGLightingAction,
		FExecuteAction::CreateRaw(this, &FSGLightingModule::BakeSGLighting_Clicked),
		FCanExecuteAction());

	CleanGLightingCommands->MapAction(
		FSGLightingCommands::Get().CleanSGLightingAction,
		FExecuteAction::CreateRaw(this, &FSGLightingModule::CleanSGLighting_Clicked),
		FCanExecuteAction());

	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FSGLightingModule::RegisterMenus));
}

void FSGLightingModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	UToolMenus::UnRegisterStartupCallback(this);

	UToolMenus::UnregisterOwner(this);

	FSGLightingStyle::Shutdown();

	FSGLightingCommands::Unregister();
}

void FSGLightingModule::BakeSGLighting_Clicked()
{
	// Put your "OnButtonClicked" stuff here
	// FText DialogText = FText::Format(
	// 						LOCTEXT("PluginButtonDialogText", "Add code to {0} in {1} to override this button's actions"),
	// 						FText::FromString(TEXT("FSGLightingModule::PluginButtonClicked()")),
	// 						FText::FromString(TEXT("SGLighting.cpp"))
	// 				   );
	// FMessageDialog::Open(EAppMsgType::Ok, DialogText);

	
	//FLevelEditorActionCallbacks::BuildLightingOnly_Execute();
	ULevelManager* LevelManager = NewObject<ULevelManager>();
	LevelManager->GetMeshCollector()->GetAllStaticMeshActors(true);
	LevelManager = nullptr;
}

void FSGLightingModule::CleanSGLighting_Clicked()
{
	UWorld* World = GWorld;
	World->GetWorldSettings()->bForceNoPrecomputedLighting = true;
	BakeSGLighting_Clicked();
	World->GetWorldSettings()->bForceNoPrecomputedLighting = false;
}

void FSGLightingModule::RegisterMenus()
{
	// Owner will be used for cleanup in call to UToolMenus::UnregisterOwner
	FToolMenuOwnerScoped OwnerScoped(this);

	{
		UToolMenu* Menu_bake = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Build");
		{
			FToolMenuSection& Section = Menu_bake->FindOrAddSection("LevelEditorLighting");
			Section.AddMenuEntryWithCommandList(FSGLightingCommands::Get().BakeSGLightingAction, BakeSGLightingCommands);
		}
	}
	{
		UToolMenu* Menu_clean = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Build");
		{
			FToolMenuSection& Section = Menu_clean->FindOrAddSection("LevelEditorLighting");
			Section.AddMenuEntryWithCommandList(FSGLightingCommands::Get().CleanSGLightingAction, CleanGLightingCommands);
		}
	}

	// {
	// 	UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar.PlayToolBar");
	// 	{
	// 		FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("PluginTools");
	// 		{
	// 			FToolMenuEntry& Entry = Section.AddEntry(FToolMenuEntry::InitToolBarButton(FSGLightingCommands::Get().PluginAction));
	// 			Entry.SetCommandList(PluginCommands);
	// 		}
	// 	}
	// }
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FSGLightingModule, SGLighting)