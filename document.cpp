/*******************************************************************************
 * htmlreader, a small C++ wrapper to libxml2 html.
 * See the README file for copyright information and how to reach the author.
 ******************************************************************************/
#include "htmlreader.h"
#include <libxml/HTMLparser.h>


Html::Document::Document(std::stringstream& ss, uint32_t Options) {
  /* check compat: installed libxml vs compile time libxml
   * NOTE: semicolon inside macro - why..?
   */
  LIBXML_TEST_VERSION

  /* xmlInitParser - Initialization function for the XML parser.
   * Call once from the main thread before using the library in multithreaded
   * programs.
   */
  static bool once = false;
  if (not once) {
     xmlInitParser();
     once = true;
     }

  /* Parse an HTML document from memory.
   * The input buffer *must not* contain any terminating null bytes.
   */
  auto XmlDocPtr = htmlReadMemory(ss.str().c_str(), ss.str().size(), nullptr, nullptr, Options);
  if (XmlDocPtr)
     root = new Node(xmlDocGetRootElement(XmlDocPtr));
  else
     root = nullptr;
  priv = XmlDocPtr;
}

Html::Document::~Document() {
  auto XmlDocPtr = (xmlDoc*) priv;
  if (XmlDocPtr) {
     // free our own memory
     delete root;
     // free memory for XML document (DOM representation)
     xmlFreeDoc(XmlDocPtr);
     }
  // clean up memory allocated by libxml2
  xmlCleanupParser();
}

Html::Node* Html::Document::Root(void) {
  return root;
}
