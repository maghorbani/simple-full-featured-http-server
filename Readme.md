What is it
==========
It is a simple, general purpose web sever, written in C/C++.

features
========
*   reading http/1.1 request and creates a `Request` object, with structuring the request params:
    *  query params
    *  url encoded form data
    *  multipart form data (just key, value data. multipart form data is not handled yet)
    *  json body 
*   general purpose request handling
    *   can get functions for different `routes` and `methods`
    *   can get functions for all `routes` of a `method`
*   json serializer and parser
*   ability to respond `html`, `json`, `plain text` and `binary` data with builtin methods
    *   also ability to respond any kind of `content-type` is available, but it should be handled in the application that is using the library

ready examples
==============
**take a loop at `examples` directory for examples**
*   example file server
*   example json CRUD app
*   minimal get request example
  
how to use
==========
simplest example
```cpp
#include <httpServer.h>
 int main(){
    http::Server server;

    server.get("/", [](http::Request req, http::Response res){
        res.write("Hello World");
    });
    
    server.listen(8000);
 }
 ```

 by default it uses half of available threads on the machine for responding, it can be customized (for example: `12`)
 ```cpp
    server.listen(8000, 12);
 }
 ```
`get`, `post`, `put`,`patch`, and `delete` methods are available:
```cpp
    server.get("/", handler);
    server.post("/", handler);
    server.put("/", handler);
    server.patch("/", handler);
    server.delete("/", handler);
```
the respond `content-type` can be changed like:
```cpp
    std::string _json("[12,13,15,\"hello\"]");
    res.contentType('application/json').write(_json.c_str());
```
or simpler
```cpp
    res.json("{\"message\": \"hello\"}");
```
for json case, it is possible to create a json object:
```cpp

#include "json.h"
// other lines
// ...
// ...
    server.get("/", [](http::Request req, http::Response res){
        http::json _json = http::json::object();
        _json["message"] = "Hello World";
        res.json(_json);
    });
```
and the status can be changed like:
```cpp
    server.get("/", [](http::Request req, http::Response res){
        http::json _json = http::json::object();
        _json["message"] = "Bad Request";
        _json["success"] = false;
        res.status(400).json(_json);
    });
```
also html data can be sent like:
```cpp
    server.get("/", [](http::Request req, http::Response res){
        res.html("<h1>Hello World</h1>");
    });
```

by default, a `plain/text` respond with status `404` will be sent to clients requesting the routes you did not handled, but it is possible to handle it in the application:
```cpp
    server.get("*", [](http::Request req, http::Response res) {
      res.status(404).html(
          "<div style=\"display:flex; width:100% ;height:100%; "
          "align-items:center; justify-content:center\">"
          "<p style=\"padding:5px 20px; background-color:#ccc; "
          "border-radius:5px\">Not Found</p>"
          "</div>");
    });
```
or from a file
```cpp
    server.get("*", [](http::Request req, http::Response res) {
        std::ifstream file("path/to/404.html", std::ios::binary);
        std::vector<char> buffer(std::istreambuf_iterator<char>(file), {});
        res.contentType("text/html").write(&buffer[0], buffer.size());
    });
```

Maintenance
===========
using the app inside `examples/simpleGetRequest` I could handle **10,000** simultaneous clients (but the speed damps in this case)

Performance
===========
About the `socket`, `read`, `write`, `select`, and ... parts. these functions are not thread safe, of course, reading is ok to be called from multiple threads (but should be considered, maybe locked writing, affect reading on other threads). but to write to socket, all the threads and clients should wait for a write operation to be done.

so long story short, any thing other than write (write to socket, and write to multithreading management data), is multithreaded; and eventually, responding to clients could not be fully multithreaded. 

but the acceptor is so fast, it just accepts new clients and throws the `_fd` of new clients, somewhere else, then it will notify a responder thread (like: hey, a new client, service....) and the client will be served...

considering all these, increasing the number of thread, does not make a meaningful increase in benchmark results.

for example on an `i5` with `4 threads` and `8GB of RAM` machine, (the app uses really low amount of RAM), and testing with **Apache AB** it takes something like `5 seconds` to respond **100,000** requests, with a simple *OK* message.
