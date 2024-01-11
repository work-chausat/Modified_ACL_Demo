#pragma once
#include "utils.h"
#include "acl/acl.h"

aclError aclmdlLoadFromFileModified(const char *modelPath, uint32_t *modelId, ge::ModelData *data);
static aclError ModelLoadFromFileWithMemModified(const char *const modelPath, uint32_t *const modelId, void *const weightPtr,
                                                 const size_t weightSize, const int32_t priority, ge::ModelData *data);

aclError aclmdlLoadFromFileWithMemModified(const char *modelPath, uint32_t *modelId,
                                   void *workPtr, size_t workSize,
                                   void *weightPtr, size_t weightSize, ge::ModelData *data);