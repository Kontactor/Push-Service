#pragma once

#include "server.h"

namespace tests_utils {
// Create deque of clients with size equal to num
std::deque<std::shared_ptr<IClient>> CreateClients(const uint32_t num);

// Create vector with pushes with size equal to num
std::vector<IPush> CreatePushes(const uint32_t num);

// =====================
// functions for testing
// =====================
// verify number of received pushes by clients
bool TestNumberOfReceivedPushes(const uint32_t clients_num, const uint32_t pushes_num);

// verify order of received pushes by clients
bool TestOrderOfPushes(const uint32_t pushes_num);
}  // namespace tests_utils
