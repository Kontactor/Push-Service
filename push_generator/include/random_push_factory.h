#pragma once

#include <random>
#include <string>

#include "push.h"

class RandomPushFactory {
public:
    static Push CreateUserPush() {
        return Push{RandomPriority(), RandomCategory(), RandomSource(), RandomText()};
    }

    static Push CreateAdminPush() {
        return Push{PushPriority::Critical, PushCategory::Error, PushSource::Source1, "[ADMIN] System alert"};
    }

private:
    static PushPriority RandomPriority() {
        return static_cast<PushPriority>(rand() % 3);
    }

    static PushCategory RandomCategory() {
        return static_cast<PushCategory>(rand() % 3);
    }

    static PushSource RandomSource() {
        return static_cast<PushSource>(rand() % 3);
    }

    static std::string RandomText() {
        static const std::vector<std::string> texts{"Hello!",
                                                    "Update available",
                                                    "Warning!",
                                                    "Check settings",
                                                    "Reminder",
                                                    "Action required"};
        return texts[rand() % texts.size()];
    }
};
