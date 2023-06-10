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

#define private public
#define protected public
#include "enterprise_device_mgr_ability.h"
#include "iplugin_template.h"
#undef protected
#undef private
#include "enterprise_device_mgr_ability_test.h"
#include <gtest/gtest.h>
#include <string>
#include <vector>
#include "iplugin_template_test.h"
#include "plugin_manager_test.h"
#include "utils.h"

using namespace testing::ext;

namespace OHOS {
namespace EDM {
namespace TEST {
constexpr int32_t TEST_USER_ID = 101;
constexpr int32_t ARRAY_MAP_TESTPLUGIN_POLICYCODE = 13;
constexpr int32_t HANDLE_POLICY_BIFUNCTIONPLG_POLICYCODE = 23;
constexpr int32_t HANDLE_POLICY_JSON_BIFUNCTIONPLG_POLICYCODE = 30;
constexpr int32_t HANDLE_POLICY_BIFUNCTION_UNSAVE_PLG_POLICYCODE = 31;
constexpr int32_t INVALID_POLICYCODE = 123456;
const std::string ADMIN_PACKAGENAME = "com.edm.test.demo";
const std::string EDM_MANAGE_DATETIME_PERMISSION = "ohos.permission.ENTERPRISE_SET_DATETIME";
const std::string EDM_TEST_PERMISSION = "ohos.permission.EDM_TEST_PERMISSION";
const std::string ARRAY_MAP_TESTPLG_POLICYNAME = "ArrayMapTestPlugin";

void EnterpriseDeviceMgrAbilityTest::SetUp()
{
    plugin_ = PLUGIN::ArrayMapTestPlugin::GetPlugin();
    edmMgr_ = EnterpriseDeviceMgrAbility::GetInstance();
    edmMgr_->adminMgr_ = AdminManager::GetInstance();
    edmMgr_->pluginMgr_ = PluginManager::GetInstance();
    edmMgr_->policyMgr_.reset(new (std::nothrow) PolicyManager(DEFAULT_USER_ID));
}

void EnterpriseDeviceMgrAbilityTest::TearDown()
{
    edmMgr_->adminMgr_->instance_.reset();
    edmMgr_->pluginMgr_->instance_.reset();
    edmMgr_->policyMgr_.reset();
    edmMgr_->instance_.clear();
    edmMgr_.clear();
}

// Give testAdmin and plugin_ Initial value
void EnterpriseDeviceMgrAbilityTest::PrepareBeforeHandleDevicePolicy()
{
    Admin testAdmin;
    testAdmin.adminInfo_.packageName_ = ADMIN_PACKAGENAME;
    testAdmin.adminInfo_.permission_ = {EDM_MANAGE_DATETIME_PERMISSION};
    std::vector<std::shared_ptr<Admin>> adminVec = {std::make_shared<Admin>(testAdmin)};
    edmMgr_->adminMgr_->admins_.
        insert(std::pair<int32_t, std::vector<std::shared_ptr<Admin>>>(DEFAULT_USER_ID, adminVec));
    plugin_->permission_ = EDM_MANAGE_DATETIME_PERMISSION;
    edmMgr_->pluginMgr_->AddPlugin(plugin_);
}

/**
 * @tc.name: HandleDevicePolicyFuncTest001
 * @tc.desc: Test EnterpriseDeviceMgrAbility::HandleDevicePolicy function.(return ERR_OK)
 * @tc.desc: Test whether HandleDevicePolicy runs to the end
 * @tc.type: FUNC
 * @tc.require: issueI5PBT1
 */
HWTEST_F(EnterpriseDeviceMgrAbilityTest, HandleDevicePolicyFuncTest001, TestSize.Level1)
{
    PrepareBeforeHandleDevicePolicy();
    const char* permissions[] = {EDM_MANAGE_DATETIME_PERMISSION.c_str()};
    Utils::SetNativeTokenTypeAndPermissions(permissions, sizeof(permissions) / sizeof(permissions[0]));
    uint32_t code = POLICY_FUNC_CODE((std::uint32_t)FuncOperateType::SET, ARRAY_MAP_TESTPLUGIN_POLICYCODE);
    AppExecFwk::ElementName elementName;
    elementName.SetBundleName(ADMIN_PACKAGENAME);
    MessageParcel data;
    MessageParcel reply;
    ErrCode res = edmMgr_->HandleDevicePolicy(code, elementName, data, reply, DEFAULT_USER_ID);
    ASSERT_TRUE(res == ERR_OK);
    Utils::ResetTokenTypeAndUid();
}
/**
 * @tc.name: HandleDevicePolicyFuncTest002
 * @tc.desc: Test EnterpriseDeviceMgrAbility::HandleDevicePolicy function.(if (deviceAdmin == nullptr))
 * @tc.desc: Test if deviceAdmin is empty
 * @tc.type: FUNC
 * @tc.require: issueI5PBT1
 */
HWTEST_F(EnterpriseDeviceMgrAbilityTest, HandleDevicePolicyFuncTest002, TestSize.Level1)
{
    PrepareBeforeHandleDevicePolicy();
    uint32_t code = POLICY_FUNC_CODE((std::uint32_t)FuncOperateType::SET, ARRAY_MAP_TESTPLUGIN_POLICYCODE);
    AppExecFwk::ElementName elementName;
    elementName.SetBundleName("com.edm.test.demoFail");
    MessageParcel data;
    MessageParcel reply;
    ErrCode res = edmMgr_->HandleDevicePolicy(code, elementName, data, reply, DEFAULT_USER_ID);
    ASSERT_TRUE(res == EdmReturnErrCode::ADMIN_INACTIVE);
}

/**
 * @tc.name: HandleDevicePolicyFuncTest003
 * @tc.desc: Test EnterpriseDeviceMgrAbility::HandleDevicePolicy function.(if (plugin == nullptr))
 * @tc.desc: Test if plugin is empty
 * @tc.type: FUNC
 * @tc.require: issueI5PBT1
 */
HWTEST_F(EnterpriseDeviceMgrAbilityTest, HandleDevicePolicyFuncTest003, TestSize.Level1)
{
    PrepareBeforeHandleDevicePolicy();
    uint32_t code = POLICY_FUNC_CODE((std::uint32_t)FuncOperateType::SET, INVALID_POLICYCODE);
    AppExecFwk::ElementName elementName;
    elementName.SetBundleName(ADMIN_PACKAGENAME);
    MessageParcel data;
    MessageParcel reply;
    ErrCode res = edmMgr_->HandleDevicePolicy(code, elementName, data, reply, DEFAULT_USER_ID);
    ASSERT_TRUE(res == EdmReturnErrCode::INTERFACE_UNSUPPORTED);
}

/**
 * @tc.name: HandleDevicePolicyFuncTest004
 * @tc.desc: Test EnterpriseDeviceMgrAbility::HandleDevicePolicy function.
 * @tc.desc: if (!deviceAdmin->CheckPermission(plugin->GetPermission()))
 * @tc.type: FUNC
 * @tc.require: issueI5PBT1
 */
HWTEST_F(EnterpriseDeviceMgrAbilityTest, HandleDevicePolicyFuncTest004, TestSize.Level1)
{
    PrepareBeforeHandleDevicePolicy();
    Admin testAdmin;
    testAdmin.adminInfo_.packageName_ = ADMIN_PACKAGENAME;
    std::vector<std::shared_ptr<Admin>> adminVec = {std::make_shared<Admin>(testAdmin)};
    edmMgr_->adminMgr_->admins_.clear();
    edmMgr_->adminMgr_->admins_.
        insert(std::pair<int32_t, std::vector<std::shared_ptr<Admin>>>(DEFAULT_USER_ID, adminVec));
    uint32_t code = POLICY_FUNC_CODE((std::uint32_t)FuncOperateType::SET, ARRAY_MAP_TESTPLUGIN_POLICYCODE);
    AppExecFwk::ElementName elementName;
    elementName.SetBundleName(ADMIN_PACKAGENAME);
    MessageParcel data;
    MessageParcel reply;
    ErrCode res = edmMgr_->HandleDevicePolicy(code, elementName, data, reply, DEFAULT_USER_ID);
    ASSERT_TRUE(res == EdmReturnErrCode::ADMIN_EDM_PERMISSION_DENIED);
}

/**
 * @tc.name: HandleDevicePolicyFuncTest005
 * @tc.desc: Test EnterpriseDeviceMgrAbility::HandleDevicePolicy function.
 * @tc.desc: if (!VerifyCallingPermission(plugin->GetPermission()))
 * @tc.type: FUNC
 * @tc.require: issueI5PBT1
 */
HWTEST_F(EnterpriseDeviceMgrAbilityTest, HandleDevicePolicyFuncTest005, TestSize.Level1)
{
    PrepareBeforeHandleDevicePolicy();
    Admin testAdmin;
    testAdmin.adminInfo_.packageName_ = ADMIN_PACKAGENAME;
    testAdmin.adminInfo_.permission_ = {EDM_TEST_PERMISSION};
    std::vector<std::shared_ptr<Admin>> adminVec = {std::make_shared<Admin>(testAdmin)};
    edmMgr_->adminMgr_->admins_.clear();
    edmMgr_->adminMgr_->admins_.
        insert(std::pair<int32_t, std::vector<std::shared_ptr<Admin>>>(DEFAULT_USER_ID, adminVec));
    plugin_->permission_ = EDM_TEST_PERMISSION;
    edmMgr_->pluginMgr_->pluginsCode_.clear();
    edmMgr_->pluginMgr_->pluginsName_.clear();
    edmMgr_->pluginMgr_->AddPlugin(plugin_);
    uint32_t code = POLICY_FUNC_CODE((std::uint32_t)FuncOperateType::SET, ARRAY_MAP_TESTPLUGIN_POLICYCODE);
    AppExecFwk::ElementName elementName;
    elementName.SetBundleName(ADMIN_PACKAGENAME);
    MessageParcel data;
    MessageParcel reply;
    ErrCode res = edmMgr_->HandleDevicePolicy(code, elementName, data, reply, DEFAULT_USER_ID);
    ASSERT_TRUE(res == EdmReturnErrCode::PERMISSION_DENIED);
}

/**
 * @tc.name: HandleDevicePolicyFuncTest006
 * @tc.desc: Test EnterpriseDeviceMgrAbility::HandleDevicePolicy function.
 * @tc.desc: plugin->OnHandlePolicy(code, data, reply, policyValue, isChanged)
 * @tc.desc: Test the result of plugin->OnHandlePolicy is not OK
 * @tc.type: FUNC
 * @tc.require: issueI5PBT1
 */
HWTEST_F(EnterpriseDeviceMgrAbilityTest, HandleDevicePolicyFuncTest006, TestSize.Level1)
{
    PrepareBeforeHandleDevicePolicy();
    const char* permissions[] = {EDM_MANAGE_DATETIME_PERMISSION.c_str()};
    Utils::SetNativeTokenTypeAndPermissions(permissions, sizeof(permissions) / sizeof(permissions[0]));
    plugin_ = PLUGIN::HandlePolicyBiFunctionPlg::GetPlugin();
    plugin_->permission_ = EDM_MANAGE_DATETIME_PERMISSION;
    edmMgr_->pluginMgr_->pluginsCode_.clear();
    edmMgr_->pluginMgr_->pluginsName_.clear();
    edmMgr_->pluginMgr_->AddPlugin(plugin_);
    uint32_t code = POLICY_FUNC_CODE((std::uint32_t)FuncOperateType::SET, HANDLE_POLICY_BIFUNCTIONPLG_POLICYCODE);
    AppExecFwk::ElementName elementName;
    elementName.SetBundleName(ADMIN_PACKAGENAME);
    MessageParcel data;
    MessageParcel reply;
    data.WriteString("ErrorData");
    ErrCode res = edmMgr_->HandleDevicePolicy(code, elementName, data, reply, DEFAULT_USER_ID);
    ASSERT_TRUE(res == ERR_EDM_OPERATE_JSON);
    Utils::ResetTokenTypeAndUid();
}

/**
 * @tc.name: HandleDevicePolicyFuncTest007
 * @tc.desc: Test EnterpriseDeviceMgrAbility::HandleDevicePolicy function.(if (plugin->NeedSavePolicy() && isChanged))
 * @tc.desc: Test run into the branch if (plugin ->NeedSavePolicy() && isChanged)
 * @tc.type: FUNC
 * @tc.require: issueI5PBT1
 */
HWTEST_F(EnterpriseDeviceMgrAbilityTest, HandleDevicePolicyFuncTest007, TestSize.Level1)
{
    PrepareBeforeHandleDevicePolicy();
    const char* permissions[] = {EDM_MANAGE_DATETIME_PERMISSION.c_str()};
    Utils::SetNativeTokenTypeAndPermissions(permissions, sizeof(permissions) / sizeof(permissions[0]));
    plugin_ = PLUGIN::HandlePolicyBiFunctionPlg::GetPlugin();
    plugin_->permission_ = EDM_MANAGE_DATETIME_PERMISSION;
    edmMgr_->pluginMgr_->pluginsCode_.clear();
    edmMgr_->pluginMgr_->pluginsName_.clear();
    edmMgr_->pluginMgr_->AddPlugin(plugin_);
    uint32_t code = POLICY_FUNC_CODE((std::uint32_t)FuncOperateType::SET, HANDLE_POLICY_BIFUNCTIONPLG_POLICYCODE);
    AppExecFwk::ElementName elementName;
    elementName.SetBundleName(ADMIN_PACKAGENAME);
    MessageParcel data;
    MessageParcel reply;
    data.WriteString("testValue");
    ErrCode res = edmMgr_->HandleDevicePolicy(code, elementName, data, reply, DEFAULT_USER_ID);
    ASSERT_TRUE(res == ERR_OK);
    res = edmMgr_->policyMgr_->SetPolicy(ADMIN_PACKAGENAME, plugin_->GetPolicyName(), "", "");
    ASSERT_TRUE(res == ERR_OK);

    plugin_ = PLUGIN::HandlePolicyBiFunctionUnsavePlg::GetPlugin();
    plugin_->permission_ = EDM_MANAGE_DATETIME_PERMISSION;
    edmMgr_->pluginMgr_->pluginsCode_.clear();
    edmMgr_->pluginMgr_->pluginsName_.clear();
    edmMgr_->pluginMgr_->AddPlugin(plugin_);
    code = POLICY_FUNC_CODE((std::uint32_t)FuncOperateType::SET, HANDLE_POLICY_BIFUNCTION_UNSAVE_PLG_POLICYCODE);
    data.WriteString("{\"name\" : \"testValue\"}");
    res = edmMgr_->HandleDevicePolicy(code, elementName, data, reply, DEFAULT_USER_ID);
    ASSERT_TRUE(res == ERR_OK);
    Utils::ResetTokenTypeAndUid();
}

/**
 * @tc.name: HandleDevicePolicyFuncTest008
 * @tc.desc: Test EnterpriseDeviceMgrAbility::HandleDevicePolicy function.
 * @tc.desc: run into plugin->MergePolicyData(admin.GetBundleName(), mergedPolicy)
 * @tc.desc: Test the MergePolicyData processing result is not OK
 * @tc.type: FUNC
 * @tc.require: issueI5PBT1
 */
HWTEST_F(EnterpriseDeviceMgrAbilityTest, HandleDevicePolicyFuncTest008, TestSize.Level1)
{
    PrepareBeforeHandleDevicePolicy();
    const char* permissions[] = {EDM_MANAGE_DATETIME_PERMISSION.c_str()};
    Utils::SetNativeTokenTypeAndPermissions(permissions, sizeof(permissions) / sizeof(permissions[0]));
    plugin_ = PLUGIN::HandlePolicyJsonBiFunctionPlg::GetPlugin();
    plugin_->permission_ = EDM_MANAGE_DATETIME_PERMISSION;
    edmMgr_->pluginMgr_->pluginsCode_.clear();
    edmMgr_->pluginMgr_->pluginsName_.clear();
    edmMgr_->pluginMgr_->AddPlugin(plugin_);
    AdminValueItemsMap map;
    std::string errJsonStr = "v1,v2v3??v4"; // Enter a string that cannot be parsed by JSON
    map.insert(std::pair<std::string, std::string>(ADMIN_PACKAGENAME, errJsonStr));
    map.insert(std::pair<std::string, std::string>("com.edm.test.demo2", errJsonStr));
    edmMgr_->policyMgr_->policyAdmins_.
        insert(std::pair<std::string, AdminValueItemsMap>("HandlePolicyJsonBiFunctionPlg", map));
    uint32_t code = POLICY_FUNC_CODE((std::uint32_t)FuncOperateType::SET,
        HANDLE_POLICY_JSON_BIFUNCTIONPLG_POLICYCODE);
    AppExecFwk::ElementName elementName;
    elementName.SetBundleName(ADMIN_PACKAGENAME);
    MessageParcel data;
    MessageParcel reply;
    data.WriteString("{\"name\" : \"testValue\"}");
    ErrCode res = edmMgr_->HandleDevicePolicy(code, elementName, data, reply, DEFAULT_USER_ID);
    ASSERT_TRUE(res == ERR_OK);
    res = edmMgr_->policyMgr_->SetPolicy(ADMIN_PACKAGENAME, plugin_->GetPolicyName(), "", "");
    ASSERT_TRUE(res == ERR_OK);
    Utils::ResetTokenTypeAndUid();
}

/**
 * @tc.name: GetDevicePolicyFuncTest001
 * @tc.desc: Test EnterpriseDeviceMgrAbility::GetDevicePolicy function.
 * @tc.desc: Test if (plugin == nullptr)
 * @tc.type: FUNC
 * @tc.require: issueI5PBT1
 */
HWTEST_F(EnterpriseDeviceMgrAbilityTest, GetDevicePolicyFuncTest001, TestSize.Level1)
{
    uint32_t code = POLICY_FUNC_CODE((std::uint32_t)FuncOperateType::SET, INVALID_POLICYCODE);
    MessageParcel data;
    MessageParcel reply;
    data.WriteInt32(1);
    ErrCode res = edmMgr_->GetDevicePolicy(code, data, reply, DEFAULT_USER_ID);
    ASSERT_TRUE(res == EdmReturnErrCode::INTERFACE_UNSUPPORTED);
}

/**
 * @tc.name: GetDevicePolicyFuncTest002
 * @tc.desc: Test EnterpriseDeviceMgrAbility::GetDevicePolicy function.
 * @tc.desc: Test if admin == nullptr
 * @tc.type: FUNC
 * @tc.require: issueI5PBT1
 */
HWTEST_F(EnterpriseDeviceMgrAbilityTest, GetDevicePolicyFuncTest002, TestSize.Level1)
{
    const char* permissions[] = {EDM_MANAGE_DATETIME_PERMISSION.c_str()};
    Utils::SetNativeTokenTypeAndPermissions(permissions, sizeof(permissions) / sizeof(permissions[0]));
    uint32_t code = POLICY_FUNC_CODE((std::uint32_t)FuncOperateType::SET, ARRAY_MAP_TESTPLUGIN_POLICYCODE);
    MessageParcel data;
    MessageParcel reply;
    data.WriteInt32(1);
    plugin_->permission_ = EDM_MANAGE_DATETIME_PERMISSION;
    edmMgr_->pluginMgr_->AddPlugin(plugin_);
    ErrCode res = edmMgr_->GetDevicePolicy(code, data, reply, DEFAULT_USER_ID);
    ASSERT_TRUE(res == ERR_OK);
    Utils::ResetTokenTypeAndUid();
}

/**
 * @tc.name: GetDevicePolicyFuncTest003
 * @tc.desc: Test EnterpriseDeviceMgrAbility::GetDevicePolicy function.
 * @tc.desc: Test if admin != nullptr && deviceAdmin == nullptr
 * @tc.type: FUNC
 * @tc.require: issueI5PBT1
 */
HWTEST_F(EnterpriseDeviceMgrAbilityTest, GetDevicePolicyFuncTest003, TestSize.Level1)
{
    uint32_t code = POLICY_FUNC_CODE((std::uint32_t)FuncOperateType::SET, ARRAY_MAP_TESTPLUGIN_POLICYCODE);
    plugin_->permission_ = EDM_MANAGE_DATETIME_PERMISSION;
    edmMgr_->pluginMgr_->AddPlugin(plugin_);
    AppExecFwk::ElementName admin;
    admin.SetBundleName("com.test.demoFail");
    MessageParcel data;
    MessageParcel reply;
    data.WriteInt32(0);
    data.WriteParcelable(&admin);
    ErrCode res = edmMgr_->GetDevicePolicy(code, data, reply, DEFAULT_USER_ID);
    ASSERT_TRUE(res == EdmReturnErrCode::ADMIN_INACTIVE);
}

/**
 * @tc.name: GetDevicePolicyFuncTest004
 * @tc.desc: Test EnterpriseDeviceMgrAbility::GetDevicePolicy function.
 * @tc.desc: Test if admin != nullptr && (deviceAdmin->CheckPermission fail)
 * @tc.type: FUNC
 * @tc.require: issueI5PBT1
 */
HWTEST_F(EnterpriseDeviceMgrAbilityTest, GetDevicePolicyFuncTest004, TestSize.Level1)
{
    PrepareBeforeHandleDevicePolicy();
    plugin_->permission_ = EDM_TEST_PERMISSION;
    edmMgr_->pluginMgr_->AddPlugin(plugin_);
    uint32_t code = POLICY_FUNC_CODE((std::uint32_t)FuncOperateType::SET, ARRAY_MAP_TESTPLUGIN_POLICYCODE);
    AppExecFwk::ElementName admin;
    admin.SetBundleName(ADMIN_PACKAGENAME);
    MessageParcel data;
    MessageParcel reply;
    data.WriteInt32(0);
    data.WriteParcelable(&admin);
    ErrCode res = edmMgr_->GetDevicePolicy(code, data, reply, DEFAULT_USER_ID);
    ASSERT_TRUE(res == EdmReturnErrCode::ADMIN_EDM_PERMISSION_DENIED);
}

/**
 * @tc.name: GetDevicePolicyFuncTest005
 * @tc.desc: Test EnterpriseDeviceMgrAbility::GetDevicePolicy function.
 * @tc.desc: Test if admin != nullptr && (if (!VerifyCallingPermission(plugin->GetPermission())))
 * @tc.type: FUNC
 * @tc.require: issueI5PBT1
 */
HWTEST_F(EnterpriseDeviceMgrAbilityTest, GetDevicePolicyFuncTest005, TestSize.Level1)
{
    Admin testAdmin;
    testAdmin.adminInfo_.packageName_ = ADMIN_PACKAGENAME;
    testAdmin.adminInfo_.permission_ = {EDM_TEST_PERMISSION};
    std::vector<std::shared_ptr<Admin>> adminVec = {std::make_shared<Admin>(testAdmin)};
    edmMgr_->adminMgr_->admins_.
        insert(std::pair<int32_t, std::vector<std::shared_ptr<Admin>>>(DEFAULT_USER_ID, adminVec));
    plugin_->permission_ = EDM_TEST_PERMISSION;
    edmMgr_->pluginMgr_->AddPlugin(plugin_);
    uint32_t code = POLICY_FUNC_CODE((std::uint32_t)FuncOperateType::SET, ARRAY_MAP_TESTPLUGIN_POLICYCODE);
    AppExecFwk::ElementName admin;
    admin.SetBundleName(ADMIN_PACKAGENAME);
    MessageParcel data;
    MessageParcel reply;
    data.WriteInt32(0);
    data.WriteParcelable(&admin);
    ErrCode res = edmMgr_->GetDevicePolicy(code, data, reply, DEFAULT_USER_ID);
    ASSERT_TRUE(res == EdmReturnErrCode::PERMISSION_DENIED);
}

/**
 * @tc.name: GetDevicePolicyFuncTest006
 * @tc.desc: Test EnterpriseDeviceMgrAbility::GetDevicePolicy function.
 * @tc.desc: Test if (plugin->NeedSavePolicy())
 * @tc.type: FUNC
 * @tc.require: issueI5PBT1
 */
HWTEST_F(EnterpriseDeviceMgrAbilityTest, GetDevicePolicyFuncTest006, TestSize.Level1)
{
    const char* permissions[] = {EDM_MANAGE_DATETIME_PERMISSION.c_str()};
    Utils::SetNativeTokenTypeAndPermissions(permissions, sizeof(permissions) / sizeof(permissions[0]));
    PrepareBeforeHandleDevicePolicy();
    plugin_ = PLUGIN::HandlePolicyBiFunctionUnsavePlg::GetPlugin();
    plugin_->permission_ = EDM_MANAGE_DATETIME_PERMISSION;
    edmMgr_->pluginMgr_->pluginsCode_.clear();
    edmMgr_->pluginMgr_->pluginsName_.clear();
    edmMgr_->pluginMgr_->AddPlugin(plugin_);
    uint32_t code = POLICY_FUNC_CODE((std::uint32_t)FuncOperateType::SET,
        HANDLE_POLICY_BIFUNCTION_UNSAVE_PLG_POLICYCODE);
    AppExecFwk::ElementName admin;
    admin.SetBundleName(ADMIN_PACKAGENAME);
    MessageParcel data;
    MessageParcel reply;
    data.WriteInt32(0);
    data.WriteParcelable(&admin);
    ErrCode res = edmMgr_->GetDevicePolicy(code, data, reply, DEFAULT_USER_ID);
    ASSERT_TRUE(res == ERR_OK);
    Utils::ResetTokenTypeAndUid();
}

/**
 * @tc.name: GetAndSwitchPolicyManagerByUserId
 * @tc.desc: Test EnterpriseDeviceMgrAbility::GetAndSwitchPolicyManagerByUserId function.
 * @tc.desc: Test if policyMgrMap_ is empty
 * @tc.type: FUNC
 * @tc.require: issueI6QU75
 */
HWTEST_F(EnterpriseDeviceMgrAbilityTest, GetAndSwitchPolicyManagerByUserIdTest001, TestSize.Level1)
{
    edmMgr_->policyMgr_ = edmMgr_->GetAndSwitchPolicyManagerByUserId(DEFAULT_USER_ID);
    ASSERT_TRUE(edmMgr_->policyMgr_.get() == IPolicyManager::policyManagerInstance_);
    ASSERT_TRUE(IPolicyManager::policyManagerInstance_ == edmMgr_->policyMgrMap_[DEFAULT_USER_ID].get());
    ASSERT_TRUE(edmMgr_->policyMgrMap_.size() > 0);
}

/**
 * @tc.name: GetAndSwitchPolicyManagerByUserId
 * @tc.desc: Test EnterpriseDeviceMgrAbility::GetAndSwitchPolicyManagerByUserId function.
 * @tc.desc: Test if policyMgrMap_ is not empty.
 * @tc.type: FUNC
 * @tc.require: issueI6QU75
 */
HWTEST_F(EnterpriseDeviceMgrAbilityTest, GetAndSwitchPolicyManagerByUserIdTest002, TestSize.Level1)
{
    edmMgr_->policyMgrMap_.insert(std::pair<std::uint32_t, std::shared_ptr<PolicyManager>>(DEFAULT_USER_ID,
        edmMgr_->policyMgr_));
    PolicyManager* defaultPolcyMgr = edmMgr_->policyMgr_.get();
    std::shared_ptr<PolicyManager> guestPolicyMgr;
    guestPolicyMgr.reset(new (std::nothrow) PolicyManager(TEST_USER_ID));
    edmMgr_->policyMgrMap_.insert(std::pair<std::uint32_t, std::shared_ptr<PolicyManager>>(TEST_USER_ID,
        guestPolicyMgr));
    edmMgr_->policyMgr_ = edmMgr_->GetAndSwitchPolicyManagerByUserId(TEST_USER_ID);
    ASSERT_TRUE(edmMgr_->policyMgr_.get() == IPolicyManager::policyManagerInstance_);
    ASSERT_TRUE(IPolicyManager::policyManagerInstance_ == edmMgr_->policyMgrMap_[TEST_USER_ID].get());
    ASSERT_TRUE(edmMgr_->policyMgr_.get() != defaultPolcyMgr);
}
} // namespace TEST
} // namespace EDM
} // namespace OHOS