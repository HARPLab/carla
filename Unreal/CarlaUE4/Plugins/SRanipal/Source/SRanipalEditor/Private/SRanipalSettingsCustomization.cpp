//========= Copyright 2019-2020, HTC Corporation. All rights reserved. ===========


#include "SRanipalSettingsCustomization.h"
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Text/SRichTextBlock.h"
#include "DetailWidgetRow.h"

#define LOCTEXT_NAMESPACE "SRanipalSettings"
TSharedRef<IDetailCustomization> FSRanipalSettingsCustomization::MakeInstance()
{
	return MakeShareable(new FSRanipalSettingsCustomization);
}

void FSRanipalSettingsCustomization::CustomizeDetails(IDetailLayoutBuilder & DetailLayout)
{
	IDetailCategoryBuilder& EyeCategory = DetailLayout.EditCategory(TEXT("Eye Settings"));
	IDetailCategoryBuilder& LipCategory = DetailLayout.EditCategory(TEXT("Lip Settings"));

	EyeCategory.AddCustomRow(LOCTEXT("EyeUsageInfo", "Eye Usage Info"), false)
		.WholeRowWidget
		[
			SNew(SBorder)
			.Padding(1)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.Padding(FMargin(10, 10, 10, 10))
				.FillWidth(1.0f)
				[
					SNew(SRichTextBlock)
					.Text(LOCTEXT("EyeUsageInfoMessage", "<RichTextBlock.TextHighlight>NOTE:</> Enable \"Enable Eye by Default\" setting will start SRanipal eye framework automatically when startup. \n If you want to self control SRanipal eye framework situation by function blueprint, please <RichTextBlock.TextHighlight>disable</> this setting."))
					.TextStyle(FEditorStyle::Get(), "MessageLog")
					.DecoratorStyleSet(&FEditorStyle::Get())
					.AutoWrapText(true)
				]
			]
		];

	LipCategory.AddCustomRow(LOCTEXT("LipUsageInfo", "Lip Usage Info"), false)
		.WholeRowWidget
		[
			SNew(SBorder)
			.Padding(1)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.Padding(FMargin(10, 10, 10, 10))
				.FillWidth(1.0f)
				[
					SNew(SRichTextBlock)
					.Text(LOCTEXT("LipUsageInfoMessage", "<RichTextBlock.TextHighlight>NOTE:</> Enable \"Enable Lip by Default\" setting will start SRanipal lip framework automatically when startup. \n If you want to self control SRanipal lip framework situation by function blueprint, please <RichTextBlock.TextHighlight>disable</> this setting."))
					.TextStyle(FEditorStyle::Get(), "MessageLog")
					.DecoratorStyleSet(&FEditorStyle::Get())
					.AutoWrapText(true)
				]
			]
		];

}

FSRanipalSettingsCustomization::FSRanipalSettingsCustomization()
{
}

