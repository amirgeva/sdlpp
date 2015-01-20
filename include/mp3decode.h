#ifndef H_MP3DECODE
#define H_MP3DECODE

class MP3Decoder
{
protected:
  virtual ~MP3Decoder() {}
public:
  virtual void destroy() = 0;
  virtual void decode(const unsigned char* input, unsigned inlen, unsigned char* output, unsigned* outlen) = 0;
  virtual unsigned get_frequency() = 0;
  virtual unsigned get_channels() = 0;
};

MP3Decoder* create_mp3_decoder();


#endif // H_MP3DECODE
