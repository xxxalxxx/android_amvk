#include "anim_node.h"

const double AnimNode::DEFAULT_TICKS_PER_SECOND = 25.0;
const double AnimNode::DEFAULT_TICKS_DURATION = 100.0;


AnimNode::AnimNode(aiNode& assimpNode): 
    mAssimpNode(assimpNode),
	boneIndex(0)
{
}

AnimNode::~AnimNode()
{
}



bool AnimNode::isAnimatedAtIndex(unsigned int animIndex) const
{
    return animIndex < mAnimTypes.size() && mAnimTypes[animIndex] != NULL;
}

aiMatrix4x4 AnimNode::getAnimatedTransform(float progress, unsigned int animIndex)
{

    if (!isAnimatedAtIndex(animIndex)) 
		return mAssimpNode.mTransformation;

    aiNodeAnim& channel     = *mAnimTypes[animIndex];

    aiMatrix4x4 translation = getTranslation(progress, animIndex, channel), 
                rotation    = getRotation(progress, animIndex, channel), 
                scaling     = getScaling(progress, animIndex, channel);

	return translation * rotation * scaling;
}

aiMatrix4x4 AnimNode::getTranslation(float progress, unsigned int animIndex, aiNodeAnim& channel)
{   
    aiMatrix4x4 translation;    
    if (channel.mNumPositionKeys == 0) 
		return translation;   
    if (channel.mNumPositionKeys == 1) {
        aiMatrix4x4::Translation(channel.mPositionKeys[0].mValue, translation);
        return translation;
    }
     
    for (size_t i = 0; i < channel.mNumPositionKeys - 1; ++i) {
        if (progress < channel.mPositionKeys[i + 1].mTime) {
            aiVectorKey& pk0 = channel.mPositionKeys[i];
            aiVectorKey& pk1 = channel.mPositionKeys[i + 1];
            // [0,1] interpolation scalar = (progress - t0)/(dt), dt = t1 - t0
            float t = (progress - pk0.mTime) / (pk1.mTime - pk0.mTime);
            // one of [progress, key time, ticks, duration] is wrong, return identity               
            if (t < 0.0f || t > 1.0f) 
				return translation;
            aiVector3D p = pk0.mValue + t * (pk1.mValue - pk0.mValue);
            aiMatrix4x4::Translation(p, translation);
            return translation;
        }
    }
    return translation;
}

aiMatrix4x4 AnimNode::getRotation(float progress, unsigned int animIndex, aiNodeAnim& channel)
{
    if (channel.mNumRotationKeys == 0) 
		return aiMatrix4x4();   
    if (channel.mNumRotationKeys == 1) 
		return aiMatrix4x4(channel.mRotationKeys[0].mValue.GetMatrix());

    for (size_t i = 0; i < channel.mNumRotationKeys - 1; ++i) {
        if (progress < channel.mRotationKeys[i+1].mTime) {
            aiQuatKey& qk0 = channel.mRotationKeys[i];
            aiQuatKey& qk1 = channel.mRotationKeys[i+1];

            float t = (progress - qk0.mTime)/(qk1.mTime - qk0.mTime);    

            if (t < 0.0f || t > 1.0f) 
				return aiMatrix4x4();                
            
            aiQuaternion q;
            aiQuaternion::Interpolate(q, qk0.mValue, qk1.mValue, t);
            q.Normalize();
            
            return aiMatrix4x4(q.GetMatrix());
        }
    }
    return aiMatrix4x4();
}

aiMatrix4x4 AnimNode::getScaling(float progress, unsigned int animIndex, aiNodeAnim& channel)
{
    aiMatrix4x4 scaling;
    if (channel.mNumScalingKeys == 0) 
		return scaling;
    if (channel.mNumScalingKeys == 1) {
        aiMatrix4x4::Scaling(channel.mScalingKeys[0].mValue, scaling);
        return scaling;
    }

    for (size_t i = 0; i < channel.mNumScalingKeys - 1; ++i) {
        if (progress < channel.mScalingKeys[i+1].mTime) {
            aiVectorKey& sk0 = channel.mScalingKeys[i];
            aiVectorKey& sk1 = channel.mScalingKeys[i+1];
            
            float t = (progress - sk0.mTime) / (sk1.mTime - sk0.mTime);

            if (t < 0.0f || t > 1.0f) 
				return scaling;

            aiVector3D p = sk0.mValue + t * (sk1.mValue - sk0.mValue);
            aiMatrix4x4::Scaling(p, scaling);
            return scaling;
        }   
    }
    return scaling;
}

