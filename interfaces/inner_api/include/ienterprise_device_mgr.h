/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef INTERFACES_INNER_API_INCLUDE_IENTERPRISE_DEVICE_MGR_H_
#define INTERFACES_INNER_API_INCLUDE_IENTERPRISE_DEVICE_MGR_H_
#include <string>
#include "admin_type.h"
#include "ent_info.h"
#include "edm_errors.h"
#include "element_name.h"
#include "iremote_broker.h"
#include "iremote_object.h"
#include "iremote_proxy.h"
#include "iremote_stub.h"

namespace OHOS {
namespace EDM {
class IEnterpriseDeviceMgr : public IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.edm.IEnterpriseDeviceMgr");
    virtual ErrCode ActiveAdmin(AppExecFwk::ElementName &admin, EntInfo &entInfo, AdminType type,
        int32_t userId) = 0;
    virtual ErrCode DeactiveAdmin(AppExecFwk::ElementName &admin, int32_t userId) = 0;
    virtual ErrCode DeactiveSuperAdmin(std::string &bundleName) = 0;
    virtual ErrCode HandleDevicePolicy(uint32_t code, AppExecFwk::ElementName &admin, MessageParcel &data) = 0;
    virtual ErrCode GetDevicePolicy(uint32_t code, AppExecFwk::ElementName *admin, MessageParcel &reply) = 0;
    virtual ErrCode GetActiveAdmin(AdminType type, std::vector<std::string> &activeAdminList) = 0;
    virtual ErrCode GetEnterpriseInfo(AppExecFwk::ElementName &admin, MessageParcel &reply) = 0;
    virtual ErrCode SetEnterpriseInfo(AppExecFwk::ElementName &admin, EntInfo &entInfo) = 0;
    virtual bool IsSuperAdmin(std::string &bundleName) = 0;
    virtual bool IsAdminActive(AppExecFwk::ElementName &admin) = 0;
    enum {
        ADD_DEVICE_ADMIN = 1,
        REMOVE_DEVICE_ADMIN = 2,
        GET_PERMISSION_REQUEST = 3,
        REMOVE_SUPER_ADMIN = 4,
        GET_ACTIVE_ADMIN = 5,
        GET_ENT_INFO = 6,
        SET_ENT_INFO = 7,
        IS_SUPER_ADMIN = 8,
        IS_ADMIN_ACTIVE = 9,
    };
};
} // namespace EDM
} // namespace OHOS
#endif // INTERFACES_INNER_API_INCLUDE_IENTERPRISE_DEVICE_MGR_H_
