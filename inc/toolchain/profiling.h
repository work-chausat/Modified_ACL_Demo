/**
 * @file profiling.h
 *
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef ACL_PROFILING_H_
#define ACL_PROFILING_H_

#include "acl/acl_base.h"
#include "toolchain/prof_callback.h"
#include <string>
#include "common/log_inner.h"

namespace acl {
    class AclProfiling {
    public:
        static aclError HandleProfilingConfig(const char_t *const configPath);

    private:
        static aclError HandleProfilingCommand(const std::string &config, const bool configFileFlag,
            const bool noValidConfig);

        static bool GetProfilingConfigFile(std::string &fileName);
    };


aclError AclProfCtrlHandle(uint32_t dataType, void* data, uint32_t dataLen);

}

#endif // ACL_PROFILING_H_

