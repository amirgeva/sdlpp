#ifndef random_h__
#define random_h__

#include <random>
#include <sysdep.h>

namespace SDLPP
{

  class Random
  {
    std::mt19937 m_Gen;
    std::uniform_real_distribution<double> m_Uniform;
    std::normal_distribution<double> m_Normal;
  public:
    Random(unsigned seed = 0)
      : m_Uniform(0, 1)
    {
      if (seed == 0) seed = unsigned(get_tick_count());
      m_Gen.seed(seed);
    }

    float operator() () { return as_float(); }

    template<class T>
    T operator() (const T& t) { return T(t*as_double()); }

    double as_double()
    {
      return m_Uniform(m_Gen);
    }

    float as_float()
    {
      return float(as_double());
    }

    int as_int(int range)
    {
      double d = as_double()*range;
      return int(d);
    }

    double normal(double sig_mult)
    {
      return sig_mult*m_Normal(m_Gen);
    }
  };



  class GlobalRandom : public Random
  {
  public:
    static GlobalRandom* instance()
    {
      static std::unique_ptr<GlobalRandom> ptr(new GlobalRandom);
      return ptr.get();
    }

  private:
    friend struct std::default_delete < GlobalRandom > ;
    GlobalRandom() {}
    ~GlobalRandom() {}
    GlobalRandom(const GlobalRandom&) {}
    GlobalRandom& operator= (const GlobalRandom&) { return *this; }
  };

  inline double urnd(double m = 1)
  {
    return m*GlobalRandom::instance()->as_double();
  }

  inline double nrnd(double sigma = 1)
  {
    return GlobalRandom::instance()->normal(sigma);
  }


} // namespace SDLPP




#endif // random_h__
