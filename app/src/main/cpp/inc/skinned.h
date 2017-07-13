#ifndef AMVK_SKINNED_MODEL_H
#define AMVK_SKINNED_MODEL_H

#include "macro.h"


#ifdef __ANDROID__
#include "vulkan_wrapper.h"
#else
#include <vulkan/vulkan.h>
#endif

#include <cstring>
#include <cstddef>
#include <cstdint>
#include <string>
#include <array>
#include <vector>
#include <stack>
#include <unordered_map>
#include <unordered_set>

#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags



#include "macro.h"

#include <glm/gtc/type_ptr.hpp>

#include "util.h"
#include "pipeline_cache.h"
#include "buffer_helper.h"
#include "vulkan_image_creator.h"
#include "vulkan_image_info.h"
#include "vulkan_render_pass_creator.h"
#include "vulkan_state.h"
#include "texture_manager.h"
#include "pipeline_creator.h"
#include "timer.h"
#include "camera.h"
#include "anim_node.h"

#define MAX_SAMPLERS_PER_VERTEX 4

class Skinned {
public:
	typedef int ModelFlags;
	static constexpr int ModelFlag_stripFullPath = 1;
	
	static const aiTextureType* TEXTURE_TYPES;
	static const uint32_t NUM_TEXTURE_TYPES;



	static constexpr uint32_t const MAX_BONES = 64;
	static constexpr uint32_t const MAX_BONES_PER_VERTEX = 4;
	static constexpr uint32_t const DEFAULT_FLAGS = 
								aiProcess_Triangulate | 
								aiProcess_GenSmoothNormals | 
								aiProcess_ImproveCacheLocality | 
							//	aiProcess_JoinIdenticalVertices | 
								aiProcess_SortByPType | 
								aiProcess_FindDegenerates | 
								aiProcess_FindInvalidData | 
								//aiProcess_FlipUVs | 
								aiProcess_CalcTangentSpace | 
								aiProcess_OptimizeMeshes | 
								aiProcess_OptimizeGraph;

	static void convertVector(const aiVector3D& src, glm::vec3& dest);
	static void convertVector(const aiVector3D& src, glm::vec2& dest);

	struct Vertex {
		glm::vec3 pos;
		glm::vec3 normal;
		glm::vec3 tangent;
		glm::vec3 bitangent;
		glm::vec2 texCoord;
		glm::uvec4 boneIndices;
		glm::vec4 weights;
		glm::uvec4 samplerIndices;
	};

	struct UBO {
	    glm::mat4 model;
		glm::mat4 view;
		glm::mat4 proj;
		std::array<glm::mat4, MAX_BONES> bones;
	};

	struct MaterialTexture {
		MaterialTexture():
			index(0),
			image(NULL) {}
		uint32_t index;
		aiTextureType type;
		ImageInfo* image;
	};

	struct Material {
		std::vector<MaterialTexture> textures;
		std::vector<uint32_t> diffuseIndices, specularIndices, heightIndices, ambientIndices;
	}; 

	struct Mesh {
		Mesh(): baseVertex(0), numVertices(0), baseIndex(0), numIndices(0), materialIndex(0) {}
		uint32_t baseVertex, numVertices;
		uint32_t baseIndex, numIndices;
		uint32_t materialIndex;
	};

	Skinned(VulkanState& vulkanState);
	virtual ~Skinned();

	void init(const char* modelPath, unsigned int pFlags = DEFAULT_FLAGS, ModelFlags modelFlags = 0);
	
	void init(std::string modelPath, unsigned int pFlags = DEFAULT_FLAGS, ModelFlags modelFlags = 0); 

	void createAnimNode(aiNode* node, AnimNode* parent);
	void processMeshVertices(std::vector<Vertex>& vertices, aiMesh& mesh, Mesh& meshInfo);
	void processMeshBones(
			aiNode* node, 
			std::unordered_map<std::string, uint32_t>& boneNameToIndexMap, 
			std::vector<Vertex>& vertices, 
			std::vector<uint32_t>& vertexWeightIndices,
			aiMesh& mesh, 
			Mesh& meshInfo);
	void processMeshIndices(std::vector<uint32_t>& indices, aiMesh& mesh, Mesh& meshInfo);
	void processMeshMaterials(aiMesh& mesh, Mesh& meshInfo);
	void processModel(const aiScene& scene);
	
	void createCommonBuffer(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);
	void createVertexBuffer(std::vector<Vertex>& vertices);
	void createIndexBuffer(std::vector<uint32_t>& indices);
	void createUniformBuffer();
	void createDescriptorPool();
	void createDescriptorSet();

	void processAnimNode(float progress, aiMatrix4x4& parentTransform, AnimNode* animNode, uint32_t animationIndex);
	void update(VkCommandBuffer& commandBuffer, const Timer& timer, Camera& camera, uint32_t animationIndex = 0);
	void draw(VkCommandBuffer& commandBuffer, VkPipeline& pipeline, VkPipelineLayout& pipelineLayout);

	void throwError(const char* error);
	void throwError(std::string& error);
	
	float animSpeedScale;
	uint32_t numVertices, numIndices, numBones, numSamplers;
	VkDeviceSize uniformBufferOffset,  
				 vertexBufferOffset, 
				 indexBufferOffset;
	UBO ubo;

protected:
	std::vector<Mesh> mMeshes;
	uint32_t mNumSamplerDescriptors;
	VkDescriptorPool mDescriptorPool;
	VkDescriptorSet mUniformDescriptorSet;
	VkDescriptorSet mSamplersDescriptorSet;

	VulkanState& mState;
	BufferInfo mCommonBufferInfo;
	BufferInfo mCommonStagingBufferInfo;

	std::string mPath, mFolder;
	ModelFlags mModelFlags;

	std::unordered_map<uint32_t, Material> mMaterialIndexToMaterial;

    const aiScene* mScene;
    Assimp::Importer importer;    
    AnimNode* mAnimNodeRoot;
    aiMatrix4x4 mModelSpaceTransform;

	std::unordered_map<aiNode*, uint32_t> mNodeToBoneIndexMap;
	std::vector<aiMatrix4x4> mBoneTransforms;
};

#endif
