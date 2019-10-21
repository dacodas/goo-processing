#include <filesystem>

#include "GrabEntryDefinition.hpp"

int main()
{
    // std::filesystem::path entriesPath {"../dictionary-entries"};
    std::filesystem::path entriesPath {"/mnt/ram/dictionary-entries"};
    processEntries(entriesPath);

    return 0;
}
