#pragma once

#include "client_event_handler.h"

namespace console_ui {

class ConsoleUI {
public:
    ConsoleUI();
    ~ConsoleUI();

    void run();

private:
    // helpers
    void printHelp();
    void processCommand(const std::string& cmd);
    // commands handlers
    void handleConnect();
    void handleDisconnect();
    void handleShow();
    void handleStatus() const;
    void handleExit();
    void handleHistory() const;
    void handleClearHistory();

    void show();
    void showPushes();
    void showMessages();

    void loading(std::string text);

private:
    std::shared_ptr<PushQueue> push_queue_;
    std::shared_ptr<MsgQueue> msg_queue_;
    std::condition_variable cv_;

    std::shared_ptr<ClientEventHandler> handler_;
    client_lib::ClientApp client_;

    std::unique_ptr<std::jthread> show_thread_;

    std::atomic<bool> show_pushes_ = false;
    std::atomic<bool> show_msg_ = false;
    std::atomic<bool> running_ = false;

    uint32_t id_{1};
};
}  // namespace console_ui
