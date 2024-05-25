// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Components/TimelineComponent.h"
#include "Blueprint/UserWidget.h"
#include "XhPawn.generated.h"

class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

//Pawn的飞行数据
struct FlyToData {
	FlyToData()
	{
		Location = FVector();
		Rotation = FRotator();
		ArmLength = 1000.f;
		bCallback = false;
		Time = 1.f;
	}
	FVector Location;
	FRotator Rotation;
	float ArmLength;
	bool bCallback;
	float Time;
public:
	static FlyToData LerpData(const FlyToData& OldData, const FlyToData& NewData, float CurrentValue)
	{
		FlyToData CurrentData;
		CurrentData.Location = FMath::Lerp(OldData.Location, NewData.Location, CurrentValue);
		CurrentData.Rotation = FMath::Lerp(OldData.Rotation, NewData.Rotation, CurrentValue);
		CurrentData.ArmLength = FMath::Lerp(OldData.ArmLength, NewData.ArmLength, CurrentValue);
		return CurrentData;
	}
};



DECLARE_LOG_CATEGORY_EXTERN(LogXhPawn, Log, All);
UCLASS()
class XHSMARTCITY_API AXhPawn : public APawn
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* RotateAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Component")
	class USpringArmComponent* SpringArm;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Component")
	class UCameraComponent* Camera;
public:
	//是否允许移动
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "XhSettings")
	bool bAllowMove;
	//是否允许旋转
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "XhSettings")
	bool bAllowRotate;
	//是否允许缩放
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "XhSettings")
	bool bAllowLook;
	//缩放速度
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "XhSettings")
	float LookSpped;
	//缩放最近距离
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "XhSettings")
	float MinArmLength;
	//缩放最远距离
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "XhSettings")
	float MaxArmLength;
	//旋转速度
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "XhSettings")
	FVector2D RotateSpeed;
	//移动速度
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "XhSettings")
	FVector2D MoveSpeed;
	//一帧中鼠标偏移最小值(小于此最小值将不执行输入操作)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "XhSettings")
	FVector2D MouseTolerance;
	//旋转的视角范围
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "XhSettings")
	float MinPitch;
	//旋转的视角范围
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "XhSettings")
	float MaxPitch;
	//websocket通信
	UPROPERTY()
	AXhWebSocketManager* SocketServerManager;
public:
	
protected:
	bool bIsMove;
	bool bIsRotate;
	FVector2D LastMouseLocation;
	FVector2D GetMouseOffset();
	void PawnRotate();
	void PawnMove();
#pragma region FlyToTimeline
protected:
	
	// 浮点数曲线
	UPROPERTY()
	UCurveFloat* FlyCurveFloat;
	// 时间轴组件
	UPROPERTY()
	UTimelineComponent* FlyTimeline;
	// 声明时间轴Update回调函数
	FOnTimelineFloat TimelineUpdateCallback;
	// 声明时间轴结束事件
	FOnTimelineEvent TimelineFinishedCallback;

	FlyToData OldFlyToData;
	FlyToData NewFlyToData;

	//初始化时间轴
	void InitTimeLine();
	// 时间轴开始函数
	UFUNCTION()
	void TimelineUpdate(float Value);
	UFUNCTION()
	void TimelineFinished();
public:
	void FlyTo(FlyToData InNewFlyToData);
#pragma endregion
public:
	// Sets default values for this pawn's properties
	AXhPawn();


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for movement input */
	void StopMove(const FInputActionValue& Value);

	/** Called for movement input */
	void Rotate(const FInputActionValue& Value);

	/** Called for movement input */
	void StopRotate(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

	//bool IsWidgetCanClick();
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
