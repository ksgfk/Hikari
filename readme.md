# Hikari

学习OpenGL

C++17标准

## Dependent 依赖

* [glfw](https://github.com/glfw/glfw) 实在搞不定Native窗口，以及怎么创建OpenGL上下文...（照着抄都抄歪来

* [glad](https://github.com/Dav1dde/glad) OpenGL函数加载，自己写一遍太麻烦了（

* [stb_image](https://github.com/nothings/stb) 不了解图片格式，还是借助其他库吧

* [tinyobjloader](https://github.com/tinyobjloader/tinyobjloader) 写一个完备的.obj加载有亿点麻烦（

* [glslang](https://github.com/KhronosGroup/glslang) 用来预处理include指令，没其它用

* [SPIRV-Cross](https://github.com/KhronosGroup/SPIRV-Cross) 将glslang生成的spir-v字节码编译回glsl

## Structure 结构

* src和include是封装OpenGL的源码（lib HikariCommon）

* ext是第三方依赖

* app是练习的源码

* scene存放app运行需要的资源

## Compile and Run 编译运行

CMake工程，没有花里胡哨的配置，按一般编译步骤来就行（

Windows下，Visual Studio 2019（MSVC v142）编译运行通过

Linux Ubuntu下，Clang10编译运行通过，GCC没试（懒

**注意：** glfw在linux下创建窗口用X11，依赖一些包：`libx11-dev`、`libxrandr-dev`、`libxinerama-dev`、`libxcursor-dev`、`libxi-dev`，需要提前安装好

如果使用在linux使用clang编译还需要安装`libc++-dev`和`libc++abi-dev`

**注意：**有些app必须输入资源文件的根目录，例如`app.exe -A assetPath`。路径有误的话，可能会直接crash（

src/application.cpp文件里，`GetArgs`函数里列出了所有支持的命令行参数

## Show 结果展示
### 4.Shadow Map

<img src="show/4-shadow_map.png" alt="4-shadow_map.png">

**注意：** 需要加载资源

非常简单的shadow map，截图看上去锯齿还好？高分辨率深度图出奇迹（红圈圈会自己转动

### 3.Blinn Phong

<img src="show/3-blinn.png" alt="3-blinn.png">

directional light，超级简单的blinn phong高光模型。场景同样可以查看四周（操作看2.Sky box

### 2.Sky box

<img src="show/2-skybox.png" alt="2-skybox.png">

**注意：** 需要加载资源

天空盒和法线可视化，可以在窗口中按住鼠标左键查看四周，右键可以改变视角，滑动鼠标滚轮可以缩放

### 1.Simple Triangle

<img src="show/1-triangle.png" alt="1-triangle.png">

（triangle...?这tm不是矩形吗

### 0.Hello world

没啥好展示的，就一空白窗口（