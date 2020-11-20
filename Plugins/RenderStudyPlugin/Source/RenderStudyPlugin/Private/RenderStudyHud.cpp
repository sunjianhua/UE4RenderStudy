// Fill out your copyright notice in the Description page of Project Settings.

#include "RenderStudyHud.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"
#include "TriangleRendering.h"

ARenderStudyHud::ARenderStudyHud(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	RenderTarget = NewObject<UTextureRenderTarget2D>(GetTransientPackage(), NAME_None, RF_Transient);
	RenderTarget->RenderTargetFormat = RTF_RGBA8;
	RenderTarget->InitAutoFormat(1024, 1024);
	RenderTarget->ClearColor = FLinearColor::Blue;
	RenderTarget->UpdateResource();
}

void ARenderStudyHud::DrawHUD()
{
	Super::DrawHUD();

	ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(GetOwningPlayerController()->Player);
	if (!LocalPlayer)
	{
		return;
	}

	FDrawTriangle FDrawTriangle;
	FDrawTriangle.Render(RenderTarget);

	FIntPoint IntPoint = LocalPlayer->ViewportClient->Viewport->GetSizeXY();

	DrawTexture(RenderTarget, 0, 0, IntPoint.X, IntPoint.Y, 0, 0, 1, 1);

	//DrawLine(0.f, IntPoint.Y / 2, IntPoint.X, IntPoint.Y / 2, FLinearColor(1, 0, 0), 10.f);

	//DrawLine
}
