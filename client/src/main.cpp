/**
 * @file client/src/main.cpp
 * @ingroup client
 *
 * @author COMP Omega <compomega@tutanota.com>
 *
 * @brief Main client source file.
 *
 * This file is part of the COMP_hack Test Client (client).
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

// Qt Includes
#include <QApplication>

// cpp-optparse Includes
#include <OptionParser.h>

// libhack Includes
#include <Log.h>

// client Includes
#include "ControlCenter.h"
#include "ScriptWorker.h"

// libclient Includes
#include <MessageRunScript.h>

// Standard C Includes
#include <cstdlib>

int main(int argc, char *argv[]) {
  optparse::OptionParser parser;
  parser.description("Test client for running system tests.");
  parser.usage(
      "%prog [OPTIONS...] SCRIPT\n       %prog --gui [OPTIONS...] [SCRIPT]\n   "
      "    %prog -g [OPTIONS...] [SCRIPT]");
  parser.add_option("-r", "--root")
      .dest("root")
      .help("Path to root client directory (or archive file)");
  parser.add_option("-g", "--gui")
      .action("store_true")
      .dest("gui")
      .set_default("0")
      .help("Run the client in GUI mode");

  optparse::Values options;
  std::vector<std::string> args;

  try {
    options = parser.parse_args(argc, argv);
    args = parser.args();

    if (1 < args.size()) {
      parser.error("only one script file may be specified");
    } else if (!options.get("gui") && args.empty()) {
      parser.error("a script file must be specified in command line mode");
    }
  } catch (int ret) {
    return ret;
  }

  int returnCode = EXIT_SUCCESS;

  libcomp::Exception::RegisterSignalHandler();

  // Enable the log so it prints to the console.
  std::unique_ptr<libcomp::BaseLog> log(libhack::Log::GetSingletonPtr());
  log->AddStandardOutputHook();
  log->SetLogLevel(to_underlying(libcomp::BaseLogComponent_t::ScriptEngine),
                   libcomp::BaseLog::Level_t::LOG_LEVEL_INFO);

  if (options.get("gui")) {
    QApplication app(argc, argv);

    // These settings are used to specify how the settings are stored. On
    // Windows, there settings are stored in the registry at
    // HKEY_CURRENT_USER\Software\COMP_hack\COMP_hack Test Client
    // On Linux, these settings will be stored in the file
    // $HOME/.config/COMP_hack/COMP_hack Test Client.conf
    // Consult the QSettings documentation in the Qt API reference for more
    // information on how the settings work (and where they are on Mac OS X).
    app.setOrganizationName("COMP_hack");
    app.setOrganizationDomain("comp.hack");
    app.setApplicationName("COMP_hack Test Client");

    script::ControlCenter controlCenter;
    controlCenter.show();

    if (!args.empty()) {
      controlCenter.runScriptFromCommandLine(
          QString::fromLocal8Bit(args.front().c_str()));
    }

    // Run the Qt event loop.
    returnCode = app.exec();
  } else {
    auto worker = std::make_shared<script::ScriptWorker>();

    worker->SetScriptCallback([&returnCode](script::ScriptWorker *pWorker,
                                            const libcomp::String &path,
                                            bool result) {
      (void)pWorker;
      (void)path;

      if (!result) {
        returnCode = EXIT_FAILURE;
      }
    });

    worker->Start("script");
    worker->SendToScript(new logic::MessageRunScript(args.front()));
    worker->Shutdown();
    worker->Join();
    worker.reset();
  }

  return returnCode;
}
