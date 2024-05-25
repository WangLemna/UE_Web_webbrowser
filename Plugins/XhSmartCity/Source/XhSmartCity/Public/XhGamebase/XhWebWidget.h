// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "XhGamebase/WSS_Manager.h"
#include "XhWebWidget.generated.h"

/**
 * 
 */

//点击区域
struct ClickArea {
public:
	FVector2D MinArea;
	FVector2D MaxArea;
	ClickArea(FVector2D InMinArea, FVector2D InMaxArea) {
		MinArea = InMinArea;
		MaxArea = InMaxArea;
	}
public:
	FString ToString() {
		return "MinArea: " + MinArea.ToString() + "MaxArea: " + MaxArea.ToString();
	}
	bool operator==(ClickArea Area) {
		return Area.MinArea.Equals(MinArea, 1) && Area.MaxArea.Equals(MaxArea, 1);
	}
	static bool IsContain(FVector2D InPosition, ClickArea Area)
	{
		return InPosition.X > Area.MinArea.X
			&& InPosition.Y > Area.MinArea.Y
			&& InPosition.X < Area.MaxArea.X
			&& InPosition.Y < Area.MaxArea.Y;
	}
};

class UWebBrowser;
struct FTimerHandle;
UCLASS()
class XHSMARTCITY_API UXhWebWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	//UXhWebWidget();
	UPROPERTY(meta = (BindWidget))
	UWebBrowser* WebBrowser;
	TArray<ClickArea> CanClickAreaOriginalData;
	TArray<ClickArea> CanClickArea;

	float HtmlWidth;
	float Zoom;
	FVector2D WebBrowserPosition;
	FVector2D WebBrowserSize;
	AWSS_Manager* SocketServerManager;
	FTimerHandle ResizedTimerHandle;
protected:
	void NativeConstruct() override;
	void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
public:
	bool IsWidgetCanClick();
	void XhResized();
	void ViewportResized(FViewport* InViewPort, uint32 InWidth);
	void SetCanClickArea();
	void ResizedTimerFun();
};
