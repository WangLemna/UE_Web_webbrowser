// Fill out your copyright notice in the Description page of Project Settings.


#include "XhGamebase/XhGameMode.h"
#include "XhWebSocket/XhWebSocketManager.h"
#include "Blueprint/UserWidget.h"
#include "XhGamebase/XhPawn.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "XhGamebase/WSS_Manager.h"

AXhGameMode::AXhGameMode()
{
	bLoadWeb = false;
	bUseSocket = false;
}

void AXhGameMode::BeginPlay()
{
	Super::BeginPlay();
	if (bLoadWeb)
	{
		WebWidget = CreateWidget<UUserWidget>(GetWorld(), WebUI);
		if (WebWidget != nullptr)
		{
			WebWidget->AddToViewport();
		}
	}
	if (bUseSocket)
	{
		AXhWebSocketManager* SocketServerActor = GetWorld()->SpawnActor<AXhWebSocketManager>(SocketServer);
		AWSS_Manager* WSS_Manager = Cast<AWSS_Manager>(SocketServerActor);
		if (WebWidget && WSS_Manager)
		{
			UXhWebWidget* XhWebWidget = Cast<UXhWebWidget>(WebWidget);
			WSS_Manager->XhWebWidget = XhWebWidget;
			XhWebWidget->SocketServerManager = WSS_Manager;
			AXhPawn* XhPawn = Cast<AXhPawn>(UGameplayStatics::GetPlayerPawn(this, 0));
			if (XhPawn != nullptr)
			{
				XhPawn->SocketServerManager = WSS_Manager;
			}
		}
	}
}
