#pragma once

#include "Core.h"
#include "EventManager.h"
#include "esp_err.h"
#include <esp_http_server.h>
#include "httpServer.h"

__attribute__((noinline)) static esp_err_t post_handler(httpd_req_t *req, uint8_t type, uint8_t subtype, size_t max_content_len = 0, bool isRequest = false)
{
  const static char *msg_failed = "Failed to receive";

  if (req->content_len > max_content_len)
  {
    httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, msg_failed);
    httpd_resp_sendstr_chunk(req, NULL);
    return ESP_FAIL;
  }

  char *inputData = max_content_len > 0 ? (char *)malloc(sizeof(char) * req->content_len + 1) : nullptr;
  if (max_content_len > 0 && req->content_len == 0)
    inputData[0] = '\0';

  if (max_content_len > 0 && req->content_len > 0 && httpd_req_recv(req, inputData, req->content_len + 1) <= 0)
  {
    httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, msg_failed);
    httpd_resp_sendstr_chunk(req, NULL);
    return ESP_FAIL;
  }
  if (isRequest)
  {
    EventRequestData *eventReq = (EventRequestData *)eventManager
                                     ->AddEvent(type, subtype, inputData, isRequest);
    eventReq->Wait();
    httpd_resp_sendstr(req, (char *)eventReq->getValue());
    eventReq->Done();
  }
  else
    eventManager->AddEvent(type, subtype, inputData, isRequest);

  httpd_resp_sendstr_chunk(req, NULL);
  return ESP_OK;
}

__attribute__((noinline)) static esp_err_t HandleMethod(const char *method, httpd_req_t *req, esp_err_t(handler)(httpd_req_t *))
{
  if (strlen(req->uri) > strlen(method) && cmpstr(&req->uri[strlen(req->uri) - strlen(method)], method))
    return handler(req);
  return ESP_FAIL;
}

#define API_METHOD(url, handler)         \
  err = HandleMethod(url, req, handler); \
  if (err > ESP_FAIL)                    \
    return err;


