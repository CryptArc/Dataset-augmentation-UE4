// Fill out your copyright notice in the Description page of Project Settings.

#include "ToolPawn.h"
#include "UObject/ConstructorHelpers.h"
#include "Runtime/Engine/Public/HighResScreenshot.h"


// Sets default values
AToolPawn::AToolPawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	if (!RootComponent)
	{
		RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("ToolBase"));
	}

	ObjectMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ToolStaticMesh"));
	ObjectMesh->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepWorldTransform);
	ObjectMesh->SetRelativeScale3D(FVector(75, 75, 75));

	/*   FRotator(Y, Z, X)   */
	Arrowx = CreateDefaultSubobject<UArrowComponent>(TEXT("ArrowX"));
	Arrowx->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepWorldTransform);
	Arrowx->SetArrowColor(FLinearColor(255., 0., 0.));
	Arrowy = CreateDefaultSubobject<UArrowComponent>(TEXT("ArrowY"));
	Arrowy->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepWorldTransform);
	Arrowy->SetArrowColor(FLinearColor(0., 255., 0.));
	Arrowy->SetRelativeRotation(FRotator(0, 90, 0));
	Arrowz = CreateDefaultSubobject<UArrowComponent>(TEXT("ArrowZ"));
	Arrowz->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepWorldTransform);
	Arrowz->SetArrowColor(FLinearColor(0., 0., 255.));
	Arrowz->SetRelativeRotation(FRotator(90, 0, 0));

	/*   Frotator(Y, Z, X)   */
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepWorldTransform);
	FRotator initialRot = FRotator(90, 0, 90);
	Camera->AddLocalRotation(initialRot);

	CameraCapture = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("CameraCapture"));
	CameraCapture->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepWorldTransform);
	CameraCapture->AddLocalRotation(initialRot);
}

// Called when the game starts or when spawned
void AToolPawn::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void AToolPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AToolPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	PlayerInputComponent->BindAction("RightClick", IE_Pressed, this, &AToolPawn::SaveImage);

}

void AToolPawn::SaveTexture2DDebug()
{
	FString Filename = "C:/Users/owner/Desktop/K/screenshot.png";
	/*Texture2D->UpdateResource();
	FTexture2DMipMap* MM = &Texture2D->PlatformData->Mips[0];
	TArray<FColor> OutBMP;
	int w = MM->SizeX;
	int h = MM->SizeY;

	OutBMP.InsertZeroed(0, w*h);

	FByteBulkData* RawImageData = &MM->BulkData;

	FColor* FormatedImageData = static_cast<FColor*>(RawImageData->Lock(LOCK_READ_ONLY));

	for (int i = 0; i < (w*h); ++i)
	{
		OutBMP[i] = FormatedImageData[i];
		OutBMP[i].A = 255;
	}

	RawImageData->Unlock();
	FIntPoint DestSize(w, h);

	FString ResultPath;
	FHighResScreenshotConfig& HighResScreenshotConfig = GetHighResScreenshotConfig();
	bool bSaved = HighResScreenshotConfig.SaveImage(Filename, OutBMP, DestSize, &ResultPath);
	
	FTexture2DResource* PR = (FTexture2DResource*)PTexture->Resource;

	if (PR)
	{
	uint32 Stride;
	void* buf = RHILockTexture2D(PR->GetTexture2DRHI(), 0, RLM_ReadOnly, Stride, false);e
	}*/

}


void AToolPawn::SaveImage() {
	UE_LOG(LogTemp, Warning, TEXT("T'as click gros !!"));
	AToolPawn::SaveTexture2DDebug();
	FString nimp = "C'est OK";
	UE_LOG(LogTemp, Warning, TEXT("T'as click gros 22 !!"));
}