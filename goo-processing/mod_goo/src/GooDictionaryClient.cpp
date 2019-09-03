#include <string>
#include <sstream>
#include <iostream>

#include "GooDictionaryClient.hpp"

size_t write_callback(char *ptr, size_t size, size_t nmemb, void *userdata)
{
    // Ignore size
    auto string = static_cast<std::string*>(userdata);
    string->append(ptr, nmemb);
    return nmemb;
}

using GooDictionaryClient = CURL;

GooDictionaryClient* goo_dictionary_initialize()
{
    curl_global_init(CURL_GLOBAL_ALL);
    CURL* curl = curl_easy_init();

    if ( !curl )
        return nullptr;

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);

    return curl;
}

void goo_dictionary_query(GooDictionaryClient* client, const std::string& host, const std::string& entry, std::string& result)
{
    CURLcode curl_result;

    std::ostringstream url_oss; 
    url_oss << host << "/" << entry << ".html";
    std::string url { std::move(url_oss.str()) };

    curl_easy_setopt(client, CURLOPT_URL, url.c_str());
    curl_easy_setopt(client, CURLOPT_WRITEDATA, &result);

    curl_result = curl_easy_perform(client);

    if (curl_result != CURLE_OK)
    {
        fprintf(stderr, "curl_easy_perform() failed: %s\n",
                curl_easy_strerror(curl_result));
    }
}

void goo_dictionary_cleanup(GooDictionaryClient* client)
{
    curl_easy_cleanup(client);
    curl_global_cleanup();
}

int goo_dictionary_test()
// int main()
{
    GooDictionaryClient* goo_dictionary_client = goo_dictionary_initialize();
    
    if ( goo_dictionary_client == nullptr )
        std::cerr << "Error initializing GooDictionaryClient\n";

    std::string result {};
    goo_dictionary_query(goo_dictionary_client, "http://localhost:8080", "34085", result);

    std::cout << result << "\n";

    return 0;
}
