#include <iostream>
#include <sstream>

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

int main(int argc, char* argv[])
{
    std::string result = curl_test();
    test_xml(result);

    return 0;
}


