#include <stdio.h>
#include <iostream>
#include <fstream>
#include <v8.h>
#include <nan.h>
//#include <libraw/libraw.h>

#include "libraw/libraw.h"

namespace node_libraw {
  using v8::Exception;
  using v8::Function;
  using v8::FunctionCallbackInfo;
  using v8::FunctionTemplate;
  using v8::Isolate;
  using v8::Local;
  using v8::Number;
  using v8::Object;
  using v8::String;
  using v8::Value;
  using v8::Null;

  NAN_METHOD(Extract) {
    Nan::HandleScope scope;

    LibRaw RawProcessor;

    v8::String::Utf8Value filenameFromArgs(info[0]->ToString());
    std::string filename = std::string(*filenameFromArgs);
    v8::String::Utf8Value outputFromArgs(info[1]->ToString());
    std::string output = std::string(*outputFromArgs);

    Nan::Callback *callback = new Nan::Callback(Local<Function>::Cast(info[2]));

    std::ifstream file;
    file.open(filename.c_str(), std::ios::binary | std::ios::ate);
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<char> buffer(size);

    if (file.read(buffer.data(), size)) {
      RawProcessor.imgdata.params.output_tiff = 0;
      RawProcessor.imgdata.params.output_bps = 8;
      RawProcessor.imgdata.params.use_camera_wb = 1;
      RawProcessor.imgdata.params.user_qual = 0;

      RawProcessor.open_buffer(buffer.data(), size);
      RawProcessor.unpack();
      RawProcessor.dcraw_process();

      libraw_processed_image_t *image = RawProcessor.dcraw_make_mem_image();

      Local<v8::Value> argv[4] = {
        Nan::Null(),
        Nan::CopyBuffer(reinterpret_cast<const char*>(image->data), image->data_size).ToLocalChecked(),
        Nan::New<Number>(image->width),
        Nan::New<Number>(image->height)
      };

      callback->Call(4, argv);

      LibRaw::dcraw_clear_mem(image);
      RawProcessor.recycle();
    }

    file.close();
  }


  NAN_METHOD(ExtractThumb) {
    Nan::HandleScope scope;

    LibRaw RawProcessor;

    v8::String::Utf8Value filenameFromArgs(info[0]->ToString());
    std::string filename = std::string(*filenameFromArgs);
    v8::String::Utf8Value outputFromArgs(info[1]->ToString());
    std::string output = std::string(*outputFromArgs);

    Nan::Callback *callback = new Nan::Callback(Local<Function>::Cast(info[2]));

    std::ifstream file;
    file.open(filename.c_str(), std::ios::binary | std::ios::ate);
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<char> buffer(size);

    if (file.read(buffer.data(), size)) {
      RawProcessor.open_buffer(buffer.data(), size);
      RawProcessor.unpack();
      RawProcessor.unpack_thumb();

      if (RawProcessor.imgdata.thumbnail.tformat == LIBRAW_THUMBNAIL_JPEG) {
        //
      }

      Local<v8::Value> argv[2] = {
        Nan::Null(),
        Nan::NewBuffer(RawProcessor.imgdata.thumbnail.thumb, RawProcessor.imgdata.thumbnail.tlength).ToLocalChecked()
      };

      callback->Call(2, argv);
      
      RawProcessor.recycle();
    }

    file.close();
  }

  void init(Local<Object> exports) {
    Nan::Set(
      exports,
      Nan::New<String>("extractThumb").ToLocalChecked(),
      Nan::GetFunction(Nan::New<v8::FunctionTemplate>(ExtractThumb)).ToLocalChecked()
    );

    Nan::Set(
      exports,
      Nan::New<String>("extract").ToLocalChecked(),
      Nan::GetFunction(Nan::New<v8::FunctionTemplate>(Extract)).ToLocalChecked()
    );
  }

  NODE_MODULE(libraw, init)
}
