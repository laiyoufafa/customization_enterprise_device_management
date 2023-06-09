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
#include "enterprise_device_manager_addon.h"
#include "edm_log.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"

using namespace OHOS::EDM;

namespace {
constexpr int32_t NAPI_TYPE_NOMAL = 0;
constexpr int32_t NAPI_TYPE_SUPER = 1;

constexpr int32_t ARR_INDEX_ZERO = 0;
constexpr int32_t ARR_INDEX_ONE = 1;
constexpr int32_t ARR_INDEX_TWO = 2;
constexpr int32_t ARR_INDEX_THREE = 3;

constexpr size_t ARGS_SIZE_ONE = 1;
constexpr size_t ARGS_SIZE_TWO = 2;
constexpr size_t ARGS_SIZE_THREE = 3;
constexpr size_t ARGS_SIZE_FOUR = 4;
constexpr size_t CALLBACK_SIZE = 1;

constexpr int32_t NAPI_RETURN_ZERO = 0;
constexpr int32_t NAPI_RETURN_ONE = 1;

constexpr int32_t DEFAULT_USER_ID = 100;
}

std::shared_ptr<EnterpriseDeviceMgrProxy> EnterpriseDeviceManagerAddon::proxy_ = nullptr;
std::shared_ptr<DeviceSettingsManager> EnterpriseDeviceManagerAddon::deviceSettingsManager_ = nullptr;
thread_local napi_ref EnterpriseDeviceManagerAddon::g_classDeviceSettingsManager;

napi_value EnterpriseDeviceManagerAddon::ActivateAdmin(napi_env env, napi_callback_info info)
{
    EDMLOGI("NAPI_ActivateAdmin called");
    size_t argc = ARGS_SIZE_FOUR;
    napi_value argv[ARGS_SIZE_FOUR] = {nullptr};
    napi_value thisArg = nullptr;
    void *data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisArg, &data));
    NAPI_ASSERT(env, argc <= ARGS_SIZE_FOUR, "parameter count error");
    bool matchFlag = MatchValueType(env, argv[ARR_INDEX_ZERO], napi_object) &&
        MatchValueType(env, argv[ARR_INDEX_ONE], napi_object) && MatchValueType(env, argv[ARR_INDEX_TWO], napi_number);
    if (argc == ARGS_SIZE_FOUR) {
        EDMLOGD("argc == ARGS_SIZE_FOUR");
        matchFlag = matchFlag && MatchValueType(env, argv[ARR_INDEX_THREE], napi_function);
    }
    EDMLOGD("ActivateAdmin matchFlag %{public}d", matchFlag);
    NAPI_ASSERT(env, matchFlag, "parameter type error");
    OHOS::AppExecFwk::ElementName elementName;
    auto asyncCallbackInfo = (std::make_unique<AsyncActivateAdminCallbackInfo>()).release();
    bool ret = ParseElementName(env, asyncCallbackInfo->elementName, argv[ARR_INDEX_ZERO]);
    NAPI_ASSERT(env, ret, "element name param error");
    EDMLOGD("ActiveAdmin::asyncCallbackInfo->elementName.bundlename %{public}s, "
        "asyncCallbackInfo->abilityname:%{public}s , adminType:%{public}d",
        asyncCallbackInfo->elementName.GetBundleName().c_str(), asyncCallbackInfo->elementName.GetAbilityName().c_str(),
        asyncCallbackInfo->adminType);
    ret = ParseEnterpriseInfo(env, asyncCallbackInfo->entInfo, argv[ARR_INDEX_ONE]);
    NAPI_ASSERT(env, ret, "enterprise info param error");
    ret = ParseInt(env, asyncCallbackInfo->adminType, argv[ARR_INDEX_TWO]);
    NAPI_ASSERT(env, ret, "admin type param error");
    if (argc == ARGS_SIZE_FOUR) {
        napi_create_reference(env, argv[ARR_INDEX_THREE], NAPI_RETURN_ONE, &asyncCallbackInfo->callback);
    }
    return HandleAsyncWork(env, asyncCallbackInfo, "ActivateAdmin", NativeActivateAdmin, NativeBoolCallbackComplete);
}

void EnterpriseDeviceManagerAddon::NativeActivateAdmin(napi_env env, void *data)
{
    if (data == nullptr) {
        EDMLOGE("data is nullptr");
        return;
    }
    AsyncActivateAdminCallbackInfo *asyncCallbackInfo = static_cast<AsyncActivateAdminCallbackInfo *>(data);
    auto proxy_ = EnterpriseDeviceMgrProxy::GetInstance();
    if (proxy_ == nullptr) {
        EDMLOGE("can not get EnterpriseDeviceMgrProxy");
        return;
    }
    asyncCallbackInfo->ret = proxy_->ActivateAdmin(asyncCallbackInfo->elementName, asyncCallbackInfo->entInfo,
        static_cast<AdminType>(asyncCallbackInfo->adminType), DEFAULT_USER_ID);
}

void EnterpriseDeviceManagerAddon::NativeBoolCallbackComplete(napi_env env, napi_status status, void *data)
{
    if (data == nullptr) {
        EDMLOGE("data is nullptr");
        return;
    }
    AsyncCallbackInfo *asyncCallbackInfo = static_cast<AsyncCallbackInfo *>(data);
    if (asyncCallbackInfo->deferred != nullptr) {
        EDMLOGD("asyncCallbackInfo->deferred != nullptr");
        napi_value result = nullptr;
        if (asyncCallbackInfo->ret == ERR_OK) {
            EDMLOGD("asyncCallbackInfo->boolRet = %{public}d", asyncCallbackInfo->boolRet);
            napi_get_boolean(env, asyncCallbackInfo->boolRet, &result);
            napi_resolve_deferred(env, asyncCallbackInfo->deferred, result);
        } else {
            std::string errTip = std::to_string(asyncCallbackInfo->ret);
            napi_reject_deferred(env, asyncCallbackInfo->deferred, CreateErrorMessage(env, errTip));
        }
    } else {
        napi_value callbackValue[ARGS_SIZE_TWO] = { 0 };
        if (asyncCallbackInfo->ret == ERR_OK) {
            callbackValue[ARR_INDEX_ZERO] = CreateUndefined(env);
            EDMLOGD("asyncCallbackInfo->boolRet = %{public}d", asyncCallbackInfo->boolRet);
            napi_get_boolean(env, asyncCallbackInfo->boolRet, &callbackValue[ARR_INDEX_ONE]);
        } else {
            int32_t errCode = asyncCallbackInfo->ret;
            std::string errTip = std::to_string(errCode);
            callbackValue[ARR_INDEX_ZERO] = CreateErrorMessage(env, errTip);
            callbackValue[ARR_INDEX_ONE] = CreateUndefined(env);
        }
        napi_value callback = nullptr;
        napi_value result = nullptr;
        napi_get_reference_value(env, asyncCallbackInfo->callback, &callback);
        napi_call_function(env, nullptr, callback, std::size(callbackValue), callbackValue, &result);
        napi_delete_reference(env, asyncCallbackInfo->callback);
    }
    napi_delete_async_work(env, asyncCallbackInfo->asyncWork);
    delete asyncCallbackInfo;
}

napi_value EnterpriseDeviceManagerAddon::DeactivateAdmin(napi_env env, napi_callback_info info)
{
    EDMLOGI("NAPI_DeactivateAdmin called");
    size_t argc = ARGS_SIZE_TWO;
    napi_value argv[ARGS_SIZE_TWO] = {nullptr};
    napi_value thisArg = nullptr;
    void *data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisArg, &data));
    NAPI_ASSERT(env, argc < ARGS_SIZE_THREE, "parameter count error");
    bool matchFlag = MatchValueType(env, argv[ARR_INDEX_ZERO], napi_object);
    if (argc == ARGS_SIZE_TWO) {
        matchFlag = matchFlag && MatchValueType(env, argv[ARR_INDEX_ONE], napi_function);
    }
    NAPI_ASSERT(env, matchFlag, "parameter type error");
    OHOS::AppExecFwk::ElementName elementName;
    auto asyncCallbackInfo = (std::make_unique<AsyncDeactivateAdminCallbackInfo>()).release();
    bool ret = ParseElementName(env, asyncCallbackInfo->elementName, argv[ARR_INDEX_ZERO]);
    NAPI_ASSERT(env, ret, "element name param error");
    EDMLOGD("DeactivateAdmin::asyncCallbackInfo->elementName.bundlename %{public}s, "
        "asyncCallbackInfo->abilityname:%{public}s",
        asyncCallbackInfo->elementName.GetBundleName().c_str(),
        asyncCallbackInfo->elementName.GetAbilityName().c_str());
    if (argc == ARGS_SIZE_TWO) {
        napi_create_reference(env, argv[ARR_INDEX_ONE], NAPI_RETURN_ONE, &asyncCallbackInfo->callback);
    }

    return HandleAsyncWork(env, asyncCallbackInfo, "DeactivateAdmin", NativeDeactivateAdmin,
        NativeBoolCallbackComplete);
}

void EnterpriseDeviceManagerAddon::NativeDeactivateAdmin(napi_env env, void *data)
{
    EDMLOGI("NAPI_NativeDeactivateAdmin called");
    if (data == nullptr) {
        EDMLOGE("data is nullptr");
        return;
    }
    AsyncDeactivateAdminCallbackInfo *asyncCallbackInfo = static_cast<AsyncDeactivateAdminCallbackInfo *>(data);
    auto proxy_ = EnterpriseDeviceMgrProxy::GetInstance();
    if (proxy_ == nullptr) {
        EDMLOGE("can not get EnterpriseDeviceMgrProxy");
        return;
    }
    asyncCallbackInfo->ret = proxy_->DeactivateAdmin(asyncCallbackInfo->elementName, DEFAULT_USER_ID);
}

napi_value EnterpriseDeviceManagerAddon::DeactivateSuperAdmin(napi_env env, napi_callback_info info)
{
    EDMLOGI("NAPI_DeactivateSuperAdmin called");
    size_t argc = ARGS_SIZE_TWO;
    napi_value argv[ARGS_SIZE_TWO] = {nullptr};
    napi_value thisArg = nullptr;
    void *data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisArg, &data));
    NAPI_ASSERT(env, argc < ARGS_SIZE_THREE, "parameter count error");

    auto asyncCallbackInfo = (std::make_unique<AsyncDeactivateSuperAdminCallbackInfo>()).release();
    ParseString(env, asyncCallbackInfo->bundleName, argv[ARR_INDEX_ZERO]);
    if (argc == ARGS_SIZE_TWO) {
        bool matchFlag = MatchValueType(env, argv[ARR_INDEX_ONE], napi_function);
        napi_create_reference(env, argv[ARR_INDEX_ONE], NAPI_RETURN_ONE, &asyncCallbackInfo->callback);
        NAPI_ASSERT(env, matchFlag, "parameter type error");
    }

    EDMLOGD("DeactivateSuperAdmin: asyncCallbackInfo->elementName.bundlename %{public}s",
        asyncCallbackInfo->bundleName.c_str());
    return HandleAsyncWork(env, asyncCallbackInfo, "DeactivateSuperAdmin", NativeDeactivateSuperAdmin,
        NativeBoolCallbackComplete);
}

void EnterpriseDeviceManagerAddon::NativeDeactivateSuperAdmin(napi_env env, void *data)
{
    EDMLOGI("NAPI_NativeDeactivateSuperAdmin called");
    if (data == nullptr) {
        EDMLOGE("data is nullptr");
        return;
    }
    AsyncDeactivateSuperAdminCallbackInfo *asyncCallbackInfo =
        static_cast<AsyncDeactivateSuperAdminCallbackInfo *>(data);
    auto proxy_ = EnterpriseDeviceMgrProxy::GetInstance();
    if (proxy_ == nullptr) {
        EDMLOGE("can not get EnterpriseDeviceMgrProxy");
        return;
    }
    asyncCallbackInfo->ret = proxy_->DeactivateSuperAdmin(asyncCallbackInfo->bundleName);
}

napi_value EnterpriseDeviceManagerAddon::GetEnterpriseInfo(napi_env env, napi_callback_info info)
{
    EDMLOGI("NAPI_GetEnterpriseInfo called");
    size_t argc = ARGS_SIZE_TWO;
    napi_value argv[ARGS_SIZE_TWO] = {nullptr};
    napi_value thisArg = nullptr;
    void *data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisArg, &data));
    NAPI_ASSERT(env, argc < ARGS_SIZE_THREE, "parameter count error");
    bool matchFlag = MatchValueType(env, argv[ARR_INDEX_ZERO], napi_object);
    if (argc == ARGS_SIZE_TWO) {
        matchFlag = matchFlag && MatchValueType(env, argv[ARR_INDEX_ONE], napi_function);
    }
    NAPI_ASSERT(env, matchFlag, "parameter type error");
    auto asyncCallbackInfo = (std::make_unique<AsyncGetEnterpriseInfoCallbackInfo>()).release();
    OHOS::AppExecFwk::ElementName elementName;
    bool ret = ParseElementName(env, asyncCallbackInfo->elementName, argv[ARR_INDEX_ZERO]);
    NAPI_ASSERT(env, ret, "element name param error");
    EDMLOGD("ActiveAdmin: asyncCallbackInfo->elementName.bundlename %{public}s, "
        "asyncCallbackInfo->abilityname:%{public}s",
        asyncCallbackInfo->elementName.GetBundleName().c_str(),
        asyncCallbackInfo->elementName.GetAbilityName().c_str());
    if (argc == ARGS_SIZE_TWO) {
        EDMLOGD("GetEnterpriseInfo by callback");
        napi_create_reference(env, argv[ARR_INDEX_ONE], NAPI_RETURN_ONE, &asyncCallbackInfo->callback);
    }

    return HandleAsyncWork(env, asyncCallbackInfo, "GetEnterpriseInfo", NativeGetEnterpriseInfo,
        NativeGetEnterpriseInfoComplete);
}

void EnterpriseDeviceManagerAddon::NativeGetEnterpriseInfo(napi_env env, void *data)
{
    EDMLOGI("NAPI_NativeDeactivateSuperAdmin called");
    if (data == nullptr) {
        EDMLOGE("data is nullptr");
        return;
    }
    AsyncGetEnterpriseInfoCallbackInfo *asyncCallbackInfo = static_cast<AsyncGetEnterpriseInfoCallbackInfo *>(data);
    auto proxy_ = EnterpriseDeviceMgrProxy::GetInstance();
    if (proxy_ == nullptr) {
        EDMLOGE("can not get EnterpriseDeviceMgrProxy");
        return;
    }
    asyncCallbackInfo->ret = proxy_->GetEnterpriseInfo(asyncCallbackInfo->elementName, asyncCallbackInfo->entInfo);
}

void EnterpriseDeviceManagerAddon::NativeGetEnterpriseInfoComplete(napi_env env, napi_status status, void *data)
{
    if (data == nullptr) {
        EDMLOGE("data is nullptr");
        return;
    }
    AsyncGetEnterpriseInfoCallbackInfo *asyncCallbackInfo = static_cast<AsyncGetEnterpriseInfoCallbackInfo *>(data);

    napi_value result[2] = { 0 };
    if (asyncCallbackInfo->ret == ERR_OK) {
        napi_create_uint32(env, 0, &result[0]);
        napi_create_object(env, &result[1]);
        ConvertEnterpriseInfo(env, result[1], asyncCallbackInfo->entInfo);
        EDMLOGD("NativeGetEnterpriseInfoComplete::asyncCallbackInfo->entInfo->enterpriseName %{public}s, "
            "asyncCallbackInfo->entInfo->description:%{public}s",
            asyncCallbackInfo->entInfo.enterpriseName.c_str(), asyncCallbackInfo->entInfo.description.c_str());
    } else {
        napi_create_int32(env, 1, &result[0]);
        napi_get_undefined(env, &result[1]);
    }
    if (asyncCallbackInfo->deferred) {
        if (asyncCallbackInfo->ret == ERR_OK) {
            napi_resolve_deferred(env, asyncCallbackInfo->deferred, result[1]);
        } else {
            napi_reject_deferred(env, asyncCallbackInfo->deferred, result[0]);
        }
    } else {
        napi_value callback = nullptr;
        napi_value placeHolder = nullptr;
        napi_get_reference_value(env, asyncCallbackInfo->callback, &callback);
        napi_call_function(env, nullptr, callback, sizeof(result) / sizeof(result[0]), result, &placeHolder);
        napi_delete_reference(env, asyncCallbackInfo->callback);
    }
    napi_delete_async_work(env, asyncCallbackInfo->asyncWork);
    delete asyncCallbackInfo;
    asyncCallbackInfo = nullptr;
}

napi_value EnterpriseDeviceManagerAddon::SetEnterpriseInfo(napi_env env, napi_callback_info info)
{
    EDMLOGI("NAPI_SetEnterpriseInfo called");
    size_t argc = ARGS_SIZE_THREE;
    napi_value argv[ARGS_SIZE_THREE] = {nullptr};
    napi_value thisArg = nullptr;
    void *data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisArg, &data));
    NAPI_ASSERT(env, argc <= ARGS_SIZE_THREE, "parameter count error");
    bool matchFlag =
        MatchValueType(env, argv[ARR_INDEX_ZERO], napi_object) && MatchValueType(env, argv[ARR_INDEX_ONE], napi_object);
    if (argc == ARGS_SIZE_THREE) {
        matchFlag = matchFlag && MatchValueType(env, argv[ARR_INDEX_TWO], napi_function);
    }
    NAPI_ASSERT(env, matchFlag, "parameter type error");
    OHOS::AppExecFwk::ElementName elementName;
    auto asyncCallbackInfo = (std::make_unique<AsyncSetEnterpriseInfoCallbackInfo>()).release();
    bool ret = ParseElementName(env, asyncCallbackInfo->elementName, argv[ARR_INDEX_ZERO]);
    NAPI_ASSERT(env, ret, "element name param error");
    ret = ParseEnterpriseInfo(env, asyncCallbackInfo->entInfo, argv[ARR_INDEX_ONE]);
    NAPI_ASSERT(env, ret, "enterprise info param error");
    EDMLOGD("SetEnterpriseInfo: asyncCallbackInfo->elementName.bundlename %{public}s, "
        "asyncCallbackInfo->abilityname:%{public}s",
        asyncCallbackInfo->elementName.GetBundleName().c_str(),
        asyncCallbackInfo->elementName.GetAbilityName().c_str());
    if (argc == ARGS_SIZE_THREE) {
        napi_create_reference(env, argv[ARR_INDEX_TWO], NAPI_RETURN_ONE, &asyncCallbackInfo->callback);
    }

    return HandleAsyncWork(env, asyncCallbackInfo, "SetEnterpriseInfo", NativeSetEnterpriseInfo,
        NativeBoolCallbackComplete);
}

napi_value EnterpriseDeviceManagerAddon::SetDateTime(napi_env env, napi_callback_info info)
{
    EDMLOGI("NAPI_SetDateTime called");
    size_t argc = ARGS_SIZE_THREE;
    napi_value argv[ARGS_SIZE_THREE] = {nullptr};
    napi_value thisArg = nullptr;
    void *data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisArg, &data));
    NAPI_ASSERT(env, argc <= ARGS_SIZE_THREE, "parameter count error");
    bool matchFlag =
        MatchValueType(env, argv[ARR_INDEX_ZERO], napi_object) && MatchValueType(env, argv[ARR_INDEX_ONE], napi_number);
    if (argc == ARGS_SIZE_THREE) {
        matchFlag = matchFlag && MatchValueType(env, argv[ARR_INDEX_TWO], napi_function);
    }
    NAPI_ASSERT(env, matchFlag, "parameter type error");
    OHOS::AppExecFwk::ElementName elementName;
    auto asyncCallbackInfo = (std::make_unique<AsyncSetDateTimeCallbackInfo>()).release();
    bool ret = ParseElementName(env, asyncCallbackInfo->elementName, argv[ARR_INDEX_ZERO]);
    NAPI_ASSERT(env, ret, "element name param error");
    EDMLOGD("SetDateTime: asyncCallbackInfo->elementName.bundlename %{public}s, "
        "asyncCallbackInfo->abilityname:%{public}s",
        asyncCallbackInfo->elementName.GetBundleName().c_str(),
        asyncCallbackInfo->elementName.GetAbilityName().c_str());
    ret = ParseLong(env, asyncCallbackInfo->time, argv[ARR_INDEX_ONE]);
    NAPI_ASSERT(env, ret, "time param error");
    if (argc == ARGS_SIZE_THREE) {
        EDMLOGD("NAPI_SetDateTime argc == ARGS_SIZE_THREE");
        napi_create_reference(env, argv[ARR_INDEX_TWO], NAPI_RETURN_ONE, &asyncCallbackInfo->callback);
    }

    return HandleAsyncWork(env, asyncCallbackInfo, "SetDateTime", NativeSetDateTime, NativeBoolCallbackComplete);
}

void EnterpriseDeviceManagerAddon::NativeSetDateTime(napi_env env, void *data)
{
    EDMLOGI("NAPI_NativeSetDateTime called");
    if (data == nullptr) {
        EDMLOGE("data is nullptr");
        return;
    }
    AsyncSetDateTimeCallbackInfo *asyncCallbackInfo = static_cast<AsyncSetDateTimeCallbackInfo *>(data);
    auto deviceSettingsManager_ = DeviceSettingsManager::GetDeviceSettingsManager();
    if (deviceSettingsManager_ == nullptr) {
        EDMLOGE("can not get DeviceSettingsManager");
        return;
    }
    bool success = deviceSettingsManager_->SetDateTime(asyncCallbackInfo->elementName, asyncCallbackInfo->time);
    asyncCallbackInfo->ret = ERR_OK;
    asyncCallbackInfo->boolRet = success;
    EDMLOGD("NativeSetDateTime asyncCallbackInfo->boolRet %{public}d", asyncCallbackInfo->boolRet);
}

void EnterpriseDeviceManagerAddon::NativeSetEnterpriseInfo(napi_env env, void *data)
{
    EDMLOGI("NAPI_NativeSetEnterpriseInfo called");
    if (data == nullptr) {
        EDMLOGE("data is nullptr");
        return;
    }
    AsyncSetEnterpriseInfoCallbackInfo *asyncCallbackInfo = static_cast<AsyncSetEnterpriseInfoCallbackInfo *>(data);
    auto proxy_ = EnterpriseDeviceMgrProxy::GetInstance();
    if (proxy_ == nullptr) {
        EDMLOGE("can not get EnterpriseDeviceMgrProxy");
        return;
    }
    asyncCallbackInfo->ret = proxy_->SetEnterpriseInfo(asyncCallbackInfo->elementName, asyncCallbackInfo->entInfo);
}

napi_value EnterpriseDeviceManagerAddon::IsSuperAdmin(napi_env env, napi_callback_info info)
{
    EDMLOGI("NAPI_IsSuperAdmin called");
    size_t argc = ARGS_SIZE_TWO;
    napi_value argv[ARGS_SIZE_TWO] = {nullptr};
    napi_value thisArg = nullptr;
    void *data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisArg, &data));
    NAPI_ASSERT(env, argc < ARGS_SIZE_THREE, "parameter count error");
    auto asyncCallbackInfo = (std::make_unique<AsyncIsSuperAdminCallbackInfo>()).release();
    ParseString(env, asyncCallbackInfo->bundleName, argv[ARR_INDEX_ZERO]);
    EDMLOGD("IsSuperAdmin: asyncCallbackInfo->elementName.bundlename %{public}s",
        asyncCallbackInfo->bundleName.c_str());
    if (argc == ARGS_SIZE_TWO) {
        bool matchFlag = MatchValueType(env, argv[ARR_INDEX_ONE], napi_function);
        NAPI_ASSERT(env, matchFlag, "parameter type error");
        napi_create_reference(env, argv[ARR_INDEX_ONE], NAPI_RETURN_ONE, &asyncCallbackInfo->callback);
    }

    return HandleAsyncWork(env, asyncCallbackInfo, "IsSuperAdmin", NativeIsSuperAdmin, NativeBoolCallbackComplete);
}

napi_value EnterpriseDeviceManagerAddon::IsAdminAppActive(napi_env env, napi_callback_info info)
{
    EDMLOGI("IsAdminAppActive called");
    size_t argc = ARGS_SIZE_TWO;
    napi_value argv[ARGS_SIZE_TWO] = {nullptr};
    napi_value thisArg = nullptr;
    void *data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisArg, &data));
    NAPI_ASSERT(env, argc < ARGS_SIZE_THREE, "parameter count error");
    bool matchFlag = MatchValueType(env, argv[ARR_INDEX_ZERO], napi_object);
    if (argc == ARGS_SIZE_TWO) {
        matchFlag = matchFlag && MatchValueType(env, argv[ARR_INDEX_ONE], napi_function);
    }
    NAPI_ASSERT(env, matchFlag, "parameter type error");
    auto asyncCallbackInfo = (std::make_unique<AsyncIsAdminActiveCallbackInfo>()).release();
    OHOS::AppExecFwk::ElementName elementName;
    bool ret = ParseElementName(env, asyncCallbackInfo->elementName, argv[ARR_INDEX_ZERO]);
    NAPI_ASSERT(env, ret, "element name param error");
    EDMLOGD("IsAdminAppActive::asyncCallbackInfo->elementName.bundlename %{public}s, "
        "asyncCallbackInfo->abilityname:%{public}s",
        asyncCallbackInfo->elementName.GetBundleName().c_str(),
        asyncCallbackInfo->elementName.GetAbilityName().c_str());
    if (argc == ARGS_SIZE_TWO) {
        napi_create_reference(env, argv[ARR_INDEX_ONE], NAPI_RETURN_ONE, &asyncCallbackInfo->callback);
    }

    return HandleAsyncWork(env, asyncCallbackInfo, "IsAdminAppActive", NativeIsAdminActive, NativeBoolCallbackComplete);
}

void EnterpriseDeviceManagerAddon::NativeIsSuperAdmin(napi_env env, void *data)
{
    EDMLOGI("NAPI_NativeIsSuperAdmin called");
    if (data == nullptr) {
        EDMLOGE("data is nullptr");
        return;
    }
    AsyncIsSuperAdminCallbackInfo *asyncCallbackInfo = static_cast<AsyncIsSuperAdminCallbackInfo *>(data);
    auto proxy_ = EnterpriseDeviceMgrProxy::GetInstance();
    if (proxy_ == nullptr) {
        EDMLOGE("can not get EnterpriseDeviceMgrProxy");
        return;
    }
    asyncCallbackInfo->ret = ERR_OK;
    asyncCallbackInfo->boolRet = proxy_->IsSuperAdmin(asyncCallbackInfo->bundleName);
}

void EnterpriseDeviceManagerAddon::NativeIsAdminActive(napi_env env, void *data)
{
    EDMLOGI("NAPI_NativeIsAdminActive called");
    if (data == nullptr) {
        EDMLOGE("data is nullptr");
        return;
    }
    AsyncIsAdminActiveCallbackInfo *asyncCallbackInfo = static_cast<AsyncIsAdminActiveCallbackInfo *>(data);
    auto proxy_ = EnterpriseDeviceMgrProxy::GetInstance();
    if (proxy_ == nullptr) {
        EDMLOGE("can not get EnterpriseDeviceMgrProxy");
        return;
    }
    asyncCallbackInfo->ret = ERR_OK;
    asyncCallbackInfo->boolRet = proxy_->IsAdminActive(asyncCallbackInfo->elementName);
}

napi_value EnterpriseDeviceManagerAddon::CreateUndefined(napi_env env)
{
    napi_value result = nullptr;
    napi_get_undefined(env, &result);
    return result;
}

bool EnterpriseDeviceManagerAddon::MatchValueType(napi_env env, napi_value value, napi_valuetype targetType)
{
    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, value, &valueType);
    return valueType == targetType;
}

napi_value EnterpriseDeviceManagerAddon::HandleAsyncWork(napi_env env, AsyncCallbackInfo *context, std::string workName,
    napi_async_execute_callback execute, napi_async_complete_callback complete)
{
    napi_value result = nullptr;
    if (context->callback == nullptr) {
        napi_create_promise(env, &context->deferred, &result);
    } else {
        napi_get_undefined(env, &result);
    }
    napi_value resource = CreateUndefined(env);
    napi_value resourceName = nullptr;
    napi_create_string_utf8(env, workName.data(), NAPI_AUTO_LENGTH, &resourceName);
    napi_create_async_work(env, resource, resourceName, execute, complete,
        static_cast<void *>(context), &context->asyncWork);
    napi_queue_async_work(env, context->asyncWork);
    return result;
}

napi_value EnterpriseDeviceManagerAddon::CreateErrorMessage(napi_env env, std::string msg)
{
    napi_value result = nullptr;
    napi_value message = nullptr;
    napi_create_string_utf8(env, static_cast<char *>(msg.data()), msg.size(), &message);
    napi_create_error(env, nullptr, message, &result);
    return result;
}

napi_value EnterpriseDeviceManagerAddon::ParseInt(napi_env env, int32_t &param, napi_value args)
{
    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, args, &valuetype));
    EDMLOGD("ParseInt param=%{public}d.", valuetype);
    NAPI_ASSERT(env, valuetype == napi_number, "Wrong argument type. int32 expected.");
    int32_t value = 0;
    napi_get_value_int32(env, args, &value);
    param = value;
    // create result code
    napi_value result = nullptr;
    napi_status status = napi_create_int32(env, NAPI_RETURN_ONE, &result);
    NAPI_ASSERT(env, status == napi_ok, "napi_create_int32 error!");
    return result;
}

napi_value EnterpriseDeviceManagerAddon::ParseLong(napi_env env, int64_t &param, napi_value args)
{
    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, args, &valuetype));
    EDMLOGD("ParseLong param=%{public}d.", valuetype);
    NAPI_ASSERT(env, valuetype == napi_number, "Wrong argument type. int64 expected.");
    int64_t value = 0;
    napi_get_value_int64(env, args, &value);
    param = value;
    // create result code
    napi_value result = nullptr;
    napi_status status = napi_create_int32(env, NAPI_RETURN_ONE, &result);
    NAPI_ASSERT(env, status == napi_ok, "napi_create_int32 error!");
    return result;
}

bool EnterpriseDeviceManagerAddon::ParseEnterpriseInfo(napi_env env, EntInfo &enterpriseInfo, napi_value args)
{
    napi_status status;
    napi_valuetype valueType;
    NAPI_CALL(env, napi_typeof(env, args, &valueType));
    if (valueType != napi_object) {
        EDMLOGE("param type mismatch!");
        return false;
    }
    std::string name;
    std::string description;
    napi_value prop = nullptr;
    EDMLOGD("ParseEnterpriseInfo name");
    status = napi_get_named_property(env, args, "name", &prop);
    name = GetStringFromNAPI(env, prop);
    EDMLOGD("ParseEnterpriseInfo name %{public}s ", name.c_str());

    EDMLOGD("ParseEnterpriseInfo description");
    prop = nullptr;
    status = napi_get_named_property(env, args, "description", &prop);
    description = GetStringFromNAPI(env, prop);
    EDMLOGD("ParseEnterpriseInfo description %{public}s", description.c_str());

    enterpriseInfo.enterpriseName = name;
    enterpriseInfo.description = description;
    return true;
}

void EnterpriseDeviceManagerAddon::ConvertEnterpriseInfo(napi_env env, napi_value objEntInfo, EntInfo &entInfo)
{
    std::string enterpriseName = entInfo.enterpriseName;
    std::string description = entInfo.description;
    napi_value nEnterpriseName;
    NAPI_CALL_RETURN_VOID(env,
        napi_create_string_utf8(env, enterpriseName.c_str(), NAPI_AUTO_LENGTH, &nEnterpriseName));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objEntInfo, "name", nEnterpriseName));

    napi_value nDescription;
    NAPI_CALL_RETURN_VOID(env, napi_create_string_utf8(env, description.c_str(), NAPI_AUTO_LENGTH, &nDescription));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objEntInfo, "description", nDescription));
}

bool EnterpriseDeviceManagerAddon::ParseElementName(napi_env env, AppExecFwk::ElementName &elementName, napi_value args)
{
    napi_status status;
    napi_valuetype valueType;
    NAPI_CALL(env, napi_typeof(env, args, &valueType));
    if (valueType != napi_object) {
        EDMLOGE("param type mismatch!");
        return false;
    }
    std::string bundleName;
    std::string abilityName;
    napi_value prop = nullptr;
    EDMLOGD("ParseElementName name");
    status = napi_get_named_property(env, args, "bundleName", &prop);
    bundleName = GetStringFromNAPI(env, prop);
    EDMLOGD("ParseElementName name %{public}s ", bundleName.c_str());

    EDMLOGD("ParseElementName description");
    prop = nullptr;
    status = napi_get_named_property(env, args, "abilityName", &prop);
    abilityName = GetStringFromNAPI(env, prop);
    EDMLOGD("ParseElementName description %{public}s", abilityName.c_str());

    elementName.SetBundleName(bundleName);
    elementName.SetAbilityName(abilityName);
    return true;
}

std::string EnterpriseDeviceManagerAddon::GetStringFromNAPI(napi_env env, napi_value value)
{
    std::string result;
    size_t size = 0;

    if (napi_get_value_string_utf8(env, value, nullptr, NAPI_RETURN_ZERO, &size) != napi_ok) {
        EDMLOGE("can not get string size");
        return "";
    }
    result.reserve(size + NAPI_RETURN_ONE);
    result.resize(size);
    if (napi_get_value_string_utf8(env, value, result.data(), (size + NAPI_RETURN_ONE), &size) != napi_ok) {
        EDMLOGE("can not get string value");
        return "";
    }
    return result;
}

void EnterpriseDeviceManagerAddon::CreateAdminTypeObject(napi_env env, napi_value value)
{
    napi_value nNomal;
    NAPI_CALL_RETURN_VOID(env, napi_create_int32(env, NAPI_TYPE_NOMAL, &nNomal));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, value, "ADMIN_TYPE_NORMAL", nNomal));
    napi_value nSuper;
    NAPI_CALL_RETURN_VOID(env, napi_create_int32(env, NAPI_TYPE_SUPER, &nSuper));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, value, "ADMIN_TYPE_SUPER", nSuper));
}

napi_value EnterpriseDeviceManagerAddon::ParseString(napi_env env, std::string &param, napi_value args)
{
    napi_status status;
    napi_valuetype valuetype;
    NAPI_CALL(env, napi_typeof(env, args, &valuetype));
    NAPI_ASSERT(env, valuetype == napi_string, "Wrong argument type. String expected.");
    param = GetStringFromNAPI(env, args);
    EDMLOGD("ParseString param = %{public}s.", param.c_str());
    // create result code
    napi_value result;
    status = napi_create_int32(env, NAPI_RETURN_ONE, &result);
    NAPI_ASSERT(env, status == napi_ok, "napi_create_int32 error!");
    return result;
}

napi_value EnterpriseDeviceManagerAddon::DeviceSettingsManagerConstructor(napi_env env, napi_callback_info info)
{
    napi_value jsthis = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, nullptr, nullptr, &jsthis, nullptr));
    return jsthis;
}

/**
 * Promise and async callback
 */
napi_value EnterpriseDeviceManagerAddon::GetDeviceSettingsManager(napi_env env, napi_callback_info info)
{
    EDMLOGI("GetDeviceSettingsManager called");
    size_t argc = ARGS_SIZE_ONE;
    napi_value argv[ARGS_SIZE_ONE] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, NULL, NULL));
    EDMLOGD("GetDeviceSettingsManager argc = [%{public}zu]", argc);

    AsyncGetDeviceSettingsManagerCallbackInfo *asyncCallbackInfo = new AsyncGetDeviceSettingsManagerCallbackInfo {
        .env = env,
        .asyncWork = nullptr,
        .deferred = nullptr
    };
    if (asyncCallbackInfo == nullptr) {
        return nullptr;
    }
    if (argc > (ARGS_SIZE_ONE - CALLBACK_SIZE)) {
        EDMLOGD("GetDeviceSettingsManager asyncCallback.");
        napi_valuetype valuetype = napi_undefined;
        NAPI_CALL(env, napi_typeof(env, argv[ARR_INDEX_ZERO], &valuetype));
        NAPI_ASSERT(env, valuetype == napi_function, "Wrong argument type. Function expected.");
        napi_create_reference(env, argv[ARR_INDEX_ZERO], NAPI_RETURN_ONE, &asyncCallbackInfo->callback);

        napi_value resourceName;
        napi_create_string_latin1(env, "GetDeviceSettingsManager", NAPI_AUTO_LENGTH, &resourceName);
        napi_create_async_work(
            env, nullptr, resourceName, [](napi_env env, void *data) {},
            [](napi_env env, napi_status status, void *data) {
                AsyncGetDeviceSettingsManagerCallbackInfo *asyncCallbackInfo =
                    static_cast<AsyncGetDeviceSettingsManagerCallbackInfo *>(data);
                napi_value result[ARGS_SIZE_TWO] = { 0 };
                napi_value callback = 0;
                napi_value undefined = 0;
                napi_value callResult = 0;
                napi_value m_classDeviceSettingsManager = nullptr;
                napi_get_reference_value(env, g_classDeviceSettingsManager, &m_classDeviceSettingsManager);
                napi_get_undefined(env, &undefined);
                napi_new_instance(env, m_classDeviceSettingsManager, 0, nullptr, &result[ARR_INDEX_ONE]);
                result[ARR_INDEX_ZERO] = CreateUndefined(env);
                napi_get_reference_value(env, asyncCallbackInfo->callback, &callback);
                napi_call_function(env, undefined, callback, ARGS_SIZE_TWO, &result[ARR_INDEX_ZERO], &callResult);
                if (asyncCallbackInfo->callback != nullptr) {
                    napi_delete_reference(env, asyncCallbackInfo->callback);
                }
                napi_delete_async_work(env, asyncCallbackInfo->asyncWork);
                delete asyncCallbackInfo;
                asyncCallbackInfo = nullptr;
            },
            static_cast<void *>(asyncCallbackInfo), &asyncCallbackInfo->asyncWork);
        NAPI_CALL(env, napi_queue_async_work(env, asyncCallbackInfo->asyncWork));
        napi_value result;
        NAPI_CALL(env, napi_create_int32(env, NAPI_RETURN_ONE, &result));
        return result;
    } else {
        napi_deferred deferred;
        napi_value promise;
        NAPI_CALL(env, napi_create_promise(env, &deferred, &promise));
        asyncCallbackInfo->deferred = deferred;

        napi_value resourceName;
        napi_create_string_latin1(env, "GetDeviceSettingsManager", NAPI_AUTO_LENGTH, &resourceName);
        napi_create_async_work(
            env, nullptr, resourceName, [](napi_env env, void *data) {},
            [](napi_env env, napi_status status, void *data) {
                EDMLOGI("begin load DeviceSettingsManager");
                AsyncGetDeviceSettingsManagerCallbackInfo *asyncCallbackInfo =
                    static_cast<AsyncGetDeviceSettingsManagerCallbackInfo *>(data);
                napi_value result;
                napi_value m_classDeviceSettingsManager = nullptr;
                napi_get_reference_value(env, g_classDeviceSettingsManager, &m_classDeviceSettingsManager);
                napi_new_instance(env, m_classDeviceSettingsManager, 0, nullptr, &result);
                napi_resolve_deferred(asyncCallbackInfo->env, asyncCallbackInfo->deferred, result);
                napi_delete_async_work(env, asyncCallbackInfo->asyncWork);
                delete asyncCallbackInfo;
                asyncCallbackInfo = nullptr;
            },
            static_cast<void *>(asyncCallbackInfo), &asyncCallbackInfo->asyncWork);
        napi_queue_async_work(env, asyncCallbackInfo->asyncWork);
        return promise;
    }
}

napi_value EnterpriseDeviceManagerAddon::Init(napi_env env, napi_value exports)
{
    napi_value nAdminType = nullptr;
    NAPI_CALL(env, napi_create_object(env, &nAdminType));
    CreateAdminTypeObject(env, nAdminType);

    napi_property_descriptor property[] = {
        DECLARE_NAPI_FUNCTION("activateAdmin", ActivateAdmin),
        DECLARE_NAPI_FUNCTION("deactivateAdmin", DeactivateAdmin),
        DECLARE_NAPI_FUNCTION("deactivateSuperAdmin", DeactivateSuperAdmin),
        DECLARE_NAPI_FUNCTION("isAdminAppActive", IsAdminAppActive),
        DECLARE_NAPI_FUNCTION("getEnterpriseInfo", GetEnterpriseInfo),
        DECLARE_NAPI_FUNCTION("setEnterpriseInfo", SetEnterpriseInfo),
        DECLARE_NAPI_FUNCTION("isSuperAdmin", IsSuperAdmin),
        DECLARE_NAPI_FUNCTION("getDeviceSettingsManager", GetDeviceSettingsManager),

        DECLARE_NAPI_PROPERTY("AdminType", nAdminType),
    };
    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(property) / sizeof(property[0]), property));

    napi_value m_classDeviceSettingsManager;
    napi_property_descriptor properties[] = {
        DECLARE_NAPI_FUNCTION("setDateTime", SetDateTime),
    };
    NAPI_CALL(env, napi_define_class(env, "DeviceSettingsManager", NAPI_AUTO_LENGTH, DeviceSettingsManagerConstructor,
        nullptr, sizeof(properties) / sizeof(*properties), properties, &m_classDeviceSettingsManager));
    napi_create_reference(env, m_classDeviceSettingsManager, 1, &g_classDeviceSettingsManager);
    return exports;
}

static napi_module g_edmServiceModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = EnterpriseDeviceManagerAddon::Init,
    .nm_modname = "enterpriseDeviceManager",
    .nm_priv = ((void *)0),
    .reserved = { 0 },
};

extern "C" __attribute__((constructor)) void EdmServiceRegister()
{
    napi_module_register(&g_edmServiceModule);
}
