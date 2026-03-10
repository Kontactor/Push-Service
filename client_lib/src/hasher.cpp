#include "hasher.h"

#include <functional>
#include <iomanip>
#include <openssl/sha.h>
#include <span>
#include <sstream>
#include <stdexcept>
#include <vector>

#include "MurmurHash3.h"  // https://github.com/aappleby/smhasher/blob/master/src/MurmurHash3.cpp

namespace hasher {
// TODO :
// add generation of different salt or seed
inline const std::string SALT = "Masterskaya";
constexpr uint32_t SEED = 888888;

template <typename SaltType>
using HashFunc = std::function<std::string(const std::string&, const SaltType&)>;

// converts an array of unsigned char to string of hexadecimal numbers
std::string UCharsToString(std::span<const unsigned char> data) {
    std::ostringstream ss;
    for (const auto ch : data) {
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(ch);
    }
    return ss.str();
}

// returns the hash of the input string as a string of hexadecimal numbers
// obtained based on the algorithm SHA-256
std::string HashSHA256(const std::string& source_str, const std::string& salt) {
    unsigned char hash[SHA256_DIGEST_LENGTH];  // SHA256_DIGEST_LENGTH = 32

    std::vector<unsigned char> source_data;
    source_data.reserve(source_str.size() + salt.size());
    source_data.insert(source_data.end(), source_str.begin(), source_str.end());
    source_data.insert(source_data.end(), salt.begin(), salt.end());

    SHA256(source_data.data(), source_data.size(), hash);

    return UCharsToString(hash);
}

// returns the hash of the input string as a string of hexadecimal numbers
// obtained based on the algorithm MurmurHash3
std::string MurmurHash(const std::string& source_str, uint32_t seed) {
    unsigned char hash[4];

    MurmurHash3_x86_32(source_str.c_str(), source_str.size(), seed, hash);

    return UCharsToString(hash);
}

template <typename SaltType>
class PasswordHasher : public IHasher {
public:
    explicit PasswordHasher(HashFunc<SaltType> hash, const SaltType& salt) : hash_(std::move(hash)), salt_(salt) {}

    std::string Hash(const std::string& source_str) const override {
        return hash_(source_str, salt_);
    }

private:
    HashFunc<SaltType> hash_;
    const SaltType& salt_;
};

// create smart pointer to interface Hasher
std::unique_ptr<IHasher> MakeHasher(HasherType type) {
    std::unique_ptr<IHasher> hasher;
    switch (type) {
        case HasherType::PASSWORD:
            hasher = std::make_unique<PasswordHasher<std::string>>(HashSHA256, SALT);
            break;
        case HasherType::ANOTHER:
            hasher = std::make_unique<PasswordHasher<uint32_t>>(MurmurHash, SEED);
            break;
        default:
            throw std::invalid_argument("Invalid argument of HasherType type");
    }
    return hasher;
}
}  // namespace hasher
