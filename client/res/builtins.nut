function Include(path) {
    if (!SCRIPT_ENGINE.Include(path)) {
        throw "Failed to include script: " + path;
    }
}

function CreateClient(name) {
    local uuid = SCRIPT_ENGINE.CreateClient(name);

    if (uuid.IsNull()) {
        throw "Failed to create client!";
    }

    if (SCRIPT_ENGINE.UIActive) {
        Sleep(0.25); // Wait for the UI
    }

    return uuid;
}

function DeleteClient(uuid) {
    if (!SCRIPT_ENGINE.DeleteClient(uuid)) {
        throw "Failed to delete client!";

        return false;
    }

    return true;
}

function WaitFor(fun, timeout = 1.0) {
    if(!SCRIPT_ENGINE.WaitFor(fun, timeout)) {
        throw "Script timeout!";

        return false;
    }

    return true;
}

function WaitForMessage(messageType, timeout = 1.0) {
    local retrievedMsg;

    if(!SCRIPT_ENGINE.WaitForMessage(messageType, function(msg) {
        retrievedMsg = msg;
    }, timeout)) {
        throw "Script timeout!";

        return null;
    }

    return retrievedMsg;
}

function WaitForClientMessage(clientUUID, clientMessageType, timeout = 1.0) {
    local retrievedMsg;

    if(!SCRIPT_ENGINE.WaitForClientMessage(clientUUID, clientMessageType, function(msg) {
        retrievedMsg = msg;
    }, timeout)) {
        throw "Script timeout!";

        return null;
    }

    return retrievedMsg;
}

function SendToClient(uuid, msg) {
    SCRIPT_ENGINE.SendToClient(uuid, msg);
}

function RegisterMessageCallback(messageType, fun) {
    SCRIPT_ENGINE.RegisterMessageCallback(messageType, fun);
}

function RegisterClientMessageCallback(clientMessageType, fun) {
    SCRIPT_ENGINE.RegisterClientMessageCallback(clientMessageType, fun);
}
