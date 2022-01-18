/**
 * @file libclient/src/MessageStartGame.cpp
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

#include "MessageStartGame.h"

#include <BaseScriptEngine.h>

namespace libcomp {
template <>
BaseScriptEngine &BaseScriptEngine::Using<logic::MessageRequestStartGame>() {
  if (!BindingExists("logic.MessageRequestStartGame")) {
    Using<Message::MessageClient>();

    Sqrat::DerivedClass<logic::MessageRequestStartGame, Message::MessageClient>
        binding(mVM, "logic.MessageRequestStartGame");
    Bind("logic.MessageRequestStartGame", binding);

    binding
        .Func("GetCharacterID", &logic::MessageRequestStartGame::GetCharacterID)
        .Prop("CharacterID", &logic::MessageRequestStartGame::GetCharacterID);
  }

  return *this;
}
}  // namespace libcomp
