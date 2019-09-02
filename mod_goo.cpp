#include <iostream>
#include <sstream>

#include <curl/curl.h>

// module AP_MODULE_DECLARE_DATA mod_goo =
// {
//     STANDARD20_MODULE_STUFF,
//     NULL,
//     NULL,
//     NULL,
//     NULL,
//     NULL,
//     register_hooks   /* Our hook registering function */
// };

// static void register_hooks(apr_pool_t *pool)
// {
//     /* Create a hook in the request handler, so we get called when a request arrives */
//     ap_hook_handler(example_handler, NULL, NULL, APR_HOOK_LAST);
// }

// static int example_handler(request_rec *r)
// {
//     /* Set the appropriate content type */
//     ap_set_content_type(r, "text/json");

//     /* Print out the IP address of the client connecting to us: */
//     ap_rprintf(r, "<h2>Hello, %s!</h2>", r->useragent_ip);
    
//     /* If we were reached through a GET or a POST request, be happy, else sad. */
//     if ( !strcmp(r->method, "POST") ) {
//         ap_rputs("You used a POST method, that makes us happy!<br/>", r);
//     }
//     else {
//         ap_rputs("You did not use POST, that makes us sad :(<br/>", r);
//     }

//     /* Lastly, if there was a query string, let's print that too! */
//     if (r->args) {
//         ap_rprintf(r, "Your query string was: %s", r->args);
//     }
//     return OK;
// }

size_t write_callback(char *ptr, size_t size, size_t nmemb, void *userdata)
{
    // Ignore size
    auto string = static_cast<std::string*>(userdata);
    string->append(ptr, nmemb);
    return nmemb;
}

int curl_test()
{
    int entry_number;
    std::ostringstream url_oss; 
    // url_oss << "http://goo-dictionary.service/" << entry_number << ".html";
    url_oss << "https://dacodastrack.com/";
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

    std::cout << result << "\n";

    return 0;
}

int main(int argc, char* argv[])
{
    curl_test();

    return 0;
}
