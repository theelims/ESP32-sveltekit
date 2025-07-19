#include "PsychicEndpoint.h"
#include "PsychicHttpServer.h"

PsychicEndpoint::PsychicEndpoint() :
  _server(NULL),
  _uri(""),
  _method(HTTP_GET),
  _handler(NULL)
{
}

PsychicEndpoint::PsychicEndpoint(PsychicHttpServer *server, http_method method, const char * uri) :
  _server(server),
  _uri(uri),
  _method(method),
  _handler(NULL)
{
}

PsychicEndpoint * PsychicEndpoint::setHandler(PsychicHandler *handler)
{
  //clean up old / default handler
  if (_handler != NULL)
    delete _handler;

  //get our new pointer
  _handler = handler;

  //keep a pointer to the server
  _handler->_server = _server;

  return this;
}

PsychicHandler * PsychicEndpoint::handler()
{
  return _handler;
}

String PsychicEndpoint::uri() {
  return _uri;
}

esp_err_t PsychicEndpoint::requestCallback(httpd_req_t *req)
{
  PsychicEndpoint *self = (PsychicEndpoint *)req->user_ctx;
  PsychicHandler *handler = self->handler();
  PsychicRequest request(self->_server, req);

  //make sure we have a handler
  if (handler != NULL)
  {
    if (handler->filter(&request) && handler->canHandle(&request))
    {
      //check our credentials
       if (handler->needsAuthentication(&request))
        return handler->authenticate(&request);

      //pass it to our handler
      return handler->handleRequest(&request);
    }
    //pass it to our generic handlers
    else
      return PsychicHttpServer::notFoundHandler(req, HTTPD_500_INTERNAL_SERVER_ERROR);
  }
  else
    return request.reply(500, "text/html", "No handler registered.");
}

PsychicEndpoint* PsychicEndpoint::setFilter(PsychicRequestFilterFunction fn) {
  _handler->setFilter(fn);
  return this;
}

PsychicEndpoint* PsychicEndpoint::setAuthentication(const char *username, const char *password, HTTPAuthMethod method, const char *realm, const char *authFailMsg) {
  _handler->setAuthentication(username, password, method, realm, authFailMsg);
  return this;
};