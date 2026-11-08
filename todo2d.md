todo: 

1. Create a secondary Renderer2D object, which accepts a command buffer and writes onto it?
What exactly does a renderer2D need to know in order to batch and execute draws?
Firstly, lets consider this in terms of the api.
Id like to be able to write:
```cpp

rend2d
    .push_outline_color(make_rgb(255,255,255))
    .push_fill_color(make_rgb(0,0,0));
rend2d.begin_recording(frame.commandBuffer);

rend2d.push_recorded_commands();
rend2d.submit();

```

