#pragma once
//
#include "CoreMinimal.h"
#include "GenericPlatform/GenericPlatformMisc.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Components/BoxComponent.h"
#include "ComputeShaderSceneCapture.h"
//
#include "ComputeShaderMeshFill.generated.h"
//
//
// //This struct act as a container for all the parameters that the client needs to pass to the Compute Shader Manager.
struct OOOCOMPUTESHADER_API FMeshFillCSParameters
{
	int X;
	int Y;
	int Z;

	FRenderTarget* ColorRT;
	FRenderTarget* UVRT;
	FRenderTarget* DrawRT;
	TArray<float> Sizes;
	
	FMeshFillCSParameters(int x, int y, int z)
		: X(x)
		, Y(y)
		, Z(z)
	{
	}
};

USTRUCT(BlueprintType)
struct OOOCOMPUTESHADER_API FCSGenerateParameter
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Options)
	UTexture2D* TMeshDepth;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Options)
	UTextureRenderTarget2D* Mask;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Options)
	UTextureRenderTarget2D* SceneDepth;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Options)
	UTextureRenderTarget2D* DebugView;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Options)
	UTextureRenderTarget2D* Result;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Options)
	UTextureRenderTarget2D* SceneNormal;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Options)
	UTextureRenderTarget2D* CurrentSceneDepth;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Options)
	float Size = 1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Options)
	float RandomRoation = 0;

	bool IsValid()
	{
		return TMeshDepth != nullptr && SceneDepth != nullptr &&
			SceneNormal != nullptr && DebugView != nullptr && Result != nullptr;
	}
	bool IsValidMult()
	{
		return TMeshDepth != nullptr && SceneDepth != nullptr &&
			SceneNormal != nullptr && DebugView != nullptr && Result != nullptr &&
			CurrentSceneDepth != nullptr;
	}
};

class OOOCOMPUTESHADER_API FMeshFillCSInterface
{
public:
	// Executes this shader on the render thread

	static void DispatchRenderThread(FRHICommandListImmediate& RHICmdList, FMeshFillCSParameters Params);

	// Executes this shader on the render thread from the game thread via EnqueueRenderThreadCommand
	static void DispatchGameThread(FMeshFillCSParameters Params)
	{
		ENQUEUE_RENDER_COMMAND(SceneDrawCompletion)(
			[Params](FRHICommandListImmediate& RHICmdList)
			{
				DispatchRenderThread(RHICmdList, Params);
			});
	}

	// Dispatches this shader. Can be called from any thread

	static void Dispatch(FMeshFillCSParameters Params)
	{
		if (IsInRenderingThread())
		{
			DispatchRenderThread(GetImmediateCommandList_ForRenderCommand(), Params);
		}
		else
		{
			DispatchGameThread(Params);
		}
	}

};

UCLASS() // Change the _API to match your project
class OOOCOMPUTESHADER_API UComputeShaderMeshFill : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:

	// Execute the actual load
	//template<typename T>
	virtual void Activate() override
	{
		// Create a dispatch parameters struct and set our desired seed
		FMeshFillCSParameters Params(ColorRT->SizeX, ColorRT->SizeY, 1);
		Params.ColorRT = ColorRT->GameThread_GetRenderTargetResource();
		Params.UVRT = UVRT->GameThread_GetRenderTargetResource();
		Params.DrawRT = DrawRT->GameThread_GetRenderTargetResource();

		// Dispatch the compute shader and wait until it completes
		FMeshFillCSInterface::Dispatch(Params);
	}

	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", Category = "ComputeShader", WorldContext = "WorldContextObject"))
	static UComputeShaderMeshFill* ExecuteDrawHoldTexture(UObject* WorldContextObject, UTextureRenderTarget2D* ColorRT, UTextureRenderTarget2D* UVRT, UTextureRenderTarget2D* DrawRT)
	{
		UComputeShaderMeshFill* Action = NewObject<UComputeShaderMeshFill>();
		Action->ColorRT = ColorRT;
		Action->UVRT = UVRT;
		Action->DrawRT = DrawRT;
		Action->RegisterWithGameInstance(WorldContextObject);
		
		return Action;
	}
	
	UTextureRenderTarget2D* DrawRT;
	UTextureRenderTarget2D* ColorRT;
	UTextureRenderTarget2D* UVRT;
	
};

UCLASS()
class OOOCOMPUTESHADER_API UComputeShaderMeshFillFunctions : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	UFUNCTION(BlueprintCallable, Category = "ComputeShader")
	static void CSMeshFill(ACSGenerateCaptureScene* Capturer, UStaticMesh* StaticMesh, UTextureRenderTarget2D* DubugView, UTextureRenderTarget2D*
	                       Result, UTexture2D* TMeshDepth, float SpawnSize = 1, float TestSizeScale = 1, FName Tag = FName(TEXT("Auto")));

	UFUNCTION(BlueprintCallable, Category = "ComputeShader")
	static void CSMeshFillMult(ACSGenerateCaptureScene* Capturer, UStaticMesh* StaticMesh, UTextureRenderTarget2D* DubugView, UTextureRenderTarget2D*
	                           Result, UTextureRenderTarget2D* CurrentSceneDepth, UTexture2D* TMeshDepth, int32 Iteration = 100, float SpawnSize = 1, float
	                           TestSizeScale = 1, FName Tag = FName(TEXT("Auto")));

	
	UFUNCTION(BlueprintCallable, Category = "ComputeShader")
	static void CalculateMeshLoctionAndRotation(FCSGenerateParameter Params);

	UFUNCTION(BlueprintCallable, Category = "ComputeShader")
	static void CalculateMeshLoctionAndRotationMult(FCSGenerateParameter Params, int32 NumIteraion);

	UFUNCTION(BlueprintCallable, Category = "ComputeShader")
	static void DrawHeightMap(UTextureRenderTarget2D* InHeight, UTexture2D* InTMeshDepth, float Size, float Rotator);

};

