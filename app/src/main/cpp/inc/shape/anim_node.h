#ifndef AMVK_ANIM_NODE_H
#define AMVK_ANIM_NODE_H

#include <math.h>
#include <vector>

#include <assimp/scene.h>  
#include <glm/vec4.hpp>

#define BONE_INDEX_UNSET UINT32_MAX

struct AnimNode 
{
    const static double DEFAULT_TICKS_PER_SECOND;
    const static double DEFAULT_TICKS_DURATION;

    AnimNode(aiNode& assimpNode);
    ~AnimNode();

    bool isAnimatedAtIndex(uint32_t animIndex) const;

    aiMatrix4x4 getAnimatedTransform(float progress, uint32_t animIndex);
    
    aiNode& mAssimpNode;
    
    uint32_t boneIndex;

    std::vector<AnimNode*> mChildren;
    std::vector<aiNodeAnim*> mAnimTypes;

private:
    aiMatrix4x4 getTranslation(float progress, uint32_t animIndex, aiNodeAnim& channel);
    aiMatrix4x4 getRotation(float progress, uint32_t animIndex, aiNodeAnim& channel);
    aiMatrix4x4 getScaling(float progress, uint32_t animIndex, aiNodeAnim& channel);
};

#endif
