/*
  win32ole_gettimeofday.cc
*/

#include "node_win32ole.h"
#include <node.h>
#include <nan.h>
#include "ole32core.h"

#ifdef _WIN32
#include <sys/timeb.h>
#endif

using namespace v8;
using namespace ole32core;

namespace node_win32ole {

void timeval_gettimeofday(struct timeval *ptv)
{
#ifdef _WIN32
  _timeb btv;
  _ftime(&btv);
  ptv->tv_sec = btv.time; // unsafe convert __time64_t to long
  ptv->tv_usec = 1000LL * btv.millitm;
#else
  gettimeofday(ptv, NULL); // no &tz
#endif
}

NAN_METHOD(Method_gettimeofday)
{
  if (info.Length() < 1 || !info[0]->IsObject())
  {
    return Nan::ThrowTypeError("Argument 1 is not an object");
  }
  struct timeval tv;
  timeval_gettimeofday(&tv);
  Handle<Object> buf = Nan::To<Object>(info[0]).ToLocalChecked();
  if(node::Buffer::Length(buf) == sizeof(struct timeval))
  {
    memcpy(node::Buffer::Data(buf), &tv, sizeof(struct timeval));
  }
}

NAN_METHOD(Method_sleep) // ms, bool: msg, bool: \n
{
  if(info.Length() < 1 || !info[0]->IsInt32())
    return Nan::ThrowTypeError("type of argument 1 must be Int32");
  long ms = Nan::To<int32_t>(info[0]).FromJust();
  bool msg = false;
  if(info.Length() >= 2){
    if(!info[1]->IsBoolean())
      return Nan::ThrowTypeError("type of argument 2 must be Boolean");
    msg = Nan::To<bool>(info[1]).FromJust();
  }
  bool crlf = false;
  if(info.Length() >= 3){
    if(!info[2]->IsBoolean())
      return Nan::ThrowTypeError("type of argument 3 must be Boolean");
    crlf = Nan::To<bool>(info[2]).FromJust();
  }
  if(ms){
    if(msg){
      printf("waiting %ld milliseconds...", ms);
      if(crlf) printf("\n");
    }
    struct timeval tv_start;
    timeval_gettimeofday(&tv_start);
    while(true){
      struct timeval tv_now;
      timeval_gettimeofday(&tv_now);
      // it is wrong way (must release CPU)
      if((double)tv_now.tv_sec + (double)tv_now.tv_usec / 1000. / 1000.
      >= (double)tv_start.tv_sec + (double)tv_start.tv_usec / 1000. / 1000.
        + (double)ms / 1000.) break;
    }
  }
}

} // namespace node_win32ole
