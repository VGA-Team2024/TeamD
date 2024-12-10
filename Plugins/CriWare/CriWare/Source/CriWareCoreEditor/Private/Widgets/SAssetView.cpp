/****************************************************************************
 *
 * CRI Middleware SDK
 *
 * Copyright (c) 2021 CRI Middleware Co., Ltd.
 *
 * Library  : CRIWARE plugin for Unreal Engine
 * Module   : CriWareCoreEditor
 * File     : SAssetView.cpp
 *
 ****************************************************************************/

#include "SAssetView.h"

#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "SAssetDropTarget.h"
#include "AssetThumbnail.h"
#include "ThumbnailRendering/ThumbnailManager.h"
#include "DragAndDrop/AssetDragDropOp.h"
#include "ActorFactories/ActorFactory.h"
#include "AssetSelection.h"

#include "SlateGlobals.h"

#define LOCTEXT_NAMESPACE "SAssetView"

SLATE_IMPLEMENT_WIDGET(SAssetView)
void SAssetView::PrivateRegisterAttributes(FSlateAttributeInitializer& AttributeInitializer)
{
	//~ If we are in SimpleTextMode the invalidation reason can be different.
	/*struct FInvalidation
	{
		static EInvalidateWidgetReason GetInvalidationNoneIfSimpleTextMode(const SWidget& OwningWidget)
		{
			return static_cast<const STextBlock&>(OwningWidget).bSimpleTextMode ? EInvalidateWidgetReason::None : EInvalidateWidgetReason::Layout;
		}
		static EInvalidateWidgetReason GetInvalidationPaintIfSimpleTextMode(const SWidget& OwningWidget)
		{
			return static_cast<const STextBlock&>(OwningWidget).bSimpleTextMode ? EInvalidateWidgetReason::Paint : EInvalidateWidgetReason::Layout;
		}
	};*/

	//SLATE_ADD_MEMBER_ATTRIBUTE_DEFINITION(AttributeInitializer, BoundObject, EInvalidateWidgetReason::Layout);
	//SLATE_ADD_MEMBER_ATTRIBUTE_DEFINITION(AttributeInitializer, DisplayThumbnail, EInvalidateWidgetReason::Layout);
	//SLATE_ADD_MEMBER_ATTRIBUTE_DEFINITION(AttributeInitializer, ThumbnailSize, EInvalidateWidgetReason::Layout);
}

SAssetView::SAssetView()
	: Object(nullptr)
	, bIsAttributeBoundObjectBound(false)
	, bDisplayThumbnail(true)
	, ThumbnailSize(64.0f)
	, bOpenAssetOnDoubleClick(false)
	, bAllowAssetDragging(false)
	, bAllowAssetDropping(false)
{
	//SetCanTick(false);
}

SAssetView::~SAssetView()
{}

void SAssetView::Construct(const FArguments& InArgs)
{
	bDisplayThumbnail = InArgs._DisplayThumbnail;
	ThumbnailSize = InArgs._ThumbnailSize;
	bOpenAssetOnDoubleClick = InArgs._OpenAssetOnDoubleClick;
	bAllowAssetDragging = InArgs._AllowAssetDragging;
	bAllowAssetDropping = InArgs._AllowAssetDropping;
	OnAssetDropped = InArgs._OnAssetDropped;
	OnIsAssetAcceptableForDropWithReason = InArgs._OnIsAssetAcceptableForDropWithReason;
	SetObject(InArgs._Object);
}

void SAssetView::SetObject(TAttribute<UObject*> InObject)
{
	// Cache the IsBound.
	//When the attribute is not bound, we need to go through all the other bound property to check if it is bound.
	bIsAttributeBoundObjectBound = InObject.IsBound();
	Object = MoveTemp(InObject);

	ChildSlot
	[
		CreateAssetView(bDisplayThumbnail, ThumbnailSize)
	];
}

TSharedRef<SWidget> SAssetView::CreateAssetView(bool bInDisplayThumbnail, float InThumbnailSize)
{
	TSharedPtr<SHorizontalBox> ValueContentBox = nullptr;
	auto Widget = SAssignNew(ValueContentBox, SHorizontalBox);

	TObjectPtr<UObject> AssetObject = Object.Get();

	if (bInDisplayThumbnail)
	{
		const TSharedPtr<FAssetThumbnail> AssetThumbnail = 
			MakeShareable(
				new FAssetThumbnail(
					AssetObject,
					InThumbnailSize, 
					InThumbnailSize, 
					UThumbnailManager::Get().GetSharedThumbnailPool()
				)
			);
	
		FAssetThumbnailConfig AssetThumbnailConfig;
		AssetThumbnailConfig.bAllowFadeIn = false;
		AssetThumbnailConfig.bAllowRealTimeOnHovered = false;
		AssetThumbnailConfig.bForceGenericThumbnail = false;
		AssetThumbnailConfig.bAllowAssetSpecificThumbnailOverlay = true;

		/*TSharedPtr<IAssetTypeActions> AssetTypeActions;
		FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>(TEXT("AssetTools"));
		AssetTypeActions = AssetToolsModule.Get().GetAssetTypeActionsForClass(Object->StaticClass()).Pin();
		if (AssetTypeActions.IsValid())
		{
			AssetThumbnailConfig.AssetTypeColorOverride = AssetTypeActions->GetTypeColor();
		}*/

		ValueContentBox->AddSlot()
			.Padding(0,3,5,0)
			.AutoWidth()
			.VAlign(VAlign_Center)
			[
				SNew(SBorder)
				.Visibility(EVisibility::SelfHitTestInvisible)
				.Padding(FMargin(0, 0, 4, 4))
				.BorderImage(FAppStyle::Get().GetBrush("PropertyEditor.AssetTileItem.DropShadow"))
				[
					SAssignNew(ThumbnailOverlay, SOverlay)
					+SOverlay::Slot()
					.Padding(1)
					[
						SNew(SBorder)
						.Padding(0)
						.BorderImage(FStyleDefaults::GetNoBrush())
						//.OnMouseDoubleClick(this, &SPropertyEditorAsset::OnAssetThumbnailDoubleClick)
						[
							SNew(SBox)
							//.ToolTipText(TooltipAttribute)
							.WidthOverride(InThumbnailSize)
							.HeightOverride(InThumbnailSize)
							[
								AssetThumbnail->MakeThumbnailWidget(AssetThumbnailConfig)
							]
						]
					]
				]
			];
	}

	FName ObjectName = NAME_None;
	if (AssetObject)
	{
		ObjectName = AssetObject->GetFName();
	}

	ValueContentBox->AddSlot()
		.Padding(0)
		.VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.Text(FText::FromName(ObjectName))
			.ToolTipText(FText::FromName(ObjectName))
			//.Font(StructCustomizationUtils.GetRegularFont())
		];

	if (bAllowAssetDropping)
	{
		return SNew(SAssetDropTarget)
			.OnAreAssetsAcceptableForDropWithReason_Lambda([&](TArrayView<FAssetData> InAssets, FText& OutReason) -> bool
			{
				if (OnIsAssetAcceptableForDropWithReason.IsBound() && InAssets.Num() > 0)
				{
					return OnIsAssetAcceptableForDropWithReason.Execute(InAssets[0], OutReason);
				}
				return false;
			})
			.OnAssetsDropped_Lambda([&](const FDragDropEvent& Event, TArrayView<FAssetData> InAssets) -> void
			{
				if (InAssets.Num() > 0)
				{
					OnAssetDropped.ExecuteIfBound(Event, InAssets[0]);
				}
			})
			.Content()
			[
				Widget
			];
	}

	return Widget;
}

FReply SAssetView::OnMouseButtonDoubleClick(const FGeometry& Geometry, const FPointerEvent& PointerEvent)
{
	bool bOpenAssset = false;

	if (bOpenAssetOnDoubleClick && PointerEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		if (bDisplayThumbnail)
		{
			bOpenAssset = ThumbnailOverlay->IsHovered();
		}
		else
		{
			bOpenAssset = true;
		}
	}

	if (bOpenAssset)
	{
		TObjectPtr<UObject> AssetObject = Object.Get();
		if (AssetObject && GEditor)
		{
			GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->OpenEditorForAsset(AssetObject);
			return FReply::Handled();
		}
	}

	return FReply::Unhandled();
}

FReply SAssetView::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (bAllowAssetDragging && MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		return FReply::Handled().DetectDrag(SharedThis(this), EKeys::LeftMouseButton);
	}

	return FReply::Unhandled();
}

FReply SAssetView::OnDragDetected(const FGeometry& Geometry, const FPointerEvent& PointerEvent)
{
		TObjectPtr<UObject> AssetObject = Object.Get();
		if (AssetObject)
		{
			// Spawn a loaded asset using the registered actor factory if found. Just like any other asset from the Content Browser.
			const FAssetData AssetData = FAssetData(AssetObject);
			if (AssetData.IsValid())
			{
				UActorFactory* ActorFactory = FActorFactoryAssetProxy::GetFactoryForAssetObject(AssetObject);
				auto DragDropOp = FAssetDragDropOp::New(AssetData, ActorFactory);

				return FReply::Handled().BeginDragDrop(DragDropOp);
			}
		}

	return FReply::Unhandled();
}

#undef LOCTEXT_NAMESPACE
