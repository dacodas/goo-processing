#include <iostream>
#include <sstream>
#include <fstream>
#include <syslog.h>
#include <string>

#include "GrabEntryDefinition.hpp"

int main()
{
    std::ifstream test_html_file("/tmp/test.html");
    std::stringstream buffer;
    buffer << test_html_file.rdbuf();

    setlogmask(LOG_UPTO (LOG_INFO));
    // openlog("mod_goo", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);
    openlog("mod_goo", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_DAEMON);

    // std::cout << buffer.str();
    std::string result = GrabEntryDefinition(buffer.str());

    std::cout << result << "\n";
    std::cout << result.size() << "\n";

    return 0;
}

