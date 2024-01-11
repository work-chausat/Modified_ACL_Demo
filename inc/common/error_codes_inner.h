/**
 * @file error_codes_inner.h
 *
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef ACL_ERROR_CODES_INNER_H_
#define ACL_ERROR_CODES_INNER_H_

#include "error_codes_api.h"

#define ACL_GET_ERRCODE_ACL(innerErrCode)   ACL_GET_ERRCODE(acl::MODULE_ACL, (innerErrCode))
#define ACL_GET_ERRCODE_GE(innerErrCode)    ACL_GET_ERRCODE(acl::MODULE_GE, static_cast<int32_t>(innerErrCode))
#define ACL_GET_ERRCODE_RTS(innerErrCode)   ACL_GET_ERRCODE(acl::MODULE_RTS, (innerErrCode))

#endif // ACL_ERROR_CODES_INNER_H_
