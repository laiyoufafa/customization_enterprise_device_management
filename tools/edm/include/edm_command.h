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

#ifndef EDM_TOOLS_EDM_INCLUDE_EDM_COMMAND_H_
#define EDM_TOOLS_EDM_INCLUDE_EDM_COMMAND_H_

#include "shell_command.h"
#include "enterprise_device_mgr_proxy.h"
#include "ienterprise_device_mgr.h"

namespace OHOS {
namespace EDM {
namespace {
const std::string TOOL_NAME = "edm";

const std::string HELP_MSG = "usage: edm <command> <options>\n"
                             "These are common edm commands list:\n"
                             "  help                      list available commands\n"
                             "  activate-admin            activate a admin with options\n"
                             "  activate-super-admin      activate a super admin with options\n"
                             "  deactivate-admin          deactivate a admin with options\n"
                             "  deactivate-super-admin    deactivate a super admin with options\n";
}  // namespace

class EdmCommand : public ShellCommand {
public:
    EdmCommand(int argc, char *argv[]);
    ~EdmCommand() override
    {}

private:
    ErrCode CreateCommandMap() override;
    ErrCode CreateMessageMap() override;
    ErrCode init() override;
    ErrCode RunAsHelpCommand();
    ErrCode RunAsActivateCommand(AdminType type);
    ErrCode RunAsActivateAdminCommand();
    ErrCode RunAsActivateSuperAdminCommand();
    ErrCode RunDeactivateNormalAdminCommand();
    ErrCode RunDeactivateSuperAdminCommand();
    std::vector<std::string> split(const std::string &str, const std::string &pattern);

    std::shared_ptr<EnterpriseDeviceMgrProxy> enterpriseDeviceMgrProxy_;
};
} // namespace EDM
} // namespace OHOS

#endif // EDM_TOOLS_EDM_INCLUDE_EDM_COMMAND_H_
