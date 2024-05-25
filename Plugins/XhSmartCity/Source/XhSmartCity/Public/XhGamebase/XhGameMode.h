// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "XhGameMode.generated.h"

/**
 * 
 */
class UUserWidget;
UCLASS()
class XHSMARTCITY_API AXhGameMode : public AGameMode
{
	GENERATED_BODY()
public:

	//是否加载Web？用于调试时不需要WebUI的时候使用
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "XhGameMode")
	bool bLoadWeb;

	//是否使用WebSocket
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "XhGameMode")
	bool bUseSocket;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "XhGameMode")
	TSubclassOf<UUserWidget> WebUI;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "XhGameMode")
	TSubclassOf<class AXhWebSocketManager> SocketServer;

protected:
	UUserWidget* WebWidget;
	AXhGameMode();
	virtual void BeginPlay() override;
};
