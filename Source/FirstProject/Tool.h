// Fill out your copyright notice in the Description page of Project Settings.



#pragma once

#include "CoreMinimal.h"

#include "Camera/CameraComponent.h"
#include "Classes/Components/StaticMeshComponent.h"
#include "Classes/Components/ArrowComponent.h"
#include "Classes/Components/SceneCaptureComponent2D.h"
#include "Runtime/Engine/Classes/GameFramework/SpringArmComponent.h"
#include "Runtime/Engine/Classes/Components/InstancedStaticMeshComponent.h"
#include "Runtime/Engine/Classes/Components/DirectionalLightComponent.h"
#include "Runtime/Engine/Classes/Materials/Material.h"
#include "Runtime/Engine/Classes/Engine/ObjectLibrary.h"
#include "Runtime/Engine/Classes/Components/SkyLightComponent.h"

#include "GameFramework/Actor.h"
#include "Tool.generated.h"

UCLASS()
class FIRSTPROJECT_API ATool : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATool();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Useful for test and debugging
	UFUNCTION(BlueprintCallable, Category = "Test")
		void Touch();
	UFUNCTION(BlueprintNativeEvent, Category = "Test")
		void Touched();
	UFUNCTION(BlueprintCallable, Category = "Test")
		void VisualizeCameraPoses();
	
	// Handling the lights
	UFUNCTION(BlueprintCallable, Category = "Lights")
		void RandomChangeLighting();
	// Set the light to enlight in the direction of the camera
	UFUNCTION(BlueprintCallable, Category = "Lights")
		void EnlightCameraAxis(float instensity=10);

	// Randomly change the background material
	UFUNCTION(BlueprintNativeEvent, Category = "Background")
		void RandomChangeBackground();

	UFUNCTION(BlueprintCallable, Category = "Saving/Reading")
		bool GoToCameraPose(FString FilePath, bool visualizeCameraPose = false, bool saveImageFromCameraPose = false);
	UFUNCTION(BlueprintCallable, Category = "Saving/Reading")
		bool GoToNextCameraPose(bool visualizeCameraPose = false, bool saveImageFromCameraPose = false);
	UFUNCTION(BlueprintCallable, Category = "Saving/Reading")
		bool GoToAllCameraPoses(bool visualizeCameraPose = false, bool randomizeLightning = true, bool randomizeBackground = true, float intensity = 10, bool saveImageFromCameraPose = false);
	// Saving the image from the Render Target
	UFUNCTION(BlueprintCallable, Category = "Saving/Reading")
		bool SaveImage(FString FileBaseName="", bool increment=true);
	// Get the file names in a directory into a TArray
	UFUNCTION(BlueprintCallable, Category = "Saving/Reading")
		TArray<FString> GetFilesNames(FString DirPath);
	// Read a rotation file and apply it to the camera
	UFUNCTION(BlueprintCallable, Category = "Saving/Reading")
		bool ReadRotation();
		bool ReadRotation(FString FilePath, FVector *AxisAngles);
	// Read a translation file and apply it to the camera
	UFUNCTION(BlueprintCallable, Category = "Saving/Reading")
		bool ReadTranslation();
		bool ReadTranslation(FString FilePath, FVector *TranslationVector);
	// Randomly rotate the camera around the object a certain amount of times and save the rotations in txt files
	UFUNCTION(BlueprintCallable, Category = "Saving/Reading")
		void GenerateRandomRotationsData(int nbRot, bool randomizeLightning = true, bool randomizeBackground = true, float intensity = 10, bool randomizeArmLength=false, bool randomTranslate = false);
	// Rotate the camera around the object degree by degree and save the rotations in txt files
	UFUNCTION(BlueprintCallable, Category = "Saving/Reading")
		void GenerateNonRandomRotationsData(float degreeStep = 20, bool randomizeLightning = true, bool randomizeBackground = true, float intensity = 10);

	UFUNCTION(BlueprintCallable, Category = "Projection")
		void ProjectObject();
	
	// Toogle the rotation mode to rotate around the object
	UFUNCTION(BlueprintCallable, Category = "Rotate camera")
		void ToogleRotationMode();
	// Rotate the camera around the object given a rotator
	UFUNCTION(BlueprintCallable, Category = "Rotate camera")
		void RotateCamera(FRotator Rotator);
	// Randomly rotate the camera
	UFUNCTION(BlueprintCallable, Category = "Rotate camera")
		void RandomRotateCamera(bool randomizeLightning = true, bool randomizeBackground = true, float intensity = 10, bool randomizeArmLength=false, bool randomTranslate=false);
	// Rotate the camera base on an axis angles vector
	UFUNCTION(BlueprintCallable, Category = "Rotate camera")
		void RotateCameraFromAxisAngles(FVector AxisAnglesRightHanded);


private:
	// 3D meh of the tool
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mesh", meta = (AllowPrivateAccess = "true"))
		UStaticMeshComponent *ObjectMesh;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh", meta = (AllowPrivateAccess = "true"))
		float MeshScale = 200;

	// Scene component placed in the center of mass of the mesh
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Center of mass", meta = (AllowPrivateAccess = "true"))
		USceneComponent *CoMComponent;


	// Instanced static mesh for visualize the camera poses in the frame of the mesh
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera poses", meta = (AllowPrivateAccess = "true"))
		UInstancedStaticMeshComponent* Poses;

	// Background
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Background", meta = (AllowPrivateAccess = "true"))
		UStaticMeshComponent *BackgroundMesh;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Background", meta = (AllowPrivateAccess = "true"))
		USceneComponent *BackgroundComponent;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Background", meta = (AllowPrivateAccess = "true"))
		UMaterial *BackgroundMaterial;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Background", meta = (AllowPrivateAccess = "true"))
		float BackgroundDistance = 3000;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Background", meta = (AllowPrivateAccess = "true"))
		UObjectLibrary *MaterialsLibrary;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Background", meta = (AllowPrivateAccess = "true"))
		TArray<UMaterial*> MaterialsArray;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Background", meta = (AllowPrivateAccess = "true"))
		int BackgroundScale = 60;

	// Light component
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lights", meta = (AllowPrivateAccess = "true"))
		UDirectionalLightComponent *Light1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lights", meta = (AllowPrivateAccess = "true"))
		UDirectionalLightComponent *Light2;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lights", meta = (AllowPrivateAccess = "true"))
		UDirectionalLightComponent *Light3;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lights", meta = (AllowPrivateAccess = "true"))
		UDirectionalLightComponent *Light4;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lights", meta = (AllowPrivateAccess = "true"))
		USkyLightComponent *Skylight;


	// Spring arm component to maintain the orientation of the cameras
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, Category = "Camera", meta = (AllowPrivateAccess = "true"))
		USpringArmComponent *SpringArm;
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, Category = "Camera", meta = (AllowPrivateAccess = "true"))
		USpringArmComponent *SpringArm2;
	// Initial rotation of the spring arm so that the image is correctly orientated
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
		FRotator initialRot = FRotator(90, -90, 0);
	// Camera to see what is filled in the editor's viewport
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, Category = "Camera", meta = (AllowPrivateAccess = "true"))
		UCameraComponent *Camera;
	// CameraCapture to capture the view and assign the corresponding image to a RenderTarget for further export
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera", meta = (AllowPrivateAccess = "true"))
		USceneCaptureComponent2D *CameraCapture;
	// Rotation between the initial pose to the current camera pose, axis angles representation
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	FVector AxisAnglesVector;
	// Min and max distance to the tool center
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
		float MinArmLength = 500;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
		float MaxArmLenght = 2000;
	// Bool to know if we're in rotation mode or not
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
		bool isInRotationMode = false;

	// Path to the directory where the images will be saved
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Saving/Reading", meta = (AllowPrivateAccess = "true"))
		FString  fileSaveDir = "C:/Users/owner/Desktop/K/Unreal/Tests";
	// Name of the image file to be saved 
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Saving/Reading", meta = (AllowPrivateAccess = "true"))
		FString  fileSaveName = "YellowTool";
	// Path to the directory where the rotation will be loaded from
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Saving/Reading", meta = (AllowPrivateAccess = "true"))
		FString fileLoadDir = "C:/Users/owner/Desktop/K/Unreal/Tests/RandomExtrinsics";
		FString fileLoadDirRot;
		FString fileLoadDirTrans;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Saving/Reading", meta = (AllowPrivateAccess = "true"))
		TArray<FString> RotFiles;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Saving/Reading", meta = (AllowPrivateAccess = "true"))
		TArray<FString> TransFiles;
	// Timer for delay between ;oving the camera and saving the image
		FTimerHandle timerHandle;
	// Vector to store the read axis angles
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, Category = "Saving/Reading", meta = (AllowPrivateAccess = "true"))
		FVector ReadAxisAnglesVector;
	// Vector to store the read translation
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, Category = "Saving/Reading", meta = (AllowPrivateAccess = "true"))
		FVector ReadTranslationVector;
	// Index of the file being saved or read
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Saving/Reading", meta = (AllowPrivateAccess = "true"))
		int index;
	
	// bool to determine if the generation of images is ongoing or not
	bool isGenerating = false;
	// number of images to generate
	int nbImagesToGenerate = 0;
	// number of images generated so far
	int nbImagesGenerated = 0;
	// parameters while generating
	bool randomizeLightningGenerting;
	bool randomizeBackgroundGenerting;
	float intensityGenerting;
	bool randomizeArmLengthGenerting;
	bool randomTranslateGenerting;
};

