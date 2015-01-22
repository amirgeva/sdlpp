#include <sdlpp.h>
#include <sysdep.h>
#include <mp3decode.h>


namespace SDLPP {

static std::ofstream* flog=0;
static const int FRAMES = 32;
static const int BUFFER_SIZE = SOUND_BUFFER_SIZE;
Accumulator<int> SoundManager::s_MixingBuffer(BUFFER_SIZE);

namespace {
  class LogInit
  {
  public:
    LogInit() 
    { 
      #ifdef _DEBUG
      //flog=new std::ofstream("sound.log"); 
      #endif
    }

    ~LogInit()
    {
      delete flog;
      flog=0;
    }
  } g_LogInit;
}


////////////////////////////////////////////////
//  Code for integrating SDL_sound into this library.


namespace SDL_sound {
  
  class MP3DecoderWrapper : public Singleton
  {
  public:
    static MP3DecoderWrapper* instance()
    {
      static std::unique_ptr<MP3DecoderWrapper> ptr(new MP3DecoderWrapper);
      return ptr.get();
    }
  
    virtual void shutdown() override
    {
    }
    
    MP3Decoder* create()
    {
      if (!create_mp3_decoder) return 0;
      return create_mp3_decoder();
    }
  private:
    friend struct std::default_delete<MP3DecoderWrapper>;
    MP3DecoderWrapper() 
    {
      create_mp3_decoder = (create_func)get_function("mp3decode", "create_mp3_decoder");
    }
    ~MP3DecoderWrapper() {}
    MP3DecoderWrapper(const MP3DecoderWrapper&) {}
    MP3DecoderWrapper& operator= (const MP3DecoderWrapper&) { return *this; }

    typedef MP3Decoder* (*create_func)();
    create_func create_mp3_decoder;
  };
  
}


SoundClip::SoundClip(const xstring& filename)
: m_Wave(0)
{
  ResourceFile* rf = get_default_resource_file();
  if (rf && !filename.empty())
  {
    SDL_RWops* rw=rf->get(filename);
    if (!rw) 
      THROW ("Resource not found: "+filename);
    load_from_rwop(rw,filename);
  }
  else
    THROW("Resource not found: " + filename);
}

SoundClip::~SoundClip()
{
  destroy();
}

void SoundClip::destroy()
{
  if (m_Wave)
  {
    SDL_FreeWAV(m_Wave);
    m_Wave = 0;
  }
  m_Buffer.clear();
}

void SoundClip::load_from_rwop(SDL_RWops* rwops, const char* name)
{
  SDL_AudioSpec *spec=SoundManager::instance()->get_audio_spec();
  SDL_AudioSpec wav_spec;
  Uint32 length;
  if (!SDL_LoadWAV_RW(rwops, 1, &wav_spec, &m_Wave, &length))
    THROW("File not found " << name);
  if (spec->format     != wav_spec.format     ||
      spec->channels   != wav_spec.channels   || 
      spec->freq       != wav_spec.freq)
  {
    SDL_AudioCVT cvt;
    if (SDL_BuildAudioCVT(&cvt,wav_spec.format,wav_spec.channels,wav_spec.freq,
                          spec->format,spec->channels,spec->freq) < 0)
      THROW("WAV format cannot be converted to match audio hardware: " << name);    
    cvt.len=length;
    int new_len=length * cvt.len_mult;
    m_Buffer.resize(new_len);
    //Uint8* new_buffer=new Uint8[new_len];
    std::copy(m_Wave, m_Wave + length, &m_Buffer[0]);
    SDL_FreeWAV(m_Wave);
    m_Wave = 0;
    cvt.buf=&m_Buffer[0];
    int rc=SDL_ConvertAudio(&cvt);
  }
}

////////////////////////////////////////////////////////////////////

SoundStream::SoundStream(const xstring& filename)
: m_Data(new Data(BUFFER_SIZE*2*FRAMES))
{
  if (!filename.empty()) load(filename);
}

SoundStream::SoundStream(ResourceFile& rf, const xstring& name)
: m_Data(new Data(BUFFER_SIZE*2*FRAMES))
{
  SDL_RWops* rw=rf.get(name);
  if (!rw) 
    THROW ("Resource not found: " << name);
  load_from_rwop(rw,name);
  //delete is;
}

SoundStream::~SoundStream()
{
  destroy();
}

void SoundStream::stop()
{
  m_Data->m_Finished=true;
  while (!m_Data->m_ThreadDone) SDL_Delay(10);
}

void SoundStream::destroy()
{
  delete[] m_Data->m_Buffer;
  m_Data->m_Buffer=0;
  //if (m_Data->m_State.sample) SDL_sound::Wrapper.free_sample(m_Data->m_State);
  if (m_Data->m_Decoder) m_Data->m_Decoder->destroy();
  delete m_Data;
  m_Data=0;
}

int SoundStream::frames_available()
{
  int diff=m_Data->m_Stop-m_Data->m_Start;
  if (diff<0) diff+=FRAMES*BUFFER_SIZE*2;
  return diff/(BUFFER_SIZE*2);
}

int SoundStream::frames_to_fill()
{
  return FRAMES-1-frames_available();
}

Uint8* SoundStream::get_frame(int bytes)
{
  int fa=frames_available();
  if (fa > 0)
  {
    Uint8* frame=&m_Data->m_Buffer[m_Data->m_Start];
    if (0)
    {
      static std::ofstream fout("gf_dump.raw",std::ios::out|std::ios::binary);
      fout.write((char*)frame,BUFFER_SIZE*2);
    }
    //m_Data->m_Start+=BUFFER_SIZE*2;
    m_Data->m_Start+=bytes;
    if (m_Data->m_Start>=(BUFFER_SIZE*2*FRAMES)) 
    {
      if (flog) *flog << "Rewinding start ptr from: " << m_Data->m_Start 
                      << " / " << (BUFFER_SIZE*2*FRAMES) << std::endl;
      m_Data->m_Start=0;
    }
    //SDL_SemPost(m_Data->m_Semaphore);
    return frame;
  }
  return 0;
}

int SoundStream::decode_thread()
{
  int rc=0;
  m_Data->m_Decoder=SDL_sound::MP3DecoderWrapper::instance()->create();
  if (!m_Data->m_Decoder)
  {
    if (flog) *flog << "Could not create decoder.\n";
    m_Data->m_Finished=true;
  }
  unsigned char src_buffer[BUFFER_SIZE];
  //unsigned char tmp_buf[BUFFER_SIZE];
  //unsigned      tmp_buf_size=0;
  while (!m_Data->m_Finished)
  {
    if (frames_to_fill()<=2)
    {
      SDL_Delay(10);
      continue;
      //if ((rc=SDL_SemWait(m_Data->m_Semaphore)) < 0) break;
    }
    Uint8* dst=&(m_Data->m_Buffer[m_Data->m_Stop]);
    if (flog)
    {
      //*flog << "start=" << m_Data->m_Start << "   stop=" << m_Data->m_Stop << std::endl;
      //*flog << "FA=" << frames_available() << std::endl;
    }
    unsigned missing=0,act=BUFFER_SIZE;
    m_Data->m_Decoder->decode(0,0,dst,&act);
    if (act<BUFFER_SIZE)
    {
      if (flog)
        *flog << "Decoded only " << act << " bytes.  Completing to BUFFER_SIZE" << std::endl;
      m_Data->m_Stop+=act;
      dst+=act;
      missing=BUFFER_SIZE-act;
      act=0;
    }
    while (act==0)
    {
      unsigned input_size=m_Data->m_Source->read(m_Data->m_Source,src_buffer,1,BUFFER_SIZE);
      if (flog)
      {
        *flog << "Reading " << input_size << " bytes from file." << std::endl;
      }
      if (input_size==0) break; // EOF
      if (missing>0) act=missing;
      else           act=BUFFER_SIZE;
      m_Data->m_Decoder->decode(src_buffer,input_size,dst,&act);
    }

    if (flog && act>0)
      *flog << "Decoded " << act << " bytes" << std::endl;
    m_Data->m_Stop+=act;
    if (m_Data->m_Stop>=(BUFFER_SIZE*2*FRAMES)) m_Data->m_Stop=0;
  }
  m_Data->m_ThreadDone=true;
  if (flog) *flog << "Thread exiting\n";
  return rc;
}

struct SoundStream_Thread
{
  static int thread(void* stream)
  {
    SoundStream* s=(SoundStream*)stream;
    return s->decode_thread();
  }
};

void SoundStream::load_from_rwop(SDL_RWops* rwops, const char* name)
{
  m_Data->m_Source=rwops;
  SDL_CreateThread(SoundStream_Thread::thread,"SoundStream",this);
  SDL_Delay(200);
}

void SoundStream::load(const xstring& filename)
{
  SDL_RWops* rw = SDL_RWFromFile(filename, "rb");
  load_from_rwop(rw,filename);
}




////////////////////////////////////////////////////////////////////





void SoundManager::cleanup()
{
  clip_seq::iterator b=m_Clips.begin(),e=m_Clips.end();
  while(b!=e)
  {
    SoundClipState& scs=*b;
    if (scs.pos<0) b=m_Clips.erase(b);
    else ++b;
  }
  stream_seq::iterator sb=m_Streams.begin(),se=m_Streams.end();
  while(sb!=se)
  {
    sound_stream_ptr s=*sb;
    if (s->frames_available()==0) sb=m_Streams.erase(sb);
    else ++sb;
  }
}

void SoundManager::AudioCallback(Uint8 *stream, int len)
{
  int start_ticks=SDL_GetTicks(),stop_ticks=0;
  int samples=len/2; // 16 bit samples
  if (int(s_MixingBuffer.size()) != samples)
    s_MixingBuffer.resize(samples);
  s_MixingBuffer.reset();
  int max_samples=0;
  for(auto& scs : m_Clips)
  {
    if (scs.pos<0) continue;
    short* sbuf=(short*)(&scs.clip->m_Buffer[0]);
    int    slen = scs.clip->m_Buffer.size() / 2;
    int    spos=scs.pos/2;
    if (flog) *flog << spos << '/' << slen << "    ";
    BufferReader<short> br(sbuf,slen,spos,scs.loop);
    s_MixingBuffer.start_sequence();
    int n=s_MixingBuffer.acc_data(br.current(),br.end());
    br.advance(n);
    scs.pos=br.get_position()*2;
    if (scs.pos>=int(scs.clip->m_Buffer.size())) scs.pos=-1;
  }
  if (flog) *flog << std::endl;
  stream_seq::iterator sb=m_Streams.begin(),se=m_Streams.end();
  while (sb!=se)
  {
    sound_stream_ptr s=*sb;
    Uint8* buffer=s->get_frame(len);
    if (buffer)
    {
      short* sbuf=(short*)buffer;
      //int slen=BUFFER_SIZE;
      int slen=samples;
      s_MixingBuffer.start_sequence();
      s_MixingBuffer.acc_data(sbuf,sbuf+slen);
      ++sb;
    }
    else
    {
      if (flog) *flog << "Failed to retrieve stream frame. Removing stream\n";
      sb=m_Streams.erase(sb);
    }
  }
  s_MixingBuffer.flush();
  short* outsbuf=(short*)stream;
  Accumulator<int>::const_iterator it=s_MixingBuffer.begin();
  for(int i=0;i<samples;++i,++it)
  {
    //int sample=s_MixingBuffer[i];
    int sample=*it;
    if (m_Fading)
    { 
      sample=int(sample*m_Gain); 
      m_Gain+=m_dGain; 
      if (m_Gain<=0.0) 
      { 
        m_Gain=0.0; 
        m_dGain=0.0; 
      } 
    }
    if (sample>0x7FFF)  sample=0x00007FFF;
    if (sample<-0x8000) sample=0xFFFF8000;
    outsbuf[i]=short(sample);
  }
  if (0)
  {
    static std::ofstream dump("dump.raw",std::ios::binary|std::ios::out);
    dump.write((char*)outsbuf,samples*2);
  }
  if (flog)
  {
    stop_ticks=SDL_GetTicks();
    //*flog << "ticks=" << start_ticks << "     bytes=" << len << "   cb took: " << (stop_ticks-start_ticks) << std::endl;
  }
}

void SoundManager::AudioCallback(void *userdata, Uint8 *stream, int len)
{
  SoundManager* mgr=(SoundManager*)userdata;
  mgr->AudioCallback(stream,len);
}

void SoundManager::initialize(int freq, bool stereo)
{
  m_Gain=1.0;
  m_dGain=0.0;
  SDL_AudioSpec requested;
  requested.freq=freq;
  requested.format=AUDIO_S16;
  requested.channels=stereo?2:1;
  requested.samples=BUFFER_SIZE;
  requested.callback=AudioCallback;
  requested.userdata=(void*)this;
  SDL_OpenAudio(&requested,&m_Spec);
}

void SoundManager::pause(bool state)
{
  SDL_PauseAudio(state?1:0);
}

void SoundManager::clear(int duration)
{
  SDL_LockAudio();
  m_Gain=1.0;
  m_dGain=-1000.0/(duration*m_Spec.freq);
  m_Fading=true;
  SDL_UnlockAudio();
  while (m_Gain>0) SDL_Delay(10);
  SDL_LockAudio();
  m_Clips.clear();
  m_Fading=false;
  m_Gain=1.0;
  m_dGain=0;
  SDL_UnlockAudio();
}

void SoundManager::play(sound_clip_ptr clip, bool loop)
{
  SDL_LockAudio();
  cleanup();
  m_Clips.push_back(SoundClipState(clip,loop));
  SDL_UnlockAudio();
}

void SoundManager::play(sound_stream_ptr stream)
{
  SDL_LockAudio();
  cleanup();
  bool exists=false;
  for (sound_stream_ptr& s : m_Streams)
  {
    if (s->m_Data->m_Decoder == stream->m_Data->m_Decoder)
      exists=true;
  }
  if (!exists) m_Streams.push_back(stream);
  SDL_UnlockAudio();
}

void SoundManager::shutdown() 
{ 
  //SDL_sound::Wrapper.destroy();
  instance(true); 
}

void play(sound_clip_ptr clip, bool loop)
{
  SoundManager::instance()->play(clip, loop);
}

} // namespace SDLPP
