#ifndef H_SDLPP_SOUND
#define H_SDLPP_SOUND

#include "sdlpp_common.h"
#include "sdlpp_io.h"

class MP3Decoder;

namespace SDLPP {

const int SOUND_BUFFER_SIZE = 1024;

//////////////////////////////////////////////////
//
// Sound functions.
//   Initialize sound engine by using the init_audio function in the Application object
//   Create as many SoundClip objects as you want and play them when you want
//   Note that destroying a SoundClip object while it's playing is not recommended.
//
//////////////////////////////////////////////////


/// SoundClip is a finite (usually short) audio clip.
/// It is loaded from WAV files and can be played to provide sound effects.
/// The clip can be played multiple times, even concurrently, without duplicating
/// its data.
/// SoundClips in their nature allocate memory.  In order to make their usage simple
/// and still not waste resources by duplicating clips in memory, SoundClip objects
/// should be used through shared_ptr called   sound_clip_ptr
class SoundClip
{
  SoundClip(const SoundClip& rhs) {}
  SoundClip& operator= (const SoundClip& rhs) { return *this; }
public:
  SoundClip(const xstring& filename);
  virtual ~SoundClip();

private:
  void destroy();
  void load_from_rwop(SDL_RWops* rwops, const char* name);
  friend class SoundManager;
  uint8_vec m_Buffer;
  Uint8*    m_Wave;
};

typedef std::shared_ptr<SoundClip> sound_clip_ptr;

/** Start playback of this sound clip. */
void play(sound_clip_ptr clip, bool loop = false);



/// SoundStream is usually used for music or other long (potential infinite) audio.
/// It can either be decoded from some compressed audio source, or retrieved from a network location.
/// The stream can only have a single instance playing at any given moment, though it is 
/// possible to create multiple streams from the same source of data.
/// SoundStream objects in their nature allocate memory and resources.  
class SoundStream
{
public:
  /** Construct a sound stream and optionally load it from a file. */
  SoundStream(const xstring& filename="");
  /** Construct a sound Stream by decoding audio from resource file. */
  SoundStream(ResourceFile& rf, const xstring& name);
  virtual ~SoundStream();
private:
  SoundStream(const SoundStream& rhs);
  SoundStream& operator= (const SoundStream& rhs);
public:

  /** Start playback of this sound stream. */
  void play();

  /** Stop playback of this stream */
  void stop();
protected:
  void destroy();

  virtual Uint8* get_frame(int bytes);
private:
  void load_from_rwop(SDL_RWops* rwops, const char* name);
  void load(const xstring& filename);
  friend class SoundManager;
  friend struct SoundStream_Thread;

  int frames_available();
  int frames_to_fill();

  int    decode_thread();

  struct Data
  {
    Data(int size)
      : //m_DataStream(0),
        m_Source(0),
        m_Decoder(0),
        m_Buffer(new Uint8[size]),
        m_Start(0), 
        m_Stop(0),
        m_DecodeThread(0),
        m_Semaphore(0),
        m_Finished(false),
        m_ThreadDone(false)
    {}
    SDL_RWops*           m_Source;
    //Sound_State          m_State;
    MP3Decoder*          m_Decoder;
    std::vector<Uint8>   m_EncodedStream;
    Uint8*               m_Buffer;
    int                  m_Start,m_Stop;
    SDL_Thread*          m_DecodeThread;
    SDL_sem*             m_Semaphore;
    bool                 m_Finished;
    bool                 m_ThreadDone;
  } *m_Data;
};

typedef std::shared_ptr<SoundStream> sound_stream_ptr;

/// Audio subsystem management singleton object
/// Provides management, mixing and streaming functionality.
/// It is initialized by the Application::init_audio function and then used automatically
/// by SoundClip objects.
class SoundManager : public Singleton
{
  struct SoundClipState
  {
    SoundClipState(sound_clip_ptr c, bool l) : clip(c), pos(0), loop(l) {}
    sound_clip_ptr  clip;
    int             pos;
    bool            loop;
  };

  typedef std::list<SoundClipState> clip_seq;
  typedef std::list<sound_stream_ptr> stream_seq;
  clip_seq   m_Clips;
  stream_seq m_Streams;
  double     m_Gain,m_dGain;
  bool       m_Fading;

  void cleanup();
  void AudioCallback(Uint8 *stream, int len);
  static void AudioCallback(void *userdata, Uint8 *stream, int len);
  static Accumulator<int> s_MixingBuffer;
public:
  /** Returns a pointer to the sound management object */
  static SoundManager* instance(bool destroy=false)
  {
    static std::unique_ptr<SoundManager> ptr;
    if (!ptr && !destroy) ptr.reset(new SoundManager);
    else
    if (ptr && destroy) { ptr.reset(); }
    return ptr.get();
  }

  /** Destroys the sound management object.  
      This is called automatically before program exit. */
  virtual void shutdown();

  /// Initialize audio system to 16 bit signed.
  /// Specify sampling frequency and mono/stereo.
  void initialize(int freq, bool stereo);

  /** Stop all playback.   Clips will continue from where they were when unpause is done. */
  void pause(bool state=true);

  /** Remove all clips and silence all audio
      Optionally, a fade out can be selected, by specifying positive duration*/
  void clear(int duration=0);

  /** Returns the sampling frequency used.  
      This may differ from what was requested in initialization, if the hardware does
      not support the requested frequency.
  */
  int  get_freq() const { return m_Spec.freq; }
  bool is_stereo() const { return m_Spec.channels==2; }

  /** Send a sound clip */
  void play(sound_clip_ptr clip, bool loop);

  /** Send a stream for playback */
  void play(sound_stream_ptr stream);

  SDL_AudioSpec* get_audio_spec() { return &m_Spec; }
private:
  friend struct std::default_delete<SoundManager>;
  SoundManager() : m_Fading(false), m_Gain(1.0), m_dGain(0.0) {}
  ~SoundManager() 
  {
    SDL_CloseAudio();
  }
  SoundManager(const SoundManager&) {}

  SDL_AudioSpec m_Spec;
};


} // namespace SDLPP


#endif // H_SDLPP_SOUND


