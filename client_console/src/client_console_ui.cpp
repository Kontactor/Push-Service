#include "client_console_ui.h"

#include <iostream>

namespace console_ui {

const std::string HELP =
    "Commands:\n"
    "  connect     - connect to server\n"
    "  show        - show receiving pushes\n"
    "  status      - show connection status\n"
    "  history     - show push history\n"
    "  clear       - clear push history\n"
    "  disconnect  - disconnect from server\n"
    "  exit        - quit\n"
    "  help        - show available commands\n";

// ***************************** ConsoleUI methods ********************************
ConsoleUI::ConsoleUI() :
    push_queue_(std::make_shared<PushQueue>()), msg_queue_(std::make_shared<MsgQueue>()),
    handler_(std::make_shared<ClientEventHandler>(push_queue_, msg_queue_)), client_(handler_) {}

ConsoleUI::~ConsoleUI() {
    if (running_) {
        handleExit();
    }
}

void ConsoleUI::run() {
    client_.runClientAppLoop();
    running_ = true;
    show();
    printHelp();

    while (running_) {
        print("> ");

        std::string cmd;
        std::getline(std::cin, cmd);
        if (show_pushes_) {
            show_pushes_ = false;
        }

        show_msg_ = false;

        if (!cmd.empty()) {
            processCommand(cmd);
        }
        show_msg_ = true;
    }
}

void ConsoleUI::printHelp() {
    print(HELP);
}

void ConsoleUI::processCommand(const std::string& cmd) {
    if (cmd == "connect") {
        handleConnect();
    } else if (cmd == "show") {
        handleShow();
    } else if (cmd == "status") {
        handleStatus();
    } else if (cmd == "history") {
        handleHistory();
    } else if (cmd == "clear") {
        handleClearHistory();
    } else if (cmd == "disconnect") {
        handleDisconnect();
    } else if (cmd == "exit") {
        handleExit();
    } else if (cmd == "help") {
        printHelp();
    } else {
        printMsg(ConsoleMsgType::Error, "Unknown command. Type 'help'");
    }
}

void ConsoleUI::handleConnect() {
    if (handler_->isConnected()) {
        printMsg(ConsoleMsgType::Error, "Already connected");
        return;
    }

    std::string host;
    std::string port_str;

    print("Server IP: ");
    std::getline(std::cin, host);

    if (host.empty()) {
        printMsg(ConsoleMsgType::Error, "Invalid IP address");
        return;
    }

    print("Port: ");
    std::getline(std::cin, port_str);

    int port = 0;
    try {
        port = std::stoi(port_str);
    } catch (...) {
        printMsg(ConsoleMsgType::Error, "Port must be a number");
        return;
    }

    if (port <= 0 || port > 65535) {
        printMsg(ConsoleMsgType::Error, "Port must be in range 1 - 65535");
        return;
    }
    client_.configure(host, port_str);

    client_.connect();

    std::string connect_str = std::format("Connecting to {} ", client_.connectionInfo());
    loading(connect_str);
}

void ConsoleUI::handleDisconnect() {
    if (!handler_->isConnected()) {
        printMsg(ConsoleMsgType::Error, "Not connected");
        return;
    }
    client_.disconnect();

    std::string disconnect_str = std::format("Disconnecting from {} ", client_.connectionInfo());
    loading(disconnect_str);

    push_queue_->clear();
}

void ConsoleUI::handleShow() {
    if (handler_->isConnected()) {
        show_pushes_ = true;
    } else {
        printMsg(ConsoleMsgType::System, "You're now not connected");
    }
}

void ConsoleUI::handleStatus() const {
    std::string connection_info;
    if (handler_->isConnected()) {
        connection_info = "You're now connected to: " + client_.connectionInfo();
    } else {
        connection_info = "You're now not connected";
    };
    printMsg(ConsoleMsgType::System, connection_info);
}

void ConsoleUI::handleExit() {
    client_.stopClientLoop();
    running_ = false;

    printMsg(ConsoleMsgType::System, "Now exiting, Bye bye!");
}

void ConsoleUI::handleHistory() const {
    if (!handler_->historySize()) {
        printMsg(ConsoleMsgType::System, "No push history");
        return;
    }

    for (const auto& push_ptr : handler_->getHistory()) {
        printWithColor("[HISTORY] " + formatPush(*push_ptr), colorForPriority(push_ptr->priority));
        printNewLine();
    }
}

void ConsoleUI::handleClearHistory() {
    std::string clear_res;
    if (handler_->historySize()) {
        handler_->clearHistory();
        clear_res = "History cleared";
    } else {
        clear_res = "No push history";
    }
    printMsg(ConsoleMsgType::System, clear_res);
}

void ConsoleUI::show() {
    if (!show_thread_) {
        show_thread_ = std::make_unique<std::jthread>([this]() {
            while (running_) {
                showMessages();
                showPushes();
            }
        });
    }
}

void ConsoleUI::showPushes() {
    while (show_pushes_ && !push_queue_->empty()) {
        if (!msg_queue_->empty()) {
            break;
        }

        if (!running_) {
            break;
        }

        auto push = push_queue_->front();
        printWithColor(formatPush(*push), colorForPriority(push->priority));
        printNewLine();
        push_queue_->pop();
        printMsg(ConsoleMsgType::Hint, "Press ENTER to begin enter commands");
    }
}

void ConsoleUI::showMessages() {
    while (show_msg_ && !msg_queue_->empty()) {
        if (!running_) {
            break;
        }

        auto msg = msg_queue_->front();
        printMsg(msg.type, msg.msg);
        msg_queue_->pop();
        print("> ");
    }
}

// show loading
void ConsoleUI::loading(std::string text) {
    size_t length = text.length() + sizeof(styleFor(ConsoleMsgType::System).label) + 1;

    printMsg(ConsoleMsgType::System, std::move(text), false);
    hideCursor();

    uint16_t i = 0;
    while (msg_queue_->empty()) {
        showLoading(i++);
    }

    clearLine(length);
    showCursor();
}
}  // namespace console_ui
