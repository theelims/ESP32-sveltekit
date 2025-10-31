#include "PsychicStreamResponse.h"
#include "PsychicResponse.h"
#include "PsychicRequest.h"

PsychicStreamResponse::PsychicStreamResponse(PsychicRequest *request, const String& contentType)
 : PsychicResponse(request), _buffer(NULL) {

  setContentType(contentType.c_str());
  addHeader("Content-Disposition", "inline");
}

 
PsychicStreamResponse::PsychicStreamResponse(PsychicRequest *request, const String& contentType, const String& name)
  : PsychicResponse(request), _buffer(NULL) {

  setContentType(contentType.c_str());

  char buf[26+name.length()];
  snprintf(buf, sizeof (buf), "attachment; filename=\"%s\"", name.c_str());
  addHeader("Content-Disposition", buf);
}

 
PsychicStreamResponse::~PsychicStreamResponse()
{
  endSend();
}


esp_err_t PsychicStreamResponse::beginSend()
{
  if(_buffer)
    return ESP_OK;

  //Buffer to hold ChunkPrinter and stream buffer. Using placement new will keep us at a single allocation.
  _buffer = (uint8_t*)heap_caps_malloc_prefer(STREAM_CHUNK_SIZE + sizeof(ChunkPrinter), 2, MALLOC_CAP_SPIRAM, MALLOC_CAP_INTERNAL);
  
  if(!_buffer)
  {
    /* Respond with 500 Internal Server Error */
    httpd_resp_send_err(_request->request(), HTTPD_500_INTERNAL_SERVER_ERROR, "Unable to allocate memory.");
    return ESP_FAIL;
  }

  _printer = new (_buffer) ChunkPrinter(this, _buffer + sizeof(ChunkPrinter), STREAM_CHUNK_SIZE);

  sendHeaders();
  return ESP_OK;
}


esp_err_t PsychicStreamResponse::endSend()
{
  esp_err_t err = ESP_OK;
  
  if(!_buffer)
    err = ESP_FAIL;
  else
  {
    _printer->~ChunkPrinter(); //flushed on destruct
    err = finishChunking();
    heap_caps_free(_buffer);
    _buffer = NULL;
  }
  return err;
}


void PsychicStreamResponse::flush()
{
  if(_buffer)
    _printer->flush();
}


size_t PsychicStreamResponse::write(uint8_t data)
{
  return _buffer ? _printer->write(data) : 0;
}


size_t PsychicStreamResponse::write(const uint8_t *buffer, size_t size)
{
  return _buffer ? _printer->write(buffer, size) : 0;
}


size_t PsychicStreamResponse::copyFrom(Stream &stream)
{
  if(_buffer)
    return _printer->copyFrom(stream);

  return 0;
}
