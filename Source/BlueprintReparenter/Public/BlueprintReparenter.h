#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FBlueprintReparenterModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	void RegisterMenuExtender();
	TSharedRef<class FExtender> OnExtendContentBrowserMenu(const TArray<FAssetData>& SelectedAssets);
	void AddReparentOption(class FMenuBuilder& MenuBuilder, TArray<FAssetData> SelectedAssets);
	void OnReparentBlueprints(TArray<FAssetData> BlueprintAssets);
	static TSharedRef<FExtender> OnExtendContentBrowserMenu_Static(const TArray<FAssetData>& SelectedAssets);
};
