#include <iostream>
#include <sstream>

#include <stdio.h>

#include "httpd.h"
#include "http_core.h"
#include "http_protocol.h"
#include "http_request.h"

#include "apr_strings.h"
#include "apr_network_io.h"
#include "apr_md5.h"
#include "apr_sha1.h"
#include "apr_hash.h"
#include "apr_base64.h"
#include "apr_dbd.h"
#include <apr_file_info.h>
#include <apr_file_io.h>
#include <apr_tables.h>
#include "util_script.h"

#include "GooDictionaryClient.hpp"
#include "GooTrieClient.hpp"
#include "GrabEntryDefinition.hpp"

/* Define prototypes of our functions in this module */
extern "C" void register_hooks(apr_pool_t *pool);
extern "C" int goo_handler(request_rec *r);

/* Define our module as an entity and assign a function for registering hooks  */
module AP_MODULE_DECLARE_DATA   goo_module =
{
    STANDARD20_MODULE_STUFF,
    NULL,            // Per-directory configuration handler
    NULL,            // Merge handler for per-directory configurations
    NULL,            // Per-server configuration handler
    NULL,            // Merge handler for per-server configurations
    NULL,            // Any directives we may have for httpd
    register_hooks   // Our hook registering function
};


/* register_hooks: Adds a hook to the httpd process */
extern "C" void register_hooks(apr_pool_t *pool) 
{
    
    /* Hook the request handler */
    ap_hook_handler(goo_handler, NULL, NULL, APR_HOOK_LAST);
}

/* The handler function for our module.
 * This is where all the fun happens!
 */

apr_table_t* goo_parse_args(request_rec* r) {
    
    apr_table_t* GET;
    apr_array_header_t* POST;
    
    ap_args_to_table(r, &GET);
    ap_parse_form_data(r, NULL, &POST, -1, 8192);
}

const char* goo_get_value(apr_table_t* table, const char* key) 
{
    const apr_array_header_t    *fields;
    int                         i;
    //apr_table_elts
    
}

extern "C" int goo_handler(request_rec *r)
{
    apr_table_t* GET;
    apr_array_header_t* POST;
    
    // Check that the "goo-handler" handler is being called.
    if (!r->handler || strcmp(r->handler, "goo-handler")) return (DECLINED);
    
    // Set the appropriate content type
    ap_set_content_type(r, "text/html");
    
    // Print a title and some general information
    ap_rprintf(r, "<b>Hello</b> world!<br/>");
    
    // Let Apache know that we responded to this request.
    return OK;
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


