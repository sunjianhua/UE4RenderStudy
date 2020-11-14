// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "RenderStudyHud.generated.h"

/**
 * 
 */
UCLASS()
class RENDERSTUDYPLUGIN_API ARenderStudyHud : public AHUD
{
	GENERATED_UCLASS_BODY()

public:
	virtual void DrawHUD();

protected:
	class UTextureRenderTarget2D* RenderTarget;
};
