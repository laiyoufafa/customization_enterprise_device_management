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

#include "device_settings_manager.h"
#include "edm_log.h"
#include "func_code.h"
#include "policy_info.h"

namespace OHOS {
namespace EDM {
std::shared_ptr<DeviceSettingsManager> DeviceSettingsManager::instance_ = nullptr;
std::mutex DeviceSettingsManager::mutexLock_;
const std::u16string DESCRIPTOR = u"ohos.edm.IEnterpriseDeviceMgr";

DeviceSettingsManager::DeviceSettingsManager() {}

DeviceSettingsManager::~DeviceSettingsManager() {}

std::shared_ptr<DeviceSettingsManager> DeviceSettingsManager::GetDeviceSettingsManager()
{
    if (instance_ == nullptr) {
        std::lock_guard<std::mutex> lock(mutexLock_);
        if (instance_ == nullptr) {
            std::shared_ptr<DeviceSettingsManager> temp = std::make_shared<DeviceSettingsManager>();
            instance_ = temp;
        }
    }
    return instance_;
}

bool DeviceSettingsManager::SetDateTime(AppExecFwk::ElementName &admin, int64_t time)
{
    EDMLOGD("DeviceSettingsManager::SetDateTime");
    auto proxy = EnterpriseDeviceMgrProxy::GetInstance();
    if (proxy == nullptr) {
        EDMLOGE("can not get EnterpriseDeviceMgrProxy");
        return false;
    }
    MessageParcel data;
    std::uint32_t funcCode = POLICY_FUNC_CODE((std::uint32_t)FuncOperateType::SET, SET_DATETIME);
    data.WriteInterfaceToken(DESCRIPTOR);
    data.WriteParcelable(&admin);
    data.WriteInt64(time);
    proxy->HandleDevicePolicy(funcCode, data);
    return true;
}
} // namespace EDM
} // namespace OHOS
