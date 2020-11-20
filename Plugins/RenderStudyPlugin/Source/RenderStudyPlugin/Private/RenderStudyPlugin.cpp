// Copyright Epic Games, Inc. All Rights Reserved.

#include "RenderStudyPlugin.h"
#include "Interfaces/IPluginManager.h"

#define LOCTEXT_NAMESPACE "FRenderStudyPluginModule"

void FRenderStudyPluginModule::StartupModule()
{
	FString PluginShaderDir = FPaths::Combine(IPluginManager::Get().FindPlugin(TEXT("RenderStudyPlugin"))->GetBaseDir(), TEXT("Shaders"));
	AddShaderSourceDirectoryMapping(TEXT("/Plugin/RenderStudyPlugin"), PluginShaderDir);
}

void FRenderStudyPluginModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FRenderStudyPluginModule, RenderStudyPlugin)