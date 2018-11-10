#include <iostream>
#include <cstring>
#include <string>

int main(int argc, char* argv[])
{

    int buffer_length = 50000;
    int offset = 0;

    std::string buffer(buffer_length, '\0');

    while ( !std::cin.eof() )
    {
        std::cin.read(&buffer[offset], buffer_length - offset);

        offset = 0;

        // printf("Buffer is now %s\n", buffer.c_str());

        std::string search_string("</html>");

        std::size_t found = buffer.find(search_string);
        if (found!=std::string::npos)
        {
            printf("Found at position %d\n", found);

            std::string remainder(buffer, found + search_string.length());
            printf("The rest of the string starts with %.15s\n", remainder.c_str());

            strcpy(&buffer[0], remainder.c_str());
            offset = buffer_length - (found + search_string.length());
        }
    }
}
