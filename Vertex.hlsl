// TODO: 2i
// an ultra simple hlsl vertex shader
// TODO: Part 2b
struct obj_attributes
{
    float3 Kd; // diffuse reflectivity
    float d; // dissolve (transparency) 
    float3 Ks; // specular reflectivity
    float Ns; // specular exponent
    float3 Ka; // ambient reflectivity
    float sharpness; // local reflection map sharpness
    float3 Tf; // transmission filter
    float Ni; // optical density (index of refraction)
    float3 Ke; // emissive reflectivity
    uint illum; // illumination model
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
// TODO: Part 4a

// TODO: Part 1f
struct VS
{
    float3 pos : POSITION;
    float3 uvw : UVW;
    float3 nrm : NORMAL;
};
struct VS_OUT
{
    float4 pos : SV_POSITION;
    float3 uvw : UVW;
    float3 nrm : NORMAL;
    float3 posW : WORLD;
};
// TODO: Part 4b
VS_OUT main(VS inputVertex, uint instanceNum : SV_InstanceID)
{
    // TODO: Part 1h
    VS_OUT var = (VS_OUT) 0;
    var.pos = float4(inputVertex.pos, 1);
    var.uvw = inputVertex.uvw;
    var.nrm = inputVertex.nrm;

	// TODO: Part 2i
    var.pos = mul(SceneData[0].matricies[WMoffset + instanceNum], var.pos);
    var.nrm = mul(SceneData[0].matricies[WMoffset + instanceNum], float4(var.nrm, 0)).xyz;
    var.posW = var.pos;
    var.pos = mul(SceneData[0].VM, var.pos);
    var.pos = mul(SceneData[0].PM, var.pos);
    return var;
		// TODO: Part 4e
	// TODO: Part 4b
    
		// TODO: Part 4e
};