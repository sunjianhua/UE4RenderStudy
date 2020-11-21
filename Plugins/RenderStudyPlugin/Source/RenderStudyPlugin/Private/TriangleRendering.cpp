// Fill out your copyright notice in the Description page of Project Settings.

#include "TriangleRendering.h"
#include "TextureResource.h"
#include "Engine/World.h"
#include "SceneInterface.h"
#include "RHIStaticStates.h"
#include "PipelineStateCache.h"
#include "ShaderParameterUtils.h"

//
// 三角形数据描述 相关
//
struct FColorVertex {
	FVector2D Position;
	FVector4 Color;
};

class FTriangleVertexBuffer : public FVertexBuffer
{
public:
	void InitRHI() override
	{
		TResourceArray<FColorVertex, VERTEXBUFFER_ALIGNMENT> Vertices;
		Vertices.SetNumUninitialized(3);

		Vertices[0].Position = FVector2D(0, 0.75);
		Vertices[0].Color = FVector4(1, 0, 0, 1);

		Vertices[1].Position = FVector2D(0.75, -0.75);
		Vertices[1].Color = FVector4(0, 1, 0, 1);

		Vertices[2].Position = FVector2D(-0.75, -0.75);
		Vertices[2].Color = FVector4(0, 0, 1, 1);

		FRHIResourceCreateInfo CreateInfo(&Vertices);
		VertexBufferRHI = RHICreateVertexBuffer(Vertices.GetResourceDataSize(), BUF_Static, CreateInfo);
	}
};

TGlobalResource<FTriangleVertexBuffer> GTriangleVertexBuffer;

class FTriangleVertexDeclaration : public FRenderResource
{
public:
	FVertexDeclarationRHIRef VertexDeclarationRHI;
	virtual void InitRHI() override
	{
		FVertexDeclarationElementList Elements;
		uint32 Stride = sizeof(FColorVertex);
		Elements.Add(FVertexElement(0, STRUCT_OFFSET(FColorVertex, Position), VET_Float2, 0, Stride));
		Elements.Add(FVertexElement(0, STRUCT_OFFSET(FColorVertex, Color), VET_Float4, 1, Stride));
		VertexDeclarationRHI = RHICreateVertexDeclaration(Elements);
	}
	virtual void ReleaseRHI() override
	{
		VertexDeclarationRHI.SafeRelease();
	}
};

TGlobalResource<FTriangleVertexDeclaration> GTriangleVertexDeclaration;

class FTriangleIndexBuffer : public FIndexBuffer
{
public:
	void InitRHI() override
	{
		const uint16 Indices[] = { 0, 1, 2 };

		TResourceArray<uint16, INDEXBUFFER_ALIGNMENT> IndexBuffer;
		uint32 NumIndices = ARRAY_COUNT(Indices);
		IndexBuffer.AddUninitialized(NumIndices);
		FMemory::Memcpy(IndexBuffer.GetData(), Indices, NumIndices * sizeof(uint16));

		FRHIResourceCreateInfo CreateInfo(&IndexBuffer);
		IndexBufferRHI = RHICreateIndexBuffer(sizeof(uint16), IndexBuffer.GetResourceDataSize(), BUF_Static, CreateInfo);
	}
};

TGlobalResource<FTriangleIndexBuffer> GTriangleIndexBuffer;

//
// FTriangleRenderingShader
//
FTriangleRenderingShader::FTriangleRenderingShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer) : FGlobalShader(Initializer)
{
	TestOutColor.Bind(Initializer.ParameterMap, TEXT("TestOutColor"));
}

template<typename TShaderRHIParamRef>
void FTriangleRenderingShader::SetParameters(FRHICommandListImmediate& RHICmdList, const TShaderRHIParamRef ShaderRHI)
{
	FLinearColor NewColorValue(1.f, 0.f, 0.f, 1.f);

	SetShaderValue(RHICmdList, ShaderRHI, TestOutColor, NewColorValue);
}

//
// IMPLEMENT_SHADER_TYPE
//
IMPLEMENT_SHADER_TYPE(, FTriangleRenderingShaderVS, TEXT("/Plugin/RenderStudyPlugin/RenderStudy.usf"), TEXT("MainVS"), SF_Vertex);
IMPLEMENT_SHADER_TYPE(, FTriangleRenderingShaderPS, TEXT("/Plugin/RenderStudyPlugin/RenderStudy.usf"), TEXT("MainPS"), SF_Pixel);

//
// FDrawTriangle
//
void FDrawTriangle::Render(UTextureRenderTarget2D* OutputRenderTarget)
{
	check(IsInGameThread());

	check(GWorld && GWorld->Scene);

	FTextureRenderTargetResource* TextureRenderTargetResource = OutputRenderTarget->GameThread_GetRenderTargetResource();

	ERHIFeatureLevel::Type FeatureLevel = GWorld->Scene->GetFeatureLevel();

	ENQUEUE_RENDER_COMMAND(CaptureCommand)(
		[TextureRenderTargetResource, FeatureLevel](FRHICommandListImmediate& RHICmdList)
		{
			DrawTriangle_RenderThread(
				RHICmdList,
				TextureRenderTargetResource,
				FeatureLevel);
		}
	);
}

//
// DrawTriangle_RenderThread
//
void DrawTriangle_RenderThread(FRHICommandListImmediate& RHICmdList, class FTextureRenderTargetResource* OutTextureRenderTargetResource, ERHIFeatureLevel::Type FeatureLevel)
{
	check(IsInRenderingThread());

#if WANTS_DRAW_MESH_EVENTS
	FString EventName(TEXT("DrawTriangle"));
	SCOPED_DRAW_EVENTF(RHICmdList, SceneCapture, TEXT("RenderStudy %s"), *EventName);
#endif

	FRHITexture2D* RenderTargetTexture = OutTextureRenderTargetResource->GetRenderTargetTexture();

	RHICmdList.TransitionResource(EResourceTransitionAccess::EWritable, RenderTargetTexture);

	FRHIRenderPassInfo RPInfo(RenderTargetTexture, ERenderTargetActions::DontLoad_Store, OutTextureRenderTargetResource->TextureRHI);
	RHICmdList.BeginRenderPass(RPInfo, TEXT("DrawTriangle"));
	{
		FIntPoint DisplacementMapResolution(OutTextureRenderTargetResource->GetSizeX(), OutTextureRenderTargetResource->GetSizeY());

		// Update viewport.
		RHICmdList.SetViewport(
			0, 0, 0.f,
			DisplacementMapResolution.X, DisplacementMapResolution.Y, 1.f);

		// Get shaders.
		FGlobalShaderMap* GlobalShaderMap = GetGlobalShaderMap(FeatureLevel);
		TShaderMapRef< FTriangleRenderingShaderVS > VertexShader(GlobalShaderMap);
		TShaderMapRef< FTriangleRenderingShaderPS > PixelShader(GlobalShaderMap);

		// Set the graphic pipeline state.
		FGraphicsPipelineStateInitializer GraphicsPSOInit;
		RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);
		GraphicsPSOInit.DepthStencilState = TStaticDepthStencilState<false, CF_Always>::GetRHI();
		GraphicsPSOInit.BlendState = TStaticBlendState<>::GetRHI();
		GraphicsPSOInit.RasterizerState = TStaticRasterizerState<>::GetRHI();
		GraphicsPSOInit.PrimitiveType = PT_TriangleList;
		GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GTriangleVertexDeclaration.VertexDeclarationRHI;
		GraphicsPSOInit.BoundShaderState.VertexShaderRHI = VertexShader.GetVertexShader();
		GraphicsPSOInit.BoundShaderState.PixelShaderRHI = PixelShader.GetPixelShader();
		SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit);

		// Update viewport.
		RHICmdList.SetViewport(
			0, 0, 0.f,
			OutTextureRenderTargetResource->GetSizeX(), OutTextureRenderTargetResource->GetSizeY(), 1.f);

		// VertexShader->SetParameters(RHICmdList, VertexShader.GetVertexShader());
		PixelShader->SetParameters(RHICmdList, PixelShader.GetPixelShader());

		// Set VertextBuffer
		RHICmdList.SetStreamSource(0, GTriangleVertexBuffer.VertexBufferRHI, 0);

		RHICmdList.DrawIndexedPrimitive(
			GTriangleIndexBuffer.IndexBufferRHI,
			/*BaseVertexIndex=*/ 0,
			/*MinIndex=*/ 0,
			/*NumVertices=*/ 3,
			/*StartIndex=*/ 0,
			/*NumPrimitives=*/ 1,
			/*NumInstances=*/ 1);
	}
	RHICmdList.EndRenderPass();
}
