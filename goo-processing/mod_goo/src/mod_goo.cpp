#include <iostream>
#include <sstream>

#include <stdio.h>
#include "ap_config.h"
#include "ap_provider.h"
#include "httpd.h"
#include "http_core.h"
#include "http_config.h"
#include "http_log.h"
#include "http_protocol.h"
#include "http_request.h"

#include "GooDictionaryClient.hpp"
#include "GooTrieClient.hpp"
#include "GrabEntryDefinition.hpp"



extern "C" int example_handler(request_rec *r)
{
    /* Set the appropriate content type */
    ap_set_content_type(r, "text/json");

    /* Print out the IP address of the client connecting to us: */
    ap_rprintf(r, "<h2>Hello, %s!</h2>", r->useragent_ip);
    
    /* If we were reached through a GET or a POST request, be happy, else sad. */
    if ( !strcmp(r->method, "POST") ) {
        ap_rputs("You used a POST method, that makes us happy!<br/>", r);
    }
    else {
        ap_rputs("You did not use POST, that makes us sad :(<br/>", r);
    }

    /* Lastly, if there was a query string, let's print that too! */
    if (r->args) {
        ap_rprintf(r, "Your query string was: %s", r->args);
    }
    return OK;
}

extern "C" void register_hooks(apr_pool_t *pool)
{
    /* Create a hook in the request handler, so we get called when a request arrives */
    ap_hook_handler(example_handler, NULL, NULL, APR_HOOK_LAST);
}

extern "C" {
    module AP_MODULE_DECLARE_DATA mod_goo =
    {
        STANDARD20_MODULE_STUFF,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        register_hooks   /* Our hook registering function */
    };
}

// int main(int argc, char* argv[])
// {
//     GooDictionaryClient* dict_client = goo_dictionary_initialize();
//     if ( dict_client == nullptr )
//     {
//         std::cerr << "Error initializing GooDictionaryClient\n";
//         return 1;
//     }

//     int trie_client = goo_trie_connect("localhost", "7081");
//     if ( trie_client < 0 )
//     {
//         std::cerr << "Error initializing goo_trie_client\n";
//         return 1;
//     }

//     GooTrieEntries entries = goo_trie_query("俺", trie_client);

//     for ( const GooTrieEntry& entry : entries )
//     {
//         std::cout << "Name: " << entry.first << ", number: " << entry.second << "\n";

//         std::string result{};
//         goo_dictionary_query(dict_client, "http://localhost:8080", entry.second, result);

//         std::cout << "Got response of length " << result.size() << "!\n";

//         std::string definition = GrabEntryDefinition(result);
//         std::cout << "Got definition of length " << definition.size() << "!\n";
//     }

//     return 0;
// }


