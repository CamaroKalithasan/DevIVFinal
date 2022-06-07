// Simple Pixel Shader
// TODO: Part 2b
struct obj_attributes
{
	float3    Kd; // diffuse reflectivity
	float	  d; // dissolve (transparency) 
	float3    Ks; // specular reflectivity
	float     Ns; // specular exponent
	float3    Ka; // ambient reflectivity
	float     sharpness; // local reflection map sharpness
	float3    Tf; // transmission filter
	float     Ni; // optical density (index of refraction)
	float3    Ke; // emissive reflectivity
	uint      illum; // illumination model
};
struct SHADER_VARS
{
	matrix VM;
	matrix PM;
	float4 LD;
	float4 LC;
	matrix matricies[1024];
	obj_attributes materials[1024];
    float4 ambient;
    float4 camPos;
};

// TODO: Part 4g
// TODO: Part 2i
StructuredBuffer<SHADER_VARS> SceneData;
// TODO: Part 3e
[[vk::push_constant]]
cbuffer MESH_INDEX
{
    uint materialOffset;
    uint WMoffset;
    uint materialIndex;
    float padding[29];
};

struct PS
{
    float4 pos : SV_POSITION;
    float3 uvw : UVW;
    float3 nrm : NORMAL;
    float3 posW : WORLD;
};
// an ultra simple hlsl pixel shader
// TODO: Part 4b
float4 main(PS pixelInd) : SV_TARGET 
{	
    //return float4(SceneData[0].materials[materialOffset + materialIndex].Kd, 1);
    float4 ambience = (SceneData[0].ambient) * float4(SceneData[0].materials[materialOffset + materialIndex].Kd, 1);
    //ambience = normalize(ambience);
	//return float4(1.0f ,0.0f, 0.0f, 0); // TODO: Part 1a
	// TODO: Part 3a
	//return float4(SceneData[0].materials[mesh_ID].Kd, 1);
	// TODO: Part 4c
    //float3 lightDir = float3(0, 0, 1);
    //lightDir = normalize(lightDir);
    pixelInd.nrm = normalize(pixelInd.nrm);
    float lightRatio = clamp(dot(-SceneData[0].LD.xyz, pixelInd.nrm), 0, 1);
    //float3 lightRatio = saturate(dot(-SceneData[0].LD.xyz, pixelInd.nrm));
	
    //float3 lightOut = lightRatio * SceneData[0].materials[mesh_ID].Kd * SceneData[0].LC.xyz;
    //return float4(lightOut, 1);
	// TODO: Part 4g (half-vector or reflect method your choice)
    //float4 Direct = lightRatio * SceneData[0].LC;
    //float4 Indirect = SceneData[0].LC * 0.15;
    float3 dirView = normalize(SceneData[0].camPos.xyz - pixelInd.posW);
    float3 halfVec = normalize(-SceneData[0].LD.xyz + dirView);
    //float saturation = saturate(dot(pixelInd.nrm, halfVec));
    //float intensity = max(pow(clamp SceneData[0].materials[mesh_ID].Ns), 0);
    float dotRes = dot(halfVec, pixelInd.nrm);
    float satRes = saturate(dotRes);
    float powRes = pow(satRes, SceneData[0].materials[materialOffset + materialIndex].Ns);
    float intensity = max(powRes, 0);
    //float3 intensity = max(pow(saturate(dot(pixelInd.nrm, halfVec)), SceneData[0].materials[mesh_ID].Ns), 1), 0);
    float specular = saturate(intensity * float4(SceneData[0].materials[materialOffset + materialIndex].Ks, 1));
    //float3 specular = SceneData[0].LC.xyz * SceneData[0].materials[materialOffset + materialIndex].Kd * intensity;
    //return saturate(Direct + Indirect) * lightOut + specular;
    float3 lightOut = lightRatio * SceneData[0].materials[materialOffset + materialIndex].Kd * SceneData[0].LC.xyz;
    float3 lightRes = lightOut + specular + float3(ambience.xyz);
    
    return float4(lightRes,1);
    //return ambience;
    //return float4(lightRatio * SceneData[0].LC.xyz, 1) + SceneData[0].ambient;
    //return float4(SceneData[0].ambient);
    //return saturate(Direct + Indirect) * lightOut + specular;
};