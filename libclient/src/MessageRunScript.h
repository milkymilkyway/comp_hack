/**
 * @file libclient/src/MessageRunScript.h
 * @ingroup libclient
 *
 * @author COMP Omega <compomega@tutanota.com>
 *
 * @brief Client message.
 *
 * This file is part of the COMP_hack Client Library (libclient).
 *
 * Copyright (C) 2012-2020 COMP_hack Team <compomega@tutanota.com>
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

#ifndef LIBCLIENT_SRC_MESSAGERUNSCRIPT_H
#define LIBCLIENT_SRC_MESSAGERUNSCRIPT_H

// libobjgen Includes
#include <UUID.h>

// libcomp Includes
#include <CString.h>
#include <MessageClient.h>

namespace logic {

/**
 * Message requesting to run a script file.
 */
class MessageRunScript : public libcomp::Message::MessageClient {
 public:
  /**
   * Create the message.
   * @param path Path to the script file to run.
   */
  MessageRunScript(const libcomp::String& path)
      : libcomp::Message::MessageClient(NULLUUID), mPath(path) {}

  /**
   * Cleanup the message.
   */
  ~MessageRunScript() override {}

  virtual Message* Clone() const { return new MessageRunScript(*this); }

  /**
   * Get the path to the script file to run.
   * @returns Path to the script file to run.
   */
  libcomp::String GetPath() const { return mPath; }

  /**
   * Get the specific client message type.
   * @return The message's client message type
   */
  libcomp::Message::MessageClientType GetMessageClientType() const override {
    return libcomp::Message::MessageClientType::RUN_SCRIPT;
  }

  /**
   * Dump the message for logging.
   * @return String representation of the message.
   */
  libcomp::String Dump() const override {
    return libcomp::String("Message: Run script\nPath: %1").Arg(mPath);
  }

 protected:
  /// Path to the script file to run.
  libcomp::String mPath;
};

}  // namespace logic

#endif  // LIBCLIENT_SRC_MESSAGERUNSCRIPT_H
