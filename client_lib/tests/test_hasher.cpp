#include <gtest/gtest.h>
#include <iostream>
#include <unordered_set>
#include <vector>

#include "hasher.h"

using namespace hasher;
using namespace std::literals;

// checks the sha-256 algorithm for stability
bool TestSHA256Stability() {
    const std::string HELLO_WORLD_SHA256_HASH = "2c9dace5f8f44ae08eff0fd45c71b19fff3764e16ed950368abe1f5bd737ef1c"s;
    const std::string PASSWORD = "hello world"s;

    auto hasher_sha256 = MakeHasher(HasherType::PASSWORD);
    return HELLO_WORLD_SHA256_HASH == hasher_sha256->Hash(PASSWORD);
}

// generates a vector of words whose length is equal to size
// using characters from the alphabet in number equal to words_count
std::vector<std::string> GenerateWords(const std::string& alphabet, uint32_t size, size_t words_count) {
    std::vector<std::string> res_words;
    res_words.reserve(words_count);

    std::vector<uint32_t> indixes(size, 0);  // Indexes of characters in the alphabet

    uint32_t i = 0;
    while (true) {
        // Forming a row based on the current indexes
        std::string word;
        for (uint32_t j = 0; j < size; ++j) {
            word += alphabet[indixes[j]];
        }
        res_words.push_back(word);

        // Moving on to the next combination of indexes
        int pos = size - 1;
        while (pos >= 0 && indixes[pos] == alphabet.size() - 1) {
            indixes[pos] = 0;
            --pos;
        }

        ++i;
        if (pos < 0 || i == words_count) {
            break;
        }

        indixes[pos]++;
    }
    return res_words;
}

bool TestCollisions(std::unique_ptr<IHasher> hasher) {
    const size_t PAS_SIZE = 8;
    const size_t PAS_COUNT = 1e5;
    const size_t LETTER_COUNT = 26;

    // fills in the alphabet
    std::string alphabet(LETTER_COUNT, 'a');
    for (size_t i = 0; i < LETTER_COUNT; ++i) {
        alphabet[i] += i;
    }
    // generates passwords
    std::vector<std::string> passwords = GenerateWords(alphabet, PAS_SIZE, PAS_COUNT);

    std::unordered_set<std::string> hashes;
    hashes.reserve(PAS_COUNT);

    for (size_t i = 0; i < PAS_COUNT; ++i) {
        hashes.emplace(hasher->Hash(passwords[i]));
    }

    return passwords.size() == hashes.size();
}

TEST(ClientLibTest, HasherSHA256Stability) {
    ASSERT_TRUE(TestSHA256Stability());
}

TEST(ClientLibTest, TestHasherCollisions) {
    auto hasher_sha256 = MakeHasher(HasherType::PASSWORD);
    ASSERT_TRUE(TestCollisions(std::move(hasher_sha256)));

    auto hasher_murmur = MakeHasher(HasherType::ANOTHER);
    ASSERT_TRUE(TestCollisions(std::move(hasher_murmur)));
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);

    int tests_res = RUN_ALL_TESTS();

    auto sha256_hasher = MakeHasher(HasherType::PASSWORD);
    auto mummur_hasher = MakeHasher(HasherType::ANOTHER);
    std::string password;

    std::cout << "\nEnter a password or \"exit\": "s;
    while (std::getline(std::cin, password)) {
        if (password == "exit"s) {
            break;
        }

        std::string sha256_hash = sha256_hasher->Hash(password);
        std::cout << "SHA-256 hash = "s << sha256_hash << std::endl;

        std::string murmur_hash = mummur_hasher->Hash(password);
        std::cout << "Murmur3 hash = "s << murmur_hash << std::endl;

        std::cout << "\nEnter a password or \"exit\": "s;
    }
}
