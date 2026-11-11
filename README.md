# mc vulkan rewrite (vmc)
Using everything ive learnt in recent experiments, im rewriting mc from scratch with a focus on distant horizons level scale (LODs), maximum performance with Vulkan, and better abstractions from the start.

I have also created a base for a c++26 based toml serializer and deserializer, which will let me change, load and save values on the fly. Main repo is [here](https://github.com/llewellynmeldrum/cpp26-refl-experiments).


# Building
Non-vendored requirements are: 
- Vulkan 1.4+ (and obviously an arch+gpu+OS that will run it. Most dev/testing is done on an M1 macbook pro using MoltenVK.)
- SDL3 




# learning vulkan
<img width="400" alt="square_vk" src="https://github.com/user-attachments/assets/53c3042d-3524-46f6-9a5f-b68a951bc5ca" />


In order to update my minecraft clone to vulkan, im going to take some time to learn it separately and make some test apps first. My plan:


- [ ] Complete however many chapters of [vkguide.dev](https://vkguide.dev/) feels necessary
- [ ] Do some basic 2d rendering
- [ ] Remake a simple board game (Petteia probably)
- [ ] do some basic 3d rendering
- [ ] ...
- [ ] hw accelerated raytracing ????



## vkguide chapter progress:
- [X] chapter 1 ([8b6a13](https://github.com/llewellynmeldrum/vkguide-follow-along/commit/c01598bb0b15dc9d3b2fbc5d85b50c3fc18b6a13))
- [X] chapter 2 ([92a4d0](https://github.com/llewellynmeldrum/vkguide-follow-along/commit/ddb8b6b257b15fce535af95938cc9d54c492a4d0)) 

Switched over to the vulkan spec tutorial [docs.vulkan.org/tutorial](https://docs.vulkan.org/tutorial/latest/)

