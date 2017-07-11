#include "swap_chain_desc.h"

SwapChainDesc::SwapChainDesc() 
{

}

bool SwapChainDesc::supported() const 
{
	return !surfaceFormats.empty() && !presentModes.empty();
}
