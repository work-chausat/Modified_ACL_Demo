/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2023. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __INC_METADEF_OP_TYPE_UTILS_H
#define __INC_METADEF_OP_TYPE_UTILS_H
#include <string>
#include "graph/node.h"

namespace ge {
class OpTypeUtils {
 public:
  static bool IsDataNode(const std::string &type);
  static bool IsVariableNode(const std::string &type);
  static bool IsVarLikeNode(const std::string &type);
  static bool IsAssignLikeNode(const std::string &type);
  static bool IsIdentityLikeNode(const std::string &type);
  static graphStatus GetOriginalType(const ge::NodePtr &node, std::string &type);
};
} // namespace ge
#endif // __INC_METADEF_OP_TYPE_UTILS_H
