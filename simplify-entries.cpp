/*
 Copyright (C) 2015-2016 Alexander Borisov
 
 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 2.1 of the License, or (at your option) any later version.
 
 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Lesser General Public License for more details.
 
 You should have received a copy of the GNU Lesser General Public
 License along with this library; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
 
 Author: lex.borisov@gmail.com (Alexander Borisov)
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <memory.h>
#include <unistd.h>


#include <fstream>
#include <iostream>
#include <sstream>
#include <cstring>
#include <string>

// #include <myhtml/myhtml.h>
#include <myhtml/api.h>

#include "example.h"

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
            break;
        }

        title_helper(myhtml_node_child(node));
        node = myhtml_node_next(node);
    }
} 

void print_title(myhtml_tree_node_t* title_node)
{
    auto h1_node = myhtml_node_next(myhtml_node_child(title_node));
    title_helper(myhtml_node_child(h1_node));
    printf("\n");
}

int main(int argc, const char * argv[])
{
    int buffer_length = 50000;
    int offset = 0;
    int failures = 0;
    int document_number = 2;

    printf("Let's go!\n");

    std::string buffer(buffer_length, '\0');
    myhtml_t* myhtml = myhtml_create();
    myhtml_tree_t* tree = myhtml_tree_create();
    myhtml_collection_t* collection;

    myhtml_init(myhtml, MyHTML_OPTIONS_DEFAULT, 1, 0);
    myhtml_tree_init(tree, myhtml);
    myhtml_encoding_set(tree, MyENCODING_UTF_8);


    while ( true )
    {
        std::cin.read(&buffer[offset], buffer_length - offset);
        auto chars_read = std::cin.gcount();

        // printf("I'm guessing this is %d\n", document_number);
        // printf("Attempted to read %d chars\n", buffer_length - offset);
        // printf("I read %d chars\n", chars_read);

        if (chars_read == 0)
        {
            break;
        }


        offset = 0;

        std::string search_string("</html>");


        std::size_t found = buffer.find(search_string);
        if (found != std::string::npos)
        {
            // printf("Found </html> at position %d\n", found);

            std::string remainder(buffer, found + search_string.length());
            // printf("The rest of the string starts with %.15s\n", remainder.c_str());

            myhtml_parse_chunk(tree, buffer.c_str(), found + search_string.length());
            myhtml_parse_chunk_end(tree);

            const char key_value_pairs[][2][100] =
            {
                {"class", "basic_title nolink jn"},
                {"class", "basic_title nolink"}
            };

            collection = NULL;
            for ( int i = 0; i < sizeof(key_value_pairs)/sizeof(*key_value_pairs); ++i )
            {
                const char* key = key_value_pairs[i][0];
                const char* value = key_value_pairs[i][1];
                // printf("Trying to match %s = %s\n", key, value);
                collection = myhtml_get_nodes_by_attribute_value(tree, NULL, NULL, true, key, strlen(key), value, strlen(value), NULL);
                if ( collection && collection->length > 0 )
                {
                    // printf("Success for %d!\n", document_number);
                    printf("%d: ", document_number);
                    print_title(collection->list[0]);
                    // myhtml_serialization_tree_callback(collection->list[0], serialization_callback, NULL);
                    // print_tree(tree, collection->list[0]);
                    break;
                } 
            }

            if ( collection == NULL || collection->length == 0 )
            {
                collection = myhtml_get_nodes_by_tag_id(tree, NULL, MyHTML_TAG_TITLE, NULL);
                
                if ( collection && collection->list && collection->length )
                {
                    myhtml_tree_node_t* text_node = myhtml_node_child(collection->list[0]);
                    const char* text = myhtml_node_text(text_node, NULL);
                    // printf("Got a title of %s\n", text);
                    if ( !std::string(text).compare(u8"の意味 - goo国語辞書") )
                    {
                        printf("No entry for %d\n", document_number);
                        document_number++;
                        continue;
                    }
                }

                printf("Failure with %d\n", document_number);
                std::cout << buffer << std::flush;
                return 1;
            }
    
            strcpy(&buffer[0], remainder.c_str());
            offset = buffer_length - (found + search_string.length());

            myhtml_collection_destroy(collection);
            myhtml_tree_destroy(tree);
            myhtml_destroy(myhtml);

            myhtml = myhtml_create();
            tree = myhtml_tree_create();

            myhtml_init(myhtml, MyHTML_OPTIONS_DEFAULT, 1, 0);
            myhtml_tree_init(tree, myhtml);
            myhtml_encoding_set(tree, MyENCODING_UTF_8);

            document_number++;
    
            // int reading_buffer_length = 1024 * 1024;
            // int suffix_offset = 0;
            // int new_length;
            // char reading_buffer[reading_buffer_length];
            // if (collection && collection->length > 0)
            // {
            //     suffix_offset = 9;
            // }
            // else 
            // {
            //     const char new_attr_value[] = "basic_title nolink";
            //     collection = myhtml_get_nodes_by_attribute_value(tree, NULL, NULL,
            //                                                      is_insensitive,
            //                                                      key, strlen(key),
            //                                                      new_attr_value, strlen(new_attr_value), NULL);

            //     if ( !collection || collection->length <= 0 )
            //     {
            //         printf("Failure...\n");

            //         std::ostringstream file_name;
            //         file_name << "/tmp/failure." << failures++;

            //         auto output = std::ofstream(file_name.str());
            //         output << buffer << std::endl;
            //         output.close();
                    
            //         continue;
            //     }

            //     suffix_offset = 6;
            // }

            // new_length = print_tree(tree, collection->list[0]->child->next, false, reading_buffer, reading_buffer_length, 0);

            // for (int i = 0; i < new_length; ++i)
            // {
            //     if (reading_buffer[i] == ' ' || reading_buffer[i] == '\n' || reading_buffer[i] == '\r')
            //     {
            //         for (int j = i; j < new_length; ++j)
            //         {
            //             reading_buffer[j] = reading_buffer[j+1];
            //         }
            //         reading_buffer[new_length-1] = '\0';
            //         --new_length;
            //         --i;
            //     }
            // }

            // reading_buffer[new_length - suffix_offset] = '\0';
            // printf("%s\n", reading_buffer);

            // // printf("\n");
    
        }
        else
        {
            myhtml_parse_chunk(tree, buffer.c_str(), buffer.length());
            // printf("Still parsing...\n");
        }

    }

    return 0;
}



