#include <string>
#include <sstream>
#include <iostream>
#include <repfunc.h>
#include "htmlreader.h"



int main() {
  std::string base_url("https://www.linuxfromscratch.org/lfs/view/development/");
  std::string html("html");

  if (not DirectoryExists(html))
     CreateDir(html);

  html += "/index.html";
  wget(base_url, html);

  auto ss = ReadFileToStream(html);
  auto document = new Html::Document(ss);
  auto root = document->Root();

  std::cout << "root name = '" << root->Name() << "'" << std::endl;


  //Sleep(5);
  // cleans up all html ressources.
  delete document;






  //  HTML::parser Parser;  Parser.Debug();
  //auto document = Parser.parse(ss.str());
//  auto body = HTML::GetBody(document);
//  auto book = HTML::GetNode(body, "div", "class", "book");
//  auto toc  = HTML::GetNode(book, "div", "class", "toc");
//
//  for(auto li:GetNodeList(toc, "li", "class", "part")) {
//     auto h3 = HTML::GetNode(body, "h3");
//     auto pcdata = GetNode(h3, "pcdata");
//     std::cerr << "h3: " << pcdata->value() << std::endl;
//     }



/*
  std::cout << "book->name() = " << book->name() << "(" << IntToStr(book->number()) << ");   class = " << GetAttribute(book, "class")->value() << std::endl;

  for(auto child:book->child_nodes()) {
     std::cout << "child_node: " << child->name() << "(" << IntToStr(child->number());
     if (child->subnumber() > 0) {
        std::cout << "." << IntToStr(child->subnumber()) << " '" << child->value() << "'" ;
        }
     std::cout << ") class = '" << GetAttribute(child, "class")->value() << "'" << std::endl;
     }
*/
  //delete toc;
  //delete book;
  //delete body;
  //delete document;





  return 0;
}
