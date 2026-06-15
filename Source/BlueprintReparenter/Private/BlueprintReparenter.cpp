#include "BlueprintReparenter.h"
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "ClassViewerModule.h"
#include "ClassViewerFilter.h"
#include "Widgets/SWindow.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Framework/Application/SlateApplication.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Engine/Blueprint.h"  
#include "Modules/ModuleManager.h"  

void FBlueprintReparenterModule::StartupModule()
{
	// Register the menu extender when ContentBrowser module is available
	if (FModuleManager::Get().IsModuleLoaded("ContentBrowser"))
	{
		RegisterMenuExtender();
	}
	else
	{
		// Wait for ContentBrowser to load
		FModuleManager::Get().OnModulesChanged().AddLambda([this](FName ModuleName, EModuleChangeReason Reason)
			{
				if (ModuleName == "ContentBrowser" && Reason == EModuleChangeReason::ModuleLoaded)
				{
					RegisterMenuExtender();
				}
			});
	}
}

void FBlueprintReparenterModule::RegisterMenuExtender()
{
	FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
	TArray<FContentBrowserMenuExtender_SelectedAssets>& MenuExtenderDelegates = ContentBrowserModule.GetAllAssetViewContextMenuExtenders();
	MenuExtenderDelegates.Add(FContentBrowserMenuExtender_SelectedAssets::CreateRaw(this, &FBlueprintReparenterModule::OnExtendContentBrowserMenu));
}

void FBlueprintReparenterModule::ShutdownModule()
{
}

TSharedRef<FExtender> FBlueprintReparenterModule::OnExtendContentBrowserMenu(const TArray<FAssetData>& SelectedAssets)
{
	TSharedRef<FExtender> Extender = MakeShared<FExtender>();

	for (const FAssetData& SelectedAsset : SelectedAssets)
	{
		if (!SelectedAsset.GetClass()->IsChildOf(UBlueprint::StaticClass()))
		{
			return Extender;
		}
	}

	Extender->AddMenuExtension(
		"GetAssetActions",
		EExtensionHook::After,
		nullptr,
		FMenuExtensionDelegate::CreateRaw(this, &FBlueprintReparenterModule::AddReparentOption, SelectedAssets)
	);

	return Extender;
}

void FBlueprintReparenterModule::AddReparentOption(FMenuBuilder& MenuBuilder, TArray<FAssetData> SelectedAssets)
{
	MenuBuilder.AddMenuEntry(
		SelectedAssets.Num() > 1 ? FText::FromString("Reparent Blueprints...") : FText::FromString("Reparent Blueprint..."),
		SelectedAssets.Num() > 1 ? FText::FromString("Change these Blueprints parent classes.") : FText::FromString("Change this Blueprint's parent class."),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateRaw(this, &FBlueprintReparenterModule::OnReparentBlueprints, SelectedAssets))
	);
}

void FBlueprintReparenterModule::OnReparentBlueprints(TArray<FAssetData> BlueprintAssets)
{
	// Open a class picker window
	FClassViewerInitializationOptions Options;
	Options.Mode = EClassViewerMode::ClassPicker;
	Options.bShowUnloadedBlueprints = true;
	Options.bShowNoneOption = false;
	Options.DisplayMode = EClassViewerDisplayMode::DefaultView;
	Options.NameTypeToDisplay = EClassViewerNameTypeToDisplay::Dynamic;

	FClassViewerModule& ClassViewerModule = FModuleManager::LoadModuleChecked<FClassViewerModule>("ClassViewer");

	TSharedPtr<SWindow> PickerWindow;
	PickerWindow = SNew(SWindow)
		.Title(FText::FromString("Select New Parent Class"))
		.ClientSize(FVector2D(400, 600))
		.SupportsMinimize(false)
		.SupportsMaximize(false)
		.Content()
		[
			ClassViewerModule.CreateClassViewer(
				Options,
				FOnClassPicked::CreateLambda([this, PickerWindow, BlueprintAssets](UClass* NewParentClass)
				{
					if (PickerWindow.IsValid())
						PickerWindow->RequestDestroyWindow();

					for (const FAssetData& BlueprintAsset : BlueprintAssets)
					{
						UBlueprint* BP = Cast<UBlueprint>(BlueprintAsset.GetAsset());
						if (!BP)
							continue;

						if (!NewParentClass || BP->ParentClass == NewParentClass)
							continue;

						// 🔧 Safely reparent Blueprint
						BP->ParentClass = NewParentClass;

						// Mark dirty and recompile to apply changes
						FKismetEditorUtilities::CompileBlueprint(BP);
						BP->MarkPackageDirty();

						// Notify the asset registry & editor
						FAssetRegistryModule::AssetCreated(BP);
						BP->PostEditChange();
					}
				})
			)
		];

	FSlateApplication::Get().AddWindow(PickerWindow.ToSharedRef());
}

TSharedRef<FExtender> FBlueprintReparenterModule::OnExtendContentBrowserMenu_Static(const TArray<FAssetData>& SelectedAssets)
{
	if (FBlueprintReparenterModule* ThisModule = FModuleManager::GetModulePtr<FBlueprintReparenterModule>("BlueprintReparenter"))
	{
		return ThisModule->OnExtendContentBrowserMenu(SelectedAssets);
	}
	return MakeShared<FExtender>();
}
IMPLEMENT_MODULE(FBlueprintReparenterModule, BlueprintReparenter)