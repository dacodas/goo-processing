#include <iostream>
#include <fstream>
#include <vector>
#include <filesystem>
#include <future>

#include <syslog.h>

#include <libxml/tree.h>
#include <libxml/HTMLparser.h>
#include <libxml/HTMLtree.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>

#include "ThreadPool.hpp"

// When doing this, make sure to prewarm the files by either mounting
// as ramfs or using vmtouch (vmtouch untested)

// The implementation of xmlXPathCompExpr (_xmlXPathCompExpr) is
// hidden, so there is no default destructor. For this reason we make
// this class
// 
// http://xmlsoft.org/html/libxml-xpath.html#xmlXPathCompExpr
class compiledXPath {

private: 

    xmlXPathCompExpr* _xmlXPathCompExpr;

public:

    // akin to std::unique_ptr<>::get() ??
    xmlXPathCompExpr* get() { return _xmlXPathCompExpr; }

    // compiledXPath(xmlXPathCompExpr* & other) : _xmlXPathCompExpr(std::exchange(other, nullptr));
    compiledXPath(const std::string& XPath) 
        {
            _xmlXPathCompExpr = xmlXPathCompile(reinterpret_cast<const xmlChar*>(XPath.c_str()));
        }

    // No copy function, so we can't use those
    compiledXPath(const compiledXPath&) = delete;
    compiledXPath& operator=(const compiledXPath&) = delete;

    ~compiledXPath() { xmlXPathFreeCompExpr(_xmlXPathCompExpr); }
    compiledXPath(compiledXPath&& other) noexcept : _xmlXPathCompExpr(std::exchange(other._xmlXPathCompExpr, nullptr)) {};
    compiledXPath& operator=(compiledXPath&& other) noexcept
        {
            std::swap(_xmlXPathCompExpr, other._xmlXPathCompExpr);
            return *this;
        }
};

using compiledXPaths = std::vector<compiledXPath>;

compiledXPaths compileXPaths(const std::vector<std::string>& XPaths)
{
    compiledXPaths compiledXPaths {};

    for ( const std::string& XPath : XPaths )
    {
        compiledXPath compiledXPath {XPath};
        compiledXPaths.emplace_back(std::move(compiledXPath));
    }

    return compiledXPaths;
}

std::string entryContents(std::string& entryPage, compiledXPaths& XPaths, FILE* parsingErrorsFile)
{
    if ( parsingErrorsFile == NULL )
        syslog(LOG_ERR, "Unable to open logfile for parsing errors");
    else 
        // Send parsing errors to this file, see documentation, not intuitive
        xmlSetGenericErrorFunc(reinterpret_cast<void*>(parsingErrorsFile), NULL);

    xmlDoc* document = htmlReadMemory(entryPage.c_str(), entryPage.size(), "http://example.com/", "UTF-8", 0);

    if ( document == NULL )
    {
        syslog(LOG_ERR, "Document not parsed correctly.\n");
        return "";
    }

    std::string entryContents {};

    // I will elect to make a new context for each xmlXPathEvalExpression:
    // https://mail.gnome.org/archives/xml/2008-October/msg00053.html
    for ( compiledXPath& XPath : XPaths )
    {
        xmlXPathContext* context {};
        xmlXPathObject* result {};

        context = xmlXPathNewContext(document);
        if ( context == NULL )
        {
            syslog(LOG_ERR, "Error creating context\n");
            continue;
        }

        result = xmlXPathCompiledEval(XPath.get(), context);
        
        if ( result == NULL )
        {
            syslog(LOG_ERR, "Error evaluating expression\n");
            continue;
        } 

        if ( xmlXPathNodeSetIsEmpty(result->nodesetval) )
        {
            syslog(LOG_ERR, "No result for that XPath\n");
            continue;
        }

        {
            xmlNodeSet* node_set {};
            xmlNode** node_array {};
            size_t node_array_size {};
            xmlBuffer* buffer {}; 
            std::string buffer_contents {};

            node_set = result->nodesetval;
            node_array = node_set->nodeTab;
            node_array_size = node_set->nodeNr;
            buffer = xmlBufferCreate();

            htmlNodeDump(buffer, document, node_array[0]);

            buffer_contents = reinterpret_cast<char*>(buffer->content);

            entryContents += buffer_contents;

            xmlBufferFree(buffer);
            xmlXPathFreeObject(result);
        }

        xmlXPathFreeContext(context);
    }

    xmlFreeDoc(document);

    return entryContents;
}

std::string readFile(const std::filesystem::path& filepath)
{
    std::string contents {};

    {
        std::ifstream input_file {filepath};
        size_t size {};

        input_file.seekg(0, std::ios::end);
        size = input_file.tellg();
        contents = std::string(size, ' ');

        input_file.seekg(0);
        input_file.read(&contents[0], size);
    }

    return contents;
}

void processEntries(std::filesystem::path& entriesDirectoryPath)
{
    FILE* parsingErrorsFile = fopen("/tmp/parsing_errors.txt", "w");

    compiledXPaths XPaths =
        compileXPaths({
                "//div[contains(@class, \"basic_title\")]/h1/text()",
                "//div[@class=\"contents\"]"
            });

    ThreadPool pool(8);
    for( const std::filesystem::directory_entry& entryPath : std::filesystem::directory_iterator(entriesDirectoryPath) )
    {
        auto result = pool.enqueue([&,entryPath]()
                                      {
                                           std::filesystem::path outputPath
                                               {
                                                   // "/home/dacoda/projects/orihime-django/goo-processing/goo-processing/just-the-entry/"
                                                   "/dictionary/"
                                                   + entryPath.path().filename().string()
                                               };

                                           if ( std::filesystem::exists(outputPath) )

                                               return true;

                                           std::ofstream output {outputPath};

                                           std::string entryPage = readFile(entryPath);
                                           std::string contents = entryContents(entryPage, XPaths, parsingErrorsFile);
                                           std::cout << "Got contents of size " << contents.size() << " for " << outputPath << "\n";
                                           output << contents;
                                           return true;
                                       }
            );
    }

    // Make sure to close file if it was correctly opened
    if ( parsingErrorsFile != NULL )
        fclose(parsingErrorsFile);
}
