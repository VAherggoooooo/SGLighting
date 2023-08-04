#include "AssetTool.h"
#include "ContentBrowserModule.h"
#include "LevelEditor.h"
#include "IContentBrowserSingleton.h"
#include "AssetToolsModule.h"
#include "ContentBrowserDelegates.h"
#include "DesktopPlatformModule.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "ObjectTools.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "Kismet/KismetSystemLibrary.h"

#define LOCTEXT_NAMESPACE "FAssetToolModule"

void FAssetToolModule::StartupModule()
{
	if (!IsRunningCommandlet())
	{
		FContentBrowserModule& ContentBrowserModule = FModuleManager::GetModuleChecked<FContentBrowserModule>("ContentBrowser");
		TArray<FContentBrowserMenuExtender_SelectedAssets>& CBMenuExtenderDelegates =  ContentBrowserModule.GetAllAssetViewContextMenuExtenders();
		FContentBrowserMenuExtender_SelectedAssets& AddedDelegate = CBMenuExtenderDelegates.Add_GetRef(FContentBrowserMenuExtender_SelectedAssets::CreateRaw(this, &FAssetToolModule::OnExtendContentBrowserAssetSelectionMenu));
		ContentBrowserExtenderDelegateHandle = AddedDelegate.GetHandle();
	}
}

void FAssetToolModule::ShutdownModule()
{
	if (FModuleManager::Get().IsModuleLoaded("ContentBrowser"))
	{
		FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));
		TArray<FContentBrowserMenuExtender_SelectedAssets>& CBMenuExtenderDelegates = ContentBrowserModule.GetAllAssetViewContextMenuExtenders();
		CBMenuExtenderDelegates.RemoveAll([this](const FContentBrowserMenuExtender_SelectedAssets& Delegate) { return Delegate.GetHandle() == ContentBrowserExtenderDelegateHandle; });
	}
}


TSharedRef<FExtender> FAssetToolModule::OnExtendContentBrowserAssetSelectionMenu(const TArray<FAssetData>& Assets)
{
	TSharedRef<FExtender> Extender(new FExtender());
	Extender->AddMenuExtension("GetAssetActions", EExtensionHook::After, nullptr,
				FMenuExtensionDelegate::CreateRaw(this, &FAssetToolModule::AddToMenuEntry));
	return Extender;
}

void FAssetToolModule::AddToMenuEntry(FMenuBuilder& MenuBuilder)
{
	MenuBuilder.BeginSection("AssetTool", FText::FromString(TEXT("AssetTool")));

	//添加主menu
	MenuBuilder.AddSubMenu(FText::FromString(TEXT("Export to PNG")),FText::FromString(TEXT("Export Current Asset Viewport to PNG.")),
		FNewMenuDelegate::CreateRaw(this,&FAssetToolModule::AddSubMenue));
	MenuBuilder.EndSection();
}

void FAssetToolModule::AddSubMenue(FMenuBuilder& MenuBuilder)
{
	MenuBuilder.BeginSection("AssetTool", FText::FromString(TEXT("AssetTool")));
	
	MenuBuilder.AddMenuEntry(FText::FromString(TEXT("Size 128")),FText::FromString(TEXT("Size 128")),FSlateIcon(),
						FUIAction(FExecuteAction::CreateRaw(this, &FAssetToolModule::Export128)));
	MenuBuilder.AddMenuEntry(FText::FromString(TEXT("Size 256")),FText::FromString(TEXT("Size 256")),FSlateIcon(),
						FUIAction(FExecuteAction::CreateRaw(this, &FAssetToolModule::Export256)));
	MenuBuilder.AddMenuEntry(FText::FromString(TEXT("Size 512")),FText::FromString(TEXT("Size 512")),FSlateIcon(),
						FUIAction(FExecuteAction::CreateRaw(this, &FAssetToolModule::Export512)));
	MenuBuilder.AddMenuEntry(FText::FromString(TEXT("Size 1024")),FText::FromString(TEXT("Size 1024")),FSlateIcon(),
						FUIAction(FExecuteAction::CreateRaw(this, &FAssetToolModule::Export1024)));
	MenuBuilder.AddMenuEntry(FText::FromString(TEXT("Size 2048")),FText::FromString(TEXT("Size 2048")),FSlateIcon(),
						FUIAction(FExecuteAction::CreateRaw(this, &FAssetToolModule::Export2048)));
	MenuBuilder.AddMenuEntry(FText::FromString(TEXT("Size 4096")),FText::FromString(TEXT("Size 4096")),FSlateIcon(),
						FUIAction(FExecuteAction::CreateRaw(this, &FAssetToolModule::Export4096)));
	
	MenuBuilder.EndSection();
}

bool FAssetToolModule::OpenExplor(const FString& Title, FString& InOutLastPath, FString& OutOpenFilenames)
{
	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
	bool opened = false; 
	if(!DesktopPlatform) return false;
	opened = DesktopPlatform->OpenDirectoryDialog(
			FSlateApplication::Get().FindBestParentWindowHandleForDialogs(nullptr),
			Title,
			InOutLastPath,
			OutOpenFilenames
			);
	return opened;
}

FString FAssetToolModule::SelectFilePath()
{
	FString Title = "Select Output Folder";
	FString OpenedFiles;
	FString DefaultLocation = FPaths::ProjectDir();
	DefaultLocation = FPaths::ConvertRelativePathToFull(DefaultLocation);
	
	bool bOpened = false;
	bOpened = OpenExplor(Title, DefaultLocation, OpenedFiles);//打开文件浏览器
	if(!bOpened) return FString("");
	
	FString ExportFileDir = FPaths::ConvertRelativePathToFull(OpenedFiles);
	return ExportFileDir;
}

void FAssetToolModule::ExportToPNG(int32 OutputSize)
{
	FString path = SelectFilePath();
	if(path == "") return;
	TArray<FAssetData> SelectedAssets;
	FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
	ContentBrowserModule.Get().GetSelectedAssets(SelectedAssets);//获得选中项
	
	for (const FAssetData& AssetData : SelectedAssets)
	{
		//UE_LOG(LogTemp, Log,TEXT("Exporting: %s"),*AssetData.AssetName.ToString());
		UObject* Asset = AssetData.GetAsset();
		if(Asset)
		{
			GetObjThumbnail(Asset, path, OutputSize);//导出
		}
	}

	
}

UTexture2D* FAssetToolModule::GetObjThumbnail(UObject *InObject,FString OutputPath, int32 OutputSize)
{
	
	int32 ImageRes = OutputSize;//TODO: 修改尺寸
	FObjectThumbnail ObjThumnail;
	ThumbnailTools::RenderThumbnail(InObject, ImageRes, ImageRes, ThumbnailTools::EThumbnailTextureFlushMode::AlwaysFlush, NULL, &ObjThumnail);
	TArray<uint8> ThumnailDatat = ObjThumnail.GetUncompressedImageData();
	FString TotalFileName = FPaths::Combine(*OutputPath, *(InObject->GetName()));//TODO: 修改路径
	TotalFileName += ".png";
	FArchive* Ar = IFileManager::Get().CreateFileWriter(*TotalFileName);
	TArray<FColor> ImageRawColor;
	for (int i =0; i < ThumnailDatat.Num(); i += 4)
	{
		ImageRawColor.Add(FColor(ThumnailDatat[i + 2], ThumnailDatat[i + 1], ThumnailDatat[i], ThumnailDatat[i + 3]));
	}
	IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
	TSharedPtr<IImageWrapper> ImageWrapper = ImageWrapperModule.CreateImageWrapper(EImageFormat::PNG);
	ImageWrapper->SetRaw(ImageRawColor.GetData(), ImageRawColor.GetAllocatedSize(), ImageRes, ImageRes, ERGBFormat::BGRA, 8);//BGRA
	const TArray64<uint8> ImageData = ImageWrapper->GetCompressed(100);
	const TArray<uint8> Convert = TArray<uint8>(ImageData);
	UTexture2D* ReTexture2d = UKismetRenderingLibrary::ImportBufferAsTexture2D(InObject->GetWorld(), Convert);//TODO: 如果需要texture2D的话
	
	Ar->Serialize((void*)ImageData.GetData(), ImageData.GetAllocatedSize());//导出
	delete Ar;
	
	const FString Text = FString::Format(TEXT("Exportint {0}!"),
				{
					*(InObject->GetName())
				});
	UKismetSystemLibrary::PrintString(GEditor->GetWorld(), Text);
	return ReTexture2d;
}


#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FAssetToolModule, AssetTool)