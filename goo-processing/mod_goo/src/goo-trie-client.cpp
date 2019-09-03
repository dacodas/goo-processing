// Modelled after http://beej.us/guide/bgnet/html/single/bgnet.html#simpleclient

#include <string>
#include <vector>
#include <iostream>
#include <sstream>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <arpa/inet.h>

using GooTrieEntry = std::pair<std::string, std::string>;
using GooTrieEntries = std::vector<GooTrieEntry>;

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int goo_trie_connect(const std::string& host, const std::string& port)
{
    // Only specify TCP
    struct addrinfo hints;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    // Get all information advertised on port
    struct addrinfo *addrinfos;
    int result = getaddrinfo(host.c_str(), port.c_str(), &hints, &addrinfos);
    if ( result != 0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(result));
        return -1;
    }

    // loop through all the results and connect to the first we can
    int sockfd;
    struct addrinfo *current_addrinfo = addrinfos;
    for ( ; current_addrinfo != NULL; current_addrinfo = current_addrinfo->ai_next )
    {
        sockfd = socket(current_addrinfo->ai_family,
                        current_addrinfo->ai_socktype,
                        current_addrinfo->ai_protocol);

        if ( sockfd == -1 )
        {
            perror("client: socket");
            continue;
        }

        int result = connect(sockfd,
                             current_addrinfo->ai_addr,
                             current_addrinfo->ai_addrlen);
        if ( result == -1 )
        {
            close(sockfd);
            perror("client: connect");
            continue;
        }

        break;
    }

    if (current_addrinfo == NULL)
    {
        fprintf(stderr, "client: failed to connect\n");
        return -2;
    }

    char address[INET6_ADDRSTRLEN];
    inet_ntop(current_addrinfo->ai_family,
              get_in_addr( (struct sockaddr *) current_addrinfo->ai_addr),
              address, sizeof address);
    printf("client: connecting to %s\n", address);

    freeaddrinfo(addrinfos); // all done with this structure

    return sockfd;
}

GooTrieEntries goo_trie_query(const std::string& query, int sockfd)
{

    std::ostringstream formatted_query_oss {};
    formatted_query_oss << "LEMMEKNOW " << query;
    std::string formatted_query = std::move(formatted_query_oss.str());

    std::vector<GooTrieEntry> entries {};

    send(sockfd, formatted_query.c_str(), formatted_query.size(), 0);
    send(sockfd, "", 0, MSG_EOR);

    const int size = 10000;
    char buffer[size];
    int numbytes = recv(sockfd, buffer, size - 1, 0);
    if ( numbytes == -1 )
    {
        perror("recv");
        return entries;
    }

    buffer[numbytes] = '\0';

    char* start = &(buffer[0]);
    while ( *start != '\0' ) 
    {
        std::string entry_heading {};
        std::string entry_number {};

        while ( *start != 0x1f and *start != '\0' )
        {
            entry_heading.push_back(*start++);
        }

        ++start;

        while ( *start != '\n' and *start != '\0' )
        {
            entry_number.push_back(*start++);
        }

        if ( *start != '\0' ) ++start;

        if ( entry_heading != "" and entry_number != "" ) 
            entries.emplace_back( GooTrieEntry( std::move(entry_heading), std::move(entry_number) ) );
    }

    // printf("client: received '%s'\n", buffer);

    return entries;
}

// int main()
int test()
{
    int socket = goo_trie_connect("localhost", "7081");
    GooTrieEntries entries = goo_trie_query("俺", socket);

    for ( const GooTrieEntry& entry : entries )
    {
        std::cout << "Name: " << entry.first << ", number: " << entry.second << "\n";
    }
}
