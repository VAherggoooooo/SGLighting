#include "SGPathTracing.h"

#include "MakeLightmapRDG.h"
#include "RenderGraphBuilder.h"
#include "RenderGraphUtils.h"



IMPLEMENT_GLOBAL_SHADER(FSGComputeShader_PT, "/Plugins/SGLighting/Private/SGPathTracing.usf", "MainCS", SF_Compute);

