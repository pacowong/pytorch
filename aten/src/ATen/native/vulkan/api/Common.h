#pragma once

#include <ATen/ATen.h>

#ifdef USE_VULKAN_WRAPPER
#include <vulkan_wrapper.h>
#else
#include <vulkan/vulkan.h>
#endif

#define VK_CHECK(function)                                  \
  {                                                         \
    const VkResult result = (function);                     \
    TORCH_CHECK(result == VK_SUCCESS, "VkResult:", result); \
  }

#define VK_DELETER(Handle) at::native::vulkan::api::destroy_##Handle
#define VK_DELETER_DISPATCHABLE_DECLARE(Handle) void destroy_##Handle(const Vk##Handle handle)
#define VK_DELETER_NON_DISPATCHABLE_DECLARE(Handle)   \
  class destroy_##Handle final {                      \
   public:                                            \
    explicit destroy_##Handle(const VkDevice device); \
    void operator()(const Vk##Handle handle) const;   \
   private:                                           \
    VkDevice device_;                                 \
  };

namespace at {
namespace native {
namespace vulkan {
namespace api {

VK_DELETER_DISPATCHABLE_DECLARE(Instance);
VK_DELETER_DISPATCHABLE_DECLARE(Device);
VK_DELETER_NON_DISPATCHABLE_DECLARE(Semaphore);
VK_DELETER_NON_DISPATCHABLE_DECLARE(Fence);
VK_DELETER_NON_DISPATCHABLE_DECLARE(Buffer);
VK_DELETER_NON_DISPATCHABLE_DECLARE(Image);
VK_DELETER_NON_DISPATCHABLE_DECLARE(Event);
VK_DELETER_NON_DISPATCHABLE_DECLARE(BufferView);
VK_DELETER_NON_DISPATCHABLE_DECLARE(ImageView);
VK_DELETER_NON_DISPATCHABLE_DECLARE(ShaderModule);
VK_DELETER_NON_DISPATCHABLE_DECLARE(PipelineCache);
VK_DELETER_NON_DISPATCHABLE_DECLARE(PipelineLayout);
VK_DELETER_NON_DISPATCHABLE_DECLARE(Pipeline);
VK_DELETER_NON_DISPATCHABLE_DECLARE(DescriptorSetLayout);
VK_DELETER_NON_DISPATCHABLE_DECLARE(Sampler);
VK_DELETER_NON_DISPATCHABLE_DECLARE(DescriptorPool);
VK_DELETER_NON_DISPATCHABLE_DECLARE(CommandPool);

// Vulkan objects are referenced via handles.  The spec defines Vulkan handles
// under two categories: dispatchable and non-dispatchable.  Dispatchable handles
// are required to be strongly typed as a result of being pointers to unique
// opaque types.  Since dispatchable handles are pointers at the heart,
// std::unique_ptr can be used to manage their lifetime with a custom deleter.
// Non-dispatchable handles on the other hand, are not required to have strong
// types, and even though they default to the same implementation as dispatchable
// handles on some platforms - making the use of std::unique_ptr possible - they
// are only required by the spec to weakly aliases 64-bit integers which is the
// implementation some platforms default to.  This makes the use of std::unique_ptr
// difficult since semantically unique_ptrs store pointers to their payload
// which is also what is passed onto the custom deleters.

template<typename Type, typename Deleter>
class Handle final {
 public:
  inline Handle(const Type payload, Deleter&& deleter)
    : payload_(payload), deleter_(std::move(deleter)) {}
  Handle(const Handle&) = delete;
  Handle& operator=(const Handle&) = delete;
  Handle(Handle&&) = default;
  Handle& operator=(Handle&&) = default;
  inline ~Handle() {
    if (payload_) {
      deleter_(payload_);
    }
  }

  inline Type get() const {
    return payload_;
  }

 private:
  Type payload_;
  Deleter deleter_;
};

} // namespace api
} // namespace vulkan
} // namespace native
} // namespace at
