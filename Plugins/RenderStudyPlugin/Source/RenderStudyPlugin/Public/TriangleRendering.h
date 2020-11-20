// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GlobalShader.h"

/**
 * 
 */
class FTriangleRenderingShader : public FGlobalShader
{
	DECLARE_INLINE_TYPE_LAYOUT(FTriangleRenderingShader, NonVirtual);

public:
	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::ES3_1);
	}

	FTriangleRenderingShader() {}

	FTriangleRenderingShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer);

	template<typename TShaderRHIParamRef>
	void SetParameters(FRHICommandListImmediate& RHICmdList, const TShaderRHIParamRef ShaderRHI);

private:
	//LAYOUT_FIELD(FShaderParameter, PixelUVSize);
};

class FTriangleRenderingShaderVS : public FTriangleRenderingShader
{
	DECLARE_SHADER_TYPE(FTriangleRenderingShaderVS, Global);
public:

	FTriangleRenderingShaderVS() {}

	FTriangleRenderingShaderVS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FTriangleRenderingShader(Initializer)
	{}
};

class FTriangleRenderingShaderPS : public FTriangleRenderingShader
{
	DECLARE_SHADER_TYPE(FTriangleRenderingShaderPS, Global);
public:

	FTriangleRenderingShaderPS() {}

	FTriangleRenderingShaderPS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FTriangleRenderingShader(Initializer)
	{}
};

class FDrawTriangle
{
public:
	FDrawTriangle() {};

	void Render(class UTextureRenderTarget2D* OutputRenderTarget);
};

static void DrawTriangle_RenderThread(FRHICommandListImmediate& RHICmdList, class FTextureRenderTargetResource* OutTextureRenderTargetResource, ERHIFeatureLevel::Type FeatureLevel);
