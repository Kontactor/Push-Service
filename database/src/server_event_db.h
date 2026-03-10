

#pragma once

#include <optional>
#include <string>

#include "common.h"
#include "sqlite_db.h"

class ServerEventDAO {
public:
    explicit ServerEventDAO(sqlite_db::Database& db);

    bool InitTable();

    bool AddServerEvent(ServerEventType type, const std::string& description);

    bool AddUserEvent(ServerEventType type, int user_id, const std::string& description);

    bool AddPushEvent(ServerEventType type, int push_id, const std::string& description);

private:
    sqlite_db::Database& db_;
};
