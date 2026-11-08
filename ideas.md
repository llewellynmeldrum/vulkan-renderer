# idea:
A custom vector library based on/using glm as a backend. Might provide types like:
```cpp
namespace vec{

struct xy{
    union{
        struct {
            T x;
            T y;
        };
        std::vector<T,2> all;
    };
};
struct xz{
    union{
        struct {
            T x;
            T z;
        };
        std::vector<T,2> all;
    };
};
};
    vec::xy pos_2d; // has pos_2d.x, pos_2d.y, and pos_2d.all (an array)
    vec::xyz pos_3d;
```
