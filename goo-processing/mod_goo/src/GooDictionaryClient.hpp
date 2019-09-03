#pragma once 

#include <string>
#include <curl/curl.h>

using GooDictionaryClient = CURL;

GooDictionaryClient* goo_dictionary_initialize();
void goo_dictionary_query(GooDictionaryClient* client, const std::string& host, const std::string& entry, std::string& result);
void goo_dictionary_cleanup(GooDictionaryClient* client);
