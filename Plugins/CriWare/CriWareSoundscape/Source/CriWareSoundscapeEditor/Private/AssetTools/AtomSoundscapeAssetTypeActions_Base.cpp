/****************************************************************************
 *
 * CRI Middleware SDK
 *
 * Copyright (c) 2021 CRI Middleware Co., Ltd.
 *
 * Library  : CRIWARE plugin for Unreal Engine
 * Module   : CriWareSoundscape
 * File     : AtomSoundscapeAssetTypeActions_Base.cpp
 *
 ****************************************************************************/

#include "AtomSoundscapeAssetTypeActions_Base.h"

#include "Styling/SlateStyleRegistry.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Images/SImage.h"

TSharedPtr<SWidget> FAtomSoundscapeAssetTypeActions_Base::GetThumbnailOverlay(const FAssetData& AssetData) const
{
	if (bEnableIconOverlay)
	{
		const ISlateStyle* AtomEditorStyle = FSlateStyleRegistry::FindSlateStyle("AtomEditorStyle");
		if (ensure(AtomEditorStyle))
		{
			const FSlateBrush* Icon = AtomEditorStyle->GetBrush("CriWare.ADXThumbnail"); // should be better with svg
			const float Ratio = 0.3f;

			return SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.FillWidth(1.0f - Ratio)
				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Fill)
				.FillWidth(Ratio)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.FillHeight(1.0f - Ratio)
					+ SVerticalBox::Slot()
					.VAlign(VAlign_Fill)
					.FillHeight(Ratio)
					[
						SNew(SBorder)
						.BorderImage(FAppStyle::GetNoBrush())
						.Visibility(EVisibility::HitTestInvisible)
						.Padding(FMargin(0.0f, 0.0f, 6.0f, 6.0f))
						.HAlign(HAlign_Fill)
						.VAlign(VAlign_Fill)
						[
							SNew(SImage)
							.Image(Icon)
						]
					]
				];
		}
	}

	return SNullWidget::NullWidget;
}