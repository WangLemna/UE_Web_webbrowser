// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "XhWebSocket/XhWebSocketManager.h"
#include "WSS_Manager.generated.h"

/**
 * 
 */
class UXhWebWidget;
USTRUCT(BlueprintType)
struct FActorRef : public  FTableRowBase
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ActorRef")
	FString Name;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ActorRef")
	TSubclassOf<AActor> ActorRef;
};

UCLASS()
class XHSMARTCITY_API AWSS_Manager : public AXhWebSocketManager
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "XhGameMode")
	TMap<FString, UClass*> ActorRefMap;

	UPROPERTY(EditAnywhere, Category = "XhGameMode")
	UDataTable* DT_ActorRef;

	TMap<FString, int32> EventNameMap;
	UXhWebWidget* XhWebWidget;
public:

	//接收客户端的XhCustomEvent
	UFUNCTION(BlueprintImplementableEvent, Category = "XhWebSocket")
	void XhCustomEvent(const FJsonObjectWrapper& InJsonObject);

	//接收客户端发来的string
	UFUNCTION(BlueprintImplementableEvent, Category = "XhWebSocket")
	void XhStringMsg(const FString& msg);
private:
	// 处理将接收到的数据。
	void ReceivedMsg(void* Data, int32 Size, FGuid ClientId) override;
	bool GetActorRefData();
	void InitEventNameMap();
	FVector JsonArrayToVector(const TSharedPtr<FJsonObject>& InJsonObj, const FString& InName);
	FRotator JsonArrayToRotator(const TSharedPtr<FJsonObject>& InJsonObj, const FString& InName);
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void BeginDestroy() override;

};
