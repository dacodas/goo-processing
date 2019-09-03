#pragma once

#include <string>
#include <vector>

using GooTrieEntry = std::pair<std::string, std::string>;
using GooTrieEntries = std::vector<GooTrieEntry>;

int goo_trie_connect(const std::string& host, const std::string& port);
GooTrieEntries goo_trie_query(const std::string& query, int sockfd);
