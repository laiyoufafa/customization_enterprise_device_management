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

#include "datetime_manager_proxy.h"
#include "edm_log.h"
#include "func_code.h"
#include "policy_info.h"

namespace OHOS {
namespace EDM {
std::shared_ptr<DatetimeManagerProxy> DatetimeManagerProxy::instance_ = nullptr;
std::mutex DatetimeManagerProxy::mutexLock_;
const std::u16string DESCRIPTOR = u"ohos.edm.IEnterpriseDeviceMgr";

DatetimeManagerProxy::DatetimeManagerProxy() {}

DatetimeManagerProxy::~DatetimeManagerProxy() {}

std::shared_ptr<DatetimeManagerProxy> DatetimeManagerProxy::GetDatetimeManagerProxy()
{
    if (instance_ == nullptr) {
        std::lock_guard<std::mutex> lock(mutexLock_);
        if (instance_ == nullptr) {
            std::shared_ptr<DatetimeManagerProxy> temp = std::make_shared<DatetimeManagerProxy>();
            instance_ = temp;
        }
    }
    return instance_;
}

int32_t DatetimeManagerProxy::SetDateTime(AppExecFwk::ElementName &admin, int64_t time)
{
    EDMLOGD("DatetimeManagerProxy::SetDateTime");
    auto proxy = EnterpriseDeviceMgrProxy::GetInstance();
    if (proxy == nullptr) {
        EDMLOGE("can not get EnterpriseDeviceMgrProxy");
        return EdmReturnErrCode::SYSTEM_ABNORMALLY;
    }
    MessageParcel data;
    std::uint32_t funcCode = POLICY_FUNC_CODE((std::uint32_t)FuncOperateType::SET, SET_DATETIME);
    data.WriteInterfaceToken(DESCRIPTOR);
    data.WriteInt32(WITHOUT_USERID);
    data.WriteParcelable(&admin);
    data.WriteInt64(time);
    return proxy->HandleDevicePolicy(funcCode, data);
}

int32_t DatetimeManagerProxy::DisallowModifyDateTime(AppExecFwk::ElementName &admin, bool disallow)
{
    EDMLOGD("DatetimeManagerProxy::DisallowModifyDateTime");
    auto proxy = EnterpriseDeviceMgrProxy::GetInstance();
    if (proxy == nullptr) {
        EDMLOGE("can not get EnterpriseDeviceMgrProxy");
        return EdmReturnErrCode::SYSTEM_ABNORMALLY;
    }
    MessageParcel data;
    std::uint32_t funcCode = POLICY_FUNC_CODE((std::uint32_t)FuncOperateType::SET, DISALLOW_MODIFY_DATETIME);
    data.WriteInterfaceToken(DESCRIPTOR);
    data.WriteInt32(WITHOUT_USERID);
    data.WriteParcelable(&admin);
    data.WriteBool(disallow);
    return proxy->HandleDevicePolicy(funcCode, data);
}

int32_t DatetimeManagerProxy::IsModifyDateTimeDisallowed(AppExecFwk::ElementName &admin, bool hasAdmin, bool &result)
{
    EDMLOGD("DatetimeManagerProxy::IsModifyDateTimeDisallowed");
    auto proxy = EnterpriseDeviceMgrProxy::GetInstance();
    if (proxy == nullptr) {
        EDMLOGE("can not get EnterpriseDeviceMgrProxy");
        return EdmReturnErrCode::SYSTEM_ABNORMALLY;
    }
    MessageParcel data;
    MessageParcel reply;
    data.WriteInterfaceToken(DESCRIPTOR);
    data.WriteInt32(WITHOUT_USERID);
    if (hasAdmin) {
        data.WriteInt32(HAS_ADMIN);
        data.WriteParcelable(&admin);
    } else {
        data.WriteInt32(WITHOUT_ADMIN);
    }
    proxy->GetPolicy(DISALLOW_MODIFY_DATETIME, data, reply);
    int32_t ret = ERR_INVALID_VALUE;
    bool blRes = reply.ReadInt32(ret) && (ret == ERR_OK);
    if (!blRes) {
        EDMLOGW("DatetimeManagerProxy:GetPolicy fail. %{public}d", ret);
        return ret;
    }
    reply.ReadBool(result);
    return ERR_OK;
}
} // namespace EDM
} // namespace OHOS
