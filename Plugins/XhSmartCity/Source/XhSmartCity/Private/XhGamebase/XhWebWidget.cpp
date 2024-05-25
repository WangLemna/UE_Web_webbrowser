// Fill out your copyright notice in the Description page of Project Settings.


#include "XhGamebase/XhWebWidget.h"
#include "WebBrowser.h"
#include "TimerManager.h"
#include "Framework/Application/SlateApplication.h"
#include "GameFramework/GameUserSettings.h"
#include "Kismet/GameplayStatics.h"

//#include "Windows/AllowWindowsPlatformTypes.h"
//#include "Windows/PreWindowsApi.h"
//#include <winsock2.h>
//#include <WinUser.h>
//#include "Windows/PostWindowsApi.h"
//#include "Windows/HideWindowsPlatformTypes.h"
void UXhWebWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	//WebBrowser->SetIsEnabled(IsWidgetCanClick());//SetVisibility
	WebBrowser->SetVisibility(IsWidgetCanClick() ? ESlateVisibility::Visible : ESlateVisibility::HitTestInvisible);
}

void UXhWebWidget::ResizedTimerFun()
{
	if (SocketServerManager)
	{
		SocketServerManager->SendAll(TEXT("{\"event\":\"GetWebSize\"}"));
	}
}

void UXhWebWidget::NativeConstruct()
{
	Super::NativeConstruct();
	//int32 width = GetSystemMetrics(SM_CXSCREEN);
	//int32 height = GetSystemMetrics(SM_CYSCREEN);
	//ScreenWidth = GetSystemMetrics(SM_CXSCREEN);
	HtmlWidth = 1.f;
	Zoom = 1.f;
	if (APlayerController * PlayerController = UGameplayStatics::GetPlayerController(this, 0))
	{
		FInputModeGameAndUI InputModeGameAndUI;
		InputModeGameAndUI.SetWidgetToFocus(nullptr)
			.SetLockMouseToViewportBehavior(EMouseLockMode::LockAlways)
			.SetHideCursorDuringCapture(false);
		PlayerController->SetInputMode(InputModeGameAndUI);
	}
	//UGameplayStatics::GetPlayerController(this, 0)->SetInputMode(FInputModeGameAndUI(nullptr, EMouseLockMode::LockAlways,false));
	GEngine->GameViewport->Viewport->ViewportResizedEvent.AddUObject(this, &UXhWebWidget::ViewportResized);
}

void UXhWebWidget::XhResized()
{
	WebBrowserPosition = FVector2D(WebBrowser->GetCachedWidget()->GetTickSpaceGeometry().GetAbsolutePosition());
	WebBrowserSize = FVector2D(WebBrowser->GetCachedWidget()->GetTickSpaceGeometry().GetAbsoluteSize());
	Zoom = WebBrowserSize.X / HtmlWidth;
	SetCanClickArea();
	UE_LOG(LogTemp, Log, TEXT("web页面尺寸为： %f"), HtmlWidth);
	UE_LOG(LogTemp, Log, TEXT("widget位置为： %s"), *WebBrowserPosition.ToString());
	UE_LOG(LogTemp, Log, TEXT("widget尺寸为： %s"), *WebBrowserSize.ToString());
	UE_LOG(LogTemp, Log, TEXT("缩放比例为： %f"), Zoom);
	for (int32 j = 0; j < CanClickArea.Num(); j++)
	{
		UE_LOG(LogTemp, Log, TEXT("可点击区域为： %s"), *CanClickArea[j].ToString());
	}
}

void UXhWebWidget::SetCanClickArea()
{
	CanClickArea.Empty();
	for (int32 i = 0; i < CanClickAreaOriginalData.Num(); i++)
	{
		CanClickArea.Add(ClickArea(CanClickAreaOriginalData[i].MinArea * Zoom, CanClickAreaOriginalData[i].MaxArea * Zoom));
	}
}

void UXhWebWidget::ViewportResized(FViewport* ViewPort, uint32 InWidth)
{
	if (ResizedTimerHandle.IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(ResizedTimerHandle);
	}
	GetWorld()->GetTimerManager().SetTimer(ResizedTimerHandle, this, &UXhWebWidget::ResizedTimerFun, GetWorld()->GetDeltaSeconds() * 5, false);
}

bool UXhWebWidget::IsWidgetCanClick()
{
	FVector2D MousePosition = FVector2D();
	if (FSlateApplication::IsInitialized())
	{
		MousePosition = FSlateApplication::Get().GetCursorPos() - WebBrowserPosition;
	}
	else
	{
		return false;
	}
	//FVector2D MousePosition = UWidgetLayoutLibrary::GetMousePositionOnViewport(this);
	for (auto& area : CanClickArea)
	{
		if (ClickArea::IsContain(MousePosition, area))
		{
			return true;
		}
	}
	return false;
}
