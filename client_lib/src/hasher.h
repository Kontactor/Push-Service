#pragma once

#include <memory>
#include <string>

namespace hasher {
enum class HasherType { PASSWORD, ANOTHER };

class IHasher {
public:
    virtual ~IHasher() = default;
    virtual std::string Hash(const std::string& source_str) const = 0;
};

std::unique_ptr<IHasher> MakeHasher(HasherType type);
}  // namespace hasher
