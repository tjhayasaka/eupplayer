/*      Copyright (C) 1995-1996, 2000 Tomoaki HAYASAKA.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

// $Id: eupplay.cc,v 1.15 2000/04/12 23:20:56 hayasaka Exp $

#include "eupplayer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string>
#include <vector>
#include <strstream>
using namespace std;
#include "eupplayer_townsEmulator.h"

static string const downcase(string const &s)
{
  string r;
  for (string::const_iterator i = s.begin(); i != s.end(); i++)
    r += (*i >= 'A' && *i <= 'Z') ? (*i - 'A' + 'a') : *i;
  return r;
}

static string const upcase(string const &s)
{
  string r;
  for (string::const_iterator i = s.begin(); i != s.end(); i++)
    r += (*i >= 'a' && *i <= 'z') ? (*i - 'a' + 'A') : *i;
  return r;
}

static FILE *openFile_inPath(string const &filename, string const &path)
{
  FILE *f = NULL;
  vector<string> fn2;
  fn2.push_back(filename);
  fn2.push_back(downcase(filename));
  fn2.push_back(upcase(filename));

  string::const_iterator i = path.begin();
  while (i != path.end()) {
    string dir;
    while (i != path.end() && *i != ':')
      dir += *i++;
    if (i != path.end() && *i == ':')
      i++;
    for (vector<string>::const_iterator j = fn2.begin(); j != fn2.end(); j++) {
      string const filename(dir + "/" + *j);
      //cerr << "trying " << filename << endl;
      f = fopen(filename.c_str(), "r");
      if (f != NULL) {
	//cerr << "loading " << filename << endl;
	return f;
      }
    }
  }
  if (f == NULL)
    fprintf(stderr, "error finding %s\n", filename.c_str());

  return f;
}

u_char *EUPPlayer_readFile(EUPPlayer *player,
			   TownsAudioDevice *device,
			   string const &nameOfEupFile)
{
  // とりあえず, TOWNS emu のみに対応.

  u_char *eupbuf = NULL;
  player->stopPlaying();

  {
    FILE *f = fopen(nameOfEupFile.c_str(), "r");
    if (f == NULL) {
      perror(nameOfEupFile.c_str());
      return NULL;
    }

    struct stat statbuf;
    if (fstat(fileno(f), &statbuf) != 0) {
      perror(nameOfEupFile.c_str());
      fclose(f);
      return NULL;
    }

    if (statbuf.st_size < 2048 + 6 + 6) {
      fprintf(stderr, "%s: too short file.\n", nameOfEupFile.c_str());
      fclose(f);
      return NULL;
    }

    eupbuf = new u_char[statbuf.st_size];
    fread(eupbuf, statbuf.st_size, 1, f);
    fclose(f);
  }

  player->tempo(eupbuf[2048 + 5] + 30);
  // 初期テンポの設定のつもり.  これで正しい?

  for (int n = 0; n < 32; n++)
    player->mapTrack_toChannel(n, eupbuf[0x394 + n]);

  for (int n = 0; n < 6; n++)
    device->assignFmDeviceToChannel(eupbuf[0x6d4 + n]);
  for (int n = 0; n < 8; n++)
    device->assignPcmDeviceToChannel(eupbuf[0x6da + n]);

  string fmbPath(".:/usr/local/share/fj2/tone:/usr/share/fj2/tone");
  string pmbPath(".:/usr/local/share/fj2/tone:/usr/share/fj2/tone");
  {
    char *s;
    s = getenv("EUP_FMINST");
    if (s != NULL)
      fmbPath = string(s);
    s = getenv("EUP_PCMINST");
    if (s != NULL)
      pmbPath = string(s);
    string eupDir = nameOfEupFile.substr(0, nameOfEupFile.rfind("/") + 1) + ".";
    fmbPath = eupDir + ":" + fmbPath;
    pmbPath = eupDir + ":" + pmbPath;
  }

  {
#if 0
    u_char instrument[] = {
      ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', // name
      1, 7, 1, 1,			      // detune / multiple
      23, 40, 38, 0,			      // output level
      140, 140, 140, 83,		      // key scale / attack rate
      13, 13, 13, 3,			      // amon / decay rate
      0, 0, 0, 0,			      // sustain rate
      19, 250, 19, 10,			      // sustain level / release rate
      58,				      // feedback / algorithm
      0xc0,				      // pan, LFO
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    };
#else
    u_char instrument[] = {
      ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', // name
      17, 33, 10, 17,			      // detune / multiple
      25, 10, 57, 0,			      // output level
      154, 152, 218, 216,		      // key scale / attack rate
      15, 12, 7, 12,			      // amon / decay rate
      0, 5, 3, 5,			      // sustain rate
      38, 40, 70, 40,			      // sustain level / release rate
      20,				      // feedback / algorithm
      0xc0,				      // pan, LFO
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    };
#endif
    for (int n = 0; n < 128; n++)
      device->setFmInstrumentParameter(n, instrument);
  }

  {
    char fn0[16];
    memcpy(fn0, eupbuf + 0x6e2, 8);
    fn0[8] = '\0';
    string fn1(string(fn0) + ".fmb");
    FILE *f = openFile_inPath(fn1, fmbPath);
    if (f != NULL) {
      struct stat statbuf;
      fstat(fileno(f), &statbuf);
      u_char buf1[statbuf.st_size];
      fread(buf1, statbuf.st_size, 1, f);
      fclose(f);
      for (int n = 0; n < (statbuf.st_size - 8) / 48; n++)
	device->setFmInstrumentParameter(n, buf1 + 8 + 48 * n);
    }
  }

  {
    char fn0[16];
    memcpy(fn0, eupbuf + 0x6ea, 8);
    fn0[8] = '\0';
    string fn1(string(fn0) + ".pmb");
    FILE *f = openFile_inPath(fn1, pmbPath);
    if (f != NULL) {
      struct stat statbuf;
      fstat(fileno(f), &statbuf);
      u_char buf1[statbuf.st_size];
      fread(buf1, statbuf.st_size, 1, f);
      fclose(f);
      device->setPcmInstrumentParameters(buf1, statbuf.st_size);
    }
  }

  return eupbuf;
}

int main(int argc, char **argv)
{
  EUP_TownsEmulator *dev = new EUP_TownsEmulator;
  dev->outputStream(stdout);
  EUPPlayer *player = new EUPPlayer();
  player->outputDevice(dev);

  int c;
  while ((c = getopt(argc, argv, "usbwt:r:v:d:")) != -1)
    switch(c) {
    case 's':
      dev->outputSampleUnsigned(false);
      break;
    case 'u':
      dev->outputSampleUnsigned(true);
      break;
    case 'b':
      dev->outputSampleSize(1);
      break;
    case 'w':
      dev->outputSampleSize(2);
      break;
    case 't':
      {
	string arg(optarg);
	if (arg == ".sb") {
	  dev->outputSampleUnsigned(false);
	  dev->outputSampleSize(1);
	} else if (arg == ".ub") {
	  dev->outputSampleUnsigned(true);
	  dev->outputSampleSize(1);
	} else if (arg == ".sw") {
	  dev->outputSampleUnsigned(false);
	  dev->outputSampleSize(2);
	  dev->outputSampleLSBFirst(true);
	} else if (arg == ".uw") {
	  dev->outputSampleUnsigned(true);
	  dev->outputSampleSize(2);
	  dev->outputSampleLSBFirst(true);
	} else if (arg == ".ws") {
	  dev->outputSampleUnsigned(false);
	  dev->outputSampleSize(2);
	  dev->outputSampleLSBFirst(false);
	} else if (arg == ".wu") {
	  dev->outputSampleUnsigned(true);
	  dev->outputSampleSize(2);
	  dev->outputSampleLSBFirst(false);
	} else {
	  fprintf(stderr, "unknown sample format %s, assuming .ub\n",
		  arg.c_str());
	  dev->outputSampleUnsigned(true);
	  dev->outputSampleSize(1);
	}
      }
      break;
    case 'r':
      dev->rate(strtol(optarg, NULL, 0));
      break;
    case 'v':
      dev->volume(strtol(optarg, NULL, 0));
      break;
    case 'd':
      {
	istrstream str(optarg);
	while (!str.eof() && !str.fail()) {
	  int n;
	  str >> n;
	  dev->enable(n, false);
	}
      }
      break;
    }

  int optindex = optind;
  if (optindex > argc-1) {
    fprintf(stderr, "usage: %s [-t format] [-r rate] [-v vol] [-b|-w] [-d ch[,ch...]] EUP-filename\n", argv[0]);
    exit(1);
  }

  char const *nameOfEupFile = argv[optindex++];
  u_char *buf = EUPPlayer_readFile(player, dev, nameOfEupFile);
  if (buf == NULL) {
    fprintf(stderr, "%s: read failed\n", argv[1]);
    exit(1);
  }

  player->startPlaying(buf + 2048 + 6);
  while (player->isPlaying())
    player->nextTick();

  delete player;
  delete dev;
  delete buf;

  return 0;
}
