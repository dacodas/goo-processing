#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <memory.h>
#include <unistd.h>
#include <vector>

#include <fstream>
#include <iostream>
#include <sstream>
#include <cstring>
#include <string>

#define PCRE2_CODE_UNIT_WIDTH 8
#include <pcre2.h>

#include <myhtml/api.h>

enum PRINT_URL_RETURN
{
    PRINT_URL_SUCCESS = 0,
    PRINT_URL_REGEX_COMPILATION_FAILED,
    PRINT_URL_REGEX_MATCH_FAILED
};

int print_url(myhtml_tree_t* tree)
{
    const char key[] = "property";
    const char value[] = "og:url";

    const int url_length = 5000;
    char url[url_length];

    myhtml_collection_t* collection = myhtml_get_nodes_by_attribute_value(tree, NULL, NULL, true, key, strlen(key), value, strlen(value), NULL);

    if ( collection && collection->list && collection->length )
    {
        myhtml_tree_attr_t* attribute = myhtml_node_attribute_last(collection->list[0]);
        snprintf(url, url_length, "%s", myhtml_attribute_value(attribute, NULL));
        // printf("%s", url);
    }

    // Taken from 'man pcre2demo', error handling optimistically
    // removed <3 <3
    int errornumber;
    int rc;
    PCRE2_SIZE *ovector;
    PCRE2_SIZE erroroffset;

    pcre2_code *re;

    PCRE2_SPTR pattern = (PCRE2_SPTR) ".*/jn/([0-9]+)/meaning/m0u/.*";
    PCRE2_SPTR subject = (PCRE2_SPTR) url;
    size_t subject_length = strlen((char *) subject);

    re = pcre2_compile(
        pattern,               /* the pattern */
        PCRE2_ZERO_TERMINATED, /* indicates pattern is zero‐terminated */
        0,                     /* default options */
        &errornumber,          /* for error number */
        &erroroffset,          /* for error offset */
        NULL);                 /* use default compile context */

    if (re == NULL)
    {
        PCRE2_UCHAR buffer[256];
        pcre2_get_error_message(errornumber, buffer, sizeof(buffer));
        printf("PCRE2 compilation failed at offset %d: %s\n", (int)erroroffset,
               buffer);
        return PRINT_URL_REGEX_COMPILATION_FAILED;
    }

    pcre2_match_data* match_data = pcre2_match_data_create_from_pattern(re, NULL);
    rc = pcre2_match(
        re,                   /* the compiled pattern */
        subject,              /* the subject string */
        subject_length,       /* the length of the subject */
        0,                    /* start at offset 0 in the subject */
        0,                    /* default options */
        match_data,           /* block for storing the result */
        NULL);                /* use default match context */

    if (re == NULL)
    {
        PCRE2_UCHAR buffer[256];
        pcre2_get_error_message(errornumber, buffer, sizeof(buffer));
        printf("PCRE2 compilation failed at offset %d: %s\n", (int)erroroffset,
               buffer);
        return PRINT_URL_REGEX_COMPILATION_FAILED;
    }

    ovector = pcre2_get_ovector_pointer(match_data);
    // printf("Match succeeded at offset %d\n", (int)ovector[0]);

    // for (int i = 0; i < rc; i++)
    // {
    //     PCRE2_SPTR substring_start = subject + ovector[2*i];
    //     size_t substring_length = ovector[2*i+1] - ovector[2*i];
    //     printf("%2d: %.*s\n", i, (int)substring_length, (char *)substring_start);
    // }

    if ( rc >= 2 )
    {
        int entry_number_match_index = 1;
        int idx = entry_number_match_index;
        PCRE2_SPTR substring_start = subject + ovector[2*idx];
        size_t substring_length = ovector[2*idx+1] - ovector[2*idx];
        printf("%.*s: ", (int) substring_length, (char*) substring_start);
    }
    else
    {
        printf("Could not match %s!\n", url);
        return PRINT_URL_REGEX_MATCH_FAILED;
    }

    return PRINT_URL_SUCCESS;
}

mystatus_t serialization_callback(const char* data, size_t len, void* ctx)
{
    printf("%.*s", (int)len, data);
    return MyCORE_STATUS_OK;
}

void title_helper(myhtml_tree_node_t* node)
{
    while (node)
    {
        myhtml_tag_id_t tag_id = myhtml_node_tag_id(node);
        if ( tag_id == MyHTML_TAG__TEXT )
        {
            printf("%s", myhtml_node_text(node, NULL));
        }
        else if ( tag_id == MyHTML_TAG_SPAN )
        {
            myhtml_tree_attr_t* attribute = myhtml_node_attribute_first(node);

            while (attribute)
            {
                const char *attribute_key= myhtml_attribute_key(attribute, NULL);
                // printf("Looking at attribute %s\n", attribute_key);
                if ( !strcmp(attribute_key, "class") )
                {
                    const char *attribute_value = myhtml_attribute_value(attribute, NULL);
                    // printf("Looking at attribute value %s\n", attribute_value);
                    if ( !strcmp(attribute_value, "meaning") )
                        goto __loop_end_do_not_print;
                }

                attribute = myhtml_attribute_next(attribute);
            }
        }

        title_helper(myhtml_node_child(node));
        node = myhtml_node_next(node);
    }

__loop_end_do_not_print:
    ;

} 

void print_title(myhtml_tree_node_t* title_node)
{
    auto h1_node = myhtml_node_next(myhtml_node_child(title_node));
    title_helper(myhtml_node_child(h1_node));
    printf("\n");
}

struct _global_struct
{
    int failure_number = 0;
    int print_url_failure_number = 0;

    myhtml_t* myhtml;
    myhtml_tree_t* tree;
    myhtml_collection_t* collection;
} globals;

void parse_page(const std::string& page_buffer)
{
    const char key_value_pairs[][2][100] =
        {
            {"class", "basic_title nolink jn"},
            {"class", "basic_title nolink"}
        };

    globals.collection = NULL;
    for ( int i = 0; i < sizeof(key_value_pairs)/sizeof(*key_value_pairs); ++i )
    {
        const char* key = key_value_pairs[i][0];
        const char* value = key_value_pairs[i][1];
        // printf("Trying to match %s = %s\n", key, value);
        globals.collection = myhtml_get_nodes_by_attribute_value(globals.tree, NULL, NULL, true, key, strlen(key), value, strlen(value), NULL);
        if ( globals.collection && globals.collection->length > 0 )
        {
            int print_url_result = print_url(globals.tree);

            if ( print_url_result == PRINT_URL_REGEX_MATCH_FAILED )
            {
                std::ostringstream print_url_failure_filename;
                print_url_failure_filename << "/tmp/print-url-failure-buffer." << globals.print_url_failure_number++;
                std::ofstream output_file(print_url_failure_filename.str());
                output_file << page_buffer << std::flush;
                continue;
            }

            print_title(globals.collection->list[0]);
            // myhtml_serialization_tree_callback(collection->list[0], serialization_callback, NULL);
            // print_tree(tree, collection->list[0]);
            break;
        } 
    }

    if ( globals.collection == NULL || globals.collection->length == 0 )
    {
        globals.collection = myhtml_get_nodes_by_tag_id(globals.tree, NULL, MyHTML_TAG_TITLE, NULL);
                
        if ( globals.collection && globals.collection->list && globals.collection->length )
        {
            myhtml_tree_node_t* text_node = myhtml_node_child(globals.collection->list[0]);
            const char* text = myhtml_node_text(text_node, NULL);
            if ( !std::string(text).compare(u8"の意味 - goo国語辞書") )
            {
                printf("The title of this document suggests there is no entry for this number\n");
                return;
            }
        }

        printf("Failure #%d!\n", globals.failure_number);
        std::ostringstream failure_filename;
        failure_filename << "/tmp/failure-buffer." << globals.failure_number++;
        std::ofstream output_file(failure_filename.str());
        output_file << page_buffer << std::flush;
        return;
    }
}

enum SIMPLIFY_ENTRIES_RUN_MODE
{
    USE_STDIN,
    USE_FILENAMES
};

int main(int argc, const char * argv[])
{
    globals.failure_number = 0;
    globals.print_url_failure_number = 0;

    globals.myhtml = myhtml_create();
    globals.tree = myhtml_tree_create();
    globals.collection;

    myhtml_init(globals.myhtml, MyHTML_OPTIONS_DEFAULT, 1, 0);
    myhtml_tree_init(globals.tree, globals.myhtml);
    myhtml_encoding_set(globals.tree, MyENCODING_UTF_8);

    SIMPLIFY_ENTRIES_RUN_MODE run_mode;

    if ( argc > 1 )
    {
        run_mode = USE_FILENAMES;
    }
    else
    {
        run_mode = USE_STDIN;
    }

    switch (run_mode)
    {
        case USE_FILENAMES:
        {
            std::ifstream file;
            std::string file_contents;
            std::vector<std::string> filenames(&argv[1], argv + argc);

            for ( const auto& filename : filenames )
            {
                file.open(filename);

                file.seekg(0, std::ios::end);
                file_contents.reserve(file.tellg());
                file.seekg(0, std::ios::beg);

                // Go from the file.rdbuf() to end-of-stream
                file_contents.assign(
                    std::istreambuf_iterator<char>(file),
                    std::istreambuf_iterator<char>()
                    );

                myhtml_parse(globals.tree, MyENCODING_UTF_8, file_contents.c_str(), file_contents.size());
                parse_page(file_contents);
                file.close();
            }
            break;
        }
        case USE_STDIN:
        {
            int buffer_length = 50000;
            int offset = 0;
            std::string buffer(buffer_length, '\0');
            std::string search_string("</html>");

            while ( true )
            {
                std::cin.read(&buffer[offset], buffer_length - offset);
                offset = 0;

                auto chars_read = std::cin.gcount();
                if (chars_read == 0)
                {
                    break;
                }

                std::size_t found = buffer.find(search_string);
                if (found != std::string::npos)
                {
                    std::string remainder(buffer, found + search_string.length());
                    // printf("Found </html> at position %d\n", found);
                    // printf("The rest of the string starts with %.15s\n", remainder.c_str());

                    myhtml_parse_chunk(globals.tree, buffer.c_str(), found + search_string.length());
                    myhtml_parse_chunk_end(globals.tree);

                    parse_page(buffer);

                    // Transfer the remainder to the beginning of the
                    // buffer
                    strcpy(&buffer[0], remainder.c_str());
                    offset = buffer_length - (found + search_string.length());

                    // Reinitialize everything
                    myhtml_collection_destroy(globals.collection);
                    myhtml_tree_destroy(globals.tree);
                    myhtml_destroy(globals.myhtml);

                    globals.myhtml = myhtml_create();
                    globals.tree = myhtml_tree_create();

                    myhtml_init(globals.myhtml, MyHTML_OPTIONS_DEFAULT, 1, 0);
                    myhtml_tree_init(globals.tree, globals.myhtml);
                    myhtml_encoding_set(globals.tree, MyENCODING_UTF_8);
                }
                else
                {
                    myhtml_parse_chunk(globals.tree, buffer.c_str(), buffer.length());
                    // printf("Still parsing...\n");
                }
            }
        }
    }
    
    myhtml_collection_destroy(globals.collection);
    myhtml_tree_destroy(globals.tree);
    myhtml_destroy(globals.myhtml);

    return 0;
}
