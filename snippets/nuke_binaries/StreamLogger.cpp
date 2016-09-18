// StreamLogger.cpp
// Copyright (c) 2016 Anthony Tan

static const char* const HELP = "monitors requests and validates through a comp stream, dumping to stdout";

#include "DDImage/PixelIop.h"
#include "DDImage/Row.h"
#include "DDImage/Knobs.h"
#include "DDImage/NoIop.h"

#include <chrono>
#include <iostream>
#include <iomanip>
#include <ctime>

#define TIMEBUFFER_STRING_SIZE 80
//  this could be smaller, but for the sake of the ~15chrs during dev.. 

using namespace DD::Image; 
class StreamLogger : public NoIop
{
  // these are knobs.
  const char *_logger_tag;
  bool _watch_request;
  bool _watch_validate;

  time_t rawtime;
  struct tm timeinfo;
  struct timespec now;
  char time_buffer[TIMEBUFFER_STRING_SIZE];   

public:
  void in_channels(int input, ChannelSet& mask) const;
  StreamLogger(Node* node) : NoIop(node),
      _logger_tag("[value name]"),
      _watch_request(true),
      _watch_validate(true)
      {}

  bool pass_transform() const { return true; }
  void pixel_engine(const Row &in, int y, int x, int r, ChannelMask, Row & out);
  void _request(int x, int y, int r, int t, ChannelMask channels, int count);  
  virtual void knobs(Knob_Callback);
  static const Iop::Description d;
  const char* Class() const { return d.name; }
  const char* node_help() const { return HELP; }
  void _validate(bool);
};


void StreamLogger::_request(int x, int y, int r, int t, ChannelMask channels, int count){

    if (_watch_request){
        clock_gettime(CLOCK_REALTIME, &now);
        gmtime_r(&(now.tv_sec), &timeinfo);
        strftime(time_buffer,TIMEBUFFER_STRING_SIZE,"[%Y-%m-%d %H:%M:%S.", &timeinfo);

        std::cout << \
           time_buffer << now.tv_nsec << " UTC]" << \
           "[" << _logger_tag<<"][_request]" << \
           "{" << \
           "'channels':'" << channels << "'," << \
           "count:" << count << "," << \
           "x:" << x << "," << \
           "y:" << y << "," << \
           "r:" << r << "," << \
           "t:" << t << \
           "}" << std::endl; 
    }

    input(0)->request(x, y, r, t, channels, count );
}


void StreamLogger::_validate(bool for_real)
{
    if (_watch_validate) {
        clock_gettime(CLOCK_REALTIME, &now);
        gmtime_r(&(now.tv_sec), &timeinfo);
        strftime(time_buffer,TIMEBUFFER_STRING_SIZE,"[%Y-%m-%d %H:%M:%S.", &timeinfo);
        std::cout << \
           time_buffer << now.tv_nsec << " UTC]" << \
           "[" << _logger_tag << "][_validate]" << \
           "{" << \
           "'OutputContext.frame':" << outputContext().frame() << "," <<
           "'OutputContext.viewname':'" << outputContext().viewname() << "'" <<
           "}" << std::endl; }

   copy_info();
}


void StreamLogger::in_channels(int input, ChannelSet& mask) const
{
  // mask is unchanged
  // std::cout << "in_channels input:" << input << std::endl;
  // std::cout << "chanmask:" << mask << std::endl;
  // this is a very expensive call to hook when you're stream outting :P
  // 
}


// TODO: convert this back to a PixelIop if you need to inspect inside
//       engine. Although. If you're doing that, you're probably screwed
/*
void StreamLogger::pixel_engine(const Row& in, int y, int x, int r,
                       ChannelMask channels, Row& out)
{
  foreach (z, channels) {
    const float* inptr = in[z] + x;
    const float* END = inptr + (r - x);
    float* outptr = out.writable(z) + x;
    while (inptr < END)
      *outptr++ = *inptr++;
  }
}*/

void StreamLogger::knobs(Knob_Callback f)
{
  String_knob(f, &_logger_tag, "logging_tag","Log Tag");
  Bool_knob(f, &_watch_request, "watch_request"); 
  SetFlags(f, Knob::STARTLINE);
  Bool_knob(f, &_watch_request, "watch_validate");
  SetFlags(f, Knob::STARTLINE);
}



static Iop* build(Node* node) { return new StreamLogger(node); }
const Iop::Description StreamLogger::d("StreamLogger", 0, build);

