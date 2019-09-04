#include <iostream>
#include <sstream>

#include <stdio.h>
#include <syslog.h>

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

#include "mod_goo.hpp"

#include "GooDictionaryClient.hpp"
#include "GooTrieClient.hpp"
#include "GrabEntryDefinition.hpp"

int print_table_keys(void* rec, const char* key, const char* value)
{
    syslog(LOG_INFO, "%s: %s", key, value);
    return 1;
}

int goo_query(request_rec* r, const char* reading)
{
    GooDictionaryClient* dict_client = goo_dictionary_initialize();
    if ( dict_client == nullptr )
    {
        syslog(LOG_ERR, "Error initializing GooDictionaryClient");
        return 1;
    }

    int trie_client = goo_trie_connect("localhost", "7081");
    if ( trie_client < 0 )
    {
        syslog(LOG_ERR, "Error initializing GooTrieClient");
        return 1;
    }

    GooTrieEntries entries = goo_trie_query(reading, trie_client);

    // for ( const GooTrieEntry& entry : entries )
    for ( size_t i = 0; i < 10 and i < entries.size(); ++i )
    {
        const GooTrieEntry entry = entries[i];

        syslog(LOG_INFO, "Name: %s, number: %s", entry.first.c_str(), entry.second.c_str());

        std::string result{};
        goo_dictionary_query(dict_client, "http://localhost:8080", entry.second, result);
        syslog(LOG_INFO, "Got response of length %d", result.size());

        std::string definition = GrabEntryDefinition(result);
        syslog(LOG_INFO, "Got definition of length %d", definition.size());

        ap_rprintf(r, R"|(<li><h1>%s</h1><div class="definition">%s</div>)|", entry.first.c_str(), definition.c_str());
    }

    return 0;
}

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
    setlogmask(LOG_UPTO (LOG_INFO));
    openlog("mod_goo", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);
    syslog(LOG_NOTICE, "mod_goo has sent you a message");
    syslog(LOG_INFO, "Here is some info, man");

    /* Hook the request handler */
    ap_hook_handler(goo_handler, NULL, NULL, APR_HOOK_LAST);
}

extern "C" int goo_handler(request_rec *r)
{
    apr_table_t* GET;
    apr_array_header_t* POST;
    
    // Check that the "goo-handler" handler is being called.
    if (!r->handler || strcmp(r->handler, "goo-handler")) return (DECLINED);

    // Print out headers
    // apr_table_do(print_table_keys, (void*) 0, r->headers_in);

    ap_args_to_table(r, &GET);
    const char* reading = apr_table_get(GET, "reading");

    const char* requested_content_type = apr_table_get(r->headers_in, "Accept");
    if ( strstr(requested_content_type, "application/vnd+orihime.goo-results+html") != NULL ) 
    {
        ap_set_content_type(r, "application/vnd+orihime.goo-results+html");
        goo_query(r, reading);
    }
    else 
    {
        ap_set_content_type(r, "text/html");
        ap_rprintf(r, goo_response_prefix);
        goo_query(r, reading);
        ap_rprintf(r, goo_response_suffix);
    }

    // Let Apache know that we responded to this request.
    return OK;
}
