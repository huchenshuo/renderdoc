/******************************************************************************
 * The MIT License (MIT)
 * 
 * Copyright (c) 2015 Baldur Karlsson
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 ******************************************************************************/

#include "vk_common.h"
#include "vk_manager.h"
#include "vk_resources.h"

// VKTODO: need a Deserialise or otherwise deallocate function to release buffers or new'd memory

VulkanResourceManager *manager = NULL;
#define MGR() VulkanResourceManager::GetInstance()

template<>
string ToStrHelper<false, DescriptorSlotType>::Get(const DescriptorSlotType &el)
{
	switch(el)
	{
		case DescSetSlot_DescSet:		return "Nested Descriptor";
		case DescSetSlot_Image:			return "Image View";
		case DescSetSlot_Memory:		return "Memory";
		case DescSetSlot_Sampler:		return "Sampler";
		case DescSetSlot_None:			return "None";
	}

	return "Unknown";
}

template<>
string ToStrHelper<false, VkQueueFlagBits>::Get(const VkQueueFlagBits &el)
{
	string ret;

	if(el & VK_QUEUE_GRAPHICS_BIT)      ret += " | VK_QUEUE_GRAPHICS_BIT";
	if(el & VK_QUEUE_COMPUTE_BIT)       ret += " | VK_QUEUE_COMPUTE_BIT";
	if(el & VK_QUEUE_DMA_BIT)           ret += " | VK_QUEUE_DMA_BIT";
	if(el & VK_QUEUE_SPARSE_MEMMGR_BIT) ret += " | VK_QUEUE_SPARSE_MEMMGR_BIT";
	if(el & VK_QUEUE_EXTENDED_BIT)      ret += " | VK_QUEUE_EXTENDED_BIT";
	
	if(!ret.empty())
		ret = ret.substr(3);

	return ret;
}

template<>
string ToStrHelper<false, VkPipelineBindPoint>::Get(const VkPipelineBindPoint &el)
{
	switch(el)
	{
		TOSTR_CASE_STRINGIZE(VK_PIPELINE_BIND_POINT_COMPUTE)
		TOSTR_CASE_STRINGIZE(VK_PIPELINE_BIND_POINT_GRAPHICS)
		default: break;
	}
	
	return StringFormat::Fmt("VkPipelineBindPoint<%d>", el);
}

template<>
string ToStrHelper<false, VkIndexType>::Get(const VkIndexType &el)
{
	switch(el)
	{
		TOSTR_CASE_STRINGIZE(VK_INDEX_TYPE_UINT16)
		TOSTR_CASE_STRINGIZE(VK_INDEX_TYPE_UINT32)
		default: break;
	}
	
	return StringFormat::Fmt("VkIndexType<%d>", el);
}

template<>
string ToStrHelper<false, VkTimestampType>::Get(const VkTimestampType &el)
{
	switch(el)
	{
		TOSTR_CASE_STRINGIZE(VK_TIMESTAMP_TYPE_TOP)
		TOSTR_CASE_STRINGIZE(VK_TIMESTAMP_TYPE_BOTTOM)
		default: break;
	}
	
	return StringFormat::Fmt("VkTimestampType<%d>", el);
}

template<>
string ToStrHelper<false, VkImageType>::Get(const VkImageType &el)
{
	switch(el)
	{
		TOSTR_CASE_STRINGIZE(VK_IMAGE_TYPE_1D)
		TOSTR_CASE_STRINGIZE(VK_IMAGE_TYPE_2D)
		TOSTR_CASE_STRINGIZE(VK_IMAGE_TYPE_3D)
		default: break;
	}
	
	return StringFormat::Fmt("VkImageType<%d>", el);
}

template<>
string ToStrHelper<false, VkImageTiling>::Get(const VkImageTiling &el)
{
	switch(el)
	{
		TOSTR_CASE_STRINGIZE(VK_IMAGE_TILING_LINEAR)
		TOSTR_CASE_STRINGIZE(VK_IMAGE_TILING_OPTIMAL)
		default: break;
	}
	
	return StringFormat::Fmt("VkImageTiling<%d>", el);
}

template<>
string ToStrHelper<false, VkImageViewType>::Get(const VkImageViewType &el)
{
	switch(el)
	{
		TOSTR_CASE_STRINGIZE(VK_IMAGE_VIEW_TYPE_1D)
		TOSTR_CASE_STRINGIZE(VK_IMAGE_VIEW_TYPE_2D)
		TOSTR_CASE_STRINGIZE(VK_IMAGE_VIEW_TYPE_3D)
		TOSTR_CASE_STRINGIZE(VK_IMAGE_VIEW_TYPE_CUBE)
		default: break;
	}
	
	return StringFormat::Fmt("VkImageViewType<%d>", el);
}

template<>
string ToStrHelper<false, VkVertexInputStepRate>::Get(const VkVertexInputStepRate &el)
{
	switch(el)
	{
		TOSTR_CASE_STRINGIZE(VK_VERTEX_INPUT_STEP_RATE_VERTEX)
		TOSTR_CASE_STRINGIZE(VK_VERTEX_INPUT_STEP_RATE_INSTANCE)
		default: break;
	}
	
	return StringFormat::Fmt("VkVertexInputStepRate<%d>", el);
}

template<>
string ToStrHelper<false, VkFillMode>::Get(const VkFillMode &el)
{
	switch(el)
	{
		TOSTR_CASE_STRINGIZE(VK_FILL_MODE_POINTS)
		TOSTR_CASE_STRINGIZE(VK_FILL_MODE_WIREFRAME)
		TOSTR_CASE_STRINGIZE(VK_FILL_MODE_SOLID)
		default: break;
	}
	
	return StringFormat::Fmt("VkFillMode<%d>", el);
}

template<>
string ToStrHelper<false, VkCullMode>::Get(const VkCullMode &el)
{
	switch(el)
	{
		TOSTR_CASE_STRINGIZE(VK_CULL_MODE_NONE)
		TOSTR_CASE_STRINGIZE(VK_CULL_MODE_FRONT)
		TOSTR_CASE_STRINGIZE(VK_CULL_MODE_BACK)
		TOSTR_CASE_STRINGIZE(VK_CULL_MODE_FRONT_AND_BACK)
		default: break;
	}
	
	return StringFormat::Fmt("VkCullMode<%d>", el);
}

template<>
string ToStrHelper<false, VkFrontFace>::Get(const VkFrontFace &el)
{
	switch(el)
	{
		TOSTR_CASE_STRINGIZE(VK_FRONT_FACE_CCW)
		TOSTR_CASE_STRINGIZE(VK_FRONT_FACE_CW)
		default: break;
	}
	
	return StringFormat::Fmt("VkFrontFace<%d>", el);
}

template<>
string ToStrHelper<false, VkBlend>::Get(const VkBlend &el)
{
	switch(el)
	{
		TOSTR_CASE_STRINGIZE(VK_BLEND_ZERO)
		TOSTR_CASE_STRINGIZE(VK_BLEND_ONE)
		TOSTR_CASE_STRINGIZE(VK_BLEND_SRC_COLOR)
		TOSTR_CASE_STRINGIZE(VK_BLEND_ONE_MINUS_SRC_COLOR)
		TOSTR_CASE_STRINGIZE(VK_BLEND_DEST_COLOR)
		TOSTR_CASE_STRINGIZE(VK_BLEND_ONE_MINUS_DEST_COLOR)
		TOSTR_CASE_STRINGIZE(VK_BLEND_SRC_ALPHA)
		TOSTR_CASE_STRINGIZE(VK_BLEND_ONE_MINUS_SRC_ALPHA)
		TOSTR_CASE_STRINGIZE(VK_BLEND_DEST_ALPHA)
		TOSTR_CASE_STRINGIZE(VK_BLEND_ONE_MINUS_DEST_ALPHA)
		TOSTR_CASE_STRINGIZE(VK_BLEND_CONSTANT_COLOR)
		TOSTR_CASE_STRINGIZE(VK_BLEND_ONE_MINUS_CONSTANT_COLOR)
		TOSTR_CASE_STRINGIZE(VK_BLEND_CONSTANT_ALPHA)
		TOSTR_CASE_STRINGIZE(VK_BLEND_ONE_MINUS_CONSTANT_ALPHA)
		TOSTR_CASE_STRINGIZE(VK_BLEND_SRC_ALPHA_SATURATE)
		TOSTR_CASE_STRINGIZE(VK_BLEND_SRC1_COLOR)
		TOSTR_CASE_STRINGIZE(VK_BLEND_ONE_MINUS_SRC1_COLOR)
		TOSTR_CASE_STRINGIZE(VK_BLEND_SRC1_ALPHA)
		TOSTR_CASE_STRINGIZE(VK_BLEND_ONE_MINUS_SRC1_ALPHA)
		default: break;
	}
	
	return StringFormat::Fmt("VK_BLEND<%d>", el);
}

template<>
string ToStrHelper<false, VkBlendOp>::Get(const VkBlendOp &el)
{
	switch(el)
	{
		TOSTR_CASE_STRINGIZE(VK_BLEND_OP_ADD)
		TOSTR_CASE_STRINGIZE(VK_BLEND_OP_SUBTRACT)
		TOSTR_CASE_STRINGIZE(VK_BLEND_OP_REVERSE_SUBTRACT)
		TOSTR_CASE_STRINGIZE(VK_BLEND_OP_MIN)
		TOSTR_CASE_STRINGIZE(VK_BLEND_OP_MAX)
		default: break;
	}
	
	return StringFormat::Fmt("VkBlendOp<%d>", el);
}

template<>
string ToStrHelper<false, VkStencilOp>::Get(const VkStencilOp &el)
{
	switch(el)
	{
		TOSTR_CASE_STRINGIZE(VK_STENCIL_OP_KEEP)
		TOSTR_CASE_STRINGIZE(VK_STENCIL_OP_ZERO)
		TOSTR_CASE_STRINGIZE(VK_STENCIL_OP_REPLACE)
		TOSTR_CASE_STRINGIZE(VK_STENCIL_OP_INC_CLAMP)
		TOSTR_CASE_STRINGIZE(VK_STENCIL_OP_DEC_CLAMP)
		TOSTR_CASE_STRINGIZE(VK_STENCIL_OP_INVERT)
		TOSTR_CASE_STRINGIZE(VK_STENCIL_OP_INC_WRAP)
		TOSTR_CASE_STRINGIZE(VK_STENCIL_OP_DEC_WRAP)
		default: break;
	}
	
	return StringFormat::Fmt("VkStencilOp<%d>", el);
}

template<>
string ToStrHelper<false, VkLogicOp>::Get(const VkLogicOp &el)
{
	switch(el)
	{
		TOSTR_CASE_STRINGIZE(VK_LOGIC_OP_COPY)
		TOSTR_CASE_STRINGIZE(VK_LOGIC_OP_CLEAR)
		TOSTR_CASE_STRINGIZE(VK_LOGIC_OP_AND)
		TOSTR_CASE_STRINGIZE(VK_LOGIC_OP_AND_REVERSE)
		TOSTR_CASE_STRINGIZE(VK_LOGIC_OP_AND_INVERTED)
		TOSTR_CASE_STRINGIZE(VK_LOGIC_OP_NOOP)
		TOSTR_CASE_STRINGIZE(VK_LOGIC_OP_XOR)
		TOSTR_CASE_STRINGIZE(VK_LOGIC_OP_OR)
		TOSTR_CASE_STRINGIZE(VK_LOGIC_OP_NOR)
		TOSTR_CASE_STRINGIZE(VK_LOGIC_OP_EQUIV)
		TOSTR_CASE_STRINGIZE(VK_LOGIC_OP_INVERT)
		TOSTR_CASE_STRINGIZE(VK_LOGIC_OP_OR_REVERSE)
		TOSTR_CASE_STRINGIZE(VK_LOGIC_OP_COPY_INVERTED)
		TOSTR_CASE_STRINGIZE(VK_LOGIC_OP_OR_INVERTED)
		TOSTR_CASE_STRINGIZE(VK_LOGIC_OP_NAND)
		TOSTR_CASE_STRINGIZE(VK_LOGIC_OP_SET)
		default: break;
	}
	
	return StringFormat::Fmt("VkLogicOp<%d>", el);
}

template<>
string ToStrHelper<false, VkCompareOp>::Get(const VkCompareOp &el)
{
	switch(el)
	{
		TOSTR_CASE_STRINGIZE(VK_COMPARE_OP_NEVER)
		TOSTR_CASE_STRINGIZE(VK_COMPARE_OP_LESS)
		TOSTR_CASE_STRINGIZE(VK_COMPARE_OP_EQUAL)
		TOSTR_CASE_STRINGIZE(VK_COMPARE_OP_LESS_EQUAL)
		TOSTR_CASE_STRINGIZE(VK_COMPARE_OP_GREATER)
		TOSTR_CASE_STRINGIZE(VK_COMPARE_OP_NOT_EQUAL)
		TOSTR_CASE_STRINGIZE(VK_COMPARE_OP_GREATER_EQUAL)
		TOSTR_CASE_STRINGIZE(VK_COMPARE_OP_ALWAYS)
		default: break;
	}
	
	return StringFormat::Fmt("VkCompareOp<%d>", el);
}

template<>
string ToStrHelper<false, VkTexFilter>::Get(const VkTexFilter &el)
{
	switch(el)
	{
		TOSTR_CASE_STRINGIZE(VK_TEX_FILTER_NEAREST)
		TOSTR_CASE_STRINGIZE(VK_TEX_FILTER_LINEAR)
		default: break;		 
	}
	
	return StringFormat::Fmt("VkTexFilter<%d>", el);
}

template<>
string ToStrHelper<false, VkTexMipmapMode>::Get(const VkTexMipmapMode &el)
{
	switch(el)
	{
		TOSTR_CASE_STRINGIZE(VK_TEX_MIPMAP_MODE_BASE)
		TOSTR_CASE_STRINGIZE(VK_TEX_MIPMAP_MODE_NEAREST)
		TOSTR_CASE_STRINGIZE(VK_TEX_MIPMAP_MODE_LINEAR)
		default: break;		 
	}
	
	return StringFormat::Fmt("VkTexMipmapMode<%d>", el);
}

template<>
string ToStrHelper<false, VkTexAddress>::Get(const VkTexAddress &el)
{
	switch(el)
	{
		TOSTR_CASE_STRINGIZE(VK_TEX_ADDRESS_WRAP)
		TOSTR_CASE_STRINGIZE(VK_TEX_ADDRESS_MIRROR)
		TOSTR_CASE_STRINGIZE(VK_TEX_ADDRESS_CLAMP)
		TOSTR_CASE_STRINGIZE(VK_TEX_ADDRESS_MIRROR_ONCE)
		TOSTR_CASE_STRINGIZE(VK_TEX_ADDRESS_CLAMP_BORDER)
		default: break;		 
	}
	
	return StringFormat::Fmt("VkTexAddress<%d>", el);
}

template<>
string ToStrHelper<false, VkBorderColor>::Get(const VkBorderColor &el)
{
	switch(el)
	{
		TOSTR_CASE_STRINGIZE(VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK)
		TOSTR_CASE_STRINGIZE(VK_BORDER_COLOR_INT_TRANSPARENT_BLACK)
		TOSTR_CASE_STRINGIZE(VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK)
		TOSTR_CASE_STRINGIZE(VK_BORDER_COLOR_INT_OPAQUE_BLACK)
		TOSTR_CASE_STRINGIZE(VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE)
		TOSTR_CASE_STRINGIZE(VK_BORDER_COLOR_INT_OPAQUE_WHITE)
		default: break;		 
	}
	
	return StringFormat::Fmt("VkBorderColor<%d>", el);
}

template<>
string ToStrHelper<false, VkImageAspect>::Get(const VkImageAspect &el)
{
	switch(el)
	{
		TOSTR_CASE_STRINGIZE(VK_IMAGE_ASPECT_COLOR)
		TOSTR_CASE_STRINGIZE(VK_IMAGE_ASPECT_DEPTH)
		TOSTR_CASE_STRINGIZE(VK_IMAGE_ASPECT_STENCIL)
		default: break;		 
	}
	
	return StringFormat::Fmt("VkImageAspect<%d>", el);
}

template<>
string ToStrHelper<false, VkPrimitiveTopology>::Get(const VkPrimitiveTopology &el)
{
	switch(el)
	{
		TOSTR_CASE_STRINGIZE(VK_PRIMITIVE_TOPOLOGY_POINT_LIST)
		TOSTR_CASE_STRINGIZE(VK_PRIMITIVE_TOPOLOGY_LINE_LIST)
		TOSTR_CASE_STRINGIZE(VK_PRIMITIVE_TOPOLOGY_LINE_STRIP)
		TOSTR_CASE_STRINGIZE(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
		TOSTR_CASE_STRINGIZE(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP)
		TOSTR_CASE_STRINGIZE(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN)
		TOSTR_CASE_STRINGIZE(VK_PRIMITIVE_TOPOLOGY_LINE_LIST_ADJ)
		TOSTR_CASE_STRINGIZE(VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_ADJ)
		TOSTR_CASE_STRINGIZE(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_ADJ)
		TOSTR_CASE_STRINGIZE(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_ADJ)
		TOSTR_CASE_STRINGIZE(VK_PRIMITIVE_TOPOLOGY_PATCH)
		default: break;
	}
	
	return StringFormat::Fmt("VkPrimitiveTopology<%d>", el);
}

template<>
string ToStrHelper<false, VkDescriptorType>::Get(const VkDescriptorType &el)
{
	switch(el)
	{
		TOSTR_CASE_STRINGIZE(VK_DESCRIPTOR_TYPE_SAMPLER)
		TOSTR_CASE_STRINGIZE(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
		TOSTR_CASE_STRINGIZE(VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE)
		TOSTR_CASE_STRINGIZE(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE)
		TOSTR_CASE_STRINGIZE(VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER)
		TOSTR_CASE_STRINGIZE(VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER)
		TOSTR_CASE_STRINGIZE(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
		TOSTR_CASE_STRINGIZE(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER)
		TOSTR_CASE_STRINGIZE(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC)
		TOSTR_CASE_STRINGIZE(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC)
		TOSTR_CASE_STRINGIZE(VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT)
		default: break;
	}
	
	return StringFormat::Fmt("VkDescriptorType<%d>", el);
}

template<>
string ToStrHelper<false, VkDescriptorPoolUsage>::Get(const VkDescriptorPoolUsage &el)
{
	switch(el)
	{
		TOSTR_CASE_STRINGIZE(VK_DESCRIPTOR_POOL_USAGE_ONE_SHOT)
		TOSTR_CASE_STRINGIZE(VK_DESCRIPTOR_POOL_USAGE_DYNAMIC)
		default: break;
	}
	
	return StringFormat::Fmt("VkDescriptorPoolUsage<%d>", el);
}

template<>
string ToStrHelper<false, VkDescriptorSetUsage>::Get(const VkDescriptorSetUsage &el)
{
	switch(el)
	{
		TOSTR_CASE_STRINGIZE(VK_DESCRIPTOR_SET_USAGE_ONE_SHOT)
		TOSTR_CASE_STRINGIZE(VK_DESCRIPTOR_SET_USAGE_STATIC)
		default: break;
	}
	
	return StringFormat::Fmt("VkDescriptorSetUsage<%d>", el);
}

template<>
string ToStrHelper<false, VkMemoryInputFlagBits>::Get(const VkMemoryInputFlagBits &el)
{
	string ret;

	if(el & VK_MEMORY_INPUT_HOST_READ_BIT)                     ret += " | VK_MEMORY_INPUT_HOST_READ_BIT";
	if(el & VK_MEMORY_INPUT_INDIRECT_COMMAND_BIT)              ret += " | VK_MEMORY_INPUT_INDIRECT_COMMAND_BIT";
	if(el & VK_MEMORY_INPUT_INDEX_FETCH_BIT)                   ret += " | VK_MEMORY_INPUT_INDEX_FETCH_BIT";
	if(el & VK_MEMORY_INPUT_VERTEX_ATTRIBUTE_FETCH_BIT)        ret += " | VK_MEMORY_INPUT_VERTEX_ATTRIBUTE_FETCH_BIT";
	if(el & VK_MEMORY_INPUT_UNIFORM_READ_BIT)                  ret += " | VK_MEMORY_INPUT_UNIFORM_READ_BIT";
	if(el & VK_MEMORY_INPUT_SHADER_READ_BIT)                   ret += " | VK_MEMORY_INPUT_SHADER_READ_BIT";
	if(el & VK_MEMORY_INPUT_COLOR_ATTACHMENT_BIT)              ret += " | VK_MEMORY_INPUT_COLOR_ATTACHMENT_BIT";
	if(el & VK_MEMORY_INPUT_DEPTH_STENCIL_ATTACHMENT_BIT)      ret += " | VK_MEMORY_INPUT_DEPTH_STENCIL_ATTACHMENT_BIT";
	if(el & VK_MEMORY_INPUT_INPUT_ATTACHMENT_BIT)              ret += " | VK_MEMORY_INPUT_INPUT_ATTACHMENT_BIT";
	if(el & VK_MEMORY_INPUT_TRANSFER_BIT)                      ret += " | VK_MEMORY_INPUT_TRANSFER_BIT";
	
	if(!ret.empty())
		ret = ret.substr(3);

	return ret;
}

template<>
string ToStrHelper<false, VkMemoryOutputFlagBits>::Get(const VkMemoryOutputFlagBits &el)
{
	string ret;

	if(el & VK_MEMORY_OUTPUT_HOST_WRITE_BIT)                    ret += " | VK_MEMORY_OUTPUT_HOST_WRITE_BIT";
	if(el & VK_MEMORY_OUTPUT_SHADER_WRITE_BIT)                  ret += " | VK_MEMORY_OUTPUT_SHADER_WRITE_BIT";
	if(el & VK_MEMORY_OUTPUT_COLOR_ATTACHMENT_BIT)              ret += " | VK_MEMORY_OUTPUT_COLOR_ATTACHMENT_BIT";
	if(el & VK_MEMORY_OUTPUT_DEPTH_STENCIL_ATTACHMENT_BIT)      ret += " | VK_MEMORY_OUTPUT_DEPTH_STENCIL_ATTACHMENT_BIT";
	if(el & VK_MEMORY_OUTPUT_TRANSFER_BIT)                      ret += " | VK_MEMORY_OUTPUT_TRANSFER_BIT";
	
	if(!ret.empty())
		ret = ret.substr(3);

	return ret;
}

template<>
string ToStrHelper<false, VkBufferViewType>::Get(const VkBufferViewType &el)
{
	switch(el)
	{
		TOSTR_CASE_STRINGIZE(VK_BUFFER_VIEW_TYPE_RAW)
		TOSTR_CASE_STRINGIZE(VK_BUFFER_VIEW_TYPE_FORMATTED)
		default: break;
	}
	
	return StringFormat::Fmt("VkBufferViewType<%d>", el);
}

template<>
string ToStrHelper<false, VkCmdBufferLevel>::Get(const VkCmdBufferLevel &el)
{
	switch(el)
	{
		TOSTR_CASE_STRINGIZE(VK_CMD_BUFFER_LEVEL_PRIMARY)
		TOSTR_CASE_STRINGIZE(VK_CMD_BUFFER_LEVEL_SECONDARY)
		default: break;
	}
	
	return StringFormat::Fmt("VkCmdBufferLevel<%d>", el);
}

template<>
string ToStrHelper<false, VkRenderPassContents>::Get(const VkRenderPassContents &el)
{
	switch(el)
	{
		TOSTR_CASE_STRINGIZE(VK_RENDER_PASS_CONTENTS_INLINE)
		TOSTR_CASE_STRINGIZE(VK_RENDER_PASS_CONTENTS_SECONDARY_CMD_BUFFERS)
		default: break;
	}
	
	return StringFormat::Fmt("VkRenderPassContents<%d>", el);
}

template<>
string ToStrHelper<false, VkImageLayout>::Get(const VkImageLayout &el)
{
	switch((int)el)
	{
		TOSTR_CASE_STRINGIZE(VK_IMAGE_LAYOUT_UNDEFINED)
		TOSTR_CASE_STRINGIZE(VK_IMAGE_LAYOUT_GENERAL)
		TOSTR_CASE_STRINGIZE(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
		TOSTR_CASE_STRINGIZE(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
		TOSTR_CASE_STRINGIZE(VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL)
		TOSTR_CASE_STRINGIZE(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		TOSTR_CASE_STRINGIZE(VK_IMAGE_LAYOUT_TRANSFER_SOURCE_OPTIMAL)
		TOSTR_CASE_STRINGIZE(VK_IMAGE_LAYOUT_TRANSFER_DESTINATION_OPTIMAL)
		TOSTR_CASE_STRINGIZE(VK_IMAGE_LAYOUT_PRESENT_SOURCE_WSI)
		default: break;
	}
	
	return StringFormat::Fmt("VkImageLayout<%d>", el);
}

template<>
string ToStrHelper<false, VkStructureType>::Get(const VkStructureType &el)
{
	switch(el)
	{
    TOSTR_CASE_STRINGIZE(VK_STRUCTURE_TYPE_APPLICATION_INFO)
    TOSTR_CASE_STRINGIZE(VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO)
    TOSTR_CASE_STRINGIZE(VK_STRUCTURE_TYPE_MEMORY_ALLOC_INFO)
    TOSTR_CASE_STRINGIZE(VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO)
    TOSTR_CASE_STRINGIZE(VK_STRUCTURE_TYPE_ATTACHMENT_VIEW_CREATE_INFO)
    TOSTR_CASE_STRINGIZE(VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO)
    TOSTR_CASE_STRINGIZE(VK_STRUCTURE_TYPE_SHADER_CREATE_INFO)
    TOSTR_CASE_STRINGIZE(VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO)
    TOSTR_CASE_STRINGIZE(VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO)
    TOSTR_CASE_STRINGIZE(VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO)
    TOSTR_CASE_STRINGIZE(VK_STRUCTURE_TYPE_DYNAMIC_VIEWPORT_STATE_CREATE_INFO)
    TOSTR_CASE_STRINGIZE(VK_STRUCTURE_TYPE_DYNAMIC_RASTER_STATE_CREATE_INFO)
    TOSTR_CASE_STRINGIZE(VK_STRUCTURE_TYPE_DYNAMIC_COLOR_BLEND_STATE_CREATE_INFO)
    TOSTR_CASE_STRINGIZE(VK_STRUCTURE_TYPE_DYNAMIC_DEPTH_STENCIL_STATE_CREATE_INFO)
    TOSTR_CASE_STRINGIZE(VK_STRUCTURE_TYPE_CMD_BUFFER_CREATE_INFO)
    TOSTR_CASE_STRINGIZE(VK_STRUCTURE_TYPE_EVENT_CREATE_INFO)
    TOSTR_CASE_STRINGIZE(VK_STRUCTURE_TYPE_FENCE_CREATE_INFO)
    TOSTR_CASE_STRINGIZE(VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO)
    TOSTR_CASE_STRINGIZE(VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO)
    TOSTR_CASE_STRINGIZE(VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO)
    TOSTR_CASE_STRINGIZE(VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO)
    TOSTR_CASE_STRINGIZE(VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO)
    TOSTR_CASE_STRINGIZE(VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO)
    TOSTR_CASE_STRINGIZE(VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO)
    TOSTR_CASE_STRINGIZE(VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO)
    TOSTR_CASE_STRINGIZE(VK_STRUCTURE_TYPE_PIPELINE_RASTER_STATE_CREATE_INFO)
    TOSTR_CASE_STRINGIZE(VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO)
    TOSTR_CASE_STRINGIZE(VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO)
    TOSTR_CASE_STRINGIZE(VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO)
    TOSTR_CASE_STRINGIZE(VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO)
    TOSTR_CASE_STRINGIZE(VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO)
    TOSTR_CASE_STRINGIZE(VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO)
    TOSTR_CASE_STRINGIZE(VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO)
    TOSTR_CASE_STRINGIZE(VK_STRUCTURE_TYPE_CMD_BUFFER_BEGIN_INFO)
    TOSTR_CASE_STRINGIZE(VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO)
    TOSTR_CASE_STRINGIZE(VK_STRUCTURE_TYPE_MEMORY_BARRIER)
    TOSTR_CASE_STRINGIZE(VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER)
    TOSTR_CASE_STRINGIZE(VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER)
    TOSTR_CASE_STRINGIZE(VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO)
    TOSTR_CASE_STRINGIZE(VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET)
    TOSTR_CASE_STRINGIZE(VK_STRUCTURE_TYPE_COPY_DESCRIPTOR_SET)
    TOSTR_CASE_STRINGIZE(VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO)
    TOSTR_CASE_STRINGIZE(VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO)
    TOSTR_CASE_STRINGIZE(VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE)
    TOSTR_CASE_STRINGIZE(VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO)
    TOSTR_CASE_STRINGIZE(VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION)
    TOSTR_CASE_STRINGIZE(VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION)
    TOSTR_CASE_STRINGIZE(VK_STRUCTURE_TYPE_SUBPASS_DEPENDENCY)
    TOSTR_CASE_STRINGIZE(VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO)
    TOSTR_CASE_STRINGIZE(VK_STRUCTURE_TYPE_CMD_POOL_CREATE_INFO)
		default: break;
	}

	return StringFormat::Fmt("VkStructureType<%d>", el);
}

template<>
string ToStrHelper<false, VkChannelSwizzle>::Get(const VkChannelSwizzle &el)
{
	switch(el)
	{
		TOSTR_CASE_STRINGIZE(VK_CHANNEL_SWIZZLE_ZERO)
		TOSTR_CASE_STRINGIZE(VK_CHANNEL_SWIZZLE_ONE)
		TOSTR_CASE_STRINGIZE(VK_CHANNEL_SWIZZLE_R)
		TOSTR_CASE_STRINGIZE(VK_CHANNEL_SWIZZLE_G)
		TOSTR_CASE_STRINGIZE(VK_CHANNEL_SWIZZLE_B)
		TOSTR_CASE_STRINGIZE(VK_CHANNEL_SWIZZLE_A)
		default: break;
	}

	return StringFormat::Fmt("VkChannelSwizzle<%d>", el);
}

template<>
string ToStrHelper<false, VkShaderStage>::Get(const VkShaderStage &el)
{
	switch(el)
	{
		TOSTR_CASE_STRINGIZE(VK_SHADER_STAGE_VERTEX)
		TOSTR_CASE_STRINGIZE(VK_SHADER_STAGE_TESS_CONTROL)
		TOSTR_CASE_STRINGIZE(VK_SHADER_STAGE_TESS_EVALUATION)
		TOSTR_CASE_STRINGIZE(VK_SHADER_STAGE_GEOMETRY)
		TOSTR_CASE_STRINGIZE(VK_SHADER_STAGE_FRAGMENT)
		TOSTR_CASE_STRINGIZE(VK_SHADER_STAGE_COMPUTE)
		default: break;
	}

	return StringFormat::Fmt("VkShaderStage<%d>", el);
}

template<>
string ToStrHelper<false, VkFormat>::Get(const VkFormat &el)
{
	switch(el)
	{
		TOSTR_CASE_STRINGIZE(VK_FORMAT_UNDEFINED)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_R4G4_UNORM)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_R4G4_USCALED)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_R4G4B4A4_UNORM)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_R4G4B4A4_USCALED)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_R5G6B5_UNORM)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_R5G6B5_USCALED)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_R5G5B5A1_UNORM)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_R5G5B5A1_USCALED)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_R8_UNORM)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_R8_SNORM)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_R8_USCALED)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_R8_SSCALED)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_R8_UINT)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_R8_SINT)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_R8_SRGB)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_R8G8_UNORM)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_R8G8_SNORM)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_R8G8_USCALED)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_R8G8_SSCALED)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_R8G8_UINT)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_R8G8_SINT)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_R8G8_SRGB)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_R8G8B8_UNORM)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_R8G8B8_SNORM)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_R8G8B8_USCALED)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_R8G8B8_SSCALED)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_R8G8B8_UINT)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_R8G8B8_SINT)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_R8G8B8_SRGB)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_R8G8B8A8_UNORM)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_R8G8B8A8_SNORM)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_R8G8B8A8_USCALED)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_R8G8B8A8_SSCALED)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_R8G8B8A8_UINT)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_R8G8B8A8_SINT)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_R8G8B8A8_SRGB)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_R10G10B10A2_UNORM)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_R10G10B10A2_SNORM)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_R10G10B10A2_USCALED)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_R10G10B10A2_SSCALED)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_R10G10B10A2_UINT)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_R10G10B10A2_SINT)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_R16_UNORM)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_R16_SNORM)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_R16_USCALED)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_R16_SSCALED)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_R16_UINT)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_R16_SINT)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_R16_SFLOAT)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_R16G16_UNORM)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_R16G16_SNORM)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_R16G16_USCALED)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_R16G16_SSCALED)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_R16G16_UINT)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_R16G16_SINT)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_R16G16_SFLOAT)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_R16G16B16_UNORM)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_R16G16B16_SNORM)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_R16G16B16_USCALED)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_R16G16B16_SSCALED)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_R16G16B16_UINT)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_R16G16B16_SINT)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_R16G16B16_SFLOAT)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_R16G16B16A16_UNORM)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_R16G16B16A16_SNORM)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_R16G16B16A16_USCALED)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_R16G16B16A16_SSCALED)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_R16G16B16A16_UINT)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_R16G16B16A16_SINT)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_R16G16B16A16_SFLOAT)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_R32_UINT)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_R32_SINT)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_R32_SFLOAT)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_R32G32_UINT)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_R32G32_SINT)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_R32G32_SFLOAT)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_R32G32B32_UINT)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_R32G32B32_SINT)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_R32G32B32_SFLOAT)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_R32G32B32A32_UINT)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_R32G32B32A32_SINT)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_R32G32B32A32_SFLOAT)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_R64_SFLOAT)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_R64G64_SFLOAT)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_R64G64B64_SFLOAT)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_R64G64B64A64_SFLOAT)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_R11G11B10_UFLOAT)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_R9G9B9E5_UFLOAT)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_D16_UNORM)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_D24_UNORM)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_D32_SFLOAT)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_S8_UINT)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_D16_UNORM_S8_UINT)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_D24_UNORM_S8_UINT)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_D32_SFLOAT_S8_UINT)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_BC1_RGB_UNORM)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_BC1_RGB_SRGB)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_BC1_RGBA_UNORM)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_BC1_RGBA_SRGB)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_BC2_UNORM)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_BC2_SRGB)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_BC3_UNORM)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_BC3_SRGB)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_BC4_UNORM)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_BC4_SNORM)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_BC5_UNORM)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_BC5_SNORM)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_BC6H_UFLOAT)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_BC6H_SFLOAT)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_BC7_UNORM)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_BC7_SRGB)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_ETC2_R8G8B8_UNORM)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_ETC2_R8G8B8_SRGB)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_ETC2_R8G8B8A1_UNORM)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_ETC2_R8G8B8A1_SRGB)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_ETC2_R8G8B8A8_UNORM)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_ETC2_R8G8B8A8_SRGB)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_EAC_R11_UNORM)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_EAC_R11_SNORM)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_EAC_R11G11_UNORM)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_EAC_R11G11_SNORM)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_ASTC_4x4_UNORM)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_ASTC_4x4_SRGB)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_ASTC_5x4_UNORM)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_ASTC_5x4_SRGB)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_ASTC_5x5_UNORM)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_ASTC_5x5_SRGB)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_ASTC_6x5_UNORM)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_ASTC_6x5_SRGB)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_ASTC_6x6_UNORM)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_ASTC_6x6_SRGB)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_ASTC_8x5_UNORM)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_ASTC_8x5_SRGB)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_ASTC_8x6_UNORM)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_ASTC_8x6_SRGB)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_ASTC_8x8_UNORM)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_ASTC_8x8_SRGB)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_ASTC_10x5_UNORM)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_ASTC_10x5_SRGB)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_ASTC_10x6_UNORM)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_ASTC_10x6_SRGB)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_ASTC_10x8_UNORM)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_ASTC_10x8_SRGB)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_ASTC_10x10_UNORM)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_ASTC_10x10_SRGB)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_ASTC_12x10_UNORM)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_ASTC_12x10_SRGB)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_ASTC_12x12_UNORM)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_ASTC_12x12_SRGB)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_B4G4R4A4_UNORM)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_B5G5R5A1_UNORM)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_B5G6R5_UNORM)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_B5G6R5_USCALED)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_B8G8R8_UNORM)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_B8G8R8_SNORM)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_B8G8R8_USCALED)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_B8G8R8_SSCALED)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_B8G8R8_UINT)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_B8G8R8_SINT)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_B8G8R8_SRGB)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_B8G8R8A8_UNORM)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_B8G8R8A8_SNORM)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_B8G8R8A8_USCALED)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_B8G8R8A8_SSCALED)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_B8G8R8A8_UINT)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_B8G8R8A8_SINT)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_B8G8R8A8_SRGB)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_B10G10R10A2_UNORM)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_B10G10R10A2_SNORM)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_B10G10R10A2_USCALED)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_B10G10R10A2_SSCALED)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_B10G10R10A2_UINT)
		TOSTR_CASE_STRINGIZE(VK_FORMAT_B10G10R10A2_SINT)
		default: break;
	}
	
	return StringFormat::Fmt("VkFormat<%d>", el);
}

template<>
string ToStrHelper<false, VkExtent2D>::Get(const VkExtent2D &el)
{
	return StringFormat::Fmt("VkExtent<%d,%d>", el.width, el.height);
}

template<>
string ToStrHelper<false, VkExtent3D>::Get(const VkExtent3D &el)
{
	return StringFormat::Fmt("VkExtent<%d,%d,%d>", el.width, el.height, el.depth);
}

template<>
string ToStrHelper<false, VkOffset2D>::Get(const VkOffset2D &el)
{
	return StringFormat::Fmt("VkOffset<%d,%d>", el.x, el.y);
}

template<>
string ToStrHelper<false, VkOffset3D>::Get(const VkOffset3D &el)
{
	return StringFormat::Fmt("VkOffset<%d,%d,%d>", el.x, el.y, el.z);
}

template<>
string ToStrHelper<false, VkViewport>::Get(const VkViewport &el)
{
	return StringFormat::Fmt("VkViewport<%f,%f, %fx%f, %f-%f>", el.originX, el.originY, el.width, el.height, el.minDepth, el.maxDepth);
}

template<>
string ToStrHelper<false, VkSurfaceTransformWSI>::Get(const VkSurfaceTransformWSI &el)
{
	switch(el)
	{
		TOSTR_CASE_STRINGIZE(VK_SURFACE_TRANSFORM_NONE_WSI)
		TOSTR_CASE_STRINGIZE(VK_SURFACE_TRANSFORM_ROT90_WSI)
		TOSTR_CASE_STRINGIZE(VK_SURFACE_TRANSFORM_ROT180_WSI)
		TOSTR_CASE_STRINGIZE(VK_SURFACE_TRANSFORM_ROT270_WSI)
		TOSTR_CASE_STRINGIZE(VK_SURFACE_TRANSFORM_HMIRROR_WSI)
		TOSTR_CASE_STRINGIZE(VK_SURFACE_TRANSFORM_HMIRROR_ROT90_WSI)
		TOSTR_CASE_STRINGIZE(VK_SURFACE_TRANSFORM_HMIRROR_ROT180_WSI)
		TOSTR_CASE_STRINGIZE(VK_SURFACE_TRANSFORM_HMIRROR_ROT270_WSI)
		TOSTR_CASE_STRINGIZE(VK_SURFACE_TRANSFORM_INHERIT_WSI)
		default: break;
	}

	return StringFormat::Fmt("VkSurfaceTransformWSI<%d>", el);
}

template<>
string ToStrHelper<false, VkPresentModeWSI>::Get(const VkPresentModeWSI &el)
{
	switch(el)
	{
		TOSTR_CASE_STRINGIZE(VK_PRESENT_MODE_IMMEDIATE_WSI)
		TOSTR_CASE_STRINGIZE(VK_PRESENT_MODE_MAILBOX_WSI)
		TOSTR_CASE_STRINGIZE(VK_PRESENT_MODE_FIFO_WSI)
		default: break;
	}

	return StringFormat::Fmt("VkPresentModeWSI<%d>", el);
}

#define SerialiseObject(type, name, obj) \
			{ \
				ResourceId id; \
				if(m_Mode >= WRITING) id = MGR()->GetID(obj.handle); \
				Serialise(name, id); \
				if(m_Mode < WRITING) obj = type(MGR()->GetLiveResource(id).handle); \
			}

static void SerialiseNext(Serialiser *ser, const void *&pNext)
{
	// serialise out whether there is a next structure, its type, and contents
}

template<typename T>
void SerialiseOptionalObject(Serialiser *ser, const char *name, T *&el)
{
	bool present;

	present = el != NULL;
	ser->Serialise("present", present);
	if(present)
	{
		if(ser->IsReading())
			el = new T;
		ser->Serialise(name, *el);
	}
}

/*
#define SerialiseNext(obj) \
			{ \
				NextStructure *next = (NextStructure *)obj; \
				bool hasNext = (next != NULL); \
				Serialise("hasNext", hasNext); \
				if(m_Mode >= WRITING && hasNext) Serialise("NextStruct", *next); \
				if(m_Mode < WRITING && hasNext) obj = Serialisenext; \
			}
*/

template<>
void Serialiser::Serialise(const char *name, VkGenericStruct &el)
{
	ScopedContext scope(this, this, name, "NextStructure", 0, true);
}

template<>
void Serialiser::Serialise(const char *name, VkDeviceQueueCreateInfo &el)
{
	ScopedContext scope(this, this, name, "VkDeviceQueueCreateInfo", 0, true);

	Serialise("queueFamilyIndex", el.queueFamilyIndex);
	Serialise("queueCount", el.queueCount);
}

template<>
void Serialiser::Serialise(const char *name, VkDeviceCreateInfo &el)
{
	ScopedContext scope(this, this, name, "VkDeviceCreateInfo", 0, true);
	
	RDCASSERT(m_Mode < WRITING || el.sType == VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO);
	SerialiseNext(this, el.pNext);

	Serialise("flags", el.flags);

	Serialise("queueRecordCount", el.queueRecordCount);
	
	if(m_Mode == READING)
	{
		if(el.queueRecordCount == 0)
			el.pRequestedQueues = NULL;
		else
			el.pRequestedQueues = new VkDeviceQueueCreateInfo[el.queueRecordCount];
	}
	
	// cast away const on array so we can assign to it on reading
	VkDeviceQueueCreateInfo* queues = (VkDeviceQueueCreateInfo*)el.pRequestedQueues;
	for(uint32_t i=0; i < el.queueRecordCount; i++)
		Serialise("RequestedQueues", queues[i]);

	Serialise("extensionCount", el.extensionCount);
	
	if(m_Mode == READING)
	{
		if(el.extensionCount == 0)
			el.ppEnabledExtensionNames = NULL;
		else
			el.ppEnabledExtensionNames = new char*[el.extensionCount];
	}
	
	// cast away const on array so we can assign to it on reading
	const char **exts = (const char **)el.ppEnabledExtensionNames;
	for(uint32_t i=0; i < el.extensionCount; i++)
	{
		string s = "";
		if(m_Mode == WRITING && exts[i] != NULL)
			s = exts[i];

		Serialise("ppEnabledExtensionNames", s);

		if(m_Mode == READING)
		{
			m_StringDB.insert(s);
			exts[i] = m_StringDB.find(s)->c_str();
		}
	}
}

template<>
void Serialiser::Serialise(const char *name, VkBufferCreateInfo &el)
{
	ScopedContext scope(this, this, name, "VkBufferCreateInfo", 0, true);

	RDCASSERT(m_Mode < WRITING || el.sType == VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO);
	SerialiseNext(this, el.pNext);

	Serialise("size", el.size);
	Serialise("usage", el.usage);
	Serialise("flags", el.flags);
}

template<>
void Serialiser::Serialise(const char *name, VkBufferViewCreateInfo &el)
{
	ScopedContext scope(this, this, name, "VkBufferViewCreateInfo", 0, true);

	RDCASSERT(m_Mode < WRITING || el.sType == VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO);
	SerialiseNext(this, el.pNext);

	SerialiseObject(VkBuffer, "buffer", el.buffer);
	Serialise("viewType", el.viewType);
	Serialise("format", el.format);
	Serialise("offset", el.offset);
	Serialise("range", el.range);
}

template<>
void Serialiser::Serialise(const char *name, VkImageCreateInfo &el)
{
	ScopedContext scope(this, this, name, "VkImageCreateInfo", 0, true);
	
	RDCASSERT(m_Mode < WRITING || el.sType == VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO);
	SerialiseNext(this, el.pNext);
	
	Serialise("imageType", el.imageType);
	Serialise("format", el.format);
	Serialise("extent", el.extent);
	Serialise("mipLevels", el.mipLevels);
	Serialise("arraySize", el.arraySize);
	Serialise("samples", el.samples);
	Serialise("tiling", el.tiling);
	Serialise("usage", el.usage);
	Serialise("flags", el.flags);
}

template<>
void Serialiser::Serialise(const char *name, VkImageViewCreateInfo &el)
{
	ScopedContext scope(this, this, name, "VkImageViewCreateInfo", 0, true);
	
	RDCASSERT(m_Mode < WRITING || el.sType == VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO);
	SerialiseNext(this, el.pNext);
	
	SerialiseObject(VkImage, "image", el.image);
	Serialise("viewType", el.viewType);
	Serialise("format", el.format);
	Serialise("channels", el.channels);
	Serialise("subresourceRange", el.subresourceRange);
}

template<>
void Serialiser::Serialise(const char *name, VkFramebufferCreateInfo &el)
{
	ScopedContext scope(this, this, name, "VkFramebufferCreateInfo", 0, true);
	
	RDCASSERT(m_Mode < WRITING || el.sType == VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO);
	SerialiseNext(this, el.pNext);
	
	SerialiseObject(VkRenderPass, "renderPass", el.renderPass);
	Serialise("attachmentCount", el.attachmentCount);
	size_t sz = el.attachmentCount;
	Serialise("pAttachments", (VkAttachmentBindInfo *&)el.pAttachments, sz);
	Serialise("width", el.width);
	Serialise("height", el.height);
	Serialise("layers", el.layers);
}

template<>
void Serialiser::Serialise(const char *name, VkRenderPassCreateInfo &el)
{
	ScopedContext scope(this, this, name, "VkRenderPassCreateInfo", 0, true);
	
	RDCASSERT(m_Mode < WRITING || el.sType == VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO);
	SerialiseNext(this, el.pNext);

	size_t sz = 0;
	
	Serialise("attachmentCount", el.attachmentCount);
	sz = el.attachmentCount;
	Serialise("pAttachments", (VkAttachmentDescription *&)el.pAttachments, sz);
	
	Serialise("subpassCount", el.subpassCount);
	sz = el.subpassCount;
	Serialise("pSubpasses", (VkSubpassDescription *&)el.pSubpasses, sz);
	
	Serialise("dependencyCount", el.dependencyCount);
	sz = el.dependencyCount;
	Serialise("pDependencies", (VkSubpassDependency *&)el.pDependencies, sz);
}

template<>
void Serialiser::Serialise(const char *name, VkRenderPassBeginInfo &el)
{
	ScopedContext scope(this, this, name, "VkRenderPassBeginInfo", 0, true);
	
	RDCASSERT(m_Mode < WRITING || el.sType == VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO);
	SerialiseNext(this, el.pNext);
	
	SerialiseObject(VkRenderPass, "renderPass", el.renderPass);
	SerialiseObject(VkFramebuffer, "framebuffer", el.framebuffer);
	Serialise("renderArea", el.renderArea);
	Serialise("attachmentCount", el.attachmentCount);
	size_t sz = el.attachmentCount;
	Serialise("pAttachmentClearValues", (VkClearValue *&)el.pAttachmentClearValues, sz);
}

template<>
void Serialiser::Serialise(const char *name, VkAttachmentViewCreateInfo &el)
{
	ScopedContext scope(this, this, name, "VkAttachmentViewCreateInfo", 0, true);

	RDCASSERT(m_Mode < WRITING || el.sType == VK_STRUCTURE_TYPE_ATTACHMENT_VIEW_CREATE_INFO);
	SerialiseNext(this, el.pNext);

	SerialiseObject(VkImage, "image", el.image);
	Serialise("format", el.format);
	Serialise("mipLevel", el.mipLevel);
	Serialise("baseArraySlice", el.baseArraySlice);
	Serialise("arraySize", el.arraySize);
}

template<>
void Serialiser::Serialise(const char *name, VkDynamicViewportStateCreateInfo &el)
{
	ScopedContext scope(this, this, name, "VkDynamicVpStateCreateInfo", 0, true);
	
	RDCASSERT(m_Mode < WRITING || el.sType == VK_STRUCTURE_TYPE_DYNAMIC_VIEWPORT_STATE_CREATE_INFO);
	SerialiseNext(this, el.pNext);

	Serialise("viewportCount", el.viewportAndScissorCount);
	size_t sz = el.viewportAndScissorCount;
	Serialise("viewports", (VkViewport *&)el.pViewports, sz);
	Serialise("scissors", (VkRect2D *&)el.pScissors, sz);
}

template<>
void Serialiser::Serialise(const char *name, VkDynamicRasterStateCreateInfo &el)
{
	ScopedContext scope(this, this, name, "VkDynamicRsStateCreateInfo", 0, true);
	
	RDCASSERT(m_Mode < WRITING || el.sType == VK_STRUCTURE_TYPE_DYNAMIC_RASTER_STATE_CREATE_INFO);
	SerialiseNext(this, el.pNext);

	Serialise("depthBias", el.depthBias);
	Serialise("depthBiasClamp", el.depthBiasClamp);
	Serialise("slopeScaledDepthBias", el.slopeScaledDepthBias);
	Serialise("lineWidth", el.lineWidth);
}

template<>
void Serialiser::Serialise(const char *name, VkDynamicColorBlendStateCreateInfo &el)
{
	ScopedContext scope(this, this, name, "VkDynamicCbStateCreateInfo", 0, true);
	
	RDCASSERT(m_Mode < WRITING || el.sType == VK_STRUCTURE_TYPE_DYNAMIC_COLOR_BLEND_STATE_CREATE_INFO);
	SerialiseNext(this, el.pNext);

	Serialise<4>("blendConst", el.blendConst);
}

template<>
void Serialiser::Serialise(const char *name, VkDynamicDepthStencilStateCreateInfo &el)
{
	ScopedContext scope(this, this, name, "VkDynamicDsStateCreateInfo", 0, true);
	
	RDCASSERT(m_Mode < WRITING || el.sType == VK_STRUCTURE_TYPE_DYNAMIC_DEPTH_STENCIL_STATE_CREATE_INFO);
	SerialiseNext(this, el.pNext);
	
	Serialise("minDepthBounds", el.minDepthBounds);
	Serialise("maxDepthBounds", el.maxDepthBounds);
	Serialise("stencilReadMask", el.stencilReadMask);
	Serialise("stencilWriteMask", el.stencilWriteMask);
	Serialise("stencilFrontRef", el.stencilFrontRef);
	Serialise("stencilBackRef", el.stencilBackRef);
}

template<>
void Serialiser::Serialise(const char *name, VkVertexInputBindingDescription &el)
{
	ScopedContext scope(this, this, name, "VkVertexInputBindingDescription", 0, true);
	
	Serialise("binding", el.binding);
	Serialise("strideInBytes", el.strideInBytes);
	Serialise("stepRate", el.stepRate);
}

template<>
void Serialiser::Serialise(const char *name, VkVertexInputAttributeDescription &el)
{
	ScopedContext scope(this, this, name, "VkVertexInputAttributeDescription", 0, true);
	
	Serialise("location", el.location);
	Serialise("binding", el.binding);
	Serialise("format", el.format);
	Serialise("offsetInBytes", el.offsetInBytes);
}

template<>
void Serialiser::Serialise(const char *name, VkPipelineVertexInputStateCreateInfo &el)
{
	ScopedContext scope(this, this, name, "VkPipelineVertexInputStateCreateInfo", 0, true);
	
	RDCASSERT(m_Mode < WRITING || el.sType == VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO);
	SerialiseNext(this, el.pNext);
	
	Serialise("bindingCount", el.bindingCount);
	
	if(m_Mode == READING)
	{
		if(el.bindingCount == 0)
			el.pVertexBindingDescriptions = NULL;
		else
			el.pVertexBindingDescriptions = new VkVertexInputBindingDescription[el.bindingCount];
	}
	
	// cast away const on array so we can assign to it on reading
	VkVertexInputBindingDescription* binds = (VkVertexInputBindingDescription*)el.pVertexBindingDescriptions;
	for(uint32_t i=0; i < el.bindingCount; i++)
		Serialise("bindings", binds[i]);
	
	Serialise("attributeCount", el.attributeCount);
	
	if(m_Mode == READING)
	{
		if(el.attributeCount == 0)
			el.pVertexAttributeDescriptions = NULL;
		else
			el.pVertexAttributeDescriptions = new VkVertexInputAttributeDescription[el.attributeCount];
	}
	
	// cast away const on array so we can assign to it on reading
	VkVertexInputAttributeDescription* attrs = (VkVertexInputAttributeDescription*)el.pVertexAttributeDescriptions;
	for(uint32_t i=0; i < el.attributeCount; i++)
		Serialise("attributes", attrs[i]);
}

template<>
void Serialiser::Serialise(const char *name, VkPipelineInputAssemblyStateCreateInfo &el)
{
	ScopedContext scope(this, this, name, "VkPipelineInputAssemblyStateCreateInfo", 0, true);
	
	RDCASSERT(m_Mode < WRITING || el.sType == VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO);
	SerialiseNext(this, el.pNext);
	
	Serialise("topology", el.topology);
	Serialise("primitiveRestartEnable", el.primitiveRestartEnable);
}

template<>
void Serialiser::Serialise(const char *name, VkPipelineTessellationStateCreateInfo &el)
{
	ScopedContext scope(this, this, name, "VkPipelineTessStateCreateInfo", 0, true);
	
	RDCASSERT(m_Mode < WRITING || el.sType == VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO);
	SerialiseNext(this, el.pNext);
	
	Serialise("patchControlPoints", el.patchControlPoints);
}

template<>
void Serialiser::Serialise(const char *name, VkPipelineViewportStateCreateInfo &el)
{
	ScopedContext scope(this, this, name, "VkPipelineViewportStateCreateInfo", 0, true);
	
	RDCASSERT(m_Mode < WRITING || el.sType == VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO);
	SerialiseNext(this, el.pNext);
	
	Serialise("viewportCount", el.viewportCount);
}

template<>
void Serialiser::Serialise(const char *name, VkPipelineRasterStateCreateInfo &el)
{
	ScopedContext scope(this, this, name, "VkPipelineRasterStateCreateInfo", 0, true);
	
	RDCASSERT(m_Mode < WRITING || el.sType == VK_STRUCTURE_TYPE_PIPELINE_RASTER_STATE_CREATE_INFO);
	SerialiseNext(this, el.pNext);
	
	Serialise("depthClipEnable", el.depthClipEnable);
	Serialise("rasterizerDiscardEnable", el.rasterizerDiscardEnable);
	Serialise("fillMode", el.fillMode);
	Serialise("cullMode", el.cullMode);
	Serialise("frontFace", el.frontFace);
}

template<>
void Serialiser::Serialise(const char *name, VkPipelineMultisampleStateCreateInfo &el)
{
	ScopedContext scope(this, this, name, "VkPipelineMultisampleStateCreateInfo", 0, true);
	
	RDCASSERT(m_Mode < WRITING || el.sType == VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO);
	SerialiseNext(this, el.pNext);

	Serialise("rasterSamples", el.rasterSamples);
	Serialise("sampleShadingEnable", el.sampleShadingEnable);
	Serialise("minSampleShading", el.minSampleShading);
	Serialise("sampleMask", el.sampleMask);
}

template<>
void Serialiser::Serialise(const char *name, VkPipelineColorBlendAttachmentState &el)
{
	ScopedContext scope(this, this, name, "VkPipelineColorBlendAttachmentState", 0, true);

	Serialise("blendEnable", el.blendEnable);
	Serialise("srcBlendColor", el.srcBlendColor);
	Serialise("destBlendColor", el.destBlendColor);
	Serialise("blendOpColor", el.blendOpColor);
	Serialise("srcBlendAlpha", el.srcBlendAlpha);
	Serialise("destBlendAlpha", el.destBlendAlpha);
	Serialise("blendOpAlpha", el.blendOpAlpha);
	Serialise("channelWriteMask", el.channelWriteMask);
}

template<>
void Serialiser::Serialise(const char *name, VkPipelineColorBlendStateCreateInfo &el)
{
	ScopedContext scope(this, this, name, "VkPipelineColorBlendStateCreateInfo", 0, true);
	
	RDCASSERT(m_Mode < WRITING || el.sType == VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO);
	SerialiseNext(this, el.pNext);

	Serialise("alphaToCoverageEnable", el.alphaToCoverageEnable);
	Serialise("logicOpEnable", el.logicOpEnable);
	Serialise("logicOp", el.logicOp);

	Serialise("attachmentCount", el.attachmentCount);
	
	if(m_Mode == READING)
	{
		if(el.attachmentCount == 0)
			el.pAttachments = NULL;
		else
			el.pAttachments = new VkPipelineColorBlendAttachmentState[el.attachmentCount];
	}
	
	// cast away const on array so we can assign to it on reading
	VkPipelineColorBlendAttachmentState* atts = (VkPipelineColorBlendAttachmentState*)el.pAttachments;
	for(uint32_t i=0; i < el.attachmentCount; i++)
		Serialise("attachments", atts[i]);
}

template<>
void Serialiser::Serialise(const char *name, VkPipelineDepthStencilStateCreateInfo &el)
{
	ScopedContext scope(this, this, name, "VkPipelineDepthStencilStateCreateInfo", 0, true);
	
	RDCASSERT(m_Mode < WRITING || el.sType == VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO);
	SerialiseNext(this, el.pNext);

	Serialise("depthTestEnable", el.depthTestEnable);
	Serialise("depthWriteEnable", el.depthWriteEnable);
	Serialise("depthCompareOp", el.depthCompareOp);
	Serialise("depthBoundsEnable", el.depthBoundsEnable);
	Serialise("stencilEnable", el.stencilTestEnable);
	Serialise("front", el.front);
	Serialise("back", el.back);
}

template<>
void Serialiser::Serialise(const char *name, VkClearColorValue &el)
{
	ScopedContext scope(this, this, name, "VkClearColorValue", 0, true);
	
	Serialise<4>("u32", el.u32);
}

template<>
void Serialiser::Serialise(const char *name, VkCmdBufferCreateInfo &el)
{
	ScopedContext scope(this, this, name, "VkCmdBufferCreateInfo", 0, true);

	RDCASSERT(m_Mode < WRITING || el.sType == VK_STRUCTURE_TYPE_CMD_BUFFER_CREATE_INFO);
	SerialiseNext(this, el.pNext);
	
	SerialiseObject(VkCmdPool, "cmdPool", el.cmdPool);
	Serialise("level", el.level);
	Serialise("flags", el.flags);
}

template<>
void Serialiser::Serialise(const char *name, VkCmdBufferBeginInfo &el)
{
	ScopedContext scope(this, this, name, "VkCmdBufferBeginInfo", 0, true);

	RDCASSERT(m_Mode < WRITING || el.sType == VK_STRUCTURE_TYPE_CMD_BUFFER_BEGIN_INFO);
	SerialiseNext(this, el.pNext);
	
	Serialise("flags", el.flags);
	SerialiseObject(VkRenderPass, "renderPass", el.renderPass);
	SerialiseObject(VkFramebuffer, "framebuffer", el.framebuffer);
}

template<>
void Serialiser::Serialise(const char *name, VkStencilOpState &el)
{
	ScopedContext scope(this, this, name, "VkStencilOpState", 0, true);
	
	Serialise("stencilFailOp", el.stencilFailOp);
	Serialise("stencilPassOp", el.stencilPassOp);
	Serialise("stencilDepthFailOp", el.stencilDepthFailOp);
	Serialise("stencilCompareOp", el.stencilCompareOp);
}

template<>
void Serialiser::Serialise(const char *name, VkSamplerCreateInfo &el)
{
	ScopedContext scope(this, this, name, "VkSamplerCreateInfo", 0, true);

	RDCASSERT(m_Mode < WRITING || el.sType == VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO);
	SerialiseNext(this, el.pNext);

	Serialise("minFilter", el.minFilter);
	Serialise("magFilter", el.magFilter);
	Serialise("mipMode", el.mipMode);
	Serialise("addressU", el.addressU);
	Serialise("addressV", el.addressV);
	Serialise("addressW", el.addressW);
	Serialise("mipLodBias", el.mipLodBias);
	Serialise("maxAnisotropy", el.maxAnisotropy);
	Serialise("compareOp", el.compareOp);
	Serialise("minLod", el.minLod);
	Serialise("maxLod", el.maxLod);
	Serialise("borderColor", el.borderColor);
}

template<>
void Serialiser::Serialise(const char *name, VkPipelineShaderStageCreateInfo &el)
{
	ScopedContext scope(this, this, name, "VkPipelineShaderStageCreateInfo", 0, true);

	RDCASSERT(m_Mode < WRITING || el.sType == VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO);
	SerialiseNext(this, el.pNext);

	Serialise("stage", el.stage);
	SerialiseObject(VkShader, "shader", el.shader);
	// VKTODO: pSpecializationInfo
}

template<>
void Serialiser::Serialise(const char *name, VkPipelineCacheCreateInfo &el)
{
	ScopedContext scope(this, this, name, "VkPipelineCacheCreateInfo", 0, true);

	RDCASSERT(m_Mode < WRITING || el.sType == VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO);
	SerialiseNext(this, el.pNext);

	uint64_t initialSize = el.initialSize;
	Serialise("codeSize", initialSize);
	el.initialSize = initialSize;

	size_t sz = initialSize;
	SerialiseBuffer("initialData", (byte *&)el.initialData, sz);

	uint64_t maxSize = el.maxSize;
	Serialise("maxSize", maxSize);
	el.maxSize = maxSize;
}

template<>
void Serialiser::Serialise(const char *name, VkPipelineLayoutCreateInfo &el)
{
	ScopedContext scope(this, this, name, "VkPipelineLayoutCreateInfo", 0, true);

	RDCASSERT(m_Mode < WRITING || el.sType == VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO);
	SerialiseNext(this, el.pNext);

	Serialise("descriptorSetCount", el.descriptorSetCount);

	if(m_Mode == READING)
	{
		if(el.descriptorSetCount == 0)
			el.pSetLayouts = NULL;
		else
			el.pSetLayouts = new VkDescriptorSetLayout[el.descriptorSetCount];
	}

	// cast away const on array so we can assign to it on reading
	VkDescriptorSetLayout* layouts = (VkDescriptorSetLayout*)el.pSetLayouts;
	for(uint32_t i=0; i < el.descriptorSetCount; i++)
	{
		SerialiseObject(VkDescriptorSetLayout, "layout", layouts[i]);
	}

	Serialise("pushConstantRangeCount", el.pushConstantRangeCount);

	if(m_Mode == READING)
	{
		if(el.pushConstantRangeCount == 0)
			el.pPushConstantRanges = NULL;
		else
			el.pPushConstantRanges = new VkPushConstantRange[el.pushConstantRangeCount];
	}

	// cast away const on array so we can assign to it on reading
	VkPushConstantRange* push = (VkPushConstantRange*)el.pPushConstantRanges;
	for(uint32_t i=0; i < el.pushConstantRangeCount; i++)
		Serialise("pushConstantRanges", push[i]);
}

template<>
void Serialiser::Serialise(const char *name, VkShaderModuleCreateInfo &el)
{
	ScopedContext scope(this, this, name, "VkShaderModuleCreateInfo", 0, true);

	RDCASSERT(m_Mode < WRITING || el.sType == VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO);
	SerialiseNext(this, el.pNext);

	uint64_t codeSize = el.codeSize;
	Serialise("codeSize", codeSize);
	el.codeSize = codeSize;

	size_t sz = codeSize;
	SerialiseBuffer("pCode", (byte *&)el.pCode, sz);
	Serialise("flags", el.flags);
}

template<>
void Serialiser::Serialise(const char *name, VkShaderCreateInfo &el)
{
	ScopedContext scope(this, this, name, "VkShaderCreateInfo", 0, true);

	RDCASSERT(m_Mode < WRITING || el.sType == VK_STRUCTURE_TYPE_SHADER_CREATE_INFO);
	SerialiseNext(this, el.pNext);

	string s = "";
	if(m_Mode >= WRITING && el.pName != NULL)
		s = el.pName;

	Serialise("pName", s);

	if(m_Mode == READING)
	{
		if(s == "")
		{
			el.pName = "";
		}
		else
		{
			string str = (char *)m_BufferHead-s.length();
			m_StringDB.insert(str);
			el.pName = m_StringDB.find(str)->c_str();
		}
	}

	Serialise("flags", el.flags);
	SerialiseObject(VkShaderModule, "module", el.module);
}

template<>
void Serialiser::Serialise(const char *name, VkImageSubresourceRange &el)
{
	ScopedContext scope(this, this, name, "VkImageSubresourceRange", 0, true);

	Serialise("aspect", el.aspect);
	Serialise("baseMipLevel", el.baseMipLevel);
	Serialise("mipLevels", el.mipLevels);
	Serialise("baseArraySlice", el.baseArraySlice);
	Serialise("arraySize", el.arraySize);
}

template<>
void Serialiser::Serialise(const char *name, VkImageSubresource &el)
{
	ScopedContext scope(this, this, name, "VkImageSubresource", 0, true);
	
	Serialise("aspect", el.aspect);
	Serialise("mipLevel", el.mipLevel);
	Serialise("arraySlice", el.arraySlice);
}

template<>
void Serialiser::Serialise(const char *name, VkMemoryAllocInfo &el)
{
	ScopedContext scope(this, this, name, "VkMemoryAllocInfo", 0, true);
	
	RDCASSERT(m_Mode < WRITING || el.sType == VK_STRUCTURE_TYPE_MEMORY_ALLOC_INFO);
	SerialiseNext(this, el.pNext);

	Serialise("allocationSize", el.allocationSize);
	Serialise("memoryTypeIndex", el.memoryTypeIndex);
}

template<>
void Serialiser::Serialise(const char *name, VkMemoryBarrier &el)
{
	ScopedContext scope(this, this, name, "VkMemoryBarrier", 0, true);
	
	RDCASSERT(m_Mode < WRITING || el.sType == VK_STRUCTURE_TYPE_MEMORY_BARRIER);
	SerialiseNext(this, el.pNext);

	Serialise("outputMask", el.outputMask);
	Serialise("inputMask", el.inputMask);
}

template<>
void Serialiser::Serialise(const char *name, VkBufferMemoryBarrier &el)
{
	ScopedContext scope(this, this, name, "VkBufferMemoryBarrier", 0, true);
	
	RDCASSERT(m_Mode < WRITING || el.sType == VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER);
	SerialiseNext(this, el.pNext);

	Serialise("outputMask", el.outputMask);
	Serialise("inputMask", el.inputMask);
	Serialise("srcQueueFamilyIndex", el.srcQueueFamilyIndex);
	Serialise("destQueueFamilyIndex", el.destQueueFamilyIndex);
	SerialiseObject(VkBuffer, "buffer", el.buffer);
	Serialise("offset", el.offset);
	Serialise("size", el.size);
}

template<>
void Serialiser::Serialise(const char *name, VkImageMemoryBarrier &el)
{
	ScopedContext scope(this, this, name, "VkImageMemoryBarrier", 0, true);
	
	RDCASSERT(m_Mode < WRITING || el.sType == VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER);
	SerialiseNext(this, el.pNext);

	Serialise("outputMask", el.outputMask);
	Serialise("inputMask", el.inputMask);
	Serialise("oldLayout", el.oldLayout);
	Serialise("newLayout", el.newLayout);
	Serialise("srcQueueFamilyIndex", el.srcQueueFamilyIndex);
	Serialise("destQueueFamilyIndex", el.destQueueFamilyIndex);
	SerialiseObject(VkImage, "image", el.image);
	Serialise("subresourceRange", el.subresourceRange);
}

template<>
void Serialiser::Serialise(const char *name, VkGraphicsPipelineCreateInfo &el)
{
	ScopedContext scope(this, this, name, "VkGraphicsPipelineCreateInfo", 0, true);
	
	RDCASSERT(m_Mode < WRITING || el.sType == VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO);
	SerialiseNext(this, el.pNext);

	Serialise("flags", el.flags);
	SerialiseObject(VkPipelineLayout, "layout", el.layout);
	SerialiseObject(VkRenderPass, "renderPass", el.renderPass);
	Serialise("subpass", el.subpass);
	SerialiseObject(VkPipeline, "basePipelineHandle", el.basePipelineHandle);
	Serialise("basePipelineIndex", el.basePipelineIndex);

	SerialiseOptionalObject(this, "pVertexInputState", (VkPipelineVertexInputStateCreateInfo *&)el.pVertexInputState);
	SerialiseOptionalObject(this, "pInputAssemblyState", (VkPipelineInputAssemblyStateCreateInfo *&)el.pInputAssemblyState);
	SerialiseOptionalObject(this, "pTessellationState", (VkPipelineTessellationStateCreateInfo *&)el.pTessellationState);
	SerialiseOptionalObject(this, "pViewportState", (VkPipelineViewportStateCreateInfo *&)el.pViewportState);
	SerialiseOptionalObject(this, "pRasterState", (VkPipelineRasterStateCreateInfo *&)el.pRasterState);
	SerialiseOptionalObject(this, "pMultisampleState", (VkPipelineMultisampleStateCreateInfo *&)el.pMultisampleState);
	SerialiseOptionalObject(this, "pDepthStencilState", (VkPipelineDepthStencilStateCreateInfo *&)el.pDepthStencilState);
	SerialiseOptionalObject(this, "pColorBlendState", (VkPipelineColorBlendStateCreateInfo *&)el.pColorBlendState);

	Serialise("stageCount", el.stageCount);
	if(m_Mode == READING)
		el.pStages = new VkPipelineShaderStageCreateInfo[el.stageCount];

	for(uint32_t i=0; i < el.stageCount; i++)
		Serialise("stage", (VkPipelineShaderStageCreateInfo &)el.pStages[i]);
}

template<>
void Serialiser::Serialise(const char *name, VkComputePipelineCreateInfo &el)
{
	ScopedContext scope(this, this, name, "VkComputePipelineCreateInfo", 0, true);

	RDCASSERT(m_Mode < WRITING || el.sType == VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO);
	SerialiseNext(this, el.pNext);

	Serialise("cs", el.cs);
	Serialise("flags", el.flags);
	SerialiseObject(VkPipelineLayout, "layout", el.layout);
	SerialiseObject(VkPipeline, "basePipelineHandle", el.basePipelineHandle);
	Serialise("basePipelineIndex", el.basePipelineIndex);
}

template<>
void Serialiser::Serialise(const char *name, VkDescriptorTypeCount &el)
{
	ScopedContext scope(this, this, name, "VkDescriptorTypeCount", 0, true);

	Serialise("type", el.type);
	Serialise("count", el.count);
}

template<>
void Serialiser::Serialise(const char *name, VkDescriptorPoolCreateInfo &el)
{
	ScopedContext scope(this, this, name, "VkDescriptorPoolCreateInfo", 0, true);

	RDCASSERT(m_Mode < WRITING || el.sType == VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO);
	SerialiseNext(this, el.pNext);

	Serialise("count", el.count);

	if(m_Mode == READING)
		el.pTypeCount = new VkDescriptorTypeCount[el.count];

	for(uint32_t i=0; el.count; i++)
		Serialise("pTypeCount", (VkDescriptorTypeCount &)el.pTypeCount[i]);
}

template<>
void Serialiser::Serialise(const char *name, VkDescriptorInfo &el)
{
	ScopedContext scope(this, this, name, "VkDescriptorInfo", 0, true);
	
	SerialiseObject(VkBufferView, "bufferView", el.bufferView);
	SerialiseObject(VkSampler, "sampler", el.sampler);
	SerialiseObject(VkImageView, "imageView", el.imageView);
	SerialiseObject(VkAttachmentView, "attachmentView", el.attachmentView);
	Serialise("imageLayout", el.imageLayout);
}

template<>
void Serialiser::Serialise(const char *name, VkWriteDescriptorSet &el)
{
	ScopedContext scope(this, this, name, "VkWriteDescriptorSet", 0, true);

	RDCASSERT(m_Mode < WRITING || el.sType == VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET);
	SerialiseNext(this, el.pNext);
	
	SerialiseObject(VkDescriptorSet, "destSet", el.destSet);
	Serialise("destBinding", el.destBinding);
	Serialise("destArrayElement", el.destArrayElement);
	Serialise("descriptorType", el.descriptorType);

	Serialise("count", el.count);

	if(m_Mode == READING)
		el.pDescriptors = new VkDescriptorInfo[el.count];

	for(uint32_t i=0; el.count; i++)
		Serialise("pDescriptors", (VkDescriptorInfo &)el.pDescriptors[i]);
}

template<>
void Serialiser::Serialise(const char *name, VkCopyDescriptorSet &el)
{
	ScopedContext scope(this, this, name, "VkCopyDescriptorSet", 0, true);

	RDCASSERT(m_Mode < WRITING || el.sType == VK_STRUCTURE_TYPE_COPY_DESCRIPTOR_SET);
	SerialiseNext(this, el.pNext);
	
	SerialiseObject(VkDescriptorSet, "srcSet", el.srcSet);
	Serialise("srcBinding", el.srcBinding);
	Serialise("srcArrayElement", el.srcArrayElement);
	SerialiseObject(VkDescriptorSet, "destSet", el.destSet);
	Serialise("destBinding", el.destBinding);
	Serialise("destArrayElement", el.destArrayElement);

	Serialise("count", el.count);
}

template<>
void Serialiser::Serialise(const char *name, VkPushConstantRange &el)
{
	ScopedContext scope(this, this, name, "VkPushConstantRange", 0, true);

	Serialise("stageFlags", el.stageFlags);
	Serialise("start", el.start);
	Serialise("length", el.length);
}

template<>
void Serialiser::Serialise(const char *name, VkDescriptorSetLayoutBinding &el)
{
	ScopedContext scope(this, this, name, "VkDescriptorSetLayoutBinding", 0, true);

	Serialise("descriptorType", el.descriptorType);
	Serialise("arraySize", el.arraySize);
	Serialise("stageFlags", el.stageFlags);

	bool hasSamplers = el.pImmutableSamplers != NULL;
	Serialise("hasSamplers", hasSamplers);

	if(hasSamplers && m_Mode == READING)
		el.pImmutableSamplers = new VkSampler[el.arraySize];

	VkSampler *samplers = (VkSampler *)el.pImmutableSamplers;

	for(uint32_t i=0; hasSamplers && i < el.arraySize; i++)
	{
		SerialiseObject(VkSampler, "pImmutableSampler", samplers[i]);
	}
}

template<>
void Serialiser::Serialise(const char *name, VkDescriptorSetLayoutCreateInfo &el)
{
	ScopedContext scope(this, this, name, "VkDescriptorSetLayoutCreateInfo", 0, true);

	Serialise("count", el.count);

	if(m_Mode == READING)
		el.pBinding = new VkDescriptorSetLayoutBinding[el.count];

	for(uint32_t i=0; el.count; i++)
		Serialise("pBinding", (VkDescriptorSetLayoutBinding &)el.pBinding[i]);
}

template<>
void Serialiser::Serialise(const char *name, VkChannelMapping &el)
{
	ScopedContext scope(this, this, name, "VkChannelMapping", 0, true);

	Serialise("r", el.r);
	Serialise("g", el.g);
	Serialise("b", el.b);
	Serialise("a", el.a);
}

template<>
void Serialiser::Serialise(const char *name, VkBufferImageCopy &el)
{
	ScopedContext scope(this, this, name, "VkBufferImageCopy", 0, true);
	
	Serialise("memOffset", el.bufferOffset);
	Serialise("imageSubresource", el.imageSubresource);
	Serialise("imageOffset", el.imageOffset);
	Serialise("imageExtent", el.imageExtent);
}

template<>
void Serialiser::Serialise(const char *name, VkBufferCopy &el)
{
	ScopedContext scope(this, this, name, "VkBufferCopy", 0, true);
	
	Serialise("srcOffset", el.srcOffset);
	Serialise("destOffset", el.destOffset);
	Serialise("copySize", el.copySize);
}

template<>
void Serialiser::Serialise(const char *name, VkImageCopy &el)
{
	ScopedContext scope(this, this, name, "VkImageCopy", 0, true);

	Serialise("srcSubresource", el.srcSubresource);
	Serialise("srcOffset", el.srcOffset);
	Serialise("destSubresource", el.destSubresource);
	Serialise("destOffset", el.destOffset);
	Serialise("extent", el.extent);
}

template<>
void Serialiser::Serialise(const char *name, VkImageBlit &el)
{
	ScopedContext scope(this, this, name, "VkImageBlit", 0, true);

	Serialise("srcSubresource", el.srcSubresource);
	Serialise("srcOffset", el.srcOffset);
	Serialise("srcExtent", el.srcExtent);
	Serialise("destSubresource", el.destSubresource);
	Serialise("destOffset", el.destOffset);
	Serialise("destExtent", el.destExtent);
}

template<>
void Serialiser::Serialise(const char *name, VkImageResolve &el)
{
	ScopedContext scope(this, this, name, "VkImageResolve", 0, true);
	
	Serialise("srcSubresource", el.srcSubresource);
	Serialise("srcOffset", el.srcOffset);
	Serialise("destSubresource", el.destSubresource);
	Serialise("destOffset", el.destOffset);
	Serialise("extent", el.extent);
}

template<>
void Serialiser::Serialise(const char *name, VkRect2D &el)
{
	ScopedContext scope(this, this, name, "VkRect2D", 0, true);

	Serialise("offset", el.offset);
	Serialise("extent", el.extent);
}

template<>
void Serialiser::Serialise(const char *name, VkRect3D &el)
{
	ScopedContext scope(this, this, name, "VkRect3D", 0, true);

	Serialise("offset", el.offset);
	Serialise("extent", el.extent);
}

template<>
void Serialiser::Serialise(const char *name, VkSwapChainCreateInfoWSI &el)
{
	ScopedContext scope(this, this, name, "VkSwapChainCreateInfoWSI", 0, true);
	
	RDCASSERT(m_Mode < WRITING || el.sType == VK_STRUCTURE_TYPE_SWAP_CHAIN_CREATE_INFO_WSI);
	SerialiseNext(this, el.pNext);
	
	// VKTODO: don't need any of the info in here, I think
	//Serialise("pSurfaceDescription", *el.pSurfaceDescription);
	
	Serialise("minImageCount", el.minImageCount);
	Serialise("imageFormat", el.imageFormat);
	Serialise("imageExtent", el.imageExtent);
	Serialise("imageUsageFlags", el.imageUsageFlags);
	Serialise("preTransform", el.preTransform);
	Serialise("imageArraySize", el.imageArraySize);
	
	Serialise("presentMode", el.presentMode);

	// VKTODO: don't think we need the old swap chain
	//Serialise("oldSwapChain", el.oldSwapChain);

	Serialise("clipped", el.clipped);
}
