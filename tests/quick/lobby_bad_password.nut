uuid <- CreateClient("test client 1");
SendToClient(uuid, logic.MessageConnectToLobby(uuid, "testalpha", "wrong",
    1666, "lobby", "127.0.0.1", 10666, UUID()));
msg <- WaitForClientMessage(uuid, ClientMessageType.CONNECTED_TO_LOBBY);
assert(msg.ErrorCode == -5); /// @todo add enum for the error codes? (BAD_USERNAME_PASSWORD)
