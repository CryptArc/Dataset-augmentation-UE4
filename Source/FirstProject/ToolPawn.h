// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "Camera/CameraComponent.h"
#include "Classes/Components/StaticMeshComponent.h"
#include "Classes/Components/ArrowComponent.h"
#include "Classes/Components/SceneCaptureComponent2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/Texture2D.h"
#include "Runtime/Engine/Classes/Components/InputComponent.h"


#include "GameFramework/Pawn.h"
#include "ToolPawn.generated.h"

UCLASS()
class FIRSTPROJECT_API AToolPawn : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AToolPawn();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// Save Utextur2D into PNG
	void SaveTexture2DDebug();

	// Called on right click
	UFUNCTION(BlueprintCallable, Category = "SaveImage")
		void SaveImage();



	// 3D meh of the tool
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh", meta = (AllowPrivateAccess = "true"))
		UStaticMeshComponent* ObjectMesh;

	// Camera
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera", meta = (AllowPrivateAccess = "true"))
		UCameraComponent *Camera;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera", meta = (AllowPrivateAccess = "true"))
		USceneCaptureComponent2D *CameraCapture;

	// Texture
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Texture", meta = (AllowPrivateAccess = "true"))
		UTextureRenderTarget2D *RenderTarget;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Texture", meta = (AllowPrivateAccess = "true"))
		UTexture2D *Texture2D;

	// Arrows to visualize the camera frame
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arrows", meta = (AllowPrivateAccess = "true"))
		UArrowComponent *Arrowx;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arrows", meta = (AllowPrivateAccess = "true"))
		UArrowComponent *Arrowy;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arrows", meta = (AllowPrivateAccess = "true"))
		UArrowComponent *Arrowz;
	
};
