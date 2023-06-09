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

#ifndef INTERFACES_INNER_API_INCLUDE_ENTERPRISE_DEVICE_MGR_PROXY_H_
#define INTERFACES_INNER_API_INCLUDE_ENTERPRISE_DEVICE_MGR_PROXY_H_
#include <message_parcel.h>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>
#include "ienterprise_device_mgr.h"

namespace OHOS {
namespace EDM {
class EnterpriseDeviceMgrProxy {
public:
    EnterpriseDeviceMgrProxy();
    ~EnterpriseDeviceMgrProxy();
    static std::shared_ptr<EnterpriseDeviceMgrProxy> GetInstance();
    static void DestroyInstance();

    ErrCode ActivateAdmin(AppExecFwk::ElementName &admin, EntInfo &entInfo, AdminType type, int32_t userId);
    ErrCode DeactivateAdmin(AppExecFwk::ElementName &admin, int32_t userId);
    ErrCode DeactivateSuperAdmin(std::string bundleName);
    ErrCode GetActiveAdmin(AdminType type, std::vector<std::u16string> &activeAdminList);
    ErrCode GetEnterpriseInfo(AppExecFwk::ElementName &admin, EntInfo &entInfo);
    ErrCode SetEnterpriseInfo(AppExecFwk::ElementName &admin, EntInfo &entInfo);
    bool IsSuperAdmin(std::string bundleName);
    bool IsAdminActive(AppExecFwk::ElementName &admin);
    bool HandleDevicePolicy(int32_t policyCode, MessageParcel &data);

    void GetActiveSuperAdmin(std::string &activeAdmin);
    bool IsSuperAdminExist();
    void GetActiveAdmins(std::vector<std::string> &activeAdminList);
    bool IsPolicyDisable(int policyCode, bool &isDisabled);
    bool GetPolicyValue(int policyCode, std::string &policyData);
    bool GetPolicyArray(int policyCode, std::vector<std::string> &policyData);
    bool GetPolicyConfig(int policyCode, std::map<std::string, std::string> &policyData);

private:
    static std::shared_ptr<EnterpriseDeviceMgrProxy> instance_;
    static std::mutex mutexLock_;

    void GetActiveAdmins(std::uint32_t type, std::vector<std::string> &activeAdminList);
    sptr<IRemoteObject> GetRemoteObject();
    bool GetPolicy(int policyCode, MessageParcel &reply);
};
} // namespace EDM
} // namespace OHOS

#endif // INTERFACES_INNER_API_INCLUDE_ENTERPRISE_DEVICE_MGR_PROXY_H_
