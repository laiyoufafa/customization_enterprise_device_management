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

#ifndef SERVICES_EDM_INCLUDE_ADMIN_MANAGER_H_
#define SERVICES_EDM_INCLUDE_ADMIN_MANAGER_H_

#include <map>
#include <memory>
#include <unordered_map>
#include "admin.h"
#include "ent_info.h"
#include "edm_permission.h"
#include "json/json.h"
#include "permission_manager.h"

namespace OHOS {
namespace EDM {
class AdminManager : public std::enable_shared_from_this<AdminManager> {
public:
    static std::shared_ptr<AdminManager> GetInstance();
    ErrCode GetReqPermission(const std::vector<std::string> &permissions, std::vector<EdmPermission> &edmPermissions);
    void GetAllAdmin(std::vector<std::shared_ptr<Admin>> &allAdmin, int32_t userId);
    std::shared_ptr<Admin> GetAdminByPkgName(const std::string &packageName, int32_t userId);
    ErrCode DeleteAdmin(const std::string &packageName, int32_t userId);
    ErrCode UpdateAdmin(AppExecFwk::AbilityInfo &abilityInfo, const std::vector<std::string> &permissions,
        int32_t userId);
    ErrCode GetGrantedPermission(AppExecFwk::AbilityInfo &abilityInfo, std::vector<std::string> &permissions,
        AdminType type);
    bool IsSuperAdminExist(int32_t userId);
    void GetActiveAdmin(AdminType role, std::vector<std::string> &packageNameList, int32_t userId);
    void Init();
    void RestoreAdminFromFile();
    ErrCode SetAdminValue(AppExecFwk::AbilityInfo &abilityInfo, EntInfo &entInfo, AdminType role,
        std::vector<std::string> &permissions, int32_t userId);
    ErrCode GetEntInfo(const std::string &packageName, EntInfo &entInfo, int32_t userId);
    ErrCode SetEntInfo(const std::string &packageName, EntInfo &entInfo, int32_t userId);
    virtual ~AdminManager();
    
private:
    AdminManager();
    bool GetAdminByUserId(int32_t userId, std::vector<std::shared_ptr<Admin>> &userAdmin);
    void SaveAdmin(int32_t userId);
    void ReadJsonAdminType(Json::Value &admin, std::vector<std::shared_ptr<Admin>> &adminVector);
    void ReadJsonAdmin(const std::string &filePath, int32_t userId);
    void WriteJsonAdminType(std::shared_ptr<Admin> &activeAdmin, Json::Value &tree);
    void WriteJsonAdmin(const std::string &filePath, int32_t userId);

    std::unordered_map<int32_t, std::vector<std::shared_ptr<Admin>>> admins_;
    static std::mutex mutexLock_;
    static std::shared_ptr<AdminManager> instance_;
};
} // namespace EDM
} // namespace OHOS

#endif // SERVICES_EDM_INCLUDE_ADMIN_MANAGER_H_
