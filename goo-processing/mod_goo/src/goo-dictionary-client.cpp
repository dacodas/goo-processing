#include <curl/curl.h>

size_t write_callback(char *ptr, size_t size, size_t nmemb, void *userdata)
{
    // Ignore size
    auto string = static_cast<std::string*>(userdata);
    string->append(ptr, nmemb);
    return nmemb;
}

std::string curl_test()
{
    int entry_number;
    std::ostringstream url_oss; 
    // url_oss << "http://goo-dictionary.service/" << entry_number << ".html";
    // url_oss << "https://dacodastrack.com/";
    url_oss << "http://localhost:8080/34085.html";
    std::string url { std::move(url_oss.str()) };

    std::string result;

    CURL *curl;
    CURLcode res;
 
    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();
    if(curl) {

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &result);
        res = curl_easy_perform(curl);

        if(res != CURLE_OK)
        {
            fprintf(stderr, "curl_easy_perform() failed: %s\n",
                    curl_easy_strerror(res));
        }
        curl_easy_cleanup(curl);
    }
    curl_global_cleanup();

    return result;
}
