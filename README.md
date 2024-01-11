# 基于Caffe ResNet-50网络实现图片分类（同步推理）

## 功能描述

该样例主要是基于Caffe ResNet-50网络（单输入、单Batch）实现图片分类的功能，同时支持对离线模型文件的异或加密和解密处理。

在该样例中：

1.  先使用样例提供的脚本transferPic.py，将2张\*.jpg图片都转换为\*.bin格式，同时将图片从1024\*683的分辨率缩放为224\*224。
2.  加载离线模型om文件，对2张图片进行同步推理，分别得到推理结果，再对推理结果进行处理，输出top5置信度的类别标识。

    在加载离线模型前，提前将Caffe ResNet-50网络的模型文件转换为适配昇腾AI处理器的离线模型om文件。

3.  对离线模型的权重部分异或加密，生成加密的离线模型om文件。

4.  加载加密的om文件，对权重部分进行解密，得到解密的om文件，并加载解密后的模型文件，重复推理过程，得到和2中相同的结果。

## 目录结构

样例代码结构如下所示。

```
├── .project     //工程信息文件，包含工程类型、工程描述、运行目标设备类型等
├── CMakeLists.txt    //编译脚本，调用src目录下的CMakeLists文件
├── caffe_model
│   ├── resnet50.caffemodel		//测试数据,需要按指导获取原始模型权重，放到caffe_model目录下
│   └── resnet50.prototxt		//测试数据,需要按指导获取原始模型文件，放到caffe_model目录下

├── data
│   ├── dog1_1024_683.jpg		//测试数据,需要按指导获取测试图片，放到data目录下
│   └── dog2_1024_683.jpg		//测试数据,需要按指导获取测试图片，放到data目录下

├── inc							// 头文件集合
│   ├── acl_modified_api.h		//声明修改后的acl接口
│   ├── common
│   │   ├── ...
│   ├── external
│   │   ├── ...
│   ├── framework
│   │   ├── ...
│   ├── ge
│   │   ├── ...
│   ├── google
│   │   └── ...
│   ├── graph
│   │   ├── ...
│   ├── mmpa
│   │   ├── ...
│   ├── model_process.h		//声明模型处理相关函数的头文件
│   ├── platform
│   │   ├── ...
│   ├── proto
│   │   ├── ...
│   ├── register
│   │   ├── ...
│   ├── runtime
│   │   ├── ...
│   ├── sample_process.h		//声明资源初始化/销毁相关函数的头文件
│   ├── toolchain
│   │   ├── ...
│   └── utils.h		//声明公共函数（例如：文件读取函数）的头文件

├── model		// 离线模型文件集合，包括使用atc生成的om文件、运行样例后生成的加密和解密的om文件
│   ├── ...

├── script
│   └── transferPic.py		//将*.jpg转换为*.bin，同时将图片从1024*683的分辨率缩放为224*224

└── src
    ├── CMakeLists.txt
    ├── acl.json		//系统初始化的配置文件
    ├── acl_modified_api.cpp		// 修改后的acl接口实现
    ├── main.cpp		//主函数，图片分类功能的实现文件
    ├── model_process.cpp		//模型处理相关函数的实现文件
    ├── sample_process.cpp		//资源初始化/销毁相关函数的实现文件
    └── utils.cpp		//公共函数（例如：文件读取函数）的实现文件
```

## 环境要求

已在以下环境中测试：

-   操作系统及架构：EulerOS aarch64
-   编译器： g++
-   芯片：Ascend910ProB
-   python及依赖的库：python3.7.5、Pillow库
-   已在环境上部署昇腾AI软件栈，并配置对应的的环境变量，请参见[Link](https://www.hiascend.com/document)中对应版本的CANN安装指南，测试环境中CANN版本为CANN商用版 6.3.RC2。
    
    以下步骤中，开发环境指编译开发代码的环境，运行环境指运行算子、推理或训练等程序的环境，运行环境上必须带昇腾AI处理器。开发环境和运行环境可以合设在同一台服务器上，也可以分设，分设场景下，开发环境下编译出来的可执行文件，在运行环境下执行时，若开发环境和运行环境上的操作系统架构不同，则需要在开发环境中执行交叉编译。


## 准备模型和图片

1.  以运行用户登录开发环境。

2.  下载sample仓代码并上传至环境后，请先进入“resnet50_imagenet_classification”样例目录。
    
    请注意，下文中的样例目录均指“resnet50_imagenet_classification”目录。

3.  准备ResNet-50模型。
    1.  获取ResNet-50原始模型。

        您可以从以下链接中获取ResNet-50网络的模型文件（\*.prototxt）、权重文件（\*.caffemodel），并以运行用户将获取的文件上传至开发环境的“样例目录/caffe\_model“目录下。如果目录不存在，需要自行创建。

        -   ResNet-50网络的模型文件（\*.prototxt）：单击[Link](https://obs-9be7.obs.cn-east-2.myhuaweicloud.com/003_Atc_Models/AE/ATC%20Model/resnet50/resnet50.prototxt)下载该文件。
        -   ResNet-50网络的权重文件（\*.caffemodel）：单击[Link](https://obs-9be7.obs.cn-east-2.myhuaweicloud.com/003_Atc_Models/AE/ATC%20Model/resnet50/resnet50.caffemodel)下载该文件。 

    2.  将ResNet-50原始模型转换为适配昇腾AI处理器的离线模型（\*.om文件）。

        切换到样例目录，执行如下命令(以昇腾910ProB处理器为例)：

        ```
        atc --model=caffe_model/resnet50.prototxt --weight=caffe_model/resnet50.caffemodel --framework=0 --output=model/resnet50 --soc_version=Ascend910ProB --input_format=NCHW --input_fp16_nodes=data --output_type=FP32 --out_nodes=prob:0
        ```

        -   --model：原始模型文件路径。
        -   --weight：权重文件路径。
        -   --framework：原始框架类型。0：表示Caffe；1：表示MindSpore；3：表示TensorFlow；5：表示ONNX。
        -   --soc\_version：昇腾AI处理器的版本。进入“CANN软件安装目录/compiler/data/platform_config”目录，".ini"文件的文件名即为昇腾AI处理器的版本，请根据实际情况选择。

        -   --input\_format：输入数据的Format。
        -   --input\_fp16\_nodes：指定输入数据类型为FP16的输入节点名称。
        -   --output\_type和--out\_nodes：这2个参数配合使用，指定prob节点的第一个输出的数据类型为float32。
        -   --output：生成的resnet50.om文件存放在“样例目录/model“目录下。建议使用命令中的默认设置，否则在编译代码前，您还需要修改代码文件中的模型路径相关参数值。

4.  准备测试图片。
    1.  请从以下链接获取该样例的输入图片，并以运行用户将获取的文件上传至开发环境的“样例目录/data“目录下。如果目录不存在，需自行创建。

        [https://obs-9be7.obs.cn-east-2.myhuaweicloud.com/models/aclsample/dog1\_1024\_683.jpg](https://obs-9be7.obs.cn-east-2.myhuaweicloud.com/models/aclsample/dog1_1024_683.jpg)

        [https://obs-9be7.obs.cn-east-2.myhuaweicloud.com/models/aclsample/dog2\_1024\_683.jpg](https://obs-9be7.obs.cn-east-2.myhuaweicloud.com/models/aclsample/dog2_1024_683.jpg)

    2.  切换到“样例目录/data“目录下，执行transferPic.py脚本，将\*.jpg转换为\*.bin，同时将图片从1024\*683的分辨率缩放为224\*224。在“样例目录/data“目录下生成2个\*.bin文件。

        ```
        python3 ../script/transferPic.py
        ```

        如果执行脚本报错“ModuleNotFoundError: No module named 'PIL'”，则表示缺少Pillow库，请使用**pip3 install Pillow --user**命令安装Pillow库。


## 编译运行
1.  配置CANN基础环境变量和Python环境变量。

2.  编译代码。
    1.  以运行用户登录开发环境。

    2.  请先进入“resnet50_imagenet_classification”样例目录。

        请注意，下文中的样例目录均指“resnet50_imagenet_classification”目录。

    3. 设置环境变量，配置程序编译依赖的头文件与库文件路径。
    
       设置以下环境变量后，编译脚本会根据“{DDK_PATH}环境变量值/usr/local/Ascend/ascend-toolkit/latest/”目录查找编译依赖的头文件，根据{NPU_HOST_LIB}环境变量指向的目录查找编译依赖的库文件。请视情况替换为“Ascend-cann-toolkit”包的实际安装路径。

       ```
       export DDK_PATH=/usr/local/Ascend/ascend-toolkit/latest/
       export NPU_HOST_LIB=$DDK_PATH/runtime/lib64/
    
    4.  切换到样例目录，创建目录用于存放编译文件，例如，本文中，创建的目录为“build/intermediates/host“。
    
        ```
        mkdir -p build/intermediates/host
        ```
    
    5.  切换到“build/intermediates/host“目录，执行如下命令生成编译文件。
    
        “../../../src“表示CMakeLists.txt文件所在的目录，请根据实际目录层级修改。
    
        将DCMAKE\_SKIP\_RPATH设置为TRUE，代表不会将rpath信息（即NPU_HOST_LIB配置的路径）添加到编译生成的可执行文件中去，可执行文件运行时会自动搜索实际设置的LD_LIBRARY_PATH中的动态链接库。
    
        执行如下命令编译。
    
        ```
        cd build/intermediates/host
        cmake ../../../src -DCMAKE_CXX_COMPILER=g++ -DCMAKE_SKIP_RPATH=TRUE
        ```
    
    6.  执行如下命令，生成的可执行文件main在“样例目录/out“目录下。
    
        ```
        make
        ```


3.  运行应用。

    切换到可执行文件main所在的目录，例如“resnet50_imagenet_classification/out”，运行可执行文件。

    ```
    ./main
    ```

    执行成功后，在屏幕上的关键提示信息示例如下，提示信息中的index表示类别标识、value表示该分类的最大置信度，这些值可能会根据版本、环境有所不同，请以实际情况为准：
    
    ```
    [INFO]  acl init success
    [INFO]  set device 0 success
    [INFO]  create context success
    [INFO]  create stream success
    [INFO]  get run mode success
    [INFO]  load model ../model/resnet50.om success
    [INFO]  create model description success
    [INFO]  create model input success
    [INFO]  create model output success
    [INFO]  start to process file:../data/dog1_1024_683.bin
    [INFO]  model execute success
    [INFO]  top 1: index[161] value[0.764648]
    [INFO]  top 2: index[162] value[0.156616]
    [INFO]  top 3: index[167] value[0.038971]
    [INFO]  top 4: index[163] value[0.021698]
    [INFO]  top 5: index[166] value[0.011887]
    [INFO]  output data success
    [INFO]  start to process file:../data/dog2_1024_683.bin
    [INFO]  model execute success
    [INFO]  top 1: index[267] value[0.935059]
    [INFO]  top 2: index[266] value[0.041412]
    [INFO]  top 3: index[265] value[0.019104]
    [INFO]  top 4: index[219] value[0.002884]
    [INFO]  top 5: index[160] value[0.000311]
    [INFO]  output data success
    [INFO]  destroy model input success
    [INFO]  destroy model output success
    [INFO]  unload model success, modelId is 1
    [INFO]  destroy model description success
    [INFO]  execute sample success
    
    ------------------------------------
    [INFO]  now encrypt sample and save...
    [INFO]  load model ../model/resnet50.om success
    [INFO]  modelWeight.size: 51180576
    [INFO]  Print ModelWeight Data Here
    []
    [INFO]  unload model success, modelId is 2
    [INFO]  destroy model description success
    
    ------------------------------------
    [INFO]  now execute encrypted sample...
    [INFO]  load model ../model/resnet50_encrypted.om success
    [INFO]  modelWeight.size: 51180576
    [INFO]  Print ModelWeight Data Here
    []
    [INFO]  unload model success, modelId is 3
    
    ------------------------------------
    [INFO]  already saved decrypted om file
    [INFO]  load decrypted model ../model/resnet50_decrypted.om success
    [INFO]  create model description success
    [INFO]  create model input success
    [INFO]  create model output success
    [INFO]  start to process file:../data/dog1_1024_683.bin
    [INFO]  model has not been encrypted, no need to decrypt
    [INFO]  model execute success
    [INFO]  top 1: index[161] value[0.764648]
    [INFO]  top 2: index[162] value[0.156616]
    [INFO]  top 3: index[167] value[0.038971]
    [INFO]  top 4: index[163] value[0.021698]
    [INFO]  top 5: index[166] value[0.011887]
    [INFO]  output data success
    [INFO]  start to process file:../data/dog2_1024_683.bin
    [INFO]  model has not been encrypted, no need to decrypt
    [INFO]  model execute success
    [INFO]  top 1: index[267] value[0.935059]
    [INFO]  top 2: index[266] value[0.041412]
    [INFO]  top 3: index[265] value[0.019104]
    [INFO]  top 4: index[219] value[0.002884]
    [INFO]  top 5: index[160] value[0.000311]
    [INFO]  output data success
    [INFO]  destroy model input success
    [INFO]  destroy model output success
    [INFO]  unload model success, modelId is 4
    [INFO]  destroy model description success
    [INFO]  execute encrypted sample success
    [INFO]  end to destroy stream
    [INFO]  end to destroy context
    [INFO]  end to reset device 0
    [INFO]  end to finalize acl
    ```
    
    >**说明：** 
    >类别标签和类别的对应关系与训练模型时使用的数据集有关，本样例使用的模型是基于imagenet数据集进行训练的，您可以在互联网上查阅imagenet数据集的标签及类别的对应关系，例如，可单击[Link](https://blog.csdn.net/weixin_44676081/article/details/106755135)查看。
    >当前屏显信息中的类别标识与类别的对应关系如下：
    >"161": \["basset", "basset hound"\]、
    >"267": \["standard poodle"\]。


## 关键接口介绍

在该Sample中，涉及的关键功能点及其关键接口，如下所示：

-   **初始化**
    
    -   调用aclInit接口初始化AscendCL配置。
    -   调用aclFinalize接口实现AscendCL去初始化。
    
-   **Device管理**
    
    -   调用aclrtSetDevice接口指定用于运算的Device。
    -   调用aclrtGetRunMode接口获取昇腾AI软件栈的运行模式，根据运行模式的不同，内部处理流程不同。
    -   调用aclrtResetDevice接口复位当前运算的Device，回收Device上的资源。
    
-   **Context管理**
    -   调用aclrtCreateContext接口创建Context。
    -   调用aclrtDestroyContext接口销毁Context。

-   **Stream管理**
    -   调用aclrtCreateStream接口创建Stream。
    -   调用aclrtDestroyStream接口销毁Stream。

-   **内存管理**
    
    -   调用aclrtMalloc接口申请Device上的内存。
    -   调用aclrtFree接口释放Device上的内存。
    
-   **数据传输**

    调用aclrtMemcpy接口通过内存复制的方式实现数据传输。

-   **模型推理**
    
    -   调用aclmdlLoadFromFileWithMemModified或aclmdlLoadFromFileModified接口从\*.om文件加载模型。
    -   调用aclmdlExecute接口执行模型推理，同步接口。
    -   调用aclmdlUnload接口卸载模型。
    
-   **数据后处理**

    提供样例代码，处理模型推理的结果，直接在终端上显示top5置信度的类别编号。

## 问题整理补充


1. 依赖问题

    - 头文件

        本项目中所依赖头文件均存放在`./inc`目录下

        由于头文件依赖关系较为复杂，并且最新的`ge`代码并非开源，推荐直接使用本项目目录下构建完成的部分

        如需自行构建请注意以下几点

        > 1. 优先使用在服务器本地环境下寻找到的同名头文件
        >
        > 2. 服务器本地找不到的情况下，推荐使用Github与Gitee上Ge仓库下的头文件，但是由于Cann版本与Ge代码版本难以确认对应关系，请尽可能选择与服务器Cann版本相近的Ge代码
        >
        > 3. ./inc/google/protobuf目录下的头文件为自行生成，使用中若出现问题推荐自行编译，推荐从opensdk中获取protoc，路径为
        >
        >     Ascend/ascend-toolkit/latest/opensdk/opensdk/protoc/bin/protoc
        >
        >     > 注：opensdk只在部分社区版中才有，如本项目采用的是:
        >     >
        >     > https://www.hiascend.com/developer/download/community/result?cann=6.3.RC2.alpha003
        >
        > 4. 可能存在部分头文件中函数声明与so文件可导出符号表中的不匹配，需依据实际情况修改

    - so文件

        动态链接库路径可在`./src`下的`Cmakelists.txt`中修改

        目前需要的包括`libge_common_base.so`, `libge_executor.so`, `libascendcl.so`

        so文件所在路径根据运行环境调整

2. 重写或修改`Cmakelists.txt`后编译时，若出现"undefined reference"问题

    尝试关闭编译时c++11特性

    ```cmake
    add_compile_options(-D_GLIBCXX_USE_CXX11_ABI=0)
    ```

    原因在于so库编译时均未开启c++11特性，若开启则会出现符号不匹配（实际环境下建议自行验证）