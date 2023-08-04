#include "MyCustomShader/Data/Public/ShaderDefine.h"


IMPLEMENT_GLOBAL_SHADER(FMyVertexShader, "/Plugins/MyCustomShader/MyTestShader.usf", "MainVS", SF_Vertex);
IMPLEMENT_GLOBAL_SHADER(FMyPixelShader, "/Plugins/MyCustomShader/MyTestShader.usf", "MainPS", SF_Pixel);