# style guide
some quick definitions
|word|meaning|
|-|-|
|Parameter|The variable defined in a functions declaration/definition.|
|Argument|The specific instance of a parameter in some calling context|


## Core requirements
- ALWAYS use east const.
- STRONGLY prefer `auto` for local variables. Use `static_cast<intended_type>()` if necessary. 
(sometimes, especially for primitive types, it might be reasonable to not use auto. But try to never leave anything unitiazlied.)


## Sidenotes:
How to deal with implicit conversions or unwanted return types,
```cpp
auto vec = std::vector<bool>{true, false, true, false};
auto val = vec.at(2) ;
static_assert(std::same_as<decltype(val),bool>); 
// Static assertion failed: std::vector<bool>::at() returns proxy class 'std::vector<bool>::reference'
// The fix:

auto vec = std::vector<bool>{true, false, true, false};
auto val = static_cast<bool>(vec.at(2));
static_assert(std::same_as<decltype(val),bool>); 
// Success.



```

## Function declarations/definitions
Function declarations should follow:
```cpp
/*
// 0 arguments
[attributes] 
auto FUNCTION_NAME() [cv-qualifiers] [ref-qualifiers] -> RETURN_TYPE

// >= 1 args:
[attributes] 
auto FUNCTION_NAME(
    ARG1_T arg1,
    ARG2_T arg2, 
) [cv-qualifiers] [ref-qualifiers] -> RETURN_TYPE

*/
// Example A: 0 param function
[[nodiscard]] 
auto get_framebuffer_size() const noexcept -> vk::Extent2D;

// Example C: 3 param function
auto prepare_render_attachments(
    vk::raii::CommandBuffer const&  cmdBuf,
    u32                             imageIndex, 
    std::array<f32, 4>              clearColor
) const noexcept -> RenderAttachmentContext;

```

