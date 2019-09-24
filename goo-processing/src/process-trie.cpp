#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdint.h>
#include <unistd.h>
#include <signal.h>
#include <sstream>

#include <pcrecpp.h>
#include <utf8.h>

#ifndef GOO_TRIE_LOCATION_STRING 
#define GOO_TRIE_LOCATION_STRING "/usr/share/goo-trie/trie"
#endif 

std::string get_unicode_char(const utf8::unchecked::iterator<std::string::iterator>& character)
{
    auto start = character.base();

    // The increment operator is overloaded to move to the next UTF-8 codepoint
    auto character_copy = character;
    ++character_copy;
    auto end = character_copy.base();
    return std::string(start, end);
}


class Node
{
public:

    enum class OUTPUT_FORMAT
    {
        NORMAL,
        ELISP,
        SIMPLE
    };

    struct print_words_configuration_s
    {
        // These should both refer to objects on the stack, so dumb
        // pointers are fine
        std::string* starting_string;
        std::ostream* output;
        OUTPUT_FORMAT format;
    };

    typedef struct print_words_configuration_s print_words_configuration_t;

    Node() : map(), entry_numbers() {}

    Node(const Node& node) = delete;

    void print_all_words_from_here(print_words_configuration_t& configuration)
        {
            std::string before;
            std::string interstitial;
            std::string after;

            switch ( configuration.format )
            {
                case ( OUTPUT_FORMAT::NORMAL ):
                {
                    interstitial = " -> ";
                    break;
                }
                case ( OUTPUT_FORMAT::SIMPLE ):
                {
                    before = "";
                    interstitial = "\x1f";
                    after = "";
                    break;
                }
                case ( OUTPUT_FORMAT::ELISP ):
                {
                    before = "#(\"";
                    interstitial = "\" ";
                    after = ")";
                    break;
                }
            }

            for ( const auto& entry_number : entry_numbers )
                {
                    *configuration.output << before << *configuration.starting_string << interstitial << entry_number << after << "\n";
                }

            for ( const auto& entry : map )
            {
                const auto& character = entry.first;
                const auto& node = entry.second;

                size_t original_length = configuration.starting_string->length();
                configuration.starting_string->append(character);

                for ( const auto& entry_number : node->entry_numbers )
                {
                    *configuration.output << before << *configuration.starting_string << interstitial << entry_number << after << "\n";
                }

                node->print_all_words_from_here(configuration);
                configuration.starting_string->resize(original_length);
            }
        }

    void add_entry_number(int entry_number)
        {
            entry_numbers.push_back(entry_number);
        }

    std::shared_ptr<Node> add_character(std::string&& character)
        {
            auto result = map.find(character);
            if ( result == map.end() )
                map[character] = std::shared_ptr<Node>(new Node());

            return map[character];
        }

    std::shared_ptr<Node> lookup_character(std::string&& character)
        {
            auto result = map.find(character);
            if ( result == map.end() )
                return nullptr;
            else
                return result->second;
        }

private:
    std::unordered_map<std::string, std::shared_ptr<Node>> map;
    std::vector<int> entry_numbers;
};

int main(int argc, char* argv[])
{
    printf("Using trie file %s\n", GOO_TRIE_LOCATION_STRING);
    std::ifstream input_file (GOO_TRIE_LOCATION_STRING);
    std::string line;

    pcrecpp::RE line_pattern("(\\d+): (.*)");

    auto root_node = std::shared_ptr<Node>(new Node());

    while ( std::getline(input_file, line) )
    {
        int entry_number;
        std::string title;
        line_pattern.FullMatch(line, &entry_number, &title);
        
        std::shared_ptr<Node> current_node(root_node);

        utf8::unchecked::iterator<std::string::iterator> current_char(title.begin());
        utf8::unchecked::iterator<std::string::iterator> last_char(title.end());
        for ( ; current_char != last_char; ++current_char )
        {
            current_node = current_node->add_character(get_unicode_char(current_char));
        }

        current_node->add_entry_number(entry_number);
    }

    // root_node->print_all_words_from_here();

    uint16_t port_number = 7081;
    socklen_t client_length;
    struct sockaddr_in server_address = {}, client_address = {};
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(port_number);

    int socket_file_descriptor = socket(AF_INET, SOCK_STREAM, 0);
    if ( socket_file_descriptor < 0 )
    {
        printf("Couldn't open socket\n");
        return 1;
    }
    
    int enable = 1;
    if (setsockopt(socket_file_descriptor, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable)) < 0)
    {
        printf("setsockopt(SO_REUSEADDR) failed");
        return 1;
    }

    int bind_result = bind(socket_file_descriptor,
                           reinterpret_cast<struct sockaddr*>(&server_address),
                           sizeof(server_address));

    if ( bind_result < 0 )
    {
        printf("Failure binding socket\n");
        return 1;
    }

    int pid;
    int counter = 0;

    while ( true )
    {
        printf("Starting to listen...\n");

        listen(socket_file_descriptor, 10);
        client_length = sizeof(client_address);

        int new_socket_file_descriptor = accept(socket_file_descriptor,
                                                reinterpret_cast<struct sockaddr*>(&client_address),
                                                &client_length);

        if ( new_socket_file_descriptor < 0 )
        {
            printf("Failure accepting new socket!");
            return 1;
        }

        pid = fork();
        if ( pid == -1 )
        {
            close(new_socket_file_descriptor);
        }
        else if ( pid > 0 )
        {
            printf("I am a parent process, sir\n");
            printf("Closing socket FD because on the child should be using it now\n");
            close(new_socket_file_descriptor);
            printf("Listening for %d-th connection...\n", ++counter);
            signal(SIGCHLD, SIG_IGN);
            continue;
        }
        else if ( pid == 0 )
        {
            printf("I am a child process, sir\n");
            printf("I am the %d-th process\n", counter);
        }

        printf("Accepted connection from %s on port %d\n",
               inet_ntoa(client_address.sin_addr),
               ntohs(client_address.sin_port));

        const size_t buffer_length = 1024;
        char buffer[buffer_length];

        pcrecpp::RE pattern("LEMMEKNOW (.*)");

        int zero_length_message_count = 0;
        while ( true )
        {
            bzero(buffer, buffer_length);

            int read_result = read(new_socket_file_descriptor, buffer, buffer_length - 1);
            if (read_result < 0)
            {
                printf("Error reading from socket...");
                break;
            }
            else if ( read_result == 0 and ++zero_length_message_count > 10 )
            {
                printf("Got too many zero length messages... Leaving this connection!\n");
                break;
            }
            else
            {
                printf("Here is the message: %s\n",buffer);
                std::string word;
                std::string string_buffer(buffer, read_result);
                std::ostringstream output;

                Node::print_words_configuration_t configuration =
                    { .starting_string = &word,
                      .output = &output,
                      .format = Node::OUTPUT_FORMAT::SIMPLE};

                if ( pattern.FullMatch(string_buffer, &word) )
                {
                    printf("Looking for word %s\n", word.c_str());

                    std::shared_ptr<Node> current_node(root_node);

                    utf8::unchecked::iterator<std::string::iterator> current_char(word.begin());
                    utf8::unchecked::iterator<std::string::iterator> last_char(word.end());

                    for ( ; current_char != last_char; ++current_char )
                    {
                        current_node = current_node->lookup_character(get_unicode_char(current_char));
                    }

                    if ( current_node == nullptr )
                    {
                        printf("No results!\n");
                    }
                    else
                    {
                        // output << "#(";
                        current_node->print_all_words_from_here(configuration);
                        // output << ")";
                        auto output_string = output.str();
                        send(new_socket_file_descriptor, output_string.c_str(), output_string.length(), 0);

                        configuration.format = Node::OUTPUT_FORMAT::NORMAL;
                        configuration.output = &std::cout;
                        current_node->print_all_words_from_here(configuration);
                    }
                }
                else
                {
                    printf("Unknown request!\rn");
                }
            }
        }

        close(new_socket_file_descriptor);
        close(socket_file_descriptor);
    }

    return 0;
}
