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


#include <iostream>
#include <cstring>
#include <string>

/* #include <myhtml/api.h> */
#include <myhtml/myhtml.h>

#include "example.h"

struct res_html {
    char  *html;
    size_t size;
};

int print_tree(myhtml_tree_t* tree, myhtml_tree_node_t *node, bool print_siblings, char * target_string, int offset)
{
    int current_offset = offset;

    while (node)
    {
        myhtml_tag_id_t tag_id = myhtml_node_tag_id(node);

        if ( tag_id == MyHTML_TAG__TEXT )
        {
            /* printf("Printing at offset %d to %s\n", current_offset, target_string); */
            const char* node_text = myhtml_node_text(node, NULL);
            /* printf("Just printed a string of length %d\n", strlen(node_text)); */
            snprintf(target_string + current_offset, 50000, "%s", node_text);
            current_offset += strlen(node_text);
        }

        current_offset += print_tree(tree, myhtml_node_child(node), true, target_string, current_offset);

        if (print_siblings)
            node = myhtml_node_next(node);
        else
            node = NULL;
    }

    return current_offset - offset;
}

struct res_html load_html_file(const char* filename)
{
    FILE *fh = fopen(filename, "rb");
    if(fh == NULL) {
        fprintf(stderr, "Can't open html file: %s\n", filename);
        exit(EXIT_FAILURE);
    }
    
    if(fseek(fh, 0L, SEEK_END) != 0) {
        fprintf(stderr, "Can't set position (fseek) in file: %s\n", filename);
        exit(EXIT_FAILURE);
    }
    
    long size = ftell(fh);
    
    if(fseek(fh, 0L, SEEK_SET) != 0) {
        fprintf(stderr, "Can't set position (fseek) in file: %s\n", filename);
        exit(EXIT_FAILURE);
    }
    
    if(size <= 0) {
        fprintf(stderr, "Can't get file size or file is empty: %s\n", filename);
        exit(EXIT_FAILURE);
    }
    
    char *html = (char*)malloc(size + 1);
    if(html == NULL) {
        fprintf(stderr, "Can't allocate mem for html file: %s\n", filename);
        exit(EXIT_FAILURE);
    }
    
    size_t nread = fread(html, 1, size, fh);
    if (nread != size) {
        fprintf(stderr, "could not read %ld bytes (" MyCORE_FMT_Z " bytes done)\n", size, nread);
        exit(EXIT_FAILURE);
    }
    
    fclose(fh);
    
    struct res_html res = {html, (size_t)size};
    return res;
}

int main(int argc, const char * argv[])
{
    /* const char* path = argv[1]; */
    /* printf("%s: ", path); */
    
    /* struct res_html res = load_html_file(path); */

    int buffer_length = 50000;
    int offset = 0;

    std::string buffer(buffer_length, '\0');


    while ( !std::cin.eof() )
    {
        myhtml_t* myhtml = myhtml_create();
        myhtml_tree_t* tree = myhtml_tree_create();

        std::cin.read(&buffer[offset], buffer_length - offset);

        offset = 0;

        std::string search_string("</html>");

        std::size_t found = buffer.find(search_string);
        if (found!=std::string::npos)
        {
            // printf("Found at position %d\n", found);

            std::string remainder(buffer, found + search_string.length());
            // printf("The rest of the string starts with %.15s\n", remainder.c_str());

            strcpy(&buffer[0], remainder.c_str());
            offset = buffer_length - (found + search_string.length());

            myhtml_init(myhtml, MyHTML_OPTIONS_DEFAULT, 1, 0);
            myhtml_tree_init(tree, myhtml);
            myhtml_parse(tree, MyENCODING_UTF_8, buffer.c_str(), found + search_string.length());

            /* const char attr_value[] = "contents_area meaning_area cx"; */
            const char attr_value[] = "basic_title nolink jn";
            const char key[] = "class";
            bool is_insensitive = false;
    
            myhtml_collection_t* collection = NULL;
            collection = myhtml_get_nodes_by_attribute_value(tree, NULL, NULL,
                                                             is_insensitive,
                                                             key, strlen(key),
                                                             attr_value, strlen(attr_value), NULL);
    
            int length = 50000;
            int suffix_offset = 0;
            int new_length;
            char buffer[length];
            if (collection && collection->length > 0)
            {
                suffix_offset = 9;
            }
            else 
            {
                const char new_attr_value[] = "basic_title nolink";
                collection = myhtml_get_nodes_by_attribute_value(tree, NULL, NULL,
                                                                 is_insensitive,
                                                                 key, strlen(key),
                                                                 new_attr_value, strlen(new_attr_value), NULL);

                if ( !collection || collection->length <= 0 )
                {
                    printf("Failure...\n");
                    continue;
                }

                suffix_offset = 6;
            }

            new_length = print_tree(tree, collection->list[0]->child->next, false, buffer, 0);

            for (int i = 0; i < new_length; ++i)
            {
                if (buffer[i] == ' ' || buffer[i] == '\n' || buffer[i] == '\r')
                {
                    for (int j = i; j < new_length; ++j)
                    {
                        buffer[j] = buffer[j+1];
                    }
                    buffer[new_length-1] = '\0';
                    --new_length;
                    --i;
                }
            }

            buffer[new_length - suffix_offset] = '\0';
            printf("%s\n", buffer);

            // printf("\n");
    
            myhtml_collection_destroy(collection);

            myhtml_tree_destroy(tree);
            myhtml_destroy(myhtml);
        }
    }

    
    return 0;
}



