/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef COMMON_NATIVE_INCLUDE_EDM_ERRORS_H
#define COMMON_NATIVE_INCLUDE_EDM_ERRORS_H

#include "errors.h"

namespace OHOS {
namespace EDM {
enum {
    EDM_MODULE_EDMSERVICE = 0x00,
    EDM_MODULE_ADMINMGR = 0x02,
    EDM_MODULE_POLICYMGR = 0x03,
    EDM_MODULE_PLUGINMGR = 0x04,
    // Reserved 0x04 ~ 0x0f for new modules, Event related modules start from 0x10
};

namespace EdmReturnErrCode {
    constexpr uint32_t PARAM_ERROR = 401;
    constexpr uint32_t PERMISSION_DENIED = 201;
    constexpr uint32_t SYSTEM_API_DENIED = 202;
    constexpr uint32_t INTERFACE_UNSUPPORTED = 801;

    constexpr uint32_t ADMIN_INACTIVE = 9200001;
    constexpr uint32_t ADMIN_EDM_PERMISSION_DENIED = 9200002;
    constexpr uint32_t COMPONENT_INVALID = 9200003;
    constexpr uint32_t ENABLE_ADMIN_FAILED = 9200004;
    constexpr uint32_t DISABLE_ADMIN_FAILED = 9200005;
    constexpr uint32_t UID_INVALID = 9200006;
    constexpr uint32_t SYSTEM_ABNORMALLY = 9200007;
    constexpr uint32_t MANAGED_EVENTS_INVALID = 9200008;
};

// Error code for SERVICE: 0x2000000,value:33554432
constexpr ErrCode EDM_SERVICE_ERR_OFFSET = ErrCodeOffset(SUBSYS_CUSTOMIZATION, EDM_MODULE_EDMSERVICE);
enum {
    ERR_EDM_SERVICE_NOT_READY = EDM_SERVICE_ERR_OFFSET + 1,
    ERR_EDM_PARAM_ERROR,
    ERR_EDM_PERMISSION_ERROR,
    ERR_EDM_ADD_ADMIN_FAILED,
    ERR_EDM_DEL_ADMIN_FAILED,
    ERR_EDM_GET_PERMISSION_REQ_FAILED,
    ERR_EDM_GET_PLUGIN_MGR_FAILED,
    ERR_EDM_GET_ADMIN_MGR_FAILED,
    ERR_EDM_HANDLE_POLICY_FAILED,
    ERR_EDM_GET_POLICY_FAILED,
    ERR_EDM_BMS_ERROR,
    ERR_EDM_DEL_SUPER_ADMIN_FAILED,
    ERR_EDM_GET_ENTINFO_FAILED,
    ERR_EDM_SET_ENTINFO_FAILED,
    ERR_EDM_DUMP_FAILED,
};

// Error code for ADMINMGR: 0x2020000,value:33685504
constexpr ErrCode EDM_ADMINMGR_ERR_OFFSET = ErrCodeOffset(SUBSYS_CUSTOMIZATION, EDM_MODULE_ADMINMGR);
enum {
    // the install error code from 0x0001 ~ 0x0060.
    ERR_EDM_UNKNOWN_PERMISSION = EDM_ADMINMGR_ERR_OFFSET + 0x0001,
    ERR_EDM_EMPTY_PERMISSIONS = EDM_ADMINMGR_ERR_OFFSET + 0x0002,
    ERR_EDM_DENY_ADMIN = EDM_ADMINMGR_ERR_OFFSET + 0x0003,
    ERR_EDM_DENY_PERMISSION = EDM_ADMINMGR_ERR_OFFSET + 0x0004,
    ERR_EDM_UNKNOWN_ADMIN = EDM_ADMINMGR_ERR_OFFSET + 0x0005,
};

// Error code for POLICYMGR: 0x2030000,value:33751040
constexpr ErrCode EDM_POLICYMGR_ERR_OFFSET = ErrCodeOffset(SUBSYS_CUSTOMIZATION, EDM_MODULE_POLICYMGR);
enum {
    ERR_EDM_POLICY_OPEN_JSON_FAILED = EDM_POLICYMGR_ERR_OFFSET + 1,
    ERR_EDM_POLICY_LOAD_JSON_FAILED = EDM_POLICYMGR_ERR_OFFSET + 2,
    ERR_EDM_POLICY_PARSE_JSON_FAILED = EDM_POLICYMGR_ERR_OFFSET + 3,
    ERR_EDM_POLICY_NOT_FIND = EDM_POLICYMGR_ERR_OFFSET + 4,
    ERR_EDM_POLICY_SET_FAILED = EDM_POLICYMGR_ERR_OFFSET + 5,
    ERR_EDM_POLICY_NOT_FOUND = EDM_POLICYMGR_ERR_OFFSET + 6,
    ERR_EDM_POLICY_DEL_FAILED = EDM_POLICYMGR_ERR_OFFSET + 7,
};

// Error code for POLICYMGR: 0x2040000,value:33816576
constexpr ErrCode EDM_PLUGINMGR_ERR_OFFSET = ErrCodeOffset(SUBSYS_CUSTOMIZATION, EDM_MODULE_PLUGINMGR);
enum {
    ERR_EDM_OPERATE_JSON = EDM_PLUGINMGR_ERR_OFFSET + 1,
    ERR_EDM_WRITE_JSON = EDM_PLUGINMGR_ERR_OFFSET + 2,
    ERR_EDM_READ_PARCEL = EDM_PLUGINMGR_ERR_OFFSET + 3,
    ERR_EDM_OPERATE_PARCEL = EDM_PLUGINMGR_ERR_OFFSET + 4,
    ERR_EDM_NOT_EXIST_POLICY = EDM_PLUGINMGR_ERR_OFFSET + 5,
    ERR_EDM_NOT_EXIST_FUNC = EDM_PLUGINMGR_ERR_OFFSET + 6,
};

#define RETURN_IF_FAILED(errcode) if (FAILED(errcode)) { return errcode; }
} // namespace EDM
} // namespace OHOS

#endif // COMMON_NATIVE_INCLUDE_EDM_ERRORS_H
