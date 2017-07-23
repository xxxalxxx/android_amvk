#include "sphere.h"

Sphere::Sphere(State& state):
	vertexBufferOffset(0),
	indexBufferOffset(0),
	mState(state),
	mCommonBufferInfo(state.device)
{

}

void Sphere::init(uint32_t numStacks, uint32_t numSlices, float radius /* = 1.0f */)
{

    mRadius = radius;
	uint32_t numVertices = (numStacks + 1) * (numSlices + 1);
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

    float phiStep = M_PI / numStacks;
    float thetaStep = 2.0f * M_PI / numSlices;

	for (size_t i = 0; i <= numStacks; ++i) {
        float phi = i * phiStep;
        // vertices of ring
        for (size_t j = 0; j <= numSlices; ++j) {
            float theta = j * thetaStep;

            glm::vec3 position;
            position.x = radius * sinf(phi) * cosf(theta);
            position.y = radius * cosf(phi);
            position.z = radius * sinf(phi) * sinf(theta);
			
			Vertex vertex = { position };
            vertices.push_back(vertex);
        }
    }

    size_t northPoleIndex = 0;//vertices.size() - 1;
    size_t southPoleIndex = vertices.size() - 1;

    size_t numRingVertices = numSlices + 1;

    for (size_t i = 0; i < numStacks; ++i) {
        for (size_t j = 0; j < numSlices; ++j) {
			indices.push_back(i * numRingVertices + j);
			indices.push_back(i * numRingVertices + j + 1);
			indices.push_back((i + 1) * numRingVertices + j);

			indices.push_back((i + 1) * numRingVertices + j);
			indices.push_back(i * numRingVertices + j + 1);
			indices.push_back((i + 1) * numRingVertices + j + 1);
        }
    }

	/*

	mRadius = radius;
	uint32_t numVertices = (numStacks - 1) * (numSlices + 1) + 2;
	std::vector<Vertex> vertices(numVertices);
	std::vector<uint32_t> indices;
   
    float phiStep = M_PI / numStacks;
    float thetaStep = 2.0f * M_PI / numSlices;
	// size_t numRings = numStacks - 1;
	// create vertices except for north and south poles
    // pole logic is separate and loop starts with 1 and lacking one stack to avoid numSlices of repeating poles for first and last step
	for (size_t i = 1; i < numStacks; ++i) {
        float phi = i * phiStep;
        // vertices of ring
        for (size_t j = 0; j <= numSlices; ++j) {
            float theta = j * thetaStep;
			Vertex& vertex = vertices[(i - 1) * (numSlices + 1) + j];
            glm::vec3& position = vertex.position;
            position.x = radius * sinf(phi) * cosf(theta);
            position.y = radius * cosf(phi);
            position.z = radius * sinf(phi) * sinf(theta);
        }
    }
    
	size_t northPoleIndex = vertices.size() - 1;
    size_t southPoleIndex = vertices.size() - 2;

	vertices[northPoleIndex].position = glm::vec3(0.0f, radius, 0.0f); 
	vertices[southPoleIndex].position = glm::vec3(0.0f, -radius, 0.0f); 


    size_t numRingVertices = numSlices + 1;

    for (size_t i = 0; i < numStacks - 2; ++i) {
        for (size_t j = 0; j < numSlices; ++j) {
			indices.push_back(i * numRingVertices + j);
			indices.push_back(i * numRingVertices + j + 1);
			indices.push_back((i + 1) * numRingVertices + j);

			indices.push_back((i + 1) * numRingVertices + j);
			indices.push_back(i * numRingVertices + j + 1);
			indices.push_back((i + 1) * numRingVertices + j + 1);
        }
    }

	// build triangles around north pole
    for (size_t i = 0; i < numSlices; ++i) {
        indices.push_back(northPoleIndex);
        indices.push_back(i + 1);
        indices.push_back(i);
    }

	// build trianges around south pole
    size_t baseIndex = (numStacks - 2) * numRingVertices;
    for (size_t i = 0; i < numSlices; ++i) {
        indices.push_back(southPoleIndex);
        indices.push_back(baseIndex + i);
        indices.push_back(baseIndex + i + 1);
    }
	*/

    numIndices = indices.size();
    createBuffers(vertices, indices);
	LOG("SPHERE CREATED");
	//throw std::runtime_error("SPHERE");
	
}

void Sphere::createBuffers(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices)
{
	VkDeviceSize vertexBufferSize = sizeof(vertices[0]) * vertices.size();
	VkDeviceSize indexBufferSize = sizeof(indices[0]) * indices.size();
	vertexBufferOffset = 0;
	indexBufferOffset = vertexBufferSize;

	mCommonBufferInfo.size = vertexBufferSize + indexBufferSize;
	BufferInfo staging(mState.device, mCommonBufferInfo.size);
	BufferHelper::createStagingBuffer(mState, staging);
	
	char* data;
	vkMapMemory(mState.device, staging.memory, 0, staging.size, 0, (void**) &data);
	memcpy(data + vertexBufferOffset, vertices.data(), vertexBufferSize);
	memcpy(data + indexBufferOffset, indices.data(), indexBufferSize);
	vkUnmapMemory(mState.device, staging.memory);

	BufferHelper::createVertexAndIndexBuffer(mState, mCommonBufferInfo);

	BufferHelper::copyBuffer(
            mState,
			staging.buffer, 
			mCommonBufferInfo.buffer, 
			mCommonBufferInfo.size);
}

float Sphere::getRadius() const 
{
	return mRadius;
}
