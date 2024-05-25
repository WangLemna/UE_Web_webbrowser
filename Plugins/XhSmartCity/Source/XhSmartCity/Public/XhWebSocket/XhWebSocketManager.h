// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "INetworkingWebSocket.h"
#include "XhWebSocketManager.generated.h"


struct FJsonObjectWrapper;
class IWebSocketServer;
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnConnected_C);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnReceivedMessage_C, const FString&, Message);
//DECLARE_DYNAMIC_DELEGATE_ThreeParams(FOnReceivedMessage_S, const FString&, Data, int32, Size, FGuid, ClientId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnConnectionError_C, const FString&, Error);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnClosed_C, int32, Code, const FString&, Reason, bool, bWasClean);
UCLASS()
class XHSMARTCITY_API AXhWebSocketManager : public AActor
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category = "XhWebSocket")
	FOnConnected_C OnConnected_C;

	UPROPERTY(BlueprintAssignable, Category = "XhWebSocket")
	FOnReceivedMessage_C OnReceivedMessage_C;

	UPROPERTY(BlueprintAssignable, Category = "XhWebSocket")
	FOnConnectionError_C OnConnectionError_C;

	UPROPERTY(BlueprintAssignable, Category = "XhWebSocket")
	FOnClosed_C OnClosed_C;

	//FOnReceivedMessage_S OnReceivedMessage_S;

	UFUNCTION()
	void OnConnected_C_Callback();

	UFUNCTION()
	void OnConnectionError_C_Callback(const FString& Error);

	UFUNCTION()
	void OnReceivedMessage_C_Callback(const FString& Message);

	UFUNCTION()
	void OnClosed_C_Callback(int32 Code, const FString& Reason, bool bWasClean);

public:
	// 开启WebSocket服务器
	UFUNCTION(BlueprintCallable, Category = "XhWebSocket")
	bool StartServer(int Port);

	// 关闭WebSocket服务器 BeginDestroy时自动调用
	UFUNCTION(BlueprintCallable, Category = "XhWebSocket")
	void StopServer();

	//客户端已经连接成功
	UFUNCTION(BlueprintImplementableEvent, Category = "XhWebSocket")
	void BP_ClientSucceedConnected(const FGuid& ClientId);

	// 按客户端id，发送信息
	UFUNCTION(BlueprintCallable, Category = "XhWebSocket")
	void SendById(const FGuid& InTargetClientId, const FString SendMsg);

	// 发送信息到所有客户端 
	UFUNCTION(BlueprintCallable, Category = "XhWebSocket")
	void SendAll(const FString SendMsg);

	// 在actor tick内自动调动 用于维护websocket的连接
	bool ServerTick();


	// 返回服务器当前是否正在运行
	UFUNCTION(BlueprintCallable, Category = "WebSocketServer")
	bool IsRunning() const;

	// 当连接结束时的回调
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnConnectionClosed, FGuid /*ClientId*/);
	FOnConnectionClosed& OnConnectionClosed() { return OnConnectionClosedDelegate; }

	// 打印整合 屏幕及log
	void XhDebugLog(FString msg, float delayTime, FColor color);


	private:
		// 当websocket客户端已连接
		
		void OnWebSocketClientConnected(INetworkingWebSocket* InSocket);


		// 当websocket客户端已断开
		void OnSocketClose(INetworkingWebSocket* InSocket);

protected:
		// 处理将接收到的数据。
	virtual void ReceivedMsg(void* Data, int32 Size, FGuid ClientId);


private:
	//websocket连接管理
	class FWebSocketConnection
	{
		public:

			explicit FWebSocketConnection(INetworkingWebSocket* InSocket)
				: Socket(InSocket)
				, Id(FGuid::NewGuid())
			{
			}

			FWebSocketConnection(FWebSocketConnection&& WebSocketConnection)
				: Id(WebSocketConnection.Id)
			{
				Socket = WebSocketConnection.Socket;
				WebSocketConnection.Socket = nullptr;
			}

			~FWebSocketConnection()
			{
				if (Socket)
				{
					delete Socket;
					Socket = nullptr;
				}
			}

			FWebSocketConnection(const FWebSocketConnection&) = delete;
			FWebSocketConnection& operator=(const FWebSocketConnection&) = delete;
			FWebSocketConnection& operator=(FWebSocketConnection&&) = delete;

			
			INetworkingWebSocket* Socket = nullptr;

			//客户端Guid
			FGuid Id;
	};



	private:
		//websocketserver指针
		TUniquePtr<IWebSocketServer> Server;

		//客户端的连接
		TArray<FWebSocketConnection> Connections;

		//连接关闭的委托
		FOnConnectionClosed OnConnectionClosedDelegate;





	UFUNCTION(BlueprintCallable, Category = "XhWebSocket")
	void ConnectToServer(const FString URL);

	UFUNCTION(BlueprintCallable, Category = "XhWebSocket")
	void SendMessage_C(const FString Message);

	UFUNCTION(BlueprintCallable, Category = "XhWebSocket")
	void Disconnect();

	TSharedPtr<class IWebSocket>Socket;
public:	
	// Sets default values for this actor's properties
	AXhWebSocketManager();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void BeginDestroy() override;

};








