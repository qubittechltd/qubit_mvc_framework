#include "redirect.h"
#include "QDebug"
QByteArray redirect(QByteArray new_route){
QByteArray data=""
"HTTP/1.1 302 Found\r\n"
"Connection: keep-alive\r\n"
"Content-Type: text/html; charset=iso-8859-1\r\n";
"Location: " + new_route +"\r\n\r\n"

"<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML 2.0//EN\">\r\n"
"<html>\r\n"
"   <head>\r\n"
"       <title>302 Found</title>\r\n"
"   </head>\r\n"
"   <body>\r\n"
"       <h1>Moved Temporarily</h1>\r\n"
"       <p>The document has moved <a href=\""+new_route+"\">here</a>.</p>\r\n"
"   </body>\r\n"
"</html>\r\n";
return data;
}
