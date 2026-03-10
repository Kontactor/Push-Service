#pragma once

#include <string>

enum class PushType { Notification, Warning, Error };

enum class PushPriority { Normal, High, Critical };

class IPush {
public:
    PushType type;
    PushPriority priority;
    std::string payload;
    std::string sender;

    IPush(PushType t, PushPriority p, std::string pld, std::string sndr) :
        type(t), priority(p), payload(std::move(pld)), sender(std::move(sndr)) {}
};

// imitation of client interface
class IClient {
public:
    virtual void SendPush(std::shared_ptr<IPush> push) = 0;
    virtual std::vector<IPush> GetReceivedPushes() const = 0;
    virtual int GetId() const = 0;
    virtual ~IClient() = default;
};
