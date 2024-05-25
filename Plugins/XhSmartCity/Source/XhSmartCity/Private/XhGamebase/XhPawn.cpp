// Fill out your copyright notice in the Description page of Project Settings.


#include "XhGamebase/XhPawn.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"

#include "Engine/LocalPlayer.h"
#include "GameFramework/Controller.h"
#include "Kismet/KismetMathLibrary.h"
#include "Blueprint/WidgetLayoutLibrary.h"


//#if PLATFORM_WINDOWS
//#if 1
//#include "Windows/AllowWindowsPlatformTypes.h"
//#include "Windows/PreWindowsApi.h"
//#include <winsock2.h>
//#include "Windows/PostWindowsApi.h"
//#include "Windows/HideWindowsPlatformTypes.h"
//#endif


DEFINE_LOG_CATEGORY(LogXhPawn);
FVector2D AXhPawn::GetMouseOffset()
{
	FVector2D OffsetMouse;
	if (FSlateApplication::IsInitialized())
	{
		OffsetMouse = FSlateApplication::Get().GetCursorPos() - LastMouseLocation;
		//OffsetMouse = UWidgetLayoutLibrary::GetMousePositionOnViewport(this) - LastMouseLocation;
		FVector2D Temp = OffsetMouse.GetAbs() - MouseTolerance;
		OffsetMouse.X = Temp.X > 0 ? OffsetMouse.X : 0;
		OffsetMouse.Y = Temp.Y > 0 ? OffsetMouse.Y : 0;
	}
	else
	{
		return OffsetMouse;
	}
	return OffsetMouse;
}
void AXhPawn::PawnRotate()
{
	FRotator SelfRotator = GetActorRotation();
	FVector2D MoveLocation = GetMouseOffset() * RotateSpeed + FVector2D(SelfRotator.Yaw,SelfRotator.Pitch);
	MoveLocation.Y = MoveLocation.Y > MinPitch ? MoveLocation.Y : MinPitch;
	MoveLocation.Y = MoveLocation.Y < MaxPitch ? MoveLocation.Y : MaxPitch;
	SetActorRotation(FRotator(MoveLocation.Y, MoveLocation.X, 0));
	if (FSlateApplication::IsInitialized())
	{
		LastMouseLocation = FSlateApplication::Get().GetCursorPos();
	}
	//LastMouseLocation = UWidgetLayoutLibrary::GetMousePositionOnViewport(this);
}
void AXhPawn::PawnMove()
{
	FVector2D MoveLocation = GetMouseOffset() * MoveSpeed;
	FVector OffsetX = MoveLocation.X * GetActorRightVector();
	AddActorWorldOffset(FVector(OffsetX.X,OffsetX.Y,0));
	FVector OffsetY = MoveLocation.Y * GetActorForwardVector();
	AddActorWorldOffset(FVector(OffsetY.X,OffsetY.Y,0));
	if (FSlateApplication::IsInitialized())
	{
		LastMouseLocation = FSlateApplication::Get().GetCursorPos();
	}
	//LastMouseLocation = UWidgetLayoutLibrary::GetMousePositionOnViewport(this);
}

void AXhPawn::InitTimeLine()
{
	TimelineUpdateCallback.BindUFunction(this, FName("TimelineUpdate"));
	TimelineFinishedCallback.BindUFunction(this, FName("TimelineFinished"));
	if (!FlyCurveFloat)
	{
		FlyCurveFloat = NewObject<UCurveFloat>();
	}
	// 创建时间轴曲线
	FlyCurveFloat->FloatCurve.AddKey(0, 0);
	FlyCurveFloat->FloatCurve.AddKey(1.f, 1.f);
	FlyCurveFloat->FloatCurve.AutoSetTangents();
	// 绑定时间轴曲线
	FlyTimeline->AddInterpFloat(FlyCurveFloat, TimelineUpdateCallback);
	FlyTimeline->SetTimelineFinishedFunc(TimelineFinishedCallback);
	FlyTimeline->SetLooping(false);
}
void AXhPawn::TimelineUpdate(float Value)
{
	FlyToData CurrentFlyToData = FlyToData::LerpData(OldFlyToData, NewFlyToData, Value);
	SetActorLocation(CurrentFlyToData.Location);
	SetActorRotation(CurrentFlyToData.Rotation);
	SpringArm->TargetArmLength = CurrentFlyToData.ArmLength;
}
void AXhPawn::TimelineFinished()
{
	if (NewFlyToData.bCallback)
	{
		SocketServerManager->SendAll("{\"event\":\"FlyToCallback\"}");
	}
}
// Sets default values
AXhPawn::AXhPawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	// Create a camera boom (pulls in towards the player if there is a collision)
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComp"));
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->TargetArmLength = 3000.0f;
	SpringArm->bDoCollisionTest = false;
	SpringArm->bEnableCameraRotationLag = true;
	SpringArm->CameraLagSpeed = 15.0f;
	SpringArm->bEnableCameraLag = false;
	SpringArm->CameraRotationLagSpeed = 20.0f;
	SpringArm->SetRelativeRotation(UKismetMathLibrary::MakeRotator(0.f, -35.0f, 0.0f));

	// Create a follow camera
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	Camera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm


	bAllowMove = true;
	bAllowRotate = true;
	bAllowLook = true;
	LookSpped = 100.f;
	MinArmLength = 100.f;
	MaxArmLength = 40000.f;
	RotateSpeed = FVector2D(0.08, -0.06);
	MoveSpeed = FVector2D(1, 1);
	MouseTolerance = FVector2D(0, 0);
	MinPitch = -53.f;
	MaxPitch = 33.f;
	FlyCurveFloat = NewObject<UCurveFloat>();
	FlyTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("FlyToTimeline"));
}

// Called when the game starts or when spawned
void AXhPawn::BeginPlay()
{
	Super::BeginPlay();
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
	InitTimeLine();
}

void AXhPawn::Move(const FInputActionValue& Value)
{
	//UE_LOG(LogXhPawn, Log, TEXT("Move开始触发"));
	bIsMove = true;
	if (FSlateApplication::IsInitialized())
	{
		LastMouseLocation = FSlateApplication::Get().GetCursorPos();
	}
	//LastMouseLocation = UWidgetLayoutLibrary::GetMousePositionOnViewport(this);
}

void AXhPawn::StopMove(const FInputActionValue& Value)
{
	//UE_LOG(LogXhPawn, Log, TEXT("Move完成触发"));
	bIsMove = false;
}

void AXhPawn::Rotate(const FInputActionValue& Value)
{
	//UE_LOG(Log/XhPawn, Log, TEXT("Rotate开始触发"));
	bIsRotate = true;
	if (FSlateApplication::IsInitialized())
	{
		LastMouseLocation = FSlateApplication::Get().GetCursorPos();
	}
	//LastMouseLocation = UWidgetLayoutLibrary::GetMousePositionOnViewport(this);
}

void AXhPawn::StopRotate(const FInputActionValue& Value)
{
	//UE_LOG(LogXhPawn, Log, TEXT("Rotate完成触发"));
	bIsRotate = false;
}

void AXhPawn::Look(const FInputActionValue& Value)
{
	
	//UE_LOG(LogXhPawn, Log, TEXT("Look触发"));
	float ArmLength = SpringArm->TargetArmLength - Value.Get<float>() * LookSpped;
	ArmLength = ArmLength > MinArmLength ? ArmLength : MinArmLength;
	ArmLength = ArmLength < MaxArmLength ? ArmLength : MaxArmLength;
	SpringArm->TargetArmLength = ArmLength;
	//SpringArm->TargetArmLength = FMath::FInterpTo(SpringArm->TargetArmLength, ArmLength, GetWorld()->GetDeltaSeconds(), 5);
}

void AXhPawn::FlyTo(FlyToData InNewFlyToData)
{
	NewFlyToData = InNewFlyToData;
	OldFlyToData.Location = GetActorLocation();
	OldFlyToData.Rotation = GetActorRotation();
	OldFlyToData.ArmLength = SpringArm->TargetArmLength;

	FlyTimeline->SetPlayRate(1.f / InNewFlyToData.Time);
	FlyTimeline->PlayFromStart();
}

// Called every frame
void AXhPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (bAllowRotate && bIsRotate) {
		PawnRotate();
	}
	if (bAllowMove && bIsMove) {
		PawnMove();
	}
}

// Called to bind functionality to input
void AXhPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Started, this, &AXhPawn::Move);
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Completed, this, &AXhPawn::StopMove);

		// Rotating
		EnhancedInputComponent->BindAction(RotateAction, ETriggerEvent::Started, this, &AXhPawn::Rotate);
		EnhancedInputComponent->BindAction(RotateAction, ETriggerEvent::Completed, this, &AXhPawn::StopRotate);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AXhPawn::Look);
	}
	else
	{
		UE_LOG(LogXhPawn, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

