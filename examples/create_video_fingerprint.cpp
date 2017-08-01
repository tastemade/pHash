#include "stdio.h"
#include "pHash.h"
#include "CImg.h"
#include "cimgffmpeg.h"
#include <string>
#include <stdexcept>

#define NUM_HASHED_FRAMES 5
#define NUM_HASHES_PER_FRAME 5

#define VERSION 2

struct fingerprint_t {
  ulong64 hashes[NUM_HASHED_FRAMES * NUM_HASHES_PER_FRAME];
};


fingerprint_t createFingerprint(const char *filename);
void openVideo(const char* filename, AVFormatContext** fileFormatCtx, AVStream** videoStream);
CImgList<uint8_t>* getFrames(AVFormatContext *fileFormatCtx, AVStream* videoStream);
long getNumFrames(AVStream* videoStream);
void readFrames(CImgList<uint8_t> *frameImages, long* frameIndexes, int numFrameIndexes, AVFormatContext *fileFormatCtx, AVStream* videoStream);
CImg<float>* createDCTMatrixImage(const int n);



int main(int argc, char** argv) {
  av_log_set_level(AV_LOG_QUIET);
  av_register_all();
  
  const char* usage = "USAGE: created_video_fingerprint \"path/to/video.mp4\" [-q]\n";
  
  if (argc < 2) {
    printf("%s", usage);
    exit(1);
  }
  
  const char* filepath = argv[1];
  if (!filepath) {
    printf("%s", usage);
    exit(1);
  }
  
  bool quiet = false;
  if (argc >= 3 && string(argv[2]).compare("-q") == 0) {
    quiet = true;
  }
  
  fingerprint_t fingerprint = createFingerprint(filepath);
  
  if (!quiet) {
    fprintf(stderr, "{\"version\": %i}\n", VERSION);
  }
  
  for (int i = 0; i < NUM_HASHED_FRAMES * NUM_HASHES_PER_FRAME; ++i) {
    if (i > 0) {
      printf(" ");
    }
    // convert unsigned int64 to signed int64 (without changing binary)
    unsigned long long hash = fingerprint.hashes[i];
    long long sHash = *(long long*)&hash;
    printf("%lld", sHash);
  }
  printf("\n");
  
  exit(0);
}


fingerprint_t createFingerprint(const char *filename) {
  AVFormatContext *fileFormatCtx;
  AVStream* videoStream;
  openVideo(filename, &fileFormatCtx, &videoStream);
  
  CImgList<uint8_t>* frames = getFrames(fileFormatCtx, videoStream);
  //frames->save((string(filename) + "-out.png").c_str(), 1); // TODO: turn off
  
  fingerprint_t fingerprint = {};
  
  CImg<float> meanfilter(7, 7, 1, 1, 1);
  CImg<float> *dctMatrixImage  = createDCTMatrixImage(32);
  CImg<float> dctMatrixTransposeImage = dctMatrixImage->get_transpose();
  
  for (int i = 0; i < NUM_HASHED_FRAMES * NUM_HASHES_PER_FRAME; ++i) {
    CImg<uint8_t> frame = frames->at(i);
    
    CImg<float> img;
    if (frame.spectrum() == 3) {
      img = frame.RGBtoYCbCr().channel(0).get_convolve(meanfilter);
    }
    else if (frame.spectrum() == 4) {
      img = frame.crop(
        0, 0, 0, 0,
        img.width() - 1, img.height() - 1, img.depth() - 1, 2
      ).RGBtoYCbCr().channel(0).get_convolve(meanfilter);
    }
    else {
      img = frame.channel(0).get_convolve(meanfilter);
    }
    
    CImg<float> dctImage = (*dctMatrixImage) * img * dctMatrixTransposeImage;
    
    CImg<float> subsec = dctImage.crop(1, 1, 8, 8).unroll('x');
    float median = subsec.median();
    
    ulong64 hash = 0x0000000000000000;
    ulong64 one  = 0x0000000000000001;
    
    for (int j = 0; j < 64; ++j) {
      if (subsec(j) > median) {
        hash |= one;
      }
      one = one << 1;
    }
    
    fingerprint.hashes[i] = hash;
  }
  
  
  
  frames->clear();
  delete frames;
  frames = NULL;
  
  delete dctMatrixImage;
  dctMatrixImage = NULL;
  
  avcodec_close(videoStream->codec);
  avformat_close_input(&fileFormatCtx);
  
  return fingerprint;
}

void openVideo(const char* filename, AVFormatContext** fileFormatCtxPtr, AVStream** videoStreamPtr) {
  // open the video file
  AVFormatContext* fileFormatCtx = NULL;
  int ret = avformat_open_input(&fileFormatCtx, filename, NULL, NULL);
  if (ret < 0) {
    char errstr[200];
    throw runtime_error(string("Error opening video file: ") + av_make_error_string(errstr, 200, ret));
  }
  
  // find stream info
  ret = avformat_find_stream_info(fileFormatCtx, NULL);
  if (ret < 0) {
    char errstr[200];
    throw runtime_error(string("Error finding stream info: ") + av_make_error_string(errstr, 200, ret));
  }
  
  // get the "best" video stream
  ret = av_find_best_stream(fileFormatCtx, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
  if (ret < 0) {
    char errstr[200];
    throw runtime_error(string("Error finding best video steam: ") + av_make_error_string(errstr, 200, ret));
  }
  int videoStreamIndex = ret;
  
  AVStream* videoStream = fileFormatCtx->streams[videoStreamIndex];
  
  // get the video steam codec decoder
  if (!videoStream->codec) {
    throw runtime_error("Video steam does not have a codec context.");
  }
  
  AVCodec *videoDecoder = avcodec_find_decoder(videoStream->codec->codec_id);
  if (!videoDecoder) {
    throw runtime_error("Unable to find decoder for codec ID.");
  }
  
  // open the codec
  ret = avcodec_open2(videoStream->codec, videoDecoder, NULL);
  if (ret < 0) {
    char errstr[200];
    throw runtime_error(string("Error opening codec: ") + av_make_error_string(errstr, 200, ret));
  }
  
  *fileFormatCtxPtr = fileFormatCtx;
  *videoStreamPtr   = videoStream;
}

CImgList<uint8_t>* getFrames(AVFormatContext* fileFormatCtx, AVStream* videoStream) {
  long numFrames = getNumFrames(videoStream);
  
  long frameIndexes[NUM_HASHED_FRAMES * NUM_HASHES_PER_FRAME] = {};
  for (int i = 0; i < NUM_HASHED_FRAMES; ++i) {
    long baseFrameIndex = (numFrames * (i+1)) / (NUM_HASHED_FRAMES + 1);
    
    for (int j = 0; j < NUM_HASHES_PER_FRAME; ++j) {
      frameIndexes[i * NUM_HASHES_PER_FRAME + j] = baseFrameIndex + (j - (NUM_HASHES_PER_FRAME / 2));
    }
  }
  
  // check frame indexes (should only happen if there are not enough frames)
  if (frameIndexes[0] < 0) {
    throw runtime_error("Invalid frame indexes chosen. First index < 0");
  }
  if (frameIndexes[NUM_HASHED_FRAMES * NUM_HASHES_PER_FRAME - 1] > numFrames - 1) {
    throw runtime_error("Invalid frame indexes chosen. Last index > numFrames - 1");
  }
  for (int i = 1; i < NUM_HASHED_FRAMES * NUM_HASHES_PER_FRAME; ++i) {
    if (frameIndexes[i] <= frameIndexes[i - 1]) {
      throw runtime_error("Invalid frame indexes chosen. Indexes not in ascending order.");
    }
  }
  
  CImgList<uint8_t>* frames = new CImgList<uint8_t>();
  readFrames(frames, frameIndexes, NUM_HASHED_FRAMES * NUM_HASHES_PER_FRAME, fileFormatCtx, videoStream);
  
  if (frames->size() != NUM_HASHED_FRAMES * NUM_HASHES_PER_FRAME) {
    throw runtime_error("Incorrect number of frames read.");
  }
  
  return frames;
}

long getNumFrames(AVStream* videoStream) {
  // get the number of frames from the steam
  long numFrames = videoStream->nb_frames;
  if (numFrames > 0){
    return numFrames;
  }
  
  // count frames in the stream
  numFrames = (long)av_index_search_timestamp(videoStream, videoStream->duration, AVSEEK_FLAG_ANY|AVSEEK_FLAG_BACKWARD);
  if (numFrames > 0){
    return numFrames;
  }
  
  int timebase = videoStream->time_base.den / videoStream->time_base.num;
  numFrames = videoStream->duration / timebase;
  if (numFrames > 0){
    return numFrames;
  }
  
  throw runtime_error("Unable to get number of frames.");
}

// NOTE: frameIndexes must be in ascending order
void readFrames(CImgList<uint8_t> *frameImages, long* frameIndexes, int numFrameIndexes, AVFormatContext *fileFormatCtx, AVStream* videoStream) {
  int width = 32;
  int height = 32;
  PixelFormat pixelFormat = PIX_FMT_GRAY8;
  int channels = 1;
  
  // allocate the frames
  AVFrame* frame = avcodec_alloc_frame();
  AVFrame* img = avcodec_alloc_frame();
  if (!frame || !img) {
    throw runtime_error("Unable to allocate frames.");
  }
  
  // allocate the buffer
  int numBytes = avpicture_get_size(pixelFormat, width, height);
  uint8_t* buffer = (uint8_t *)av_malloc(numBytes * sizeof(uint8_t));
  if (!buffer) {
    throw runtime_error("Unable to allocate buffer.");
  }
  
  // setup the converted frame
  avpicture_fill((AVPicture *)img, buffer, pixelFormat, width, height);
  
  // create the scaler context
  SwsContext *swsCtx = sws_getContext(
    videoStream->codec->width, videoStream->codec->height, videoStream->codec->pix_fmt,
    width, height, pixelFormat,
    SWS_BICUBIC, NULL, NULL, NULL
  );
  
  // initialize packet
  AVPacket packet;
  av_init_packet(&packet);
    
  // read frames from the video
  bool readFromCache = false;
  int targetFrameIndexNum = 0;
  long frameIndex = 0;
  while (targetFrameIndexNum < numFrameIndexes) {
    long targetFrameIndex = frameIndexes[targetFrameIndexNum];
    
    // set packet data to NULL, let the demuxer fill it
    packet.data = NULL;
    packet.size = 0;
    
    if (!readFromCache) {
      // read a frame from the file into the packet
      int ret = av_read_frame(fileFormatCtx, &packet);
      
      // if we have reached the end of the file, stop reading from the file and start reading from the cache
      if (ret == AVERROR_EOF) {
        readFromCache = true;
      }
      else if (ret < 0) {
        char errstr[200];
        throw runtime_error(string("Error reading frame from file: ") + av_make_error_string(errstr, 200, ret));
      }
    }
    
    // check if the frame was read from the correct video steam
    if (packet.stream_index != videoStream->index) {
      continue;
    }
    
    // decode the frame using HACK for CorePNG to decode as normal PNG by default (same method used by ffmpeg)
    AVPacket decodePacket;
    av_init_packet(&decodePacket);
    decodePacket.data = packet.data;
    decodePacket.size = packet.size;
    decodePacket.flags = AV_PKT_FLAG_KEY; 
    
    int frameFinished;
    int ret = avcodec_decode_video2(videoStream->codec, frame, &frameFinished, &decodePacket);
    if (ret < 0) {
      char errstr[200];
      throw runtime_error(string("Error decoding video frame: ") + av_make_error_string(errstr, 200, ret));
    }
    
    av_free_packet(&decodePacket);
    
    if (!frameFinished) {
      if (readFromCache) {
        // if we did not get a complete frame and we are reading from the cache then we are out of frames
        throw new runtime_error("Depleted frames.");
      }
      
      continue;
    }
    
    // check if the frame is one we want
    if (frameIndex == targetFrameIndex) {
      // scale the frame
      sws_scale(swsCtx, frame->data, frame->linesize, 0, videoStream->codec->height, img->data, img->linesize);
      
      // convert the frame to an image
      CImg<uint8_t> frameImage;
      frameImage.assign(*img->data, channels, width, height, 1, true);
      frameImage.permute_axes("yzcx");
      
      // add the image to the list
      frameImages->push_back(frameImage);
      
      ++targetFrameIndexNum;
    }
    
    ++frameIndex;
  }
  
  av_free_packet(&packet);
  
  av_free(buffer);
  buffer = NULL;
  
  av_free(img);
  img = NULL;
  
  av_free(frame);
  frame = NULL;
  
  sws_freeContext(swsCtx);
  swsCtx = NULL;
}

// Discrete cosine transform
CImg<float>* createDCTMatrixImage(const int n) {
  CImg<float> *dctMatrixImage = new CImg<float>(n, n, 1, 1, 1/sqrt((float)n));
  
  const float c1 = sqrt(2.0/n);
  for (int x = 0; x < n; x++) {
    for (int y = 1; y < n; y++) {
      *dctMatrixImage->data(x, y) = c1 * cos((cimg::PI / 2 / n) * y * (2 * x + 1));
    }
  }
  
  return dctMatrixImage;
}