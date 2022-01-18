/**
 * @file libclient/src/MessageAccountDump.cpp
 * @ingroup libclient
 *
 * @author COMP Omega <compomega@tutanota.com>
 *
 * @brief Client message.
 *
 * This file is part of the COMP_hack Client Library (libclient).
 *
 * Copyright (C) 2012-2021 COMP_hack Team <compomega@tutanota.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "MessageAccountDump.h"

#include <BaseScriptEngine.h>

namespace libcomp {
template <>
BaseScriptEngine &BaseScriptEngine::Using<logic::MessageAccountDump>() {
  if (!BindingExists("logic.MessageAccountDump")) {
    Using<Message::MessageClient>();

    Sqrat::DerivedClass<logic::MessageAccountDump, Message::MessageClient>
        binding(mVM, "logic.MessageAccountDump");
    Bind("logic.MessageAccountDump", binding);

    binding.Func("GetPath", &logic::MessageAccountDump::GetPath)
        .Prop("Path", &logic::MessageAccountDump::GetPath);
  }

  return *this;
}

template <>
BaseScriptEngine &BaseScriptEngine::Using<logic::MessageAccountDumpStatus>() {
  if (!BindingExists("logic.MessageAccountDumpStatus")) {
    Using<Message::MessageClient>();

    Sqrat::DerivedClass<logic::MessageAccountDumpStatus, Message::MessageClient>
        binding(mVM, "logic.MessageAccountDumpStatus");
    Bind("logic.MessageAccountDumpStatus", binding);

    binding.Func("IsSuccess", &logic::MessageAccountDumpStatus::IsSuccess)
        .Prop("Success", &logic::MessageAccountDumpStatus::IsSuccess);
  }

  return *this;
}
}  // namespace libcomp
