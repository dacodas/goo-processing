#include <iostream>

#include <syslog.h>

#include <libxml/tree.h>
#include <libxml/HTMLparser.h>
#include <libxml/HTMLtree.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>

std::string GrabEntryDefinition(const std::string& html)
{
    std::string xpath {"//div[@class=\"contents\"]"};
    xmlDoc* document;
    xmlXPathContext* context;
    xmlXPathObject* result; 

    // Send errors to this file, see documentation, not intuitive
    FILE* parsing_errors = fopen("/tmp/parsing_errors.txt", "w");

    if ( parsing_errors == NULL )
        syslog(LOG_ERR, "Unable to open logfile for parsing errors");
    else 
        xmlSetGenericErrorFunc(reinterpret_cast<void*>(parsing_errors), NULL);

    document = htmlReadMemory(html.c_str(), html.size(), "http://example.com/", "UTF-8", 0);

    if ( parsing_errors != NULL )
        fclose(parsing_errors);

    if ( document == NULL )
    {
        syslog(LOG_ERR, "Document not parsed correctly.\n");
        return "";
    }

    context = xmlXPathNewContext(document);
    if ( context == NULL )
    {
        syslog(LOG_ERR, "Error creating context\n");
        return "";
    }

    result = xmlXPathEvalExpression(
        (xmlChar*) xpath.c_str(),
        context);
        
    if ( result == NULL )
    {
        syslog(LOG_ERR, "Error evaluating expression\n");
        return "";
    } 

    if ( xmlXPathNodeSetIsEmpty(result->nodesetval) )
    {
        syslog(LOG_ERR, "No result for that XPath\n");
        return "";
    }

    xmlNodeSet* node_set = result->nodesetval;
    xmlNode** node_array = node_set->nodeTab;
    size_t size = node_set->nodeNr;

    xmlBuffer* buffer = xmlBufferCreate();
    // xmlNodeDump(buffer, document, node_array[0], 1, 1);
    htmlNodeDump(buffer, document, node_array[0]);

    std::string string {reinterpret_cast<char*>(buffer->content)};

    xmlBufferFree(buffer);
    xmlXPathFreeObject(result);
    xmlXPathFreeContext(context);
    xmlFreeDoc(document);

    return string;
}
