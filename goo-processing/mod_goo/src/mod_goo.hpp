#pragma once 

// Instead of doing this, let's read in a static CSS file from a
// configuration directive

// https://httpd.apache.org/docs/2.4/developer/modguide.html

const char* goo_response_prefix = R"|(
<html>
    <head>
        <style>
         body
         {
             background-color: rgba(0,0,0,0.2);
         }

         div.word-selection
         {
             max-width: 50em;
             background-color: #d1f9fc;
             margin: auto;
             border-radius: 8px;
         }

         div.word-selection > ul 
         {
             padding: 1em;
             list-style-type: none;
         }
         
         div.word-selection > ul > li
         {
             padding: .5em;
             margin: 1em;
             background-color: white;
             border-style: solid;
             border-color: black;
             border-width: 3px;
             border-radius: 20px;
         }

         div.word-selection > ul > li > h1
         {
             margin: 0;
         }
        </style>
    </head>
    <body>
        <div class="word-selection">
            <ul>
)|";
              
const char* goo_response_suffix = R"|(
            </ul>
        </div>
    </body>
</html>
)|";
