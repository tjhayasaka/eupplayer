// $Id: eupplayer_townsEmulator.h,v 1.13 2000/04/12 23:12:51 hayasaka Exp $

#ifndef TJH__EUP_TOWNSEMULATOR_H
#define TJH__EUP_TOWNSEMULATOR_H

#include <sys/time.h>
#include <sys/types.h>
#include <iostream>
#include "eupplayer_towns.h"

class TownsPcmInstrument;
class TownsPcmSound;
class TownsPcmEnvelope;

class EUP_TownsEmulator_MonophonicAudioSynthesizer
{
  int _rate;
  int _velocity;
public:
  EUP_TownsEmulator_MonophonicAudioSynthesizer() {}
  virtual ~EUP_TownsEmulator_MonophonicAudioSynthesizer() {}
  virtual void setControlParameter(int control, int value) = 0;
  virtual void setInstrumentParameter(u_char const *fmInst,
				      u_char const *pcmInst) = 0;
  virtual int velocity() const { return _velocity; }
  virtual void velocity(int velo) { _velocity = velo; }
  virtual void nextTick(int *outbuf, int buflen) = 0;
  virtual int rate() const { return _rate; }
  virtual void rate(int r) { _rate = r;}
  virtual void note(int n, int onVelo, int offVelo, int gateTime) = 0;
  virtual void pitchBend(int value) = 0;
};

class TownsFmEmulator_Operator
{
  enum State { _s_ready, _s_attacking, _s_decaying, _s_sustaining, _s_releasing };
  State _state;
  State _oldState;
  int64_t _currentLevel;
  int _frequency;
  int _phase;
  int _lastOutput;
  int _feedbackLevel;
  int _detune;
  int _multiple;
  int64_t _totalLevel;
  int _keyScale;
  int _velocity;
  int _specifiedTotalLevel;
  int _specifiedAttackRate;
  int _specifiedDecayRate;
  int _specifiedSustainLevel;
  int _specifiedSustainRate;
  int _specifiedReleaseRate;
  int _tickCount;
  int _attackTime;
  // int64_t _attackRate;
  int64_t _decayRate;
  int64_t _sustainLevel;
  int64_t _sustainRate;
  int64_t _releaseRate;
public:
  TownsFmEmulator_Operator();
  ~TownsFmEmulator_Operator();
  void feedbackLevel(int level);
  void setInstrumentParameter(u_char const *instrument);
  void velocity(int velo);
  void keyOn();
  void keyOff();
  void frequency(int freq);
  int nextTick(int rate, int phaseShift);
};

class TownsFmEmulator : public EUP_TownsEmulator_MonophonicAudioSynthesizer
{
  enum { _numOfOperators = 4 };
  TownsFmEmulator_Operator *_opr;
  int _control7;
  int _gateTime;
  int _offVelocity;
  int _note;
  int _frequencyOffs;
  int _frequency;
  int _algorithm;
public:
  TownsFmEmulator();
  ~TownsFmEmulator();
  void setControlParameter(int control, int value);
  void setInstrumentParameter(u_char const *fmInst, u_char const *pcmInst);
  int velocity()
    { return EUP_TownsEmulator_MonophonicAudioSynthesizer::velocity(); }
  void velocity(int velo);
  void nextTick(int *outbuf, int buflen);
  void note(int n, int onVelo, int offVelo, int gateTime);
  void pitchBend(int value);
  void recalculateFrequency();
};

class TownsPcmEmulator : public EUP_TownsEmulator_MonophonicAudioSynthesizer
{
  int _control7;
  int _envTick;
  int _currentLevel;
  int _gateTime;
  int _offVelocity;
  int _note;
  int _frequencyOffs;
  uint64_t _phase;
  TownsPcmInstrument const *_currentInstrument;
  TownsPcmSound const *_currentSound;
  TownsPcmEnvelope *_currentEnvelope;
public:
  TownsPcmEmulator();
  ~TownsPcmEmulator();
  void setControlParameter(int control, int value);
  void setInstrumentParameter(u_char const *fmInst, u_char const *pcmInst);
  void nextTick(int *outbuf, int buflen);
  void note(int n, int onVelo, int offVelo, int gateTime);
  void pitchBend(int value);
};

class EUP_TownsEmulator_Channel
{
  enum { _maxDevices = 16 };
  EUP_TownsEmulator_MonophonicAudioSynthesizer *_dev[_maxDevices+1];
  int _lastNotedDeviceNum;
public:
  EUP_TownsEmulator_Channel();
  ~EUP_TownsEmulator_Channel();
  void add(EUP_TownsEmulator_MonophonicAudioSynthesizer *device);
  void note(int note, int onVelo, int offVelo, int gateTime);
  void setControlParameter(int control, int value);
  void setInstrumentParameter(u_char const *fmInst, u_char const *pcmInst);
  void pitchBend(int value);
  void nextTick(int *outbuf, int buflen);
  void rate(int r);
};

class EUP_TownsEmulator : public TownsAudioDevice
{
  FILE *_ostr;
  enum { _maxChannelNum = 16,
	 _maxFmInstrumentNum = 128,
	 _maxPcmInstrumentNum = 32,
	 _maxPcmSoundNum = 128, };
  EUP_TownsEmulator_Channel *_channel[_maxChannelNum];
  bool _enabled[_maxChannelNum];
  u_char _fmInstrumentData[8 + 48*_maxFmInstrumentNum];
  u_char *_fmInstrument[_maxFmInstrumentNum];
  TownsPcmInstrument *_pcmInstrument[_maxPcmInstrumentNum];
  TownsPcmSound *_pcmSound[_maxPcmSoundNum];
  int _rate;
  bool _outputSampleUnsigned;
  int _outputSampleSize;
  bool _outputSampleLSBFirst;
public:
  EUP_TownsEmulator();
  ~EUP_TownsEmulator();
  void outputSampleUnsigned(bool u) { _outputSampleUnsigned = u; }
  void outputSampleSize(int l) { _outputSampleSize = l; }
  void outputSampleLSBFirst(bool l) { _outputSampleLSBFirst = l; }
  void assignFmDeviceToChannel(int channel);
  void assignPcmDeviceToChannel(int channel);
  void setFmInstrumentParameter(int num, u_char const *instrument);
  void setPcmInstrumentParameters(u_char const *instrument, size_t size);
  void outputStream(FILE *ostr);
  void nextTick();
  void enable(int ch, bool en=true);
  void note(int channel, int n,
	    int onVelo, int offVelo, int gateTime);
  void pitchBend(int channel, int value);
  void controlChange(int channel, int control, int value);
  void programChange(int channel, int num);
  void rate(int r);
};

#endif
