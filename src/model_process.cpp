/**
* @file model_process.cpp
*
* Copyright (C) 2020. Huawei Technologies Co., Ltd. All rights reserved.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*/
#include "model_process.h"
#include "framework/common/ge_types.h"
#include "framework/common/types.h"
#include "common/auth/file_saver.h"
#include "acl_modified_api.h"
#include <iostream>
#include <map>
#include <sstream>
#include <algorithm>
#include <functional>
#include "utils.h"
#include <string.h>
// #include <dlfcn.h>

using namespace std;
extern bool g_isDevice;


struct ModelPartitionModified {
    ge::ModelPartitionType type;
    uint8_t *data = nullptr;
    uint64_t size = 0UL;
};

ModelProcess::ModelProcess() :modelId_(0), modelWorkSize_(0), modelWeightSize_(0), modelWorkPtr_(nullptr),
    modelWeightPtr_(nullptr), loadFlag_(false), encryptFlag_(false), modelDesc_(nullptr), input_(nullptr), output_(nullptr)
{
}

ModelProcess::~ModelProcess()
{
    UnloadModel();
    DestroyModelDesc();
    DestroyInput();
    DestroyOutput();
}

Result ModelProcess::LoadModel(const char *modelPath)
{
    if (loadFlag_) {
        ERROR_LOG("model has already been loaded");
        return FAILED;
    }
    aclError ret = aclmdlQuerySize(modelPath, &modelWorkSize_, &modelWeightSize_);
    if (ret != ACL_SUCCESS) {
        ERROR_LOG("query model failed, model file is %s, errorCode is %d",
            modelPath, static_cast<int32_t>(ret));
        return FAILED;
    }
    // using ACL_MEM_MALLOC_HUGE_FIRST to malloc memory, huge memory is preferred to use
    // and huge memory can improve performance.
    ret = aclrtMalloc(&modelWorkPtr_, modelWorkSize_, ACL_MEM_MALLOC_HUGE_FIRST);
    if (ret != ACL_SUCCESS) {
        ERROR_LOG("malloc buffer for work failed, require size is %zu, errorCode is %d",
            modelWorkSize_, static_cast<int32_t>(ret));
        return FAILED;
    }

    // using ACL_MEM_MALLOC_HUGE_FIRST to malloc memory, huge memory is preferred to use
    // and huge memory can improve performance.
    ret = aclrtMalloc(&modelWeightPtr_, modelWeightSize_, ACL_MEM_MALLOC_HUGE_FIRST);
    if (ret != ACL_SUCCESS) {
        ERROR_LOG("malloc buffer for weight failed, require size is %zu, errorCode is %d",
            modelWeightSize_, static_cast<int32_t>(ret));
        return FAILED;
    }

    ret = aclmdlLoadFromFileWithMem(modelPath, &modelId_, modelWorkPtr_,
        modelWorkSize_, modelWeightPtr_, modelWeightSize_);
    if (ret != ACL_SUCCESS) {
        ERROR_LOG("load model from file failed, model file is %s, errorCode is %d",
            modelPath, static_cast<int32_t>(ret));
        return FAILED;
    }

    // // 以下是由系统管理内存时，从文件加载模型的代码，现改由用户管理内存并使用能够从内存中获取模型数据的API，故注释掉以下代码
    // if (aclmdlLoadFromFile(modelPath, &modelId_) != ACL_SUCCESS) {
    //     ERROR_LOG("load model from file failed, model file is %s, errorCode is %d", modelPath, static_cast<int32_t>(ret));
    //     return FAILED;
    // }

    loadFlag_ = true;
    INFO_LOG("load model %s success", modelPath);
    return SUCCESS;
}

Result ModelProcess::LoadEncryptedModel(const char *modelPath)
{
    if (loadFlag_) {
        ERROR_LOG("model has already been loaded");
        return FAILED;
    }

    aclError ret;
    ge::ModelData data;
    // 以下是由系统管理内存时，从文件加载模型的代码
    if (aclmdlLoadFromFileModified(modelPath, &modelId_, &data) != ACL_SUCCESS) {
        ERROR_LOG("Modified load model from file failed, model file is %s", modelPath);
        return FAILED;
    }

    loadFlag_ = true;
    INFO_LOG("load model %s success", modelPath);

    // 由ModelData生成OmFileLoadHelper对象
    ge::OmFileLoadHelper omFileLoadHelper = ge::OmFileLoadHelper();
    if(omFileLoadHelper.Init(data) != SUCCESS) {
        ERROR_LOG("Init OmFileLoadHelper failed");
        return FAILED;
    }

    // 获得权重部分的数据
    ModelPartitionModified modelWeight;
    if(omFileLoadHelper.GetModelPartition(ge::ModelPartitionType::WEIGHTS_DATA, reinterpret_cast<ge::ModelPartition&>(modelWeight)) != SUCCESS) {
        ERROR_LOG("GetModelPartition failed");
        return FAILED;
    }

    // 修改权重部分[解密操作]
    for (int i = 0; i < modelWeight.size; ++i) {
        modelWeight.data[i] = modelWeight.data[i] ^ 0x5A;
    }

    // 打印权重部分(这里只打印0x1000个字节)
    INFO_LOG("modelWeight.size: %d", modelWeight.size);
    INFO_LOG("Print ModelWeight Data Here");
    printf("[");
    for (int i = 0; i < 0x1000; ++i) {
        // printf("%x, ", modelWeight.data[i]);
    }
    printf("]\n");
    auto modelHeader = reinterpret_cast<ge::ModelFileHeader *>(data.model_data);
    modelHeader->is_encrypt = 0;
    uint8_t *partitionTableOffset = static_cast<uint8_t *>(data.model_data) + sizeof(ge::ModelFileHeader);
    auto partitionTable = reinterpret_cast<ge::ModelPartitionTable *>(partitionTableOffset);
    std::vector<ge::ModelPartition> partitionDatas = omFileLoadHelper.context_.partition_datas_;
    const char *modelDecrypted = "../model/resnet50_decrypted.om";
    const std::string outputFileDecrypted(modelDecrypted);
    if(ge::FileSaver::SaveToFile(outputFileDecrypted, data.model_data, (unsigned long)data.model_len) != SUCCESS) {
        ERROR_LOG("SaveToFile failed");
        return FAILED;
    }

    UnloadModel();

    printf("\n------------------------------------\n");
    INFO_LOG("already saved decrypted om file");

    if (aclmdlLoadFromFile(modelDecrypted, &modelId_) != ACL_SUCCESS) {
        ERROR_LOG("Modified load model from file failed, model file is %s", modelPath);
        return FAILED;
    }
    encryptFlag_ = false;
    loadFlag_ = true;
    INFO_LOG("load decrypted model %s success", modelDecrypted);
    return SUCCESS;
}

Result ModelProcess::CreateModelDesc()
{
    modelDesc_ = aclmdlCreateDesc();
    if (modelDesc_ == nullptr) {
        ERROR_LOG("create model description failed");
        return FAILED;
    }

    aclError ret = aclmdlGetDesc(modelDesc_, modelId_);
    if (ret != ACL_SUCCESS) {
        ERROR_LOG("get model description failed, modelId is %u, errorCode is %d",
            modelId_, static_cast<int32_t>(ret));
        return FAILED;
    }

    INFO_LOG("create model description success");

    return SUCCESS;
}

void ModelProcess::DestroyModelDesc()
{
    if (modelDesc_ != nullptr) {
        (void)aclmdlDestroyDesc(modelDesc_);
        modelDesc_ = nullptr;
    }
    INFO_LOG("destroy model description success");
}

Result ModelProcess::GetInputSizeByIndex(const size_t index, size_t &inputSize)
{
    if (modelDesc_ == nullptr) {
        ERROR_LOG("no model description, create input failed");
        return FAILED;
    }
    inputSize = aclmdlGetInputSizeByIndex(modelDesc_, index);
    return SUCCESS;
}

Result ModelProcess::CreateInput(void *inputDataBuffer, size_t bufferSize)
{
    // om used in this sample has only one input
    if (modelDesc_ == nullptr) {
        ERROR_LOG("no model description, create input failed");
        return FAILED;
    }
    size_t modelInputSize = aclmdlGetInputSizeByIndex(modelDesc_, 0);
    if (bufferSize != modelInputSize) {
        ERROR_LOG("input image size[%zu] is not equal to model input size[%zu]", bufferSize, modelInputSize);
        return FAILED;
    }

    input_ = aclmdlCreateDataset();
    if (input_ == nullptr) {
        ERROR_LOG("can't create dataset, create input failed");
        return FAILED;
    }

    aclDataBuffer *inputData = aclCreateDataBuffer(inputDataBuffer, bufferSize);
    if (inputData == nullptr) {
        ERROR_LOG("can't create data buffer, create input failed");
        return FAILED;
    }

    aclError ret = aclmdlAddDatasetBuffer(input_, inputData);
    if (ret != ACL_SUCCESS) {
        ERROR_LOG("add input dataset buffer failed, errorCode is %d", static_cast<int32_t>(ret));
        (void)aclDestroyDataBuffer(inputData);
        inputData = nullptr;
        return FAILED;
    }
    INFO_LOG("create model input success");

    return SUCCESS;
}

void ModelProcess::DestroyInput()
{
    if (input_ == nullptr) {
        return;
    }

    for (size_t i = 0; i < aclmdlGetDatasetNumBuffers(input_); ++i) {
        aclDataBuffer *dataBuffer = aclmdlGetDatasetBuffer(input_, i);
        (void)aclDestroyDataBuffer(dataBuffer);
    }
    (void)aclmdlDestroyDataset(input_);
    input_ = nullptr;
    INFO_LOG("destroy model input success");
}

Result ModelProcess::CreateOutput()
{
    if (modelDesc_ == nullptr) {
        ERROR_LOG("no model description, create ouput failed");
        return FAILED;
    }

    output_ = aclmdlCreateDataset();
    if (output_ == nullptr) {
        ERROR_LOG("can't create dataset, create output failed");
        return FAILED;
    }

    size_t outputSize = aclmdlGetNumOutputs(modelDesc_);
    for (size_t i = 0; i < outputSize; ++i) {
        size_t modelOutputSize = aclmdlGetOutputSizeByIndex(modelDesc_, i);

        void *outputBuffer = nullptr;
        aclError ret = aclrtMalloc(&outputBuffer, modelOutputSize, ACL_MEM_MALLOC_NORMAL_ONLY);
        if (ret != ACL_SUCCESS) {
            ERROR_LOG("can't malloc buffer, size is %zu, create output failed, errorCode is %d",
                modelOutputSize, static_cast<int32_t>(ret));
            return FAILED;
        }

        aclDataBuffer *outputData = aclCreateDataBuffer(outputBuffer, modelOutputSize);
        if (outputData == nullptr) {
            ERROR_LOG("can't create data buffer, create output failed");
            (void)aclrtFree(outputBuffer);
            return FAILED;
        }

        ret = aclmdlAddDatasetBuffer(output_, outputData);
        if (ret != ACL_SUCCESS) {
            ERROR_LOG("can't add data buffer, create output failed, errorCode is %d",
                static_cast<int32_t>(ret));
            (void)aclrtFree(outputBuffer);
            (void)aclDestroyDataBuffer(outputData);
            return FAILED;
        }
    }

    INFO_LOG("create model output success");

    return SUCCESS;
}

void ModelProcess::DumpModelOutputResult()
{
    stringstream ss;
    size_t outputNum = aclmdlGetDatasetNumBuffers(output_);
    static int executeNum = 0;
    for (size_t i = 0; i < outputNum; ++i) {
        ss << "output" << ++executeNum << "_" << i << ".bin";
        string outputFileName = ss.str();
        FILE *outputFile = fopen(outputFileName.c_str(), "wb");
        if (outputFile != nullptr) {
            // get model output data
            aclDataBuffer *dataBuffer = aclmdlGetDatasetBuffer(output_, i);
            void *data = aclGetDataBufferAddr(dataBuffer);
            uint32_t len = aclGetDataBufferSizeV2(dataBuffer);

            void *outHostData = nullptr;
            aclError ret = ACL_SUCCESS;
            if (!g_isDevice) {
                ret = aclrtMallocHost(&outHostData, len);
                if (ret != ACL_SUCCESS) {
                    ERROR_LOG("aclrtMallocHost failed, malloc len[%u], errorCode[%d]",
                        len, static_cast<int32_t>(ret));
                    fclose(outputFile);
                    return;
                }

                // if app is running in host, need copy model output data from device to host
                ret = aclrtMemcpy(outHostData, len, data, len, ACL_MEMCPY_DEVICE_TO_HOST);
                if (ret != ACL_SUCCESS) {
                    ERROR_LOG("aclrtMemcpy failed, errorCode[%d]", static_cast<int32_t>(ret));
                    (void)aclrtFreeHost(outHostData);
                    fclose(outputFile);
                    return;
                }

                fwrite(outHostData, len, sizeof(char), outputFile);

                ret = aclrtFreeHost(outHostData);
                if (ret != ACL_SUCCESS) {
                    ERROR_LOG("aclrtFreeHost failed, errorCode[%d]", static_cast<int32_t>(ret));
                    fclose(outputFile);
                    return;
                }
            } else {
                // if app is running in host, write model output data into result file
                fwrite(data, len, sizeof(char), outputFile);
            }
            fclose(outputFile);
        } else {
            ERROR_LOG("create output file [%s] failed", outputFileName.c_str());
            return;
        }
    }

    INFO_LOG("dump data success");
    return;
}

void ModelProcess::OutputModelResult()
{
    for (size_t i = 0; i < aclmdlGetDatasetNumBuffers(output_); ++i) {
        // get model output data
        aclDataBuffer* dataBuffer = aclmdlGetDatasetBuffer(output_, i);
        void* data = aclGetDataBufferAddr(dataBuffer);
        uint32_t len = aclGetDataBufferSizeV2(dataBuffer);

        void *outHostData = nullptr;
        aclError ret = ACL_SUCCESS;
        float *outData = nullptr;
        if (!g_isDevice) {
            aclError ret = aclrtMallocHost(&outHostData, len);
            if (ret != ACL_SUCCESS) {
                ERROR_LOG("aclrtMallocHost failed, malloc len[%u], errorCode[%d]",
                    len, static_cast<int32_t>(ret));
                return;
            }

            // if app is running in host, need copy model output data from device to host
            ret = aclrtMemcpy(outHostData, len, data, len, ACL_MEMCPY_DEVICE_TO_HOST);
            if (ret != ACL_SUCCESS) {
                ERROR_LOG("aclrtMemcpy failed, errorCode[%d]", static_cast<int32_t>(ret));
                (void)aclrtFreeHost(outHostData);
                return;
            }

            outData = reinterpret_cast<float*>(outHostData);
        } else {
            outData = reinterpret_cast<float*>(data);
        }
        map<float, unsigned int, greater<float> > resultMap;
        for (unsigned int j = 0; j < len / sizeof(float); ++j) {
            resultMap[*outData] = j;
            outData++;
        }

        int cnt = 0;
        for (auto it = resultMap.begin(); it != resultMap.end(); ++it) {
            // print top 5
            if (++cnt > 5) {
                break;
            }

            INFO_LOG("top %d: index[%d] value[%lf]", cnt, it->second, it->first);
        }
        if (!g_isDevice) {
            ret = aclrtFreeHost(outHostData);
            if (ret != ACL_SUCCESS) {
                ERROR_LOG("aclrtFreeHost failed, errorCode[%d]", static_cast<int32_t>(ret));
                return;
            }
        }
    }

    INFO_LOG("output data success");
    return;
}

void ModelProcess::DestroyOutput()
{
    if (output_ == nullptr) {
        return;
    }

    for (size_t i = 0; i < aclmdlGetDatasetNumBuffers(output_); ++i) {
        aclDataBuffer* dataBuffer = aclmdlGetDatasetBuffer(output_, i);
        void* data = aclGetDataBufferAddr(dataBuffer);
        (void)aclrtFree(data);
        (void)aclDestroyDataBuffer(dataBuffer);
    }

    (void)aclmdlDestroyDataset(output_);
    output_ = nullptr;
    INFO_LOG("destroy model output success");
}

Result ModelProcess::Execute()
{
    aclError ret = aclmdlExecute(modelId_, input_, output_);
    if (ret != ACL_SUCCESS) {
        ERROR_LOG("execute model failed, modelId is %u, errorCode is %d",
            modelId_, static_cast<int32_t>(ret));
        return FAILED;
    }

    INFO_LOG("model execute success");
    return SUCCESS;
}

void ModelProcess::UnloadModel()
{
    if (!loadFlag_) {
        WARN_LOG("no model had been loaded, unload failed");
        return;
    }

    aclError ret = aclmdlUnload(modelId_);
    if (ret != ACL_SUCCESS) {
        ERROR_LOG("unload model failed, modelId is %u, errorCode is %d",
            modelId_, static_cast<int32_t>(ret));
    }

    if (modelDesc_ != nullptr) {
        (void)aclmdlDestroyDesc(modelDesc_);
        modelDesc_ = nullptr;
    }

    if (modelWorkPtr_ != nullptr) {
        (void)aclrtFree(modelWorkPtr_);
        modelWorkPtr_ = nullptr;
        modelWorkSize_ = 0;
    }

    if (modelWeightPtr_ != nullptr) {
        (void)aclrtFree(modelWeightPtr_);
        modelWeightPtr_ = nullptr;
        modelWeightSize_ = 0;
    }

    loadFlag_ = false;
    INFO_LOG("unload model success, modelId is %u", modelId_);
    modelId_ = 0;
}

Result ModelProcess::EncryptModelWeightAndSave(const char *modelPath)
{
    if (loadFlag_) {
        ERROR_LOG("model has already been loaded");
        return FAILED;
    }

    if (encryptFlag_) {
        ERROR_LOG("model has already been encrypted");
        return FAILED;
    }

    aclError ret;
    ge::ModelData data;

    // 以下是原样例中由用户管理内存时，从文件加载模型的代码
    ret = aclmdlQuerySize(modelPath, &modelWorkSize_, &modelWeightSize_);
    if (ret != ACL_SUCCESS) {
        ERROR_LOG("query model failed, model file is %s, errorCode is %d",
            modelPath, static_cast<int32_t>(ret));
        return FAILED;
    }

    // using ACL_MEM_MALLOC_HUGE_FIRST to malloc memory, huge memory is preferred to use
    // and huge memory can improve performance.
    ret = aclrtMalloc(&modelWorkPtr_, modelWorkSize_, ACL_MEM_MALLOC_HUGE_FIRST);
    if (ret != ACL_SUCCESS) {
        ERROR_LOG("malloc buffer for work failed, require size is %zu, errorCode is %d",
            modelWorkSize_, static_cast<int32_t>(ret));
        return FAILED;
    }

    // using ACL_MEM_MALLOC_HUGE_FIRST to malloc memory, huge memory is preferred to use
    // and huge memory can improve performance.
    ret = aclrtMalloc(&modelWeightPtr_, modelWeightSize_, ACL_MEM_MALLOC_HUGE_FIRST);
    if (ret != ACL_SUCCESS) {
        ERROR_LOG("malloc buffer for weight failed, require size is %zu, errorCode is %d",
            modelWeightSize_, static_cast<int32_t>(ret));
        return FAILED;
    }

    ret = aclmdlLoadFromFileWithMemModified(modelPath, &modelId_, modelWorkPtr_,
        modelWorkSize_, modelWeightPtr_, modelWeightSize_, &data);
    if (ret != ACL_SUCCESS) {
        ERROR_LOG("load model from file failed, model file is %s, errorCode is %d",
            modelPath, static_cast<int32_t>(ret));
        return FAILED;
    }

    loadFlag_ = true;
    INFO_LOG("load model %s success", modelPath);

    // 由ModelData生成OmFileLoadHelper对象
    ge::OmFileLoadHelper omFileLoadHelper = ge::OmFileLoadHelper();
    if(omFileLoadHelper.Init(data) != SUCCESS) {
        ERROR_LOG("Init OmFileLoadHelper failed");
        return FAILED;
    }

    // 获得权重部分的数据
    ModelPartitionModified modelWeight;
    if(omFileLoadHelper.GetModelPartition(ge::ModelPartitionType::WEIGHTS_DATA, reinterpret_cast<ge::ModelPartition&>(modelWeight)) != SUCCESS) {
        ERROR_LOG("GetModelPartition failed");
        return FAILED;
    }

    // 打印权重部分(这里只打印0x1000个字节)
    INFO_LOG("modelWeight.size: %d", modelWeight.size);
    INFO_LOG("Print ModelWeight Data Here");
    printf("[");
    for (int i = 0; i < 0x1000; ++i) {
        // printf("%x, ", modelWeight.data[i]);
        // printf("%x, ", ((unsigned char*)modelWeightPtr_)[i]); // 不能直接读Device内存 Segmentation fault (core dumped)
        // ((unsigned char*)modelWeightPtr_)[i] += 1; // 不能直接写Device内存 Segmentation fault (core dumped)
    }
    printf("]\n");

    // 修改权重部分
    for (int i = 0; i < modelWeight.size; ++i) {
        modelWeight.data[i] = modelWeight.data[i] ^ 0x5A;
    }

    /*
        OM File Struction:
    +-------------------+
    |                   |
    | model_file_header |
    |                   |
    +-------------------+
    |                   |
    |  partition_table  | size = sizeof(ModelPartitionTable) + (sizeof(ModelPartitionMemInfo) * static_cast<uint64_t>(table.num)
    |                   |
    +-------------------+
    |                   |
    | partition_data[1] |
    |                   |
    +-------------------+
    |                   |
    |partition_data[...]|
    |                   |
    +-------------------+
    文件头中存储的长度(modelHeader->length)是指除文件头之外的所有数据的长度，即partition_table和partition_data的长度之和
    modelHeader->length + sizeof(ModelFileHeader) = modelHeader->model_len
    */
    // 得到指向内存中om文件头的指针   
    auto modelHeader = reinterpret_cast<ge::ModelFileHeader *>(data.model_data);
    // 修改om文件头，使得其is_encrypt字段为1，表示已加密模型
    modelHeader->is_encrypt = 1;

    // 得到指向内存中partition table的指针
    uint8_t *partitionTableOffset = static_cast<uint8_t *>(data.model_data) + sizeof(ge::ModelFileHeader);
    // 得到ModelPartitionTable类型的指针ModelPartitionTable *
    auto partitionTable = reinterpret_cast<ge::ModelPartitionTable *>(partitionTableOffset);
    // PartitionData的集合
    std::vector<ge::ModelPartition> partitionDatas = omFileLoadHelper.context_.partition_datas_;
    // 保存的加密模型路径
    const std::string outputFileEncrypted("../model/resnet50_encrypted.om");
    // 保存加密模型
    if(ge::FileSaver::SaveToFile(outputFileEncrypted, data.model_data, (unsigned long)data.model_len) != SUCCESS) {
        ERROR_LOG("SaveToFile failed");
        return FAILED;
    }

    return SUCCESS;
}

void delegate() {
    INFO_LOG("delegate");

}

Result ModelProcess::DecryptModelWeight() {
    if(!encryptFlag_) {
        INFO_LOG("model has not been encrypted, no need to decrypt");
        return SUCCESS;
    }
    INFO_LOG("model has been encrypted, now decrypting...");

    // // 尝试直接打印加密模型的权重部分(这里只打印0x1000个字节)
    // printf("[");
    // for(int i = 0; i < 0x1000; ++i) {
    //     printf("%x, ", ((unsigned char*)modelWeightPtr_)[i]);
    // }
    // printf("]\n");
    // // 失败 Segmentation fault (core dumped)

    delegate();

    encryptFlag_ = true;
    INFO_LOG("model has been decrypted");
    return SUCCESS;
}