// Copyright (c) CD PROJEKT S.A.

#include "RedHermesLevelAtCameraCoordsEndpointEditorExtension.h"

#include "LevelEditorMenuContext.h"
#include "LevelEditorViewport.h"
#include "RedTalaria.h"
#include "RedTalariaLevelAtCameraCoordsUrls.h"
#include "ToolMenus.h"
#include "Misc/EngineVersionComparison.h"
#include "HAL/PlatformApplicationMisc.h"

#define LOCTEXT_NAMESPACE "Editor.RedHermesLevelAtCameraCoordsEndpointEditorExtension"

void URedHermesLevelAtCameraCoordsEndpointEditorExtension::Initialize(FSubsystemCollectionBase& Collection)
{
	RegisterViewportOptionMenuExtender();
}

void URedHermesLevelAtCameraCoordsEndpointEditorExtension::Deinitialize()
{
	UnregisterViewportOptionMenuExtender();
}

void URedHermesLevelAtCameraCoordsEndpointEditorExtension::RegisterViewportOptionMenuExtender()
{
	FToolMenuOwnerScoped ToolMenuOwnerScoped(this);

#if UE_VERSION_OLDER_THAN(5,6,0)
	static const FName ViewportMenuName("LevelEditor.LevelViewportToolBar.Options");
	static const FName ViewportMenuFirstSection("LevelViewportViewportOptions");
#else
	static const FName ViewportMenuName("LevelEditor.LevelViewportToolBar.Camera");
	static const FName ViewportMenuFirstSection("LevelViewportCameraType_Perspective");
#endif

	if (UToolMenu* Menu = UToolMenus::Get()->ExtendMenu(ViewportMenuName))
	{
		static auto GetPerspectiveLevelEditorViewportClient = [](const FToolMenuContext& MenuContext) -> FLevelEditorViewportClient* {
			ULevelViewportToolBarContext* Context = MenuContext.FindContext<ULevelViewportToolBarContext>();
			if (Context && Context->LevelViewportToolBarWidget.IsValid())
			{
				FLevelEditorViewportClient* ViewportClient = Context->GetLevelViewportClient();
				if (ViewportClient && ViewportClient->ViewportType == LVT_Perspective)
				{
					return ViewportClient;
				}
			}

			return nullptr;
		};

		FToolUIAction CopyLinkToLevelAtCameraCoordsAction;
		CopyLinkToLevelAtCameraCoordsAction.ExecuteAction.BindLambda([](const FToolMenuContext& MenuContext) {
			if (const FLevelEditorViewportClient* ViewportClient = GetPerspectiveLevelEditorViewportClient(MenuContext))
			{
				const FName LevelPackageName = ViewportClient->GetWorld()->GetPackage()->GetFName();
				const FVector Location = ViewportClient->GetViewLocation();
				const FRotator Rotation = ViewportClient->GetViewRotation();
				FPlatformApplicationMisc::ClipboardCopy(
					*FRedTalariaLevelAtCameraCoordsUrls::GetUrlForLevelAtLocationAtRotation(LevelPackageName, Location, Rotation));
			}
		});
		CopyLinkToLevelAtCameraCoordsAction.CanExecuteAction.BindLambda([](const FToolMenuContext& MenuContext) {
			return GetPerspectiveLevelEditorViewportClient(MenuContext) != nullptr;
		});

		FToolMenuSection& Section = Menu->AddSection(
			TEXT("Hermes"),
			LOCTEXT("LevelViewportToolBar.Section.Hermes", "Hermes"),
			FToolMenuInsert(ViewportMenuFirstSection, EToolMenuInsertType::Before));
		Section.AddMenuEntry("CopyURL",
			LOCTEXT("ViewportAction.CopyCameraCoordsUrl", "Copy camera coords URL"),
			LOCTEXT("ViewportAction.CopyCameraCoordsUrlTooltip", "Copy an URL that will open this level at the current camera position"),
			FSlateIcon(FRedTalariaModule::SlateStyleSetName, FRedTalariaModule::TalariaIconSlateStyleName),
			CopyLinkToLevelAtCameraCoordsAction);
	}
}

void URedHermesLevelAtCameraCoordsEndpointEditorExtension::UnregisterViewportOptionMenuExtender()
{
	UToolMenus::UnregisterOwner(this);
}

#undef LOCTEXT_NAMESPACE
