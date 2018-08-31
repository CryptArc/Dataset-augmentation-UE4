// Fill out your copyright notice in the Description page of Project Settings.

#include "Tool.h"
#include "UObject/ConstructorHelpers.h"
#include "Runtime/Engine/Public/HighResScreenshot.h"
#include "Runtime/Engine/Classes/Kismet/KismetRenderingLibrary.h"
#include "Runtime/Engine/Classes/Engine/World.h"
#include "Runtime/Core/Public/Misc/FileHelper.h"
#include "Runtime/Core/Public/HAL/PlatformFilemanager.h"
#include "Runtime/Engine/Classes/Kismet/KismetMathLibrary.h"
#include "Runtime/Engine/Public/SceneInterface.h"
#include "Runtime/AssetRegistry/Public/AssetData.h"




// Sets default values
ATool::ATool()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Create a RootComponent if there are none
	if (!RootComponent)
	{
		RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("ToolBase"));
	}

	// Create a Static Mesh object, attach it to the Root and scale it bigger cause the tool is usually small in the begining
	ObjectMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ToolStaticMesh"));
	ObjectMesh->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepWorldTransform);
		// The scale of the mesh needs to be adjusted with the property MeshScale, it needs to be done in C++ in the Tool.h file
	ObjectMesh->SetRelativeScale3D(FVector(MeshScale, MeshScale, MeshScale));

	// Instanced static mesh to visualize the camera poses in the frame of the mesh
	// This is used only for debugging purpose
	Poses = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("Poses"));
	Poses->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);

	/* Reminder: UE4's Rotator are as follows:   FRotator(Y, Z, X)   */

	// Create a component to mark the center of mass of the mesh
	CoMComponent = CreateDefaultSubobject<USceneComponent>(TEXT("ToolCoM"));

	// Create a spring arm and attach it to the CoMComponent so that the cameras attached to the spring are always orientated towards the CoMComponent
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
	SpringArm->TargetArmLength = 0;
	SpringArm2 = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm2"));
	SpringArm2->AttachToComponent(SpringArm, FAttachmentTransformRules::KeepRelativeTransform);
	SpringArm2->TargetArmLength = 0;
	
	// Create a camera and attahc it to the spring arm
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->AttachToComponent(SpringArm2, FAttachmentTransformRules::KeepRelativeTransform);

	// Create a capture camera to store the images later on
	CameraCapture = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("CameraCapture"));
	CameraCapture->AttachToComponent(SpringArm2, FAttachmentTransformRules::KeepRelativeTransform);
		// These settings are mandatory for saving images from the CameraCapture png format
	CameraCapture->CaptureSource = SCS_FinalColorLDR;
	CameraCapture->bCaptureEveryFrame = false;

	// Apply inintial rotation to the cameras so that the image is well orientated
	SpringArm2->SetRelativeRotation(initialRot);

	// Initializing the lights
	Light1 = CreateDefaultSubobject<UDirectionalLightComponent>(TEXT("Light1"));
	Light2 = CreateDefaultSubobject<UDirectionalLightComponent>(TEXT("Light2"));
	Light3 = CreateDefaultSubobject<UDirectionalLightComponent>(TEXT("Light3"));
	Light4 = CreateDefaultSubobject<UDirectionalLightComponent>(TEXT("Light4"));
	Skylight = CreateDefaultSubobject<USkyLightComponent>(TEXT("SkyLight"));
	// Attach the lights to the RootComponent
	Light1->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
	Light2->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
	Light3->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
	Light4->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
	Skylight->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
	// Initialize lights' intensity
	Light1->Intensity = 0;
	Light2->Intensity = 0;
	Light3->Intensity = 0;
	Light4->Intensity = 0;
	Skylight->Intensity = 2;

	// Create a component for the background and attach a mesh to it
	// The BackgroundComponent is attach to the spring arm 2 so that it is always orthogonal to the camera axis
	BackgroundComponent = CreateDefaultSubobject<USceneComponent>(TEXT("BackgroundComponent"));
	BackgroundComponent->AttachToComponent(SpringArm2, FAttachmentTransformRules::KeepWorldTransform);
	BackgroundMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BackgroundMesh"));
	BackgroundMesh->AttachToComponent(BackgroundComponent, FAttachmentTransformRules::KeepWorldTransform);
	// Rotate the BackgroundMesh and BackgroundComponent so that the Z axis of the background plane is aligned to the camera axis
	BackgroundComponent->SetRelativeRotation(FRotator(90, 0, 0));
	BackgroundMesh->SetRelativeRotation(FRotator(0, 90, 0));
	// Scale the background plane, this is done with the property BackgroundScale
	BackgroundMesh->SetRelativeScale3D(FVector(BackgroundScale, BackgroundScale, 0));

	// Get a reference to the MaterialLibrary used to store the materials that will be used for the background
	ConstructorHelpers::FObjectFinder<UObjectLibrary> ObjectLibrary(TEXT("/Game/MaterialsLibrary.MaterialsLibrary"));
	if (ObjectLibrary.Succeeded())
		MaterialsLibrary = ObjectLibrary.Object;

	index = 0;
	
}

// Called when the game starts or when spawned
void ATool::BeginPlay()
{
	// First get the center of mass (that should be in the middle of the tool) and place the CoMComponent at its location
	FVector CoM = ObjectMesh->GetCenterOfMass();
	CoMComponent->SetWorldLocation(CoM);

	// Apply the rotation of RootComponent to the CoMComponent so that it has the same orientation as the camera frame
	FRotator RootRotator= RootComponent->GetComponentRotation();
	FQuat RootQuat = FQuat(RootRotator);
	CoMComponent->SetWorldRotation(RootQuat);
	FRotator comrot = CoMComponent->GetComponentRotation();
	FRotator sprrot = SpringArm->GetComponentRotation();
	UE_LOG(LogTemp, Warning, TEXT("Path: %s"), *fileLoadDir);

	// Actualize the save and load directories
	fileLoadDirRot = fileLoadDir + "/Orientations";
	fileLoadDirTrans = fileLoadDir + "/Translations";
	RotFiles = GetFilesNames(fileLoadDirRot);
	TransFiles = GetFilesNames(fileLoadDirTrans);

	// Adjust the gamma of the scene capture's render target to have better ligthning
	CameraCapture->TextureTarget->TargetGamma = 2;

	// Load materials
	FString Path = "/Game/BackgroundMaterials";
	MaterialsLibrary->LoadAssetsFromPath(Path);
	MaterialsLibrary->GetObjects(MaterialsArray);
	for (auto& MaterialPtr : MaterialsArray) {
		UE_LOG(LogTemp, Warning, TEXT("Material %s loaded"), *MaterialPtr->GetName());
	}

	// Place the background behind the tool
	FVector bgLoc = FVector(BackgroundDistance, 0, 0);
	BackgroundComponent->SetRelativeLocation(FVector(0, 0, 0));
	BackgroundComponent->AddRelativeLocation(bgLoc);

	// Let Blueprint do the rest
	Super::BeginPlay();
}

// Called every frame
void ATool::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

/*---------------------------------------------------------  Rotate camera  ----------------------------------------------------------*/

void ATool::ToogleRotationMode()
{
	/* This function allows us to switch between RotationMode and NonRotationMode (sorry I really wasn't inspired for the name --')
		In rotation mode the camera is always pointing at the center of mass of the mesh, this allows us to rotate around the tool as we want while always having the camera pointing towards the tool.
		Basically the only time you won't be in rotation mode is when you want to reproduce a viewpoint from some camera extrinsics, for exemple in order to have rendered images with the same orientatino as some real images
			In this case we don't need the camera to always point at the tool because this is determined by loaded extrinsincs
	*/
	if (!isInRotationMode) {
		SpringArm->AttachToComponent(CoMComponent, FAttachmentTransformRules::KeepRelativeTransform);
		SpringArm2->TargetArmLength = MinArmLength;
		SpringArm->SetRelativeLocation(FVector(0,0,0));
		SpringArm2->SetRelativeRotation(initialRot);
		isInRotationMode = true;
		UE_LOG(LogTemp, Warning, TEXT("Rotation mode activated"));
	}
	else {
		SpringArm->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
		SpringArm2->TargetArmLength = 0;
		isInRotationMode = false;
		UE_LOG(LogTemp, Warning, TEXT("Rotation mode deactivated"));
	}
	SpringArm->OnAttachmentChanged();
	Tick(0.01);
}

void ATool::RotateCamera(FRotator Rotator)
{
	/*The Tool class is designed so that rotate the camera is equivqlent to applying a rotation to the SpringArm (Not SpringArm2)*/
	SpringArm->SetRelativeRotation(FQuat(Rotator));
}

void ATool::RandomRotateCamera(bool randomizeLightning, bool randomizeBackground, float intensity, bool randomizeArmLength, bool randomTranslate) {
	/* This function randomly rotate the camera and, based on the input, randomly change the lightning, the background, the distance between camera and object or randomly translate the camera
		The intensity parameter is the intensity of the lightning in case randomizeLightning is set to False
		Default values are as respectively: true, true, 10, false, false
	*/
	FRotator Rotator;
	Rotator.Yaw = FMath::FRandRange(0, 360);
	Rotator.Pitch = FMath::FRandRange(0, 360);
	Rotator.Roll = FMath::FRandRange(0, 360);
	RotateCamera(Rotator);
	while (Camera->GetComponentLocation().Z < CoMComponent->GetComponentLocation().Z) {
		Rotator.Yaw = FMath::FRandRange(0, 360);
		Rotator.Pitch = FMath::FRandRange(0, 360);
		Rotator.Roll = FMath::FRandRange(0, 360);
		RotateCamera(Rotator);
	}
	GetWorld()->UpdateActorComponentEndOfFrameUpdateState(SpringArm);
	float ArmLength = FMath::FRandRange(MinArmLength, MaxArmLenght);
	if (randomizeArmLength) {
		SpringArm2->TargetArmLength = ArmLength;
	}
	GetWorld()->UpdateActorComponentEndOfFrameUpdateState(SpringArm2);
	GetWorld()->UpdateWorldComponents(false, false);
	
	if (randomizeBackground) {
		RandomChangeBackground();
	}
	if (randomizeLightning) {
		RandomChangeLighting();
		EnlightCameraAxis(FMath::FRandRange(1, 20));
	}
	else {
		EnlightCameraAxis(intensity);
	}
	if (randomTranslate) {
		/*For some reason this part doesn't work as expected, this need to be fixed in order to produce some Yolo database*/
		float xOffset = FMath::FRandRange(-200, 200);
		float yOffset = FMath::FRandRange(-300, 300);
		float zOffset = FMath::FRandRange(-200, SpringArm2->TargetArmLength);
		SpringArm->SetRelativeLocation(FVector(xOffset, yOffset, zOffset));
		GetWorld()->UpdateActorComponentEndOfFrameUpdateState(SpringArm);
	}
}

void ATool::RotateCameraFromAxisAngles(FVector AxisAnglesRightHanded) {
	/* This is function is the ghost function:
		It is useless...
		It doesn't do anything...
		But it's here...
		Always around watchin...
		Like a ghost...
		In the shell...
	*/
}

/*------------------------------------------------------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------------------------------------------------*/





/*-------------------------------------------------------   Test   -----------------------------------------------------------------*/
void ATool::Touch() {
	/* This is the function will be called when pressing the T key
		It's actions can also be defined via blueprint with the blueprint event Touched*/
	Touched(); // This raises the blueprint event Touched
	
}

void ATool::VisualizeCameraPoses()
{
	/* This fuction reads the files in the Orientation and Translation directory and place a mesh in the corresponding camera location
		This is mostly for debugging purpose
	*/
	FString fileLoadDirRot = fileLoadDir + "/Orientations";
	FString fileLoadDirTrans = fileLoadDir + "/Translations";
	for (auto& Str : TransFiles)
	{
		UE_LOG(LogTemp, Warning, TEXT("Translation: %s"), *Str);
		FVector loc;
		ReadTranslation(*Str, &loc);
		loc.Y = -loc.Y;
		UE_LOG(LogTemp, Warning, TEXT("Yolo coodinates loaded: %f, %f, %f"), loc.X, loc.Y, loc.Z);
		Poses->AddInstance(FTransform(FRotator(0, 0, 0), Camera->GetForwardVector() * MeshScale*10000, FVector(1, 1, 1)));
		
	}
}

void ATool::Touched_Implementation() {
	UE_LOG(LogTemp, Warning, TEXT("Touched has been called"));
	
}

/*------------------------------------------------------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------------------------------------------------*/


/*------------------------------------------------   Lights   --------------------------------------------------------*/
void ATool::RandomChangeLighting()
{
	Light1->SetIntensity(FMath::FRandRange(0, 20));
	FRotator Rotator;
	Rotator.Yaw = FMath::FRandRange(0, 360);
	Rotator.Pitch = FMath::FRandRange(0, 360);
	Rotator.Roll = FMath::FRandRange(0, 360);
	Light1->SetRelativeRotation(Rotator);

	Light2->SetIntensity(FMath::FRandRange(0, 20));
	Rotator.Yaw = FMath::FRandRange(0, 360);
	Rotator.Pitch = FMath::FRandRange(0, 360);
	Rotator.Roll = FMath::FRandRange(0, 360);
	Light2->SetRelativeRotation(Rotator);

	/*Light3->SetIntensity(FMath::FRandRange(0, 20));
	Rotator.Yaw = FMath::FRandRange(0, 360);
	Rotator.Pitch = FMath::FRandRange(0, 360);
	Rotator.Roll = FMath::FRandRange(0, 360);
	Light3->SetRelativeRotation(Rotator);

	Light1->SetIntensity(FMath::FRandRange(0, 20));
	Rotator.Yaw = FMath::FRandRange(0, 360);
	Rotator.Pitch = FMath::FRandRange(0, 360);
	Rotator.Roll = FMath::FRandRange(0, 360);
	Light1->SetRelativeRotation(Rotator);*/
}

void ATool::EnlightCameraAxis(float intensity)
{
	Light1->SetIntensity(intensity);
	Light1->SetWorldRotation(Camera->GetComponentRotation());
}

/*------------------------------------------------------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------------------------------------------------*/




/*------------------------------------------------   Background   --------------------------------------------------------*/
void ATool::RandomChangeBackground_Implementation()
{
	int MaterialIndex = FMath::RoundToInt(FMath::FRand()*(MaterialsArray.Num()-1));
	BackgroundMesh->SetMaterial(0, MaterialsArray[MaterialIndex]);
	//BackgroundMesh->GetMaterial(0)->UpdateLightmassTextureTracking();
	UE_LOG(LogTemp, Warning, TEXT("Background: %s"), *MaterialsArray[MaterialIndex]->GetName());
}

/*------------------------------------------------------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------------------------------------------------*/




/*------------------------------------------------   Saving/Reading the images/texts   --------------------------------------------------------*/
bool ATool::GoToCameraPose(FString FilePath, bool visualizeCameraPose, bool saveImageFromCameraPose)
{
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	if (PlatformFile.FileExists(*FilePath)) {
		// Get the current camera poses and put a sphere at this location to visualize it
		FVector loc;
		ReadTranslation(*FilePath, &loc);
		//UE_LOG(LogTemp, Warning, TEXT("Translation file: %s"), *FilePath);
		loc.Y = -loc.Y;
		if (visualizeCameraPose) {
			Poses->AddInstance(FTransform(FRotator(0, 0, 0), MeshScale * loc, FVector(0.2, 0.2, 0.2)));
		}

		// Add offset to the SpringArm target to place the camera at the current camera pose location
		SpringArm->SetRelativeLocation(MeshScale * loc);

		// Deduce the path of the corresponding rotation based on the translation file path
		FString PathWithoutFile, FileName;
		FilePath.Split("/", &PathWithoutFile, &FileName, ESearchCase::IgnoreCase, ESearchDir::FromEnd);
		FString RotationFilePath = fileLoadDirRot + "/" + FileName;

		// Get the corresponding rotation from the file
		FVector RightHandedAxisAngles;
		ReadRotation(RotationFilePath, &RightHandedAxisAngles);

		// Deduce the Left-handed corresponding rotator from the Right-handed axis angles and apply it to the SpringArm
		float x(RightHandedAxisAngles.X), y(RightHandedAxisAngles.Y), z(RightHandedAxisAngles.Z);
		double angle = FMath::Sqrt(x*x + y*y + z*z);
		AxisAnglesVector = FVector(x, y, z);
		FRotator rot = UKismetMathLibrary::RotatorFromAxisAndAngle(FVector(x / angle, -y / angle, z / angle), (360 * angle) / (2 * PI));
		SpringArm->SetRelativeRotation(FQuat(rot));

		//Place the background
		BackgroundComponent->SetRelativeLocation(FVector(0, 0, 0));
		BackgroundComponent->AddRelativeLocation(FVector(BackgroundDistance, 0, 0));

		// Save the image if demanded
		/* Find a way to add some delay here*/
		if (saveImageFromCameraPose) {
			FString FileBaseName, extension;
			FileName.Split(".", &FileBaseName, &extension);
			fileSaveName = FileBaseName;
			if (SaveImage(FileBaseName)) {
				UE_LOG(LogTemp, Warning, TEXT("Image %s saved"), *FileBaseName);
			}
			else {
				UE_LOG(LogTemp, Warning, TEXT("Image %s was not saved"), *FileBaseName);
			}

		}

		return true;
	}
	else {
		return false;
	}
}

bool ATool::GoToNextCameraPose(bool visualizeCameraPose, bool saveImageFromCameraPose)
{
	// Get the rotations and translations file names

	bool isEverythingOk = false;
	// Verify that the file exists
	if (index < TransFiles.Num() - 1) {
		FString FilePath = TransFiles[index];
		IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
		if (PlatformFile.FileExists(*FilePath)) {
			if (GoToCameraPose(FilePath, visualizeCameraPose, saveImageFromCameraPose)) {
				index++;
				isEverythingOk = true;
			}
		}
	}
	return isEverythingOk;
}

bool ATool::GoToAllCameraPoses(bool visualizeCameraPose, bool randomizeLightning, bool randomizeBackground, float intensity, bool saveImageFromCameraPose)
{
	// Get the rotations and translations file names
	bool isEverythingOk = true;

	for (int32 i = 0; i < RotFiles.Num(); i++) {
		// Verify that the file exists
		FString FilePath = TransFiles[i];
		IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
		if (PlatformFile.FileExists(*FilePath)) {
			if (randomizeBackground) {
				RandomChangeBackground();
			}

			if (randomizeLightning) {
				RandomChangeLighting();
				EnlightCameraAxis(FMath::FRandRange(5, 20));
			}
			else {
				EnlightCameraAxis(intensity);
			}
			if (!GoToCameraPose(FilePath, visualizeCameraPose, saveImageFromCameraPose)) {
				isEverythingOk = false;
			}
		}
	}

	return isEverythingOk;
}


bool ATool::SaveImage(FString FileBaseName, bool increment) {
	// If the fileName and dirName are specified then store the image that the camera capture is currently seeing
	if (FileBaseName == "") {
		FileBaseName = fileSaveName + FString::FromInt(index);
		if (increment) {
			index++;
		}
	}

	// Be sure to update the image
	Skylight->RecaptureSky();
	FSceneInterface *SceneInterface = GetWorld()->Scene;
	SceneInterface->UpdateSceneSettings(GetWorld()->GetWorldSettings());
	SceneInterface->UpdateSceneCaptureContents(CameraCapture);
	CameraCapture->CaptureScene();

	if (fileSaveDir != "" && fileSaveName != "") {
		IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
		FString fileSaveDirRot = fileSaveDir + "/output/Orientations";
		FString fileSaveDirIm = fileSaveDir + "/Images";
		FString fileSaveDirTra = fileSaveDir + "/output/Translations";
		if (PlatformFile.CreateDirectoryTree(*fileSaveDirIm)) {
			

			// Save the image
			FString fileNameIndexed = FileBaseName + ".png";
			UE_LOG(LogTemp, Warning, TEXT("Saving the image %s"), *fileNameIndexed);
			UKismetRenderingLibrary::ExportRenderTarget(GetWorld(), CameraCapture->TextureTarget, fileSaveDirIm, fileNameIndexed);
			//UE_LOG(LogTemp, Warning, TEXT("Image saved"));

			// Save the corresponding rotation in a text file
			if (PlatformFile.CreateDirectoryTree(*fileSaveDirRot)) {
				// Construct the name of the file
				fileNameIndexed = FileBaseName + ".txt";
				//UE_LOG(LogTemp, Warning, TEXT("Saving the rotation %s"), *fileNameIndexed);
				FString AbsoluteFilePath = fileSaveDirRot + "/" + fileNameIndexed;
				// Compute the Right-handed axis angles based on the relative rotator
				FRotator spr = SpringArm->GetRelativeTransform().Rotator();
				FQuat sprq = FQuat(spr);
				float angle;
				FVector aa;
				sprq.ToAxisAndAngle(aa, angle);

				FString TextToSave = " " + FString::SanitizeFloat(angle*aa.X) + " " + FString::SanitizeFloat(-angle*aa.Y) + " " + FString::SanitizeFloat(angle*aa.Z);
				FFileHelper::SaveStringToFile(TextToSave, *AbsoluteFilePath);
				//UE_LOG(LogTemp, Warning, TEXT("Rotation saved"));
				if (PlatformFile.CreateDirectoryTree(*fileSaveDirTra)) {
					// Construct the name of the file
					fileNameIndexed = FileBaseName + ".txt";
					//UE_LOG(LogTemp, Warning, TEXT("Saving the translation %s"), *fileNameIndexed);
					FString AbsoluteFilePath = fileSaveDirTra + "/" + fileNameIndexed;
					// Compute the Right-handed translation
					FVector translation = Camera->GetComponentLocation() - RootComponent->GetComponentLocation();
					translation = RootComponent->GetComponentRotation().GetInverse().RotateVector(translation);
					//FVector translation = Camera->GetRelativeTransform().GetLocation() + SpringArm2->GetRelativeTransform().GetLocation() + SpringArm->GetRelativeTransform().GetLocation();
					translation = translation / MeshScale;
					FString TextToSave = " " + FString::SanitizeFloat(translation.X) + " " + FString::SanitizeFloat(-translation.Y) + " " + FString::SanitizeFloat(translation.Z);
					FFileHelper::SaveStringToFile(TextToSave, *AbsoluteFilePath);
					//UE_LOG(LogTemp, Warning, TEXT("Translation saved"));
					return true;
				}
				else {
					UE_LOG(LogTemp, Warning, TEXT("Problem occured, the translations directory doesn't exist and was not created"));
					return false;
				}
			}
			else {
				UE_LOG(LogTemp, Warning, TEXT("Problem occured, the rotations directory doesn't exist and was not created"));
				return false;
			}
		}
		else {
			UE_LOG(LogTemp, Warning, TEXT("Problem occured, the images directory doesn't exist and was not created"));
			return false;
		}
		
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("You need to specify a directory and a file name"));
		return false;
	}
	
}

TArray<FString> ATool::GetFilesNames(FString DirPath) {
	const TCHAR* Directory = *DirPath;
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	
	// Array that will be filled up with the files names
	TArray<FString> FilesNames;

	// Create a class inherit from FDirectoryVisitor that implements the Visit method
	class FMyVisitor : public IPlatformFile::FDirectoryVisitor
	{
	public:
		TArray<FString> *FilesNames;
	public:
		FMyVisitor(TArray<FString> *FilesNamesArray)
		{
			FilesNames = FilesNamesArray;
		}
		virtual bool Visit(const TCHAR* Filename, bool bIsDirectory)
		{
			if (bIsDirectory)
			{
				//DoSomethingToDirectory(Filename);
			}
			else
			{
				FilesNames->Add(FString(Filename));
			}
			return true; // continue searching
		}
	};

	FMyVisitor MyVisitor(&FilesNames);
	PlatformFile.IterateDirectory(Directory, MyVisitor);
	return FilesNames;
}

bool ATool::ReadRotation() {
	return false;
}


bool ATool::ReadRotation(FString FilePath, FVector *AxisAngles)
{
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	const TCHAR* AbsoluteFilePath = *FilePath;
	if (PlatformFile.FileExists(AbsoluteFilePath)) {
		FString TextRead;
		if (FFileHelper::LoadFileToString(TextRead, AbsoluteFilePath)) {
			FString TextReadTmp1, TextReadTmp2;
			TextRead.Split(" ", &TextReadTmp1, &TextReadTmp2);
			FString x, y, y2, z;
			if (TextReadTmp2.Split(FString(" "), &x, &y2) && y2.Split(FString(" "), &y, &z)) {
				float X, Y, Z;
				const TCHAR* x_ = *x;
				X = FCString::Atof(x_);
				const TCHAR* y_ = *y;
				Y = FCString::Atof(y_);
				const TCHAR* z_ = *z;
				Z = FCString::Atof(z_);
				AxisAngles->X = X;
				AxisAngles->Y = Y;
				AxisAngles->Z = Z;
				//UE_LOG(LogTemp, Warning, TEXT("AxisAngles coodinates loaded: %f, %f, %f"), AxisAngles->X, AxisAngles->Y, AxisAngles->Z);
				return true;
			}
			else {
				UE_LOG(LogTemp, Warning, TEXT("The coordinates seem not to be written correctly in the specified txt file"));
				return false;
			}
		}
		else {
			UE_LOG(LogTemp, Warning, TEXT("File failed to be read"));
			return false;
		}
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("The specified file doesn't exist"));
		return false;
	}
}


bool ATool::ReadTranslation() {
	return false;
}

bool ATool::ReadTranslation(FString FilePath, FVector *TranslationVector)
{
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	const TCHAR* AbsoluteFilePath = *FilePath;
	if (PlatformFile.FileExists(AbsoluteFilePath)) {
		FString TextRead;
		if (FFileHelper::LoadFileToString(TextRead, AbsoluteFilePath)) {
			FString TextReadTmp1, TextReadTmp2;
			TextRead.Split(" ", &TextReadTmp1, &TextReadTmp2);
			FString x, y, y2, z;
			if (TextReadTmp2.Split(FString(" "), &x, &y2) && y2.Split(FString(" "), &y, &z)) {
				float X, Y, Z;
				const TCHAR* x_ = *x;
				X = FCString::Atof(x_);
				const TCHAR* y_ = *y;
				Y = FCString::Atof(y_);
				const TCHAR* z_ = *z;
				Z = FCString::Atof(z_);
				TranslationVector->X = X;
				TranslationVector->Y = Y;
				TranslationVector->Z = Z;
				//UE_LOG(LogTemp, Warning, TEXT("Translation coodinates loaded: %f, %f, %f"), TranslationVector->X, TranslationVector->Y, TranslationVector->Z);
				return true;
			}
			else {
				UE_LOG(LogTemp, Warning, TEXT("The coordinates seem not to be written correctly in the specified txt file"));
				return false;
			}
		}
		else {
			UE_LOG(LogTemp, Warning, TEXT("File failed to be read"));
			return false;
		}
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("The specified file doesn't exist"));
		return false;
	}
}

void ATool::GenerateRandomRotationsData(int nbRot, bool randomizeLightning, bool randomizeBackground, float intensity, bool randomizeArmLength, bool randomTranslate)
{
	if (!isInRotationMode) {
		ToogleRotationMode();
		UE_LOG(LogTemp, Warning, TEXT("Activate the rotation mode before using the random rotation generation"));
	}
	else {
		for (int i = 0; i < nbRot; i++) {
			RandomRotateCamera(randomizeLightning, randomizeBackground, intensity, randomizeArmLength, randomTranslate);
			SaveImage();
		}
	}	
}

void ATool::GenerateNonRandomRotationsData(float degreeStep, bool randomizeLightning, bool randomizeBackground, float intensity)
{
	if (!isInRotationMode) {
		ToogleRotationMode();
		UE_LOG(LogTemp, Warning, TEXT("Activate the rotation mode before using the random rotation generation"));
	}
	else {
		FRotator Rotator = FRotator(0, 0, 0);
		for (int x = 0; x < 360; x += degreeStep) {
			Rotator.Roll = x;
			for (int y = 0; y < 360; y += degreeStep) {
				Rotator.Pitch = y;
				for (int z = 0; z < 360; z += degreeStep) {
					// Get rotator
					Rotator.Yaw = z;
					RotateCamera(Rotator);
					GetWorld()->UpdateWorldComponents(false, false);

					if (randomizeBackground) {
						RandomChangeBackground();
					}
					if (randomizeLightning) {
						RandomChangeLighting();
						EnlightCameraAxis(FMath::FRandRange(5, 20));
					}
					else {
						EnlightCameraAxis(intensity);
					}
					SaveImage();
				}
			}
			
		}
		UE_LOG(LogTemp, Warning, TEXT("Generation finished"));
	}
}

/*------------------------------------------------------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------------------------------------------------*/



void ATool::ProjectObject()
{
	// Define K
	K << 9.5971160723108721e+02, 0, 9.6055043729890588e+02,
		0, 9.6132797478587622e+02, 5.4029398159070877e+02,
		0, 0, 1;

}

