#include "model.h"

static const aiTextureType Model_TEXTURE_TYPES[] ={
	aiTextureType_DIFFUSE
   // aiTextureType_SPECULAR,
   // aiTextureType_HEIGHT,
	//aiTextureType_AMBIENT
};

const aiTextureType* Model::TEXTURE_TYPES = Model_TEXTURE_TYPES;

const uint32_t Model::NUM_TEXTURE_TYPES = ARRAY_SIZE(Model_TEXTURE_TYPES);

Model::Model(VulkanState& vulkanState):
	numVertices(0),
	numIndices(0),
	uniformBufferOffset(0),
	vertexBufferOffset(0),
	indexBufferOffset(0),
	mNumSamplerDescriptors(0),
	mState(vulkanState),
	mCommonBufferInfo(mState.device),
	mPath(""),
	mFolder("")
{


}


Model::~Model() 
{	
}

void Model::init(std::string modelPath, unsigned int pFlags) 
{	
	init(modelPath.c_str(), pFlags);	
}

void Model::init(const char* modelPath, unsigned int pFlags)
{
	mPath = modelPath;
	mFolder = FileManager::getFilePath(std::string(modelPath));
	LOG("FOLDER: %s", mFolder.c_str());
	Assimp::Importer importer;
#ifdef __ANDROID__
	importer.SetIOHandler(FileManager::newAssimpIOSystem());
#endif
	const aiScene* scene = importer.ReadFile(modelPath, pFlags);
	  
	  // If the import failed, report it
	if (!scene)
		throwError(importer.GetErrorString());	
	if (!scene->HasMeshes())
		throwError("No meshes found");

	processModel(*scene);
}



void Model::processModel(const aiScene& scene) 
{
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;
	mMeshes.resize(scene.mNumMeshes);

	for (size_t i = 0; i < scene.mNumMeshes; ++i) {

		aiMesh& mesh = *scene.mMeshes[i];
		Mesh& meshInfo = mMeshes[i];
		bool hasPositions = mesh.HasPositions();
		bool hasNormals = mesh.HasNormals();
		bool hasTangentsAndBitangents = mesh.HasTangentsAndBitangents();
		bool hasTexCoords = mesh.HasTextureCoords(0);
	
		// Vertices
		meshInfo.baseVertex = vertices.size();
		for (size_t j = 0; j < mesh.mNumVertices; ++j) {
			Vertex vertex;
			if (hasPositions) { 
				convertVector(mesh.mVertices[j], vertex.pos);
				vertex.pos.y *= -1;
			}
			if (hasNormals) 
				convertVector(mesh.mNormals[j], vertex.normal);
			if (hasTangentsAndBitangents) {
				convertVector(mesh.mTangents[j], vertex.tangent);
				convertVector(mesh.mBitangents[j], vertex.bitangent);
			}
			if (hasTexCoords) 
				convertVector(mesh.mTextureCoords[0][j], vertex.texCoord);
			vertices.push_back(vertex);
		}
		meshInfo.numVertices = vertices.size() - meshInfo.baseVertex;

		// Indices
		meshInfo.baseIndex = indices.size();
		for (size_t j = 0; j < mesh.mNumFaces; ++j) 
			for (size_t k = 0; k < 3; ++k)
				indices.push_back(mesh.mFaces[j].mIndices[k] + meshInfo.baseIndex);
		meshInfo.numIndices = indices.size() - meshInfo.baseIndex;
		
		// Textures
		meshInfo.materialIndex = mesh.mMaterialIndex;
		aiMaterial& material = *scene.mMaterials[mesh.mMaterialIndex];
		auto it = mMaterialIndexToMaterial.find(mesh.mMaterialIndex);
		if (it == mMaterialIndexToMaterial.end()) {
			Material materialInfo;
			for (size_t j = 0; j < NUM_TEXTURE_TYPES; ++j) {
				aiTextureType textureType = TEXTURE_TYPES[j];
				size_t numMaterials = material.GetTextureCount(textureType); 
				for (size_t k = 0; k < numMaterials; ++k) {
					aiString texturePath;
				
					material.GetTexture(textureType, k, &texturePath);
					std::string fullTexturePath = mFolder + "/";
					fullTexturePath += texturePath.C_Str();

					TextureDesc textureDesc(fullTexturePath);
					ImageInfo* imageInfo = TextureManager::load(
							mState, 
							mState.commandPool, 
							mState.graphicsQueue, 
							textureDesc);

					LOG("AFTER LOAD");
					switch(textureType) {
						case aiTextureType_DIFFUSE:
							materialInfo.diffuseImages.push_back(imageInfo);
							++materialInfo.numImages;
							break;
						case aiTextureType_SPECULAR:
							materialInfo.specularImages.push_back(imageInfo);
							++materialInfo.numImages;
							break;
						case aiTextureType_HEIGHT:
							materialInfo.heightImages.push_back(imageInfo);
							++materialInfo.numImages;
							break;
						case aiTextureType_AMBIENT:
							materialInfo.ambientImages.push_back(imageInfo);
							++materialInfo.numImages;
							break;
						default:
							break;
					}
				}
				mNumSamplerDescriptors += NUM_TEXTURE_TYPES;
			}	
			mMaterialIndexToMaterial[mesh.mMaterialIndex] = materialInfo;
		} else {LOG("MATERIAL EXISTS");}
	}

	createCommonBuffer(vertices, indices);
	createDescriptorPool();
	createDescriptorSet();
}

void Model::createCommonBuffer(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices)
{
	numVertices = vertices.size();
	numIndices = indices.size();

	VkDeviceSize uniformBufferSize = sizeof(UBO);
	VkDeviceSize vertexBufferSize = sizeof(vertices[0]) * numVertices;
	VkDeviceSize indexBufferSize = sizeof(indices[0]) * numIndices;
	
	uniformBufferOffset = 0;
	vertexBufferOffset = uniformBufferSize;
	indexBufferOffset = vertexBufferOffset + vertexBufferSize;

	mCommonBufferInfo.size = vertexBufferSize + indexBufferSize + uniformBufferSize;
	BufferInfo staging(mState.device);
	staging.size = mCommonBufferInfo.size;
	BufferHelper::createStagingBuffer(mState, staging);
	
	UBO ubo = {};
	char* data;
	vkMapMemory(mState.device, staging.memory, 0, staging.size, 0, (void**) &data);
	memcpy(data + uniformBufferOffset, &ubo, uniformBufferSize);
	memcpy(data + vertexBufferOffset, vertices.data(), vertexBufferSize);
	memcpy(data + indexBufferOffset, indices.data(), indexBufferSize);
	vkUnmapMemory(mState.device, staging.memory);

	BufferHelper::createCommonBuffer(mState, mCommonBufferInfo);

	BufferHelper::copyBuffer(
			mState,
			staging.buffer, 
			mCommonBufferInfo.buffer, 
			mCommonBufferInfo.size);
}

void Model::createDescriptorPool() 
{
	VkDescriptorPoolSize uboSize = {};
	uboSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uboSize.descriptorCount = mNumSamplerDescriptors + 1;
	
	VkDescriptorPoolSize samplerSize = {};
	samplerSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerSize.descriptorCount = mNumSamplerDescriptors + 1;

	VkDescriptorPoolSize poolSizes[] = {
		uboSize,
		samplerSize
	};
	
	VkDescriptorPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = ARRAY_SIZE(poolSizes);
	poolInfo.pPoolSizes = poolSizes;
	poolInfo.maxSets = mNumSamplerDescriptors + 1;
	
	LOG("NUM SAMPLERS: %u materials: %zu", mNumSamplerDescriptors, mMaterialIndexToMaterial.size());

	VK_CHECK_RESULT(vkCreateDescriptorPool(mState.device, &poolInfo, nullptr, &mDescriptorPool));
}

void Model::createDescriptorSet()
{
	VkDescriptorSetAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = mDescriptorPool;
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = &mState.descriptorSetLayouts.uniform;

	VK_CHECK_RESULT(vkAllocateDescriptorSets(mState.device, &allocInfo, &mUniformDescriptorSet));
	
	VkDescriptorBufferInfo buffInfo = {};
	buffInfo.buffer = mCommonBufferInfo.buffer;
	buffInfo.offset = uniformBufferOffset;
	buffInfo.range = sizeof(UBO);

	VkWriteDescriptorSet uniformWriteSet = {};
	uniformWriteSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	uniformWriteSet.dstSet = mUniformDescriptorSet;
	uniformWriteSet.dstBinding = 0;
	uniformWriteSet.dstArrayElement = 0;
	uniformWriteSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uniformWriteSet.descriptorCount = 1;
	uniformWriteSet.pBufferInfo = &buffInfo;

	vkUpdateDescriptorSets(mState.device, 1, &uniformWriteSet, 0, nullptr);
	LOG("UNIFORM SET CREATED");
	
	std::vector<VkDescriptorSetLayout> layouts(NUM_TEXTURE_TYPES, mState.descriptorSetLayouts.sampler);
	
	for (auto& materialPair : mMaterialIndexToMaterial) {
		Material& material = materialPair.second;
		material.descriptors.resize(material.maxImages);
		for (size_t i = 0; i < material.maxImages; ++i) {
			VkDescriptorSet& descriptorSet = material.descriptors[i];
			std::vector<VkWriteDescriptorSet> writeSets(NUM_TEXTURE_TYPES);

			VkDescriptorSetAllocateInfo samplerAllocInfo = {};
			samplerAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			samplerAllocInfo.descriptorPool = mDescriptorPool;
			samplerAllocInfo.descriptorSetCount = NUM_TEXTURE_TYPES;
			samplerAllocInfo.pSetLayouts = layouts.data();

			VK_CHECK_RESULT(vkAllocateDescriptorSets(mState.device, &samplerAllocInfo, &descriptorSet));

			for (size_t j = 0; j < NUM_TEXTURE_TYPES; ++j) {
				std::vector<ImageInfo*>* images = NULL;
				uint32_t binding;

				switch(TEXTURE_TYPES[j]) {
					case aiTextureType_DIFFUSE:
						images = &material.diffuseImages;
						binding = 0;
						break;
					case aiTextureType_SPECULAR:
						images = &material.specularImages;
						binding = 1;
						break;
					case aiTextureType_HEIGHT:
						images = &material.heightImages;
						binding = 2;
						break;
					default: //aiTextureType_AMBIENT
						images = &material.ambientImages;
						binding = 3;
						break;
				}

				bool hasImage = images != NULL && j < images->size();

				if (hasImage) {
					LOG("HAS IMAGE binding: %u index: %zu N: %u", binding, j, NUM_TEXTURE_TYPES);
					ImageInfo* imageInfo = images->at(i);
					VkDescriptorImageInfo descriptorInfo = {};
					descriptorInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
					descriptorInfo.imageView = imageInfo->imageView;
					descriptorInfo.sampler = imageInfo->sampler;

					writeSets[j].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
					writeSets[j].dstSet = descriptorSet;
					writeSets[j].dstBinding = binding;
					writeSets[j].dstArrayElement = 0;
					writeSets[j].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
					writeSets[j].descriptorCount = 1;
					writeSets[j].pImageInfo = &descriptorInfo;
				}
			}
			vkUpdateDescriptorSets(mState.device, writeSets.size(), writeSets.data(), 0, nullptr);
		} 
	}
}

void Model::draw(VkCommandBuffer& commandBuffer, VkPipeline& pipeline, VkPipelineLayout& pipelineLayout)
{
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
	
	VkDeviceSize offset = vertexBufferOffset;
	VkBuffer& commonBuff = mCommonBufferInfo.buffer;
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, &commonBuff, &offset);
	vkCmdBindIndexBuffer(commandBuffer, mCommonBufferInfo.buffer, indexBufferOffset, VK_INDEX_TYPE_UINT32);
	
	for (const auto& mesh : mMeshes) {
		Material& material = mMaterialIndexToMaterial[mesh.materialIndex];
		VkDescriptorSet sets[] = {
			mUniformDescriptorSet,
			material.descriptors[0]
		};

		vkCmdBindDescriptorSets(
			commandBuffer, 
			VK_PIPELINE_BIND_POINT_GRAPHICS, 
			pipelineLayout, 
			0, 
			ARRAY_SIZE(sets), 
			sets, 
			0, 
			nullptr);
		vkCmdDrawIndexed(commandBuffer, mesh.numIndices, 1, mesh.baseIndex, 0, 0);
	}
}

void Model::update(VkCommandBuffer& cmdBuffer, const Timer& timer, Camera& camera)
{
	ubo.view = camera.view();
	ubo.proj = camera.proj();

	vkCmdUpdateBuffer(
			cmdBuffer,
			mCommonBufferInfo.buffer,
			uniformBufferOffset,
			sizeof(UBO),
			&ubo);
			
}

void Model::convertVector(const aiVector3D& src, glm::vec3& dest)
{
    dest.x = src.x;
    dest.y = src.y; 
    dest.z = src.z;
}

void Model::convertVector(const aiVector3D& src, glm::vec2& dest)
{
    dest.x = src.x;
    dest.y = src.y; 
}


void Model::throwError(const char* error) 
{
	std::string errorStr = error;
	throwError(errorStr);
}

void Model::throwError(std::string& error) 
{
	error += " for scene: " + mPath;
	throw std::runtime_error(error);
}
