#pragma once
#include <string>
#include <filesystem>

std::string GrabEntryDefinition(const std::string& html);
void processEntries(std::filesystem::path& entriesDirectoryPath);
