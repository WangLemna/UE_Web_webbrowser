// Fill out your copyright notice in the Description page of Project Settings.


#include "XhWebSocket/XhWebSocketManager.h"
#include "JsonObjectWrapper.h"

#include "IWebSocket.h"
#include "IWebSocketNetworkingModule.h"
#include "WebSocketServer.h"
#include "WebSocketsModule.h"

// Sets default values
AXhWebSocketManager::AXhWebSocketManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AXhWebSocketManager::BeginPlay()
{
	Super::BeginPlay();
	//ServerTick();
}

// Called every frame
void AXhWebSocketManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	ServerTick();
}

void AXhWebSocketManager::BeginDestroy()
{
	Super::BeginDestroy();
	StopServer();
}

void AXhWebSocketManager::OnConnected_C_Callback()
{
	UE_LOG(LogTemp, Warning, TEXT("连接成功"));
	OnConnected_C.Broadcast();
}

void AXhWebSocketManager::OnConnectionError_C_Callback(const FString& Error)
{
	UE_LOG(LogTemp, Warning, TEXT("连接错误,Error:%s"), *Error);
	OnConnectionError_C.Broadcast(Error);
}

void AXhWebSocketManager::OnReceivedMessage_C_Callback(const FString& Message)
{
	UE_LOG(LogTemp, Warning, TEXT("收到消息,Message:%s"), *Message);
	OnReceivedMessage_C.Broadcast(Message);
}

void AXhWebSocketManager::OnClosed_C_Callback(int32 Code, const FString& Reason, bool bWasClean)
{
	UE_LOG(LogTemp, Warning, TEXT("连接关闭,Code:%d,reason:%s"), Code, *Reason);
	OnClosed_C.Broadcast(Code, Reason, bWasClean);
}

bool AXhWebSocketManager::StartServer(int Port)
{
	FWebSocketClientConnectedCallBack CallBack;
	CallBack.BindUObject(this, &AXhWebSocketManager::OnWebSocketClientConnected);

	Server = FModuleManager::Get().LoadModuleChecked<IWebSocketNetworkingModule>(TEXT("WebSocketNetworking")).CreateServer();
	if (!Server || !Server->Init(Port, CallBack))
	{
		Server.Reset();
		return false;
	}

	return true;
}

void AXhWebSocketManager::StopServer()
{
	if (IsRunning()) {
		Server.Reset();
	}
}

bool AXhWebSocketManager::ServerTick()
{
	if (IsRunning()) {
		Server->Tick();
		return true;
	}
	else {
		return false;
	}
	return false;
}

void AXhWebSocketManager::SendById(const FGuid& InTargetClientId, const FString SendMsg)
{
	TArray<uint8> InUTF8Payload;
	InUTF8Payload.SetNum(SendMsg.Len());
	FMemory::Memcpy(InUTF8Payload.GetData(), TCHAR_TO_ANSI(*SendMsg), SendMsg.Len());

	if (FWebSocketConnection* Connection = Connections.FindByPredicate([&InTargetClientId](const FWebSocketConnection& InConnection)
		{ return InConnection.Id == InTargetClientId; }))
	{
		Connection->Socket->Send(InUTF8Payload.GetData(), InUTF8Payload.Num(), /*PrependSize=*/false);
	}
}

void AXhWebSocketManager::SendAll(const FString Msg)
{
	FTCHARToUTF8 utf8Str(*Msg);
	int32 utf8StrLen = utf8Str.Length();

	TArray<uint8> uint8Array;
	uint8Array.SetNum(utf8StrLen);
	memcpy(uint8Array.GetData(), utf8Str.Get(), utf8StrLen);

	if (Connections.Num() > 0)
	{
		for (auto& ws : Connections) {
			ws.Socket->Send(uint8Array.GetData(), uint8Array.Num(), /*PrependSize=*/false);
		}
	}
}

bool AXhWebSocketManager::IsRunning() const
{
	return !!Server;
}

void AXhWebSocketManager::XhDebugLog(FString msg, float delayTime, FColor color)
{
	GEngine->AddOnScreenDebugMessage(-1, delayTime, color, " Jason " + msg);// 打印到屏幕
	UE_LOG(LogTemp, Log, TEXT(" Jason  %s"), *msg);// 打印到outputlog
}

void AXhWebSocketManager::OnWebSocketClientConnected(INetworkingWebSocket* InSocket)
{
	XhDebugLog("OnWebSocketClientConnected ", 10, FColor::Red);
	if (ensureMsgf(InSocket, TEXT("Socket was null while creating a new websocket connection.")))
	{
		FWebSocketConnection Connection = FWebSocketConnection{ InSocket };

		FWebSocketPacketReceivedCallBack ReceiveCallBack;
		ReceiveCallBack.BindUObject(this, &AXhWebSocketManager::ReceivedMsg, Connection.Id);
		InSocket->SetReceiveCallBack(ReceiveCallBack);

		FWebSocketInfoCallBack CloseCallback;
		CloseCallback.BindUObject(this, &AXhWebSocketManager::OnSocketClose, InSocket);
		InSocket->SetSocketClosedCallBack(CloseCallback);

		BP_ClientSucceedConnected(Connection.Id);
		Connections.Add(MoveTemp(Connection));
	}
}

void AXhWebSocketManager::ReceivedMsg(void* Data, int32 Size, FGuid ClientId)
{
}

void AXhWebSocketManager::OnSocketClose(INetworkingWebSocket* InSocket)
{
	int32 Index = Connections.IndexOfByPredicate([InSocket](const FWebSocketConnection& Connection) { return Connection.Socket == InSocket; });

	XhDebugLog("OnSocketClose " + FString::FromInt(Index), 10, FColor::Red);
	if (Index != INDEX_NONE)
	{
		OnConnectionClosed().Broadcast(Connections[Index].Id);
		Connections.RemoveAtSwap(Index);
	}
}

void AXhWebSocketManager::ConnectToServer(const FString URL)
{
	//创建Websocket对象
	Socket = FWebSocketsModule::Get().CreateWebSocket(URL);

	//绑定连接成功回调
	Socket->OnConnected().AddUObject(this, &AXhWebSocketManager::OnConnected_C_Callback);

	//绑定连接失败回调
	Socket->OnConnectionError().AddUObject(this, &AXhWebSocketManager::OnConnectionError_C_Callback);

	//绑定连接关闭回调
	Socket->OnClosed().AddUObject(this, &AXhWebSocketManager::OnClosed_C_Callback);

	//绑定收到消息回调
	Socket->OnMessage().AddUObject(this, &AXhWebSocketManager::OnReceivedMessage_C_Callback);

	Socket->Connect();
}

void AXhWebSocketManager::SendMessage_C(const FString Message)
{
	if (Socket.IsValid())
	{
		Socket->Send(Message);
	}
}

void AXhWebSocketManager::Disconnect()
{
	if (Socket.IsValid())
	{
		Socket->Close(1000, "手动关闭");
	}
}