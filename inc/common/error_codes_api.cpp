/**
* @file error_codes_api.cpp
*
* Copyright (C) Huawei Technologies Co., Ltd. 2019-2020. All Rights Reserved.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*/

#include "error_codes_api.h"

namespace acl {
namespace {
    constexpr const int32_t MIN_GE_ERROR_CODE = 145000;
    constexpr const int32_t MAX_GE_ERROR_CODE = 545999;
    constexpr const int32_t MIN_RTS_ERROR_CODE = 107000;
    constexpr const int32_t MAX_RTS_ERROR_CODE = 507999;
}
    ErrorCodeFactory& ErrorCodeFactory::Instance()
    {
        static ErrorCodeFactory instance;
        return instance;
    }

    void ErrorCodeFactory::RegisterErrorCode(const aclError errCode,
                                             ModuleId modId,
                                             int32_t moduleErrCode,
                                             const std::string &errDesc)
    {
        ErrorCodeHandle handle;
        handle.errCode = errCode;
        handle.modId = modId;
        handle.moduleErrCode = moduleErrCode;
        handle.errDesc = errDesc;

        const auto key = std::make_pair(modId, moduleErrCode);
        const auto iter = errMap_.find(key);
        if (iter != errMap_.cend()) {
            return;
        }
        errMap_[key] = handle;
    }

    aclError ErrorCodeFactory::GetErrorCode(const ModuleId modId, const int32_t moduleErrCode) const
    {
        // if ge error code is not in [MIN_GE_ERROR_CODE, MAX_GE_ERROR_CODE]
        // or [MIN_RTS_ERROR_CODE, MAX_RTS_ERROR_CODE], return common error code
        if (modId == MODULE_GE) {
            if (((moduleErrCode >= MIN_GE_ERROR_CODE) && (moduleErrCode <= MAX_GE_ERROR_CODE)) ||
                    ((moduleErrCode >= MIN_RTS_ERROR_CODE) && (moduleErrCode <= MAX_RTS_ERROR_CODE))) {
                return moduleErrCode;
            }
            return ACL_ERROR_GE_FAILURE;
        }
        return moduleErrCode;
    }

    const char_t* ErrorCodeFactory::GetErrDesc(ModuleId modId, int32_t moduleErrCode) const
    {
        const auto key = std::make_pair(modId, moduleErrCode);
        const auto iter = errMap_.find(key);
        if (iter == errMap_.end()) {
            return "";
        }
        return iter->second.errDesc.c_str();
    }
}
