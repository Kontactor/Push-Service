#include "client_event_handler.h"

using namespace console_ui;

//  *************** ClientEventHandler methods ****************************
ClientEventHandler::ClientEventHandler(std::shared_ptr<PushQueue> push_queue, std::shared_ptr<MsgQueue> msg_queue) :
    push_queue_(push_queue), msg_queue_(msg_queue) {}

void ClientEventHandler::onPushReceived(uint32_t, const Push& push) {
    auto push_ptr = std::make_shared<Push>(push);
    push_queue_->push(push_ptr);

    {
        std::lock_guard<std::mutex> lock(mtx_);
        session_last_history_.push_back(push_ptr);
    }
}

void ClientEventHandler::onConnected() {
    is_connected_ = true;
    msg_queue_->push({ConsoleMsgType::System, "You have been successfully connected, welcome!"});
}

void ClientEventHandler::onDisconnected() {
    disconnect();
    msg_queue_->push({ConsoleMsgType::System, "Disconnected"});
}

void ClientEventHandler::onReconnect() {
    disconnect();
    msg_queue_->push({ConsoleMsgType::System, "Reconnect in 3 seconds..."});
}

void ClientEventHandler::onError(const std::string& error) {
    msg_queue_->push({ConsoleMsgType::Error, error});
}

bool ClientEventHandler::isConnected() const {
    return is_connected_;
}

size_t ClientEventHandler::historySize() const {
    updateFullHistory();
    return session_full_history_.size();
}

void ClientEventHandler::clearHistory() {
    PushHistory temp_last_history;
    {
        std::lock_guard<std::mutex> lock(mtx_);
        temp_last_history.swap(session_last_history_);
    }
    session_full_history_.clear();
}

const PushHistory& ClientEventHandler::getHistory() const {
    updateFullHistory();
    return session_full_history_;
}

void ClientEventHandler::disconnect() {
    is_connected_ = false;
    updateFullHistory();
    session_last_history_.clear();
}

void ClientEventHandler::updateFullHistory() const {
    PushHistory temp_last_history;
    {
        std::lock_guard<std::mutex> lock(mtx_);
        temp_last_history.swap(session_last_history_);
    }
    session_full_history_.insert(session_full_history_.end(), temp_last_history.begin(), temp_last_history.end());
}
