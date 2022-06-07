// minimalistic code to draw a single triangle, this is not part of the API.
// TODO: Part 1b
#define M_PI 3.14159265358979323846
#include"build/../../Assets/h2bParser.h"
#include "build/../../Assets/XTime.h"
#include "shaderc/shaderc.h" // needed for compiling shaders at runtime
#ifdef _WIN32 // must use MT platform DLL libraries on windows
	#pragma comment(lib, "shaderc_combined.lib") 
#endif
// Simple Vertex Shader
//const char* vertexShaderSource = R"(
// Creation, Rendering & Cleanup

//class Level_Data
//{
//public:
//	std::vector<std::vector<H2B::VERTEX>> VertVec = {};
//	std::vector<H2B::VERTEX> IndVec = {};
//	std::vector<H2B::ATTRIBUTES> AttrVec = {};
//	std::vector<H2B::MESH> MeshVec = {};
//	std::vector<H2B::MATERIAL> MatVec = {};
//	
//};


class Renderer
{
	XTime xtime;
	// TODO: Part 2b
	struct SHADER_VARS
	{
		//GW::MATH::GMATRIXF WM;
		GW::MATH::GMATRIXF VM;
		GW::MATH::GMATRIXF PM;
		GW::MATH::GVECTORF LD;
		GW::MATH::GVECTORF LC;
		GW::MATH::GMATRIXF matricies[1024];
		H2B::ATTRIBUTES materials[1024];
		GW::MATH::GVECTORF ambient;
		GW::MATH::GVECTORF camPos;
	};
	struct MODEL
	{
		std::string Name;
		std::vector<GW::MATH::GMATRIXF> vecMat;
		std::vector<H2B::VERTEX> Vertices;
		std::vector<unsigned int> Indices;
		std::vector<H2B::MATERIAL> Materials;
		std::vector<H2B::MESH> Meshes;
		std::vector<H2B::BATCH> Batches;
		VkBuffer levelVertexHandle = nullptr;
		VkDeviceMemory levelVertexData = nullptr;

		VkDeviceMemory levelIndexData = nullptr;
		VkBuffer levelIndexHandle = nullptr;
		unsigned int instanceCount = 1;
		unsigned int matrixInstanceOffset = 0;
		unsigned int materialOffset = 0;
	};
	struct PUSH_CONSTANT
	{
		unsigned int materialOffset;
		unsigned int WMoffset;
		unsigned int materialIndex;
		float padding[29];
	};
	// proxy handles
	GW::SYSTEM::GWindow win;
	GW::GRAPHICS::GVulkanSurface vlk;
	GW::CORE::GEventReceiver shutdown;
	GW::INPUT::GInput input;
	GW::INPUT::GController controller;
	// what we need at a minimum to draw a triangle
	VkDevice device = nullptr;
	VkBuffer vertexHandle = nullptr;
	VkDeviceMemory vertexData = nullptr;
	// TODO: Part 1g
	VkBuffer indexHandle = nullptr;
	VkDeviceMemory indexData = nullptr;
	// TODO: Part 2c
	std::vector<VkBuffer> storageBuff;
	std::vector<VkDeviceMemory> storageMem;
	VkShaderModule vertexShader = nullptr;
	VkShaderModule pixelShader = nullptr;
	// pipeline settings for drawing (also required)
	VkPipeline pipeline = nullptr;
	VkPipeline minimappipeline = nullptr;
	VkPipelineLayout pipelineLayout = nullptr;
	// TODO: Part 2e
	VkDescriptorSetLayout setLayoutHandle = nullptr;
	// TODO: Part 2f
	VkDescriptorPool descPool = nullptr;
	// TODO: Part 2g
	VkDescriptorSet descSet = nullptr;
		// TODO: Part 4f
	std::vector<VkDescriptorSet> descSetVec = {};
	// TODO: Part 2a
	GW::MATH::GMATRIXF world = GW::MATH::GIdentityMatrixF;
	GW::MATH::GMATRIXF view = GW::MATH::GIdentityMatrixF;
	GW::MATH::GMATRIXF projection = GW::MATH::GIdentityMatrixF;
	
	// TODO: Part 2b
	SHADER_VARS shader;
	PUSH_CONSTANT pushConstant;
	std::vector<MODEL> models;
	GW::MATH::GMatrix CameraProxy;
	H2B::Parser parser;
	int result = 0;
	int tabclicked = 0;
	// TODO: Part 4g
	std::string ShaderAsString(const char* shaderFilePath) {
		std::string output;
		unsigned int stringLength = 0;
		GW::SYSTEM::GFile file; file.Create();
		file.GetFileSize(shaderFilePath, stringLength);
		if (stringLength && +file.OpenBinaryRead(shaderFilePath)) {
			output.resize(stringLength);
			file.Read(&output[0], stringLength);
		}
		else
			std::cout << "ERROR: Shader Source File \"" << shaderFilePath << "\" Not Found!" << std::endl;
		return output;
	}
	void LoadModel(const char* filepath)
	{
		std::ifstream filestream(filepath);
		std::string currLine;
		int bytes = 0;
		int materialBytes = 0;

		while (!filestream.eof())
		{
			GW::MATH::GMATRIXF matrix;
			std::getline(filestream, currLine);
			if (std::strcmp(currLine.c_str(),"MESH") == 0)
			{

				std::getline(filestream, currLine, '.');
				currLine = "../OBJ/" + currLine + ".h2b";
				std::string filename = currLine;
				if (parser.Parse(filename.c_str()))
				{
					bool isNewModel = true;
					for (int i = 0; i < models.size(); i++)
					{
						if (models[i].Name == filename)
						{
							isNewModel = false;
							models[i].instanceCount++;
						}
					}
					if (isNewModel)
					{
						MODEL model;
						model.Name = filename;
						model.Vertices = parser.vertices;
						model.Indices = parser.indices;
						model.Materials = parser.materials;
						model.Meshes = parser.meshes;
						model.Batches = parser.batches;

						std::cout << filename << "\n";
						std::cout << "Vertices: " << parser.vertexCount << "\n";
						std::cout << "Indices: " << parser.indexCount << "\n";
						std::cout << "Materials: " << parser.materialCount << "\n";
						std::cout << "Mesh: " << parser.meshCount << "\n";
						std::cout << "Press TAB to cycle through levels\n";
						std::cout << "WASD keys to move\n";
						std::cout << "SPACE to translate up\n";
						std::cout << "SHIFT to translate down\n";
						models.push_back(model);
					}
					std::getline(filestream, currLine, '(');
					std::getline(filestream, currLine, ',');
					float n = std::stof(currLine);
					matrix.data[0] = n;
					std::getline(filestream, currLine, ',');
					n = std::stof(currLine);
					matrix.data[1] = n;
					std::getline(filestream, currLine, ',');
					n = std::stof(currLine);
					matrix.data[2] = n;
					std::getline(filestream, currLine, ')');
					n = std::stof(currLine);
					matrix.data[3] = n;

					std::getline(filestream, currLine, '(');
					std::getline(filestream, currLine, ',');
					n = std::stof(currLine);
					matrix.data[4] = n;
					std::getline(filestream, currLine, ',');
					n = std::stof(currLine);
					matrix.data[5] = n;
					std::getline(filestream, currLine, ',');
					n = std::stof(currLine);
					matrix.data[6] = n;
					std::getline(filestream, currLine, ')');
					n = std::stof(currLine);
					matrix.data[7] = n;

					std::getline(filestream, currLine, '(');
					std::getline(filestream, currLine, ',');
					n = std::stof(currLine);
					matrix.data[8] = n;
					std::getline(filestream, currLine, ',');
					n = std::stof(currLine);
					matrix.data[9] = n;
					std::getline(filestream, currLine, ',');
					n = std::stof(currLine);
					matrix.data[10] = n;
					std::getline(filestream, currLine, ')');
					n = std::stof(currLine);
					matrix.data[11] = n;

					std::getline(filestream, currLine, '(');
					std::getline(filestream, currLine, ',');
					n = std::stof(currLine);
					matrix.data[12] = n;
					std::getline(filestream, currLine, ',');
					n = std::stof(currLine);
					matrix.data[13] = n;
					std::getline(filestream, currLine, ',');
					n = std::stof(currLine);
					matrix.data[14] = n;
					std::getline(filestream, currLine, ')');
					n = std::stof(currLine);
					matrix.data[15] = n;

					for (int i = 0; i < models.size(); i++)
					{
						if (filename == models[i].Name)
						{
							models[i].vecMat.push_back(matrix);
							break;
						}
					}
				}
			}
		}
		for (int i = 0; i < models.size(); i++)
		{
			models[i].materialOffset = materialBytes;
			models[i].matrixInstanceOffset = bytes;
			for (int j = 0; j < models[i].vecMat.size(); j++)
			{
				shader.matricies[bytes] = models[i].vecMat[j];
				bytes++;
			}
			for (int k = 0; k < models[i].Materials.size(); k++)
			{
				shader.materials[materialBytes] = models[i].Materials[k].attrib;
				materialBytes++;
			}
		}
	}

	public:
		void UpdateScene()
		{			
			if (GetAsyncKeyState(VK_TAB) & 0x01)
			{
				result++;
				result = result % 2;
				switch (result)
				{
				case 0:
					CleanUp();
					models.clear();
					//vkDeviceWaitIdle(device);
					LoadModel("../GameLevel.txt");
					CreateBuffer();
					CreateDescriptors();
					break;
				case 1:
					CleanUp();
					models.clear();
					//vkDeviceWaitIdle(device);
					LoadModel("../GameLevel2.txt");
					CreateBuffer();
					CreateDescriptors();
					break;
				}
			}
		}
public:
	Renderer(GW::SYSTEM::GWindow _win, GW::GRAPHICS::GVulkanSurface _vlk)
	{
		win = _win;
		vlk = _vlk;
		unsigned int width, height;
		win.GetClientWidth(width);
		win.GetClientHeight(height);

		// TODO: Part 2a

		GW::MATH::GMatrix proxy;
		GW::MATH::GVECTORF _eye;
		GW::MATH::GVECTORF _at;
		GW::MATH::GVECTORF _up;
		GW::MATH::GVector proxyV;
		
		CameraProxy.Create();
		input.Create(win);
		controller.Create();
		//GW::AUDIO::GMusic dungeonMusic;
		//GW::AUDIO::GAudio audioBuff;
		//const char* music = "build/../../Assets/Destinations Unknown.wav";
		//dungeonMusic.Create(music,audioBuff,1);
		//audioBuff.Create();
		//audioBuff.PlayMusic();
		proxy.Create();
		proxyV.Create();
		
		_eye = { 15, 15, -3, 1 };
		_at = { 0.15, 0.75, 0, 1 };
		_up = { 0, 1, 0, 1 };
		proxy.LookAtLHF(_eye, _at, _up, view);
		float ratio = 0.0f;
		vlk.GetAspectRatio(ratio);
		//65 degrees = 1.13446 rads, AR, near, far, outputmat 
		proxy.ProjectionDirectXLHF(1.13446, ratio, 0.1f, 100.0f, projection);
		// or vulkan??
		GW::MATH::GVECTORF lightDir = { -1.0, -0.5, 2.5, 1 };
		GW::MATH::GVECTORF lightCol = { 1.0, 1.0, 1.0, 1.0 };
		GW::MATH::GVECTORF ambient = { 0.6,0.2,0.2,1 };
		proxyV.NormalizeF(lightDir, lightDir);
		proxyV.NormalizeF(lightCol, lightCol);
		//dungeonMusic.Play();
		//audioBuff.PlayMusic();
		for (int i = 0; i < 1024; i++)
		{
			shader.matricies[i] = GW::MATH::GIdentityMatrixF;
		}
		// TODO: Part 2b
		shader.VM = view;
		shader.PM = projection;
		shader.LD = lightDir;
		shader.LC = lightCol;
		// TODO: Part 4g
		shader.ambient = ambient;
		shader.camPos = _eye;
		// TODO: Part 1c
		LoadModel("../GameLevel.txt");
		//LoadModel("../GameLevel2.txt");
		CreateBuffer();
		CreateDescriptors();
	}
	void Render()
	{
		xtime.Signal();
		// TODO: Part 2a
		float delta = xtime.Delta();
		//GW::MATH::GMatrix proxy;
		//proxy.RotateYGlobalF(world, delta, world);
		//shader.matricies[1] = world;
		
		// TODO: Part 4d
		// grab the current Vulkan commandBuffer
		unsigned int currentBuffer;
		vlk.GetSwapchainCurrentImage(currentBuffer);
		VkCommandBuffer commandBuffer;
		vlk.GetCommandBuffer(currentBuffer, (void**)&commandBuffer);
		// what is the current client area dimensions?
		unsigned int width, height;
		win.GetClientWidth(width);
		win.GetClientHeight(height);
		// setup the pipeline's dynamic settings
		VkViewport viewport = {
            0, 0, static_cast<float>(width), static_cast<float>(height), 0, 1
        };
		VkRect2D scissor = { {0, 0}, {width, height} };
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

		// now we can draw
		VkDeviceSize offsets[] = { 0 };
		// TODO: Part 4d
		for (int i = 0; i < storageBuff.size(); i++)
		{
			GvkHelper::write_to_buffer(device, storageMem[i], &shader, sizeof(shader));
		}
		// TODO: Part 2i

		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descSet, 0, VK_NULL_HANDLE);
		// TODO: Part 3b
		for (size_t j = 0; j < models.size(); j++)
		{
			vkCmdBindVertexBuffers(commandBuffer, 0, 1, &models[j].levelVertexHandle, offsets);
			vkCmdBindIndexBuffer(commandBuffer, models[j].levelIndexHandle, 0, VK_INDEX_TYPE_UINT32);
			for (int i = 0; i < models[j].Meshes.size(); i++)
			{
				pushConstant.materialIndex = models[j].Meshes[i].materialIndex;	// TODO: Part 3d
				pushConstant.WMoffset = models[j].matrixInstanceOffset;
				pushConstant.materialOffset = models[j].materialOffset;
				vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_ALL_GRAPHICS, 0, sizeof(PUSH_CONSTANT), &pushConstant);
				vkCmdDrawIndexed(commandBuffer, models[j].Meshes[i].drawInfo.indexCount, models[j].instanceCount, models[j].Meshes[i].drawInfo.indexOffset, 0, 0); // TODO: Part 1d, 1h
			}
		}

		//minimap
		VkViewport minimapviewport = {
			0, 0, static_cast<float>(width / 4), static_cast<float>(height / 4), 0, 1
		};
		vkCmdSetViewport(commandBuffer, 0, 1, &minimapviewport);
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, minimappipeline);

		// TODO: Part 3b
		for (size_t j = 0; j < models.size(); j++)
		{
			vkCmdBindVertexBuffers(commandBuffer, 0, 1, &models[j].levelVertexHandle, offsets);
			vkCmdBindIndexBuffer(commandBuffer, models[j].levelIndexHandle, 0, VK_INDEX_TYPE_UINT32);
			for (int i = 0; i < models[j].Meshes.size(); i++)
			{
				pushConstant.materialIndex = models[j].Meshes[i].materialIndex;	// TODO: Part 3d
				pushConstant.WMoffset = models[j].matrixInstanceOffset;
				pushConstant.materialOffset = models[j].materialOffset;
				vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_ALL_GRAPHICS, 0, sizeof(PUSH_CONSTANT), &pushConstant);
				vkCmdDrawIndexed(commandBuffer, models[j].Meshes[i].drawInfo.indexCount, models[j].instanceCount, models[j].Meshes[i].drawInfo.indexOffset, 0, 0); // TODO: Part 1d, 1h
			}
		}
		
	}
public:
	void CreateBuffer()
	{
		/***************** GEOMETRY INTIALIZATION ******************/
	// Grab the device & physical device so we can allocate some stuff
		VkPhysicalDevice physicalDevice = nullptr;
		vlk.GetDevice((void**)&device);
		vlk.GetPhysicalDevice((void**)&physicalDevice);


		for (int i = 0; i < models.size(); i++)
		{
			GvkHelper::create_buffer(physicalDevice, device, sizeof(H2B::VERTEX) * models[i].Vertices.size(),
				VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
				VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &models[i].levelVertexHandle, &models[i].levelVertexData);
			GvkHelper::write_to_buffer(device, models[i].levelVertexData, models[i].Vertices.data(), sizeof(H2B::VERTEX) * models[i].Vertices.size());

			GvkHelper::create_buffer(physicalDevice, device, sizeof(unsigned int) * models[i].Indices.size(),
				VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
				VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &models[i].levelIndexHandle, &models[i].levelIndexData);
			GvkHelper::write_to_buffer(device, models[i].levelIndexData, models[i].Indices.data(), sizeof(unsigned int) * models[i].Indices.size());
		}

		// TODO: Part 2d
		unsigned int numFrames = 0;
		vlk.GetSwapchainImageCount(numFrames);
		storageBuff.resize(numFrames);
		storageMem.resize(numFrames);
		for (int i = 0; i < numFrames; i++)
		{
			GvkHelper::create_buffer(physicalDevice, device, sizeof(shader),
				VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
				VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &storageBuff[i], &storageMem[i]);
			GvkHelper::write_to_buffer(device, storageMem[i], &shader, sizeof(shader));
		}
	}
public:
	void CreateDescriptors()
	{
		unsigned int width, height;
		win.GetClientWidth(width);
		win.GetClientHeight(height);
		/***************** SHADER INTIALIZATION ******************/
// Intialize runtime shader compiler HLSL -> SPIRV
		shaderc_compiler_t compiler = shaderc_compiler_initialize();
		shaderc_compile_options_t options = shaderc_compile_options_initialize();
		shaderc_compile_options_set_source_language(options, shaderc_source_language_hlsl);
		shaderc_compile_options_set_invert_y(options, true); // TODO: Part 2i
#ifndef NDEBUG
		shaderc_compile_options_set_generate_debug_info(options);
#endif
		// Create Vertex Shader
		std::string fragment;
		std::string vertex;
		fragment = ShaderAsString("../Fragment.hlsl");
		vertex = ShaderAsString("../Vertex.hlsl");
		shaderc_compilation_result_t result = shaderc_compile_into_spv( // compile
			compiler, vertex.c_str(), strlen(vertex.c_str()),
			shaderc_vertex_shader, "main.vert", "main", options);
		if (shaderc_result_get_compilation_status(result) != shaderc_compilation_status_success) // errors?
			std::cout << "Vertex Shader Errors: " << shaderc_result_get_error_message(result) << std::endl;
		GvkHelper::create_shader_module(device, shaderc_result_get_length(result), // load into Vulkan
			(char*)shaderc_result_get_bytes(result), &vertexShader);
		shaderc_result_release(result); // done
		// Create Pixel Shader
		result = shaderc_compile_into_spv( // compile
			compiler, fragment.c_str(), strlen(fragment.c_str()),
			shaderc_fragment_shader, "main.frag", "main", options);
		if (shaderc_result_get_compilation_status(result) != shaderc_compilation_status_success) // errors?
			std::cout << "Pixel Shader Errors: " << shaderc_result_get_error_message(result) << std::endl;
		GvkHelper::create_shader_module(device, shaderc_result_get_length(result), // load into Vulkan
			(char*)shaderc_result_get_bytes(result), &pixelShader);
		shaderc_result_release(result); // done
		// Free runtime shader compiler resources
		shaderc_compile_options_release(options);
		shaderc_compiler_release(compiler);

		/***************** PIPELINE INTIALIZATION ******************/
		// Create Pipeline & Layout (Thanks Tiny!)
		VkRenderPass renderPass;
		vlk.GetRenderPass((void**)&renderPass);
		VkPipelineShaderStageCreateInfo stage_create_info[2] = {};
		// Create Stage Info for Vertex Shader
		stage_create_info[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		stage_create_info[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
		stage_create_info[0].module = vertexShader;
		stage_create_info[0].pName = "main";
		// Create Stage Info for Fragment Shader
		stage_create_info[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		stage_create_info[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		stage_create_info[1].module = pixelShader;
		stage_create_info[1].pName = "main";
		// Assembly State
		VkPipelineInputAssemblyStateCreateInfo assembly_create_info = {};
		assembly_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		assembly_create_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		assembly_create_info.primitiveRestartEnable = false;
		// TODO: Part 1e
		// Vertex Input State
		VkVertexInputBindingDescription vertex_binding_description = {};
		vertex_binding_description.binding = 0;
		vertex_binding_description.stride = sizeof(float) * 9;
		vertex_binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		VkVertexInputAttributeDescription vertex_attribute_description[3] =
		{
		{ 0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0 },
		{ 1, 0, VK_FORMAT_R32G32B32_SFLOAT, 12 },
		{ 2, 0, VK_FORMAT_R32G32B32_SFLOAT, 24 } //uv, normal, etc....
		};
		VkPipelineVertexInputStateCreateInfo input_vertex_info = {};
		input_vertex_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		input_vertex_info.vertexBindingDescriptionCount = 1;
		input_vertex_info.pVertexBindingDescriptions = &vertex_binding_description;
		input_vertex_info.vertexAttributeDescriptionCount = 3;
		input_vertex_info.pVertexAttributeDescriptions = vertex_attribute_description;
		// Viewport State (we still need to set this up even though we will overwrite the values)
		VkViewport viewport = {
			0, 0, static_cast<float>(width), static_cast<float>(height), 0, 1
		};
		VkRect2D scissor = { {0, 0}, {width, height} };
		VkPipelineViewportStateCreateInfo viewport_create_info = {};
		viewport_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewport_create_info.viewportCount = 1;
		viewport_create_info.pViewports = &viewport;
		viewport_create_info.scissorCount = 1;
		viewport_create_info.pScissors = &scissor;

		VkViewport minimapviewport = {
			0, 0, static_cast<float>(width/4), static_cast<float>(height/4), 0, 1
		};
		VkPipelineViewportStateCreateInfo minimapviewport_create_info = {};
		minimapviewport_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		minimapviewport_create_info.viewportCount = 1;
		minimapviewport_create_info.pViewports = &minimapviewport;
		minimapviewport_create_info.scissorCount = 1;
		minimapviewport_create_info.pScissors = &scissor;

		// Rasterizer State
		VkPipelineRasterizationStateCreateInfo rasterization_create_info = {};
		rasterization_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterization_create_info.rasterizerDiscardEnable = VK_FALSE;
		rasterization_create_info.polygonMode = VK_POLYGON_MODE_FILL;
		rasterization_create_info.lineWidth = 1.0f;
		rasterization_create_info.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterization_create_info.frontFace = VK_FRONT_FACE_CLOCKWISE;
		rasterization_create_info.depthClampEnable = VK_FALSE;
		rasterization_create_info.depthBiasEnable = VK_FALSE;
		rasterization_create_info.depthBiasClamp = 0.0f;
		rasterization_create_info.depthBiasConstantFactor = 0.0f;
		rasterization_create_info.depthBiasSlopeFactor = 0.0f;
		// Multisampling State
		VkPipelineMultisampleStateCreateInfo multisample_create_info = {};
		multisample_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisample_create_info.sampleShadingEnable = VK_FALSE;
		multisample_create_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multisample_create_info.minSampleShading = 1.0f;
		multisample_create_info.pSampleMask = VK_NULL_HANDLE;
		multisample_create_info.alphaToCoverageEnable = VK_FALSE;
		multisample_create_info.alphaToOneEnable = VK_FALSE;
		// Depth-Stencil State
		VkPipelineDepthStencilStateCreateInfo depth_stencil_create_info = {};
		depth_stencil_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depth_stencil_create_info.depthTestEnable = VK_TRUE;
		depth_stencil_create_info.depthWriteEnable = VK_TRUE;
		depth_stencil_create_info.depthCompareOp = VK_COMPARE_OP_LESS;
		depth_stencil_create_info.depthBoundsTestEnable = VK_FALSE;
		depth_stencil_create_info.minDepthBounds = 0.0f;
		depth_stencil_create_info.maxDepthBounds = 1.0f;
		depth_stencil_create_info.stencilTestEnable = VK_FALSE;
		// Color Blending Attachment & State
		VkPipelineColorBlendAttachmentState color_blend_attachment_state = {};
		color_blend_attachment_state.colorWriteMask = 0xF;
		color_blend_attachment_state.blendEnable = VK_FALSE;
		color_blend_attachment_state.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_COLOR;
		color_blend_attachment_state.dstColorBlendFactor = VK_BLEND_FACTOR_DST_COLOR;
		color_blend_attachment_state.colorBlendOp = VK_BLEND_OP_ADD;
		color_blend_attachment_state.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		color_blend_attachment_state.dstAlphaBlendFactor = VK_BLEND_FACTOR_DST_ALPHA;
		color_blend_attachment_state.alphaBlendOp = VK_BLEND_OP_ADD;
		VkPipelineColorBlendStateCreateInfo color_blend_create_info = {};
		color_blend_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		color_blend_create_info.logicOpEnable = VK_FALSE;
		color_blend_create_info.logicOp = VK_LOGIC_OP_COPY;
		color_blend_create_info.attachmentCount = 1;
		color_blend_create_info.pAttachments = &color_blend_attachment_state;
		color_blend_create_info.blendConstants[0] = 0.0f;
		color_blend_create_info.blendConstants[1] = 0.0f;
		color_blend_create_info.blendConstants[2] = 0.0f;
		color_blend_create_info.blendConstants[3] = 0.0f;
		// Dynamic State 
		VkDynamicState dynamic_state[2] = {
			// By setting these we do not need to re-create the pipeline on Resize
			VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR
		};
		VkPipelineDynamicStateCreateInfo dynamic_create_info = {};
		dynamic_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamic_create_info.dynamicStateCount = 2;
		dynamic_create_info.pDynamicStates = dynamic_state;

		// TODO: Part 2e
		VkDescriptorSetLayoutBinding layoutBinding = {};
		layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		layoutBinding.descriptorCount = 1;
		layoutBinding.stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;
		layoutBinding.binding = 0;
		layoutBinding.pImmutableSamplers = VK_NULL_HANDLE;

		VkDescriptorSetLayoutCreateInfo set_layout_createInfo = {};
		set_layout_createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		set_layout_createInfo.bindingCount = 1;
		set_layout_createInfo.flags = 0;
		set_layout_createInfo.pBindings = &layoutBinding;
		set_layout_createInfo.pNext = VK_NULL_HANDLE;

		vkCreateDescriptorSetLayout(device, &set_layout_createInfo, nullptr, &setLayoutHandle);
		// TODO: Part 2f

		VkDescriptorPoolSize poolSize = { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1 };
		VkDescriptorPoolCreateInfo descriptorPool_createInfo = {};
		descriptorPool_createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		descriptorPool_createInfo.poolSizeCount = 1;
		descriptorPool_createInfo.pPoolSizes = &poolSize;
		descriptorPool_createInfo.flags = 0;
		descriptorPool_createInfo.maxSets = 1;
		descriptorPool_createInfo.pNext = VK_NULL_HANDLE;

		vkCreateDescriptorPool(device, &descriptorPool_createInfo, nullptr, &descPool);
		// TODO: Part 4f

	// TODO: Part 2g
		VkDescriptorSetAllocateInfo setAllocate_Info = {};
		setAllocate_Info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		setAllocate_Info.descriptorSetCount = 1;
		setAllocate_Info.descriptorPool = descPool;
		setAllocate_Info.pSetLayouts = &setLayoutHandle;
		setAllocate_Info.pNext = VK_NULL_HANDLE;

		vkAllocateDescriptorSets(device, &setAllocate_Info, &descSet);
		// TODO: Part 4f
	// TODO: Part 2h
		VkDescriptorBufferInfo bufferInfo = {};
		bufferInfo.buffer = storageBuff[0];
		bufferInfo.range = VK_WHOLE_SIZE;
		bufferInfo.offset = 0;

		VkWriteDescriptorSet writeDesc_Set = {};
		writeDesc_Set.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		writeDesc_Set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeDesc_Set.dstSet = descSet;
		writeDesc_Set.descriptorCount = 1;
		writeDesc_Set.dstBinding = 0;
		writeDesc_Set.dstArrayElement = 0;
		writeDesc_Set.pImageInfo = VK_NULL_HANDLE;
		writeDesc_Set.pNext = VK_NULL_HANDLE;
		writeDesc_Set.pTexelBufferView = VK_NULL_HANDLE;
		writeDesc_Set.pBufferInfo = &bufferInfo;

		vkUpdateDescriptorSets(device, 1, &writeDesc_Set, 0, nullptr);
		// TODO: Part 4f

	// Descriptor pipeline layout
		VkPipelineLayoutCreateInfo pipeline_layout_create_info = {};
		pipeline_layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		// TODO: Part 2e
		pipeline_layout_create_info.setLayoutCount = 1;
		pipeline_layout_create_info.pSetLayouts = &setLayoutHandle;
		// TODO: Part 3c
		VkPushConstantRange PCR = {};
		PCR.size = sizeof(PUSH_CONSTANT);
		PCR.offset = 0;
		PCR.stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;
		pipeline_layout_create_info.pushConstantRangeCount = 1;
		pipeline_layout_create_info.pPushConstantRanges = &PCR;
		vkCreatePipelineLayout(device, &pipeline_layout_create_info,
			nullptr, &pipelineLayout);
		// Pipeline State... (FINALLY) 
		VkGraphicsPipelineCreateInfo pipeline_create_info = {};
		pipeline_create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipeline_create_info.stageCount = 2;
		pipeline_create_info.pStages = stage_create_info;
		pipeline_create_info.pInputAssemblyState = &assembly_create_info;
		pipeline_create_info.pVertexInputState = &input_vertex_info;
		pipeline_create_info.pViewportState = &viewport_create_info;
		pipeline_create_info.pRasterizationState = &rasterization_create_info;
		pipeline_create_info.pMultisampleState = &multisample_create_info;
		pipeline_create_info.pDepthStencilState = &depth_stencil_create_info;
		pipeline_create_info.pColorBlendState = &color_blend_create_info;
		pipeline_create_info.pDynamicState = &dynamic_create_info;
		pipeline_create_info.layout = pipelineLayout;
		pipeline_create_info.renderPass = renderPass;
		pipeline_create_info.subpass = 0;
		pipeline_create_info.basePipelineHandle = VK_NULL_HANDLE;
		vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1,
			&pipeline_create_info, nullptr, &pipeline);

		VkGraphicsPipelineCreateInfo minimappipeline_create_info = {};
		minimappipeline_create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		minimappipeline_create_info.stageCount = 2;
		minimappipeline_create_info.pStages = stage_create_info;
		minimappipeline_create_info.pInputAssemblyState = &assembly_create_info;
		minimappipeline_create_info.pVertexInputState = &input_vertex_info;
		minimappipeline_create_info.pViewportState = &minimapviewport_create_info;
		minimappipeline_create_info.pRasterizationState = &rasterization_create_info;
		minimappipeline_create_info.pMultisampleState = &multisample_create_info;
		minimappipeline_create_info.pDepthStencilState = &depth_stencil_create_info;
		minimappipeline_create_info.pColorBlendState = &color_blend_create_info;
		minimappipeline_create_info.pDynamicState = &dynamic_create_info;
		minimappipeline_create_info.layout = pipelineLayout;
		minimappipeline_create_info.renderPass = renderPass;
		minimappipeline_create_info.subpass = 0;
		minimappipeline_create_info.basePipelineHandle = VK_NULL_HANDLE;
		vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1,
			&minimappipeline_create_info, nullptr, &minimappipeline);
		/***************** CLEANUP / SHUTDOWN ******************/
		// GVulkanSurface will inform us when to release any allocated resources
		//only works in debug mode with no errors, in release mode it runs but crashes when swapping levels, so without it in release mode it runs fine
#ifndef NDEBUG
		shutdown.Create(vlk, [&]() {
			if (+shutdown.Find(GW::GRAPHICS::GVulkanSurface::Events::RELEASE_RESOURCES, true)) {
				CleanUp(); // unlike D3D we must be careful about destroy timing
			}
			});
#endif
	}
public:
		void UpdateCamera()
		{
			view = shader.VM;
			xtime.Signal();
			// TODO: Part 4c
			CameraProxy.InverseF(view, view);
			// TODO: Part 4d
			float yValue = 0.0f;
			GW::MATH::GVECTORF camPosY;
			float keySpace = 0.0f;
			float keyLeftShift = 0.0f;
			float keyRightTrigger = 0.0f;
			float keyLeftTrigger = 0.0f;
			const float CameraSpeed = 3.0f;
			float delta;
			delta = xtime.Delta();
			input.GetState(G_KEY_SPACE, keySpace);
			input.GetState(G_KEY_LEFTSHIFT, keyLeftShift);
			controller.GetState(0, G_LEFT_TRIGGER_AXIS, keyLeftTrigger);
			controller.GetState(0, G_RIGHT_TRIGGER_AXIS, keyRightTrigger);
			yValue = keySpace - keyLeftShift + keyRightTrigger - keyLeftTrigger;
			camPosY = { 0, yValue * CameraSpeed * delta, 0, 1 };
			CameraProxy.TranslateGlobalF(view, camPosY, view);
			// TODO: Part 4e
			float perFrameSpeed = 0.0f;
			float zChange = 0.0f;
			float xChange = 0.0f;
			float keyW = 0.0f, keyS = 0.0f, keyA = 0.0f, keyD = 0.0f;
			float stickY = 0.0f, stickX = 0.0f;
			GW::MATH::GVECTORF tran;
			perFrameSpeed = CameraSpeed * delta;
			input.GetState(G_KEY_W, keyW);
			input.GetState(G_KEY_S, keyS);
			input.GetState(G_KEY_A, keyA);
			input.GetState(G_KEY_D, keyD);
			controller.GetState(0, G_LY_AXIS, stickY);
			controller.GetState(0, G_LX_AXIS, stickX);
			zChange = keyW - keyS + stickY;
			xChange = keyD - keyA + stickX;
			tran = { xChange * perFrameSpeed, 0, zChange * perFrameSpeed, 1 };
			CameraProxy.TranslateLocalF(view, tran, view);
			// TODO: Part 4f
			float thumbSpeed = 0.0f, totalPitch = 0.0f;
			float x = 0.0f, y = 0.0f, FOV = 1.13446, rYstick = 0.0f;
			unsigned int height;
			thumbSpeed = M_PI * delta;
			controller.GetState(0, G_RY_AXIS, rYstick);
			win.GetClientHeight(height);
			GW::GReturn result = input.GetMouseDelta(x, y);
			if (result == GW::GReturn::REDUNDANT)
			{
				y = 0.0f;
				x = 0.0f;
			}
			totalPitch = FOV * y / height + rYstick * -thumbSpeed;
			CameraProxy.RotateXLocalF(view, totalPitch, view);
			// TODO: Part 4g
			float totalYaw = 0.0f, rXstick = 0.0f;
			float ratio;
			unsigned int width;
			controller.GetState(0, G_RX_AXIS, rXstick);
			vlk.GetAspectRatio(ratio);
			win.GetClientWidth(width);
			totalYaw = FOV * ratio * x / width + rXstick * thumbSpeed;
			CameraProxy.RotateYGlobalF(view, totalYaw, view);
			// TODO: Part 4c
			shader.camPos = view.row4;
			CameraProxy.InverseF(view, view);
			shader.VM = view;
		}
private:
	void CleanUp()
	{
		// wait till everything has completed
		vkDeviceWaitIdle(device);
		// Release allocated buffers, shaders & pipeline
		// TODO: Part 1g
		vkDestroyBuffer(device, indexHandle, nullptr);
		vkFreeMemory(device, indexData, nullptr);
		// TODO: Part 2d
		unsigned int numFrames = 0;
		vlk.GetSwapchainImageCount(numFrames);
		for (int i = 0; i < numFrames; i++)
		{
			vkDestroyBuffer(device, storageBuff[i], nullptr);
			vkFreeMemory(device, storageMem[i], nullptr);
		}
		for (int i = 0; i < models.size(); i++)
		{
			vkDestroyBuffer(device, models[i].levelVertexHandle, nullptr);
			vkFreeMemory(device, models[i].levelVertexData, nullptr);

			vkDestroyBuffer(device, models[i].levelIndexHandle, nullptr);
			vkFreeMemory(device, models[i].levelIndexData, nullptr);
		}
		vkDestroyBuffer(device, vertexHandle, nullptr);
		vkFreeMemory(device, vertexData, nullptr);
		vkDestroyShaderModule(device, vertexShader, nullptr);
		vkDestroyShaderModule(device, pixelShader, nullptr);
		// TODO: Part 2e
		vkDestroyDescriptorSetLayout(device, setLayoutHandle, nullptr);
		// TODO: part 2f
		vkDestroyDescriptorPool(device, descPool, nullptr);
		vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
		vkDestroyPipeline(device, pipeline, nullptr);
		vkDestroyPipeline(device, minimappipeline, nullptr);
	}
};
