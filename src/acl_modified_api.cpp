#include <iostream>
#include "common/log_inner.h"
#include "common/error_codes_inner.h"
#include "framework/executor/ge_executor.h"
#include "framework/common/ge_types.h"
#include "framework/common/helper/om_file_helper.h"
#include "toolchain/profiling_manager.h"
#include "toolchain/resource_statistics.h"

static aclError ModelLoadFromFileWithMemModified(const char_t *const modelPath, uint32_t *const modelId, void *const workPtr,
    const size_t workSize, void *const weightPtr, const size_t weightSize, const int32_t priority, ge::ModelData *data)
{
    ge::GeExecutor executor;
    uint32_t id = 0U;
    const std::string path(modelPath);
    data->om_path = path;

    ge::Status ret = executor.LoadDataFromFile(path, *data);
    if (ret != ge::SUCCESS) {
        printf("[Model][FromFile]load model from file[%s] failed, ge result[%u]", modelPath, ret);
        ACL_DELETE_ARRAY(data->model_data);
        return ACL_GET_ERRCODE_GE(static_cast<int32_t>(ret));
    }
    data->priority = priority;
    ret = executor.LoadModelFromData(id, *data, workPtr, workSize, weightPtr, weightSize);
    if (ret != ge::SUCCESS) {
        printf("[Model][FromData]load model from data failed, ge result[%u]", ret);
        ACL_DELETE_ARRAY(data->model_data);
        return ACL_GET_ERRCODE_GE(static_cast<int32_t>(ret));
    }

    *modelId = id;
    return ACL_SUCCESS;
}

// modified aclmdlLoadFromFile
aclError aclmdlLoadFromFileModified(const char *modelPath, uint32_t *modelId, ge::ModelData *data)
{
    ACL_STAGES_REG(acl::ACL_STAGE_LOAD, acl::ACL_STAGE_DEFAULT);
    ACL_PROFILING_REG(acl::AclProfType::AclmdlLoadFromFile);
    ACL_ADD_APPLY_TOTAL_COUNT(acl::ACL_STATISTICS_CREATE_LOAD_UNLOAD_MODEL);
    aclError ret = ACL_SUCCESS;

    ret = ModelLoadFromFileWithMemModified(modelPath, modelId, nullptr, 0U, nullptr, 0U, 0, data);
    if (ret != ACL_SUCCESS) {
        printf("load model from file failed!");
        return ret;
    }
    ACL_ADD_APPLY_SUCCESS_COUNT(acl::ACL_STATISTICS_CREATE_LOAD_UNLOAD_MODEL);
    return ACL_SUCCESS;
}

// modified aclmdlLoadFromFileWithMem
aclError aclmdlLoadFromFileWithMemModified(const char *modelPath, uint32_t *modelId,
                                   void *workPtr, size_t workSize,
                                   void *weightPtr, size_t weightSize, ge::ModelData *data)
{
    ACL_STAGES_REG(acl::ACL_STAGE_LOAD, acl::ACL_STAGE_DEFAULT);
    ACL_PROFILING_REG(acl::AclProfType::AclmdlLoadFromFileWithMem);
    ACL_ADD_APPLY_TOTAL_COUNT(acl::ACL_STATISTICS_CREATE_LOAD_UNLOAD_MODEL);
    aclError ret = ACL_SUCCESS;

    ret = ModelLoadFromFileWithMemModified(modelPath, modelId, workPtr, workSize, weightPtr, weightSize, 0, data);
    if (ret != ACL_SUCCESS) {
        printf("load model from file failed!");
        return ret;
    }
    ACL_ADD_APPLY_SUCCESS_COUNT(acl::ACL_STATISTICS_CREATE_LOAD_UNLOAD_MODEL);
    return ACL_SUCCESS;
}