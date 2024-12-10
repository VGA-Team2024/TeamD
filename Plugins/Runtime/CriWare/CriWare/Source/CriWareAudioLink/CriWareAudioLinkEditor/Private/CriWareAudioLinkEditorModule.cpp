/****************************************************************************
 *
 * CRI Middleware SDK
 *
 * Copyright (c) 2021 CRI Middleware Co., Ltd.
 *
 * Library  : CRIWARE AudioLInk plugin for Unreal Engine
 * Module   : IModuleInterface Class for CriWareAudioLinkEditor module
 * File     : CriWareAudioLinkEditorModule.cpp
 *
 ****************************************************************************/

#include "CriWareAudioLinkEditorModule.h"

#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"
#include "Modules/ModuleManager.h"

#include "CriWareAudioLinkEditorPrivate.h"
#include "AssetTools/AtomAudioLinkSettingsActions.h"
#include "Customizations/AtomAudioLinkSettingsDetails.h"

DEFINE_LOG_CATEGORY(LogCriWareAudioLinkEditor);

//class FCriWareAudioLinkEditorModule
//	: public IModuleInterface
//{
//public:

FCriWareAudioLinkEditorModule::FCriWareAudioLinkEditorModule()
	//virtual void StartupModule() override
	{
		// Register the editor plugin asset type actions.
		IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();

		// Find or register ADX advanced category in context menu.
		auto AdxAssetCategoryBit = AssetTools.RegisterAdvancedAssetCategory(FName(TEXT("CriWare ADX")), FText::FromString(TEXT("ADX Atom")));

		// Asset Type Actions
		auto RegisterTypeAction = [this, InAssetTools = &AssetTools](const TSharedRef<IAssetTypeActions> Actions)
		{
			InAssetTools->RegisterAssetTypeActions(Actions);
			RegisteredAssetTypeActions.Add(Actions);
		};

		RegisterTypeAction(MakeShared<FAtomAudioLinkSettingsActions>(AdxAssetCategoryBit));

		FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>(TEXT("PropertyEditor"));

		// Custom Class Layouts
		auto RegisterCustomClass = [this, InPropertyModule = &PropertyModule](FName Name, FOnGetDetailCustomizationInstance InstanceGetter)
		{
			InPropertyModule->RegisterCustomClassLayout(Name, InstanceGetter);
			CustomClassLayoutNames.Add(Name);
		};

		RegisterCustomClass("AtomAudioLinkSettings", FOnGetDetailCustomizationInstance::CreateStatic(&FAtomAudioLinkSettingsDetails::MakeInstance));
	}

FCriWareAudioLinkEditorModule::~FCriWareAudioLinkEditorModule()
	//virtual void ShutdownModule() override
	{
		// Unregister Custom Class Layouts
		if (FModuleManager::Get().IsModuleLoaded("PropertyEditor"))
		{
			FPropertyEditorModule& PropertyModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");

			for (FName ClassName : CustomClassLayoutNames)
			{
				PropertyModule.UnregisterCustomClassLayout(ClassName);
			}
			CustomClassLayoutNames.Reset();
		}

		// Unregister Asset Type Actions
		if (FModuleManager::Get().IsModuleLoaded("AssetTools"))
		{
			IAssetTools& AssetToolsModule = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools").Get();

			for (auto Action : RegisteredAssetTypeActions)
			{
				AssetToolsModule.UnregisterAssetTypeActions(Action);
			}
		}
	}

//private:

	/** The collection of registered asset type actions. */
	//TArray<TSharedRef<IAssetTypeActions>> RegisteredAssetTypeActions;

	/** The collection of registered customized class names. */
	//TArray<FName> CustomClassLayoutNames;
//};

//IMPLEMENT_MODULE(FCriWareAudioLinkEditorModule, CriWareAudioLinkEditor)
