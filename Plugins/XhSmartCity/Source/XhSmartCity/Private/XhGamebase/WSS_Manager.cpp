// Fill out your copyright notice in the Description page of Project Settings.


#include "XhGamebase/WSS_Manager.h"
#include "JsonObjectWrapper.h"
#include "Kismet/GameplayStatics.h"
#include "XhGamebase/DynamicSpawnActor.h"
#include "XhGamebase/XhPawn.h"
#include "XhGamebase/XhWebWidget.h"

void AWSS_Manager::ReceivedMsg(void* Data, int32 Size, FGuid ClientId)
{
	Super::ReceivedMsg(Data, Size, ClientId);
	TArrayView<uint8> dataArrayView = MakeArrayView(static_cast<uint8*>(Data), Size);
	const std::string cstr(reinterpret_cast<const char*>(
		dataArrayView.GetData()),
		dataArrayView.Num());
	FString frameAsFString = UTF8_TO_TCHAR(cstr.c_str());
	if (frameAsFString == "HEARTBEAT")
	{
		SendById(ClientId, "HEARTBEAT");
		return;
	}
	FJsonObjectWrapper JsonObjectWrapper;
	// 创建一个TSharedPtr<FJsonObject>来存储JSON数据
	TSharedPtr<FJsonObject> JsonObject;
	// 反序列化JSON字符串到FJsonObject
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(frameAsFString);
	if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid()) {
		// 你现在可以使用JsonObject来访问你的数据
		if (JsonObject.Get()->TryGetField("event"))
		{
			FString EventName = JsonObject.Get()->GetStringField("event");
			if (!EventNameMap.Contains(EventName))
			{
				XhDebugLog("未知的event: " + EventName, 2, FColor::Red);
			}
			switch (EventNameMap[EventName])
			{
			//"CustomEvent"
			case 0:
			{
				JsonObjectWrapper.JsonObject = JsonObject.Get()->GetObjectField("option");
				XhCustomEvent(JsonObjectWrapper);
			}
				break;
			//"HiddenActor"
			case 1:
			{
				TSharedPtr<FJsonObject> Option = JsonObject.Get()->GetObjectField("option");
				FString ActorName = Option.Get()->GetStringField("type");
				FString ActorTag = Option.Get()->GetStringField("tag");
				bool bActorHidden = Option.Get()->GetBoolField("hidden");
				TArray<AActor*> HiddenActors;
				UGameplayStatics::GetAllActorsOfClassWithTag(this, ActorRefMap.FindRef(ActorName), FName(*ActorTag), HiddenActors);
				for (AActor* temp : HiddenActors)
				{
					temp->SetActorHiddenInGame(bActorHidden);
				}
			}
				break;
			//"SpawnActor"
			case 2:
			{
				TSharedPtr<FJsonObject> Option = JsonObject.Get()->GetObjectField("option");
				FString ActorName = Option.Get()->GetStringField("type");
				const TSharedPtr<FJsonObject> Transform = Option.Get()->GetObjectField("transform");
				const TSharedPtr<FJsonObject>* ExtraData;
				bool bExtraData = Option.Get()->TryGetObjectField("ExtraData", ExtraData);
				FVector  Loc = JsonArrayToVector(Transform, "location");
				FRotator Rot = JsonArrayToRotator(Transform, "rotation");
				FVector  Sca = JsonArrayToVector(Transform, "scale");
				FTransform Tans = FTransform(Rot, Loc, Sca);
				AActor* SA = GetWorld()->SpawnActor<AActor>(ActorRefMap.FindRef(ActorName), Tans);
				ADynamicSpawnActor* Temp = Cast<ADynamicSpawnActor>(SA);
				if (Temp && bExtraData)
				{
					FJsonObjectWrapper ExtraDataJsonObject;
					ExtraDataJsonObject.JsonObject = *ExtraData;
					Temp->SpawnEvent(ExtraDataJsonObject);
				}
			}
				break;
			//"DestroyActor"
			case 3:
			{
				TSharedPtr<FJsonObject> Option = JsonObject.Get()->GetObjectField("option");
				FString ActorName = Option.Get()->GetStringField("type");
				FString ActorTag = Option.Get()->GetStringField("tag");
				TArray<AActor*> DestroyActors;
				if (ActorTag == "")
				{
					UGameplayStatics::GetAllActorsOfClass(this, ActorRefMap.FindRef(ActorName), DestroyActors);
				}
				else
				{
					UGameplayStatics::GetAllActorsOfClassWithTag(this, ActorRefMap.FindRef(ActorName), FName(*ActorTag), DestroyActors);
				}
				for (AActor* temp : DestroyActors)
				{
					temp->Destroy();
				}
			}
				break;
			//"FlyTo"
			case 4:
			{
				AXhPawn* XhPawn = Cast<AXhPawn>(UGameplayStatics::GetPlayerPawn(this, 0));
				if (XhPawn)
				{
					const TSharedPtr<FJsonObject> Option = JsonObject.Get()->GetObjectField("option");
					const TSharedPtr<FJsonObject> Transform = Option.Get()->GetObjectField("transform");
					FlyToData NewFlyToData;
					NewFlyToData.Location = JsonArrayToVector(Transform, "location");
					NewFlyToData.Rotation = JsonArrayToRotator(Transform, "rotation");
					NewFlyToData.ArmLength = Option.Get()->GetNumberField("ArmLength");
					NewFlyToData.Time = Option.Get()->GetNumberField("time");
					NewFlyToData.bCallback = Option.Get()->GetBoolField("bcallback");
					XhPawn->FlyTo(NewFlyToData);
				}
			}
				break;
			//"AddArea"
			case 5:
			{
				TSharedPtr<FJsonObject> Option = JsonObject.Get()->GetObjectField("option");
				TArray<TSharedPtr<FJsonValue>> AreaArray = Option.Get()->GetArrayField("array");
				for (int32 i = 0; i < AreaArray.Num(); i++)
				{
					TArray<TSharedPtr<FJsonValue>> min = AreaArray[i].Get()->AsObject()->GetArrayField("min");
					TArray<TSharedPtr<FJsonValue>> max = AreaArray[i].Get()->AsObject()->GetArrayField("max");
					FVector2D MinArea = FVector2D(min[0].Get()->AsNumber(), min[1].Get()->AsNumber());
					FVector2D MaxArea = FVector2D(max[0].Get()->AsNumber(), max[1].Get()->AsNumber());
					if (XhWebWidget)
					{
						XhWebWidget->CanClickAreaOriginalData.Add(ClickArea(MinArea, MaxArea));
						XhWebWidget->CanClickArea.Add(ClickArea(MinArea * XhWebWidget->Zoom,MaxArea * XhWebWidget->Zoom));
						UE_LOG(LogTemp, Log, TEXT("可点击区域为： %s"), *ClickArea(MinArea * XhWebWidget->Zoom, MaxArea * XhWebWidget->Zoom).ToString());
					}
				}
			}
				break;
			//"RemoveArea"
			case 6:
			{
				TSharedPtr<FJsonObject> Option = JsonObject.Get()->GetObjectField("option");
				TArray<TSharedPtr<FJsonValue>> AreaArray = Option.Get()->GetArrayField("array");
				for (int32 i = 0; i < AreaArray.Num(); i++)
				{
					TArray<TSharedPtr<FJsonValue>> min = AreaArray[i].Get()->AsObject()->GetArrayField("min");
					TArray<TSharedPtr<FJsonValue>> max = AreaArray[i].Get()->AsObject()->GetArrayField("max");
					FVector2D MinArea = FVector2D(min[0].Get()->AsNumber(), min[1].Get()->AsNumber());
					FVector2D MaxArea = FVector2D(max[0].Get()->AsNumber(), max[1].Get()->AsNumber());
					if (XhWebWidget)
					{
						XhWebWidget->CanClickAreaOriginalData.Remove(ClickArea(MinArea, MaxArea));
						XhWebWidget->CanClickArea.Remove(ClickArea(MinArea / XhWebWidget->Zoom, MaxArea / XhWebWidget->Zoom));
					}

				}
			}
				break;
			//"HtmlWidth"
			case 7:
			{
				TSharedPtr<FJsonObject> Option = JsonObject.Get()->GetObjectField("option");
				if (XhWebWidget)
				{
					XhWebWidget->HtmlWidth = Option.Get()->GetNumberField("width");
					XhWebWidget->XhResized();
				}
				//XhWebWidget->SetScreenZoom(Option.Get()->GetNumberField("width"));
			}
				break;
			default:
				
				break;
			}
		}
		else
		{
			XhStringMsg(frameAsFString);
		}
	}
	else {
		// 反序列化失败
		XhStringMsg(frameAsFString);
	}
}

bool AWSS_Manager::GetActorRefData()
{
	if (DT_ActorRef != nullptr)
	{
		TArray<FName> RowNames;
		FString ContextString;
		RowNames = DT_ActorRef->GetRowNames();
		for (FName& name : RowNames)
		{
			FActorRef* CurrentRow = DT_ActorRef->FindRow<FActorRef>(name, ContextString);
			if (CurrentRow)
			{
				ActorRefMap.Add(CurrentRow->Name, CurrentRow->ActorRef);
			}
		}
		return true;
	}
	return false;
}

void AWSS_Manager::InitEventNameMap()
{
	TArray<FString> EvenNameArray = {
		"CustomEvent",
		"HiddenActor",
		"SpawnActor",
		"DestroyActor",
		"FlyTo",
		"AddArea",
		"RemoveArea",
		"HtmlWidth"
	};
	for (int32 i = 0; i < EvenNameArray.Num(); i++)
	{
		EventNameMap.Emplace(EvenNameArray[i],i);
	}
}

FVector AWSS_Manager::JsonArrayToVector(const TSharedPtr<FJsonObject>& InJsonObj, const FString& InName)
{
	FVector Result;
	TArray<TSharedPtr<FJsonValue>> Temp = InJsonObj.Get()->GetArrayField(InName);
	Result = FVector(Temp[0].Get()->AsNumber(), Temp[1].Get()->AsNumber(), Temp[2].Get()->AsNumber());
	return Result;
}

FRotator AWSS_Manager::JsonArrayToRotator(const TSharedPtr<FJsonObject>& InJsonObj, const FString& InName)
{
	FRotator Result;
	TArray<TSharedPtr<FJsonValue>> Temp = InJsonObj.Get()->GetArrayField(InName);
	Result = FRotator(Temp[0].Get()->AsNumber(), Temp[1].Get()->AsNumber(), Temp[2].Get()->AsNumber());
	return Result;
}

void AWSS_Manager::BeginPlay()
{
	Super::BeginPlay();
	ActorRefMap.Empty();
	if (!GetActorRefData())
	{
		UE_LOG(LogTemp, Error, TEXT("ActorRefMap初始化失败"));
	}
	InitEventNameMap();
}

void AWSS_Manager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AWSS_Manager::BeginDestroy()
{
	Super::BeginDestroy();
}
