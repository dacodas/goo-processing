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
#include <mycore/mystring.h>

enum GENERAL_RETURN
{
    GENERAL_RETURN_SUCCESS = 0,
    GENERAL_RETURN_FAILURE
};

enum PRINT_ENTRY_RETURN
{
    PRINT_ENTRY_SUCCESS = 0,
    PRINT_ENTRY_FAILURE
};

enum SIMPLE_STRING_MATCH_RETURN_CODE
{
    SIMPLE_STRING_MATCH_SUCCESS = 0,
    SIMPLE_STRING_MATCH_REGEX_COMPILATION_FAILED,
    SIMPLE_STRING_MATCH_REGEX_MATCH_FAILED
};

struct _simple_string_match_struct
{
    SIMPLE_STRING_MATCH_RETURN_CODE return_code;
    const unsigned char* matched_string;
    int matched_string_length;
};

typedef struct _simple_string_match_struct simple_string_match_struct;

simple_string_match_struct simple_string_match(const char* string, const char* regex)
{
    simple_string_match_struct return_struct = {SIMPLE_STRING_MATCH_SUCCESS, NULL, 0};

    // printf("Trying to match %s and %s\n", string, regex);

    // Taken from 'man pcre2demo'
    int errornumber;
    int rc;
    PCRE2_SIZE *ovector;
    PCRE2_SIZE erroroffset;

    pcre2_code *re;

    PCRE2_SPTR pattern = (PCRE2_SPTR) regex;
    PCRE2_SPTR subject = (PCRE2_SPTR) string;
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
        return_struct.return_code = SIMPLE_STRING_MATCH_REGEX_COMPILATION_FAILED;
        return return_struct;
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

        return_struct.return_code = SIMPLE_STRING_MATCH_REGEX_COMPILATION_FAILED;
        return return_struct;
    }

    ovector = pcre2_get_ovector_pointer(match_data);

    if ( rc >= 2 )
    {
        int entry_number_match_index = 1;
        int idx = entry_number_match_index;
        PCRE2_SPTR substring_start = subject + ovector[2*idx];
        size_t substring_length = ovector[2*idx+1] - ovector[2*idx];
        return_struct.matched_string = substring_start;
        return_struct.matched_string_length = substring_length;
    }
    else
    {
        // printf("Could not match %s!\n", string);
        return_struct.return_code = SIMPLE_STRING_MATCH_REGEX_MATCH_FAILED;
    }

    return return_struct;
}
int get_entry_number(myhtml_tree_t* tree)
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
    }

    const char* pattern = ".*/jn/([0-9]+)/meaning/m0u/.*";

    simple_string_match_struct match_result = simple_string_match(url, pattern);

    switch ( match_result.return_code )
    {
        case ( SIMPLE_STRING_MATCH_SUCCESS ):
        {
            return strtol(reinterpret_cast<const char*>(match_result.matched_string), NULL, 10);
            break;
        }
        default:
        {
            return 0;
            break;
        }
    }
}

std::string title_helper(myhtml_tree_node_t* node)
{
    std::ostringstream output_string;

    // mycore_string_raw_t str_raw;
    // mycore_string_raw_clean_all(&str_raw);
    // myhtml_serialization_tree_buffer(node, &str_raw);
    // printf("%s\n", str_raw.data);
    // mycore_string_raw_destroy(&str_raw, false);

    while (node)
    {
        myhtml_tag_id_t tag_id = myhtml_node_tag_id(node);
        if ( tag_id == MyHTML_TAG__TEXT )
        {
            output_string << myhtml_node_text(node, NULL);
        }
        else if ( tag_id == MyHTML_TAG_SPAN )
        {
            myhtml_tree_attr_t* attribute = myhtml_node_attribute_first(node);

            while (attribute)
            {
                const char *attribute_key= myhtml_attribute_key(attribute, NULL);
                if ( !strcmp(attribute_key, "class") )
                {
                    const char *attribute_value = myhtml_attribute_value(attribute, NULL);
                    if ( !strcmp(attribute_value, "meaning") )
                        goto __loop_end_do_not_print;
                }
                attribute = myhtml_attribute_next(attribute);
            }
        }
        else if ( tag_id == MyHTML_TAG_SUP)
        {
            // output_string << myhtml_node_text(myhtml_node_child(node), NULL);
        }
        title_helper(myhtml_node_child(node));
        node = myhtml_node_next(node);
    }

__loop_end_do_not_print:
    return output_string.str();
} 

std::string get_title(myhtml_tree_node_t* title_node)
{
    auto h1_node = myhtml_node_next(myhtml_node_child(title_node));
    return title_helper(myhtml_node_child(h1_node));
}

struct _global_struct
{
    int failure_number = 0;
    int print_entry_failure_number = 0;

    myhtml_t* myhtml;
    myhtml_tree_t* tree;
} globals;

myhtml_collection_t* get_page_title_div()
{
    const char key_value_pairs[][2][100] =
        {
            {"class", "basic_title nolink jn"},
            {"class", "basic_title nolink"}
        };

    myhtml_collection_t* collection;

    for ( int i = 0; i < sizeof(key_value_pairs) / sizeof(*key_value_pairs); ++i )
    {
        const char* key = key_value_pairs[i][0];
        const char* value = key_value_pairs[i][1];
        collection = myhtml_get_nodes_by_attribute_value(globals.tree, NULL, NULL, true, key, strlen(key), value, strlen(value), NULL);

        if ( collection && collection->list && collection->length )
            return collection;
    }

    return collection;
}

bool page_suggests_no_entry()
{
    myhtml_collection_t* titles_collection = myhtml_get_nodes_by_tag_id(globals.tree, NULL, MyHTML_TAG_TITLE, NULL);
    if ( titles_collection && titles_collection->list && titles_collection->length )
    {
        myhtml_tree_node_t* text_node = myhtml_node_child(titles_collection->list[0]);
        const char* text = myhtml_node_text(text_node, NULL);
        if ( !std::string(text).compare(u8"の意味 - goo国語辞書") )
        {
            // printf("The title of this document suggests there is no entry for this number\n");
            return true;
        }
    }

    return false;
}

int guess_entry_number_from_filename(const std::string& filename)
{
    simple_string_match_struct result = simple_string_match(filename.c_str(), ".*/([0-9]+).html");

    if ( result.return_code == SIMPLE_STRING_MATCH_SUCCESS )
    {
        return strtol(reinterpret_cast<const char*>(result.matched_string), NULL, 10);
    }
    else
    {
        printf("Failed!\n");
    }

    return 0;
}

void record_failure(const std::string& buffer, const std::string& description)
{
    std::ofstream conglomerate_file("/tmp/failures.description", std::ofstream::app);
    conglomerate_file << globals.failure_number << " description: " << description << "\n";

    std::ostringstream failure_filename;
    failure_filename << "/tmp/" << description << "-failure-buffer." << globals.failure_number++;
    std::ofstream output_file(failure_filename.str());
    output_file << buffer << std::flush;

    conglomerate_file.close();
}

void parse_page(const std::string& page_buffer, const std::string& filename = "")
{
    std::string title;

    myhtml_collection_t* entry_title_div_collection = get_page_title_div();
    if ( not (entry_title_div_collection && entry_title_div_collection->list && entry_title_div_collection->length) )
    {
        if ( page_suggests_no_entry() )
        {
            title = "No entry associated";
        }
        else
        {
            title = "Failure";
            record_failure(page_buffer, "title");
        }
    }
    else
    {
        title = get_title(entry_title_div_collection->list[0]);
    }

    int entry_number = get_entry_number(globals.tree);
    if ( ! entry_number )
    {
        if ( filename != "" )
        {
            entry_number = guess_entry_number_from_filename(filename);
        }
        else
        {
            record_failure(page_buffer, "entry");
        }
    }
    std::cout << entry_number << ": " << title << "\n";
}

enum SIMPLIFY_ENTRIES_RUN_MODE
{
    USE_STDIN,
    USE_FILENAMES
};

void initialize_globals()
{
    globals.myhtml = myhtml_create();
    globals.tree = myhtml_tree_create();

    myhtml_init(globals.myhtml, MyHTML_OPTIONS_DEFAULT, 1, 0);
    myhtml_tree_init(globals.tree, globals.myhtml);
    myhtml_encoding_set(globals.tree, MyENCODING_UTF_8);
}

void reinitialize_globals()
{
    // Reinitialize everything
    myhtml_tree_destroy(globals.tree);
    myhtml_destroy(globals.myhtml);

    initialize_globals();
}

void clear_failures_log()
{
    std::ofstream conglomerate_file("/tmp/failures.description");
}

int main(int argc, const char * argv[])
{
    initialize_globals();
    clear_failures_log();

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

                if (!file.is_open())
                {
                    std::cout << "Failed to open " << filename << "\n";
                    continue;
                }

                file.seekg(0, std::ios::end);
                file_contents.reserve(file.tellg());
                file.seekg(0, std::ios::beg);

                // Go from the file.rdbuf() to end-of-stream
                file_contents.assign(
                    std::istreambuf_iterator<char>(file),
                    std::istreambuf_iterator<char>()
                    );

                myhtml_parse(globals.tree, MyENCODING_UTF_8, file_contents.c_str(), file_contents.size());
                parse_page(file_contents, filename);
                reinitialize_globals();
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

                    reinitialize_globals();
                }
                else
                {
                    myhtml_parse_chunk(globals.tree, buffer.c_str(), buffer.length());
                    // printf("Still parsing...\n");
                }
            }
        }
    }
    
    myhtml_tree_destroy(globals.tree);
    myhtml_destroy(globals.myhtml);

    return 0;
}
