/*
 * test.c
 *
 * Copyright (C) 2021 celeriyacon - https://github.com/celeriyacon
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 */

#include "types.h"
#include "sh2.h"
#include "smpc.h"
#include "scsp.h"

#include "adp68k.h"

#include "samples.h"

static const uint8 driver_data[0x2000] =
{
 #include "adp68k.bin.inc"
};

static const uint8 sample_data[] =
{
 #include "samples.ladp.inc"
};

static void Wait(uint32 t)
{
 TimeBegin(); do { TimeUpdate(); } while(TimeGet() < t);
}

static void TestADPCMVoice(void)
{
 static const uint8 ids[3] = { SAMPLE_ID_VOICE_4BIT, SAMPLE_ID_VOICE_2BIT, SAMPLE_ID_VOICE_1BIT };

 for(unsigned i = 0; i < 3; i++)
 {
  while(SCSP_SCIPD & 0x20);
  volatile ADPCMChannelControl* adpcc = &adp68k_scblock->adpcm[(0 + i) & 0x7];

  adpcc->id = ids[i % 3];
  adpcc->volume[0] = 0x4000;
  adpcc->volume[1] = 0x4000;
  adpcc->action = ADP68K_ACTION_PLAY;
  SCSP_SCIPD_LO = 0x20;

  Wait(28000000 * 3);
 }

 for(unsigned i = 0; i < 8; i++)
 {
  while(SCSP_SCIPD & 0x20);
  volatile ADPCMChannelControl* adpcc = &adp68k_scblock->adpcm[(0 + i) & 0x7];

  adpcc->id = ids[i % 3];
  adpcc->volume[0] = (!(i & 1) ? 0x4000 : 0x0000);
  adpcc->volume[1] = ( (i & 1) ? 0x4000 : 0x0000);
  adpcc->action = ADP68K_ACTION_PLAY;
  SCSP_SCIPD_LO = 0x20;

  Wait(10000000);
 }
 Wait(28000000 * 3);
}

static void TestADPCMDCBias(void)
{
 static const uint8 ids[] = { SAMPLE_ID_DCBIASEND_4BIT, SAMPLE_ID_DCBIASEND_2BIT, SAMPLE_ID_DCBIASEND_1BIT };

 for(unsigned i = 0; i < sizeof(ids) / sizeof(ids[0]); i++)
 {
  volatile ADPCMChannelControl* adpcc = &adp68k_scblock->adpcm[0];

  while(SCSP_SCIPD & 0x20);
  adpcc->id = ids[i];
  adpcc->volume[0] = 0x4000;
  adpcc->volume[1] = 0x4000;
  adpcc->action = ADP68K_ACTION_PLAY;
  SCSP_SCIPD_LO = 0x20;

  Wait(40000000);

  while(SCSP_SCIPD & 0x20);
  adpcc->id = ids[i];
  adpcc->volume[0] = 0x4000;
  adpcc->volume[1] = 0x4000;
  adpcc->action = ADP68K_ACTION_PLAY;
  SCSP_SCIPD_LO = 0x20;

  Wait(28000000 / 2);

  while(SCSP_SCIPD & 0x20);
  adpcc->volume[0] = 0x4000;
  adpcc->volume[1] = 0x4000;
  adpcc->action = ADP68K_ACTION_STOP;
  SCSP_SCIPD_LO = 0x20;
  Wait(28000000 / 2);
 }
}

static void TestSquare(void)
{
 TimeBegin();
 for(unsigned i = 1; i < 1024; i++)
 {
  while(SCSP_SCIPD & 0x20);
  adp68k_scblock->psg.square_freq = i;
  adp68k_scblock->psg.square_volume[0] = ((i & 0x080) ? 0x0000 : 0x1000) >> (i >> 9);
  adp68k_scblock->psg.square_volume[1] = ((i & 0x180) ? 0x1000 : 0x0000) >> (i >> 9);
  SCSP_SCIPD_LO = 0x20;

  do { TimeUpdate(); } while(TimeGet() < i * 100000);
 }

 while(SCSP_SCIPD & 0x20);
 adp68k_scblock->psg.square_freq = 1;
 adp68k_scblock->psg.square_volume[0] = 0;
 adp68k_scblock->psg.square_volume[1] = 0;
 SCSP_SCIPD_LO = 0x20;
}

static void TestNoise(void)
{
 volatile PSGControl* psgc = &adp68k_scblock->psg;

 psgc->saw_operator[0].freq = DSP_MAKE_MEMS(-7896769);
 psgc->saw_operator[1].freq = DSP_MAKE_MEMS(-6936619);
 psgc->saw_operator[2].freq = DSP_MAKE_MEMS(-4583429);
 psgc->saw_operator[3].freq = DSP_MAKE_MEMS(-5991397);

 psgc->saw_operator[0].fb_level = DSP_MAKE_COEF(3767);
 psgc->saw_operator[1].fb_level = DSP_MAKE_COEF(3643);
 psgc->saw_operator[2].fb_level = DSP_MAKE_COEF(3271);
 psgc->saw_operator[3].fb_level = DSP_MAKE_COEF(2927);

 psgc->saw_operator[0].mod_level = DSP_MAKE_COEF(4096);
 psgc->saw_operator[1].mod_level = DSP_MAKE_COEF(4096);
 psgc->saw_operator[2].mod_level = DSP_MAKE_COEF(4096);
 psgc->saw_operator[3].mod_level = DSP_MAKE_COEF(4096);

 psgc->saw_operator[0].volume = 0;
 psgc->saw_operator[1].volume = 0;
 psgc->saw_operator[2].volume = 0;
 psgc->saw_operator[3].volume = DSP_MAKE_COEF(2048);
 //
 //
 //
 TimeBegin();
 for(unsigned i = 0; i < 1536; i++)
 {
  unsigned m = (i + 1) * 0x010;

  if(m > 0x7FF0)
   m = 0x7FF0;

  while(SCSP_SCIPD & 0x20);
  adp68k_scblock->psg.saw_iir[0] = m;
  adp68k_scblock->psg.saw_iir[1] = 0x0000;
  adp68k_scblock->psg.saw_iir[2] = 0x4000 - (m >> 1);

  adp68k_scblock->psg.saw_volume[0] = 0x1000;
  adp68k_scblock->psg.saw_volume[1] = 0x1000;
  SCSP_SCIPD_LO = 0x20;

  do { TimeUpdate(); } while(TimeGet() < (1 + i) * 220000);
 }

 while(SCSP_SCIPD & 0x20);
 adp68k_scblock->psg.saw_volume[0] = 0;
 adp68k_scblock->psg.saw_volume[1] = 0;
 SCSP_SCIPD_LO = 0x20;
}

static void TestSawVroom(void)
{
 while(SCSP_SCIPD & 0x20);
 volatile PSGControl* psgc = &adp68k_scblock->psg;

 adp68k_scblock->psg.saw_iir[0] = 0x4000;
 adp68k_scblock->psg.saw_iir[1] = -0x2000;
 adp68k_scblock->psg.saw_iir[2] = 0x1000;

 psgc->saw_volume[0] = 0x1000;
 psgc->saw_volume[1] = 0x1000;

 psgc->saw_operator[0].fb_level = DSP_MAKE_COEF(2048);
 psgc->saw_operator[1].fb_level = DSP_MAKE_COEF(2048);
 psgc->saw_operator[2].fb_level = DSP_MAKE_COEF(2048);
 psgc->saw_operator[3].fb_level = DSP_MAKE_COEF(2048);

 psgc->saw_operator[0].mod_level = DSP_MAKE_COEF(8);
 psgc->saw_operator[1].mod_level = DSP_MAKE_COEF(8);
 psgc->saw_operator[2].mod_level = DSP_MAKE_COEF(16);
 psgc->saw_operator[3].mod_level = DSP_MAKE_COEF(8);

 psgc->saw_operator[0].volume = DSP_MAKE_COEF(256);
 psgc->saw_operator[1].volume = DSP_MAKE_COEF(512);
 psgc->saw_operator[2].volume = DSP_MAKE_COEF(1024);
 psgc->saw_operator[3].volume = DSP_MAKE_COEF(2048);

 SCSP_SCIPD_LO = 0x20;

 TimeBegin();
 for(unsigned i = 128; i < 1024; i++)
 {
  while(SCSP_SCIPD & 0x20);
  psgc->saw_operator[0].freq = DSP_MAKE_MEMS(-1666 * 32 * i / 512);
  psgc->saw_operator[1].freq = DSP_MAKE_MEMS(-3000 * 32 * i / 512);
  psgc->saw_operator[2].freq = DSP_MAKE_MEMS(-2000 * 32 * i / 512);
  psgc->saw_operator[3].freq = DSP_MAKE_MEMS(-1333 * 32 * i / 512);
  SCSP_SCIPD_LO = 0x20;

  do { TimeUpdate(); } while(TimeGet() < (1 + (i - 128)) * 200000);
 }

 while(SCSP_SCIPD & 0x20);
 adp68k_scblock->psg.saw_volume[0] = 0;
 adp68k_scblock->psg.saw_volume[1] = 0;
 SCSP_SCIPD_LO = 0x20;
}

static void TestNightmare(void)
{
 while(SCSP_SCIPD & 0x20);
 volatile PSGControl* psgc = &adp68k_scblock->psg;

 adp68k_scblock->psg.saw_iir[0] = 0x2000;
 adp68k_scblock->psg.saw_iir[1] = 0x0000;
 adp68k_scblock->psg.saw_iir[2] = 0x2000;

 psgc->saw_volume[0] = 0x1000;
 psgc->saw_volume[1] = 0x1000;

 psgc->saw_operator[0].fb_level = DSP_MAKE_COEF(2048);
 psgc->saw_operator[1].fb_level = DSP_MAKE_COEF(2048);
 psgc->saw_operator[2].fb_level = DSP_MAKE_COEF(2048);
 psgc->saw_operator[3].fb_level = DSP_MAKE_COEF(2048);

 psgc->saw_operator[0].mod_level = DSP_MAKE_COEF(16);
 psgc->saw_operator[1].mod_level = DSP_MAKE_COEF(2);
 psgc->saw_operator[2].mod_level = DSP_MAKE_COEF(2);
 psgc->saw_operator[3].mod_level = DSP_MAKE_COEF(2);

 psgc->saw_operator[0].volume = DSP_MAKE_COEF(-512);
 psgc->saw_operator[1].volume = DSP_MAKE_COEF(-512);
 psgc->saw_operator[2].volume = DSP_MAKE_COEF(512);
 psgc->saw_operator[3].volume = DSP_MAKE_COEF(512);
 SCSP_SCIPD_LO = 0x20;

 uint32 wait_accum = 0;

 TimeBegin();
 for(int f = 248; f <= 568; f++)
 {
  for(int v = 0; v < 16; v++)
  {
   uint16 vt[2];

   vt[0 ^ (f & 1)] = (0x3000 * (v + 1)) >> 4;
   vt[1 ^ (f & 1)] = (0x3000 * (16 - (v + 1))) >> 4;

   while(SCSP_SCIPD & 0x20);
   psgc->saw_volume[0] =  vt[0];
   psgc->saw_volume[1] = -vt[1];

   psgc->saw_operator[0].freq = DSP_MAKE_MEMS(-( 5121 * f >> 8));
   psgc->saw_operator[1].freq = DSP_MAKE_MEMS(-(36451 * f >> 8));
   psgc->saw_operator[2].freq = DSP_MAKE_MEMS(-(36450 * f >> 8));
   psgc->saw_operator[3].freq = DSP_MAKE_MEMS(-(25600 * f >> 8));
   SCSP_SCIPD_LO = 0x20;

   wait_accum += 1000000 / 2 - ((f - 192) * 1000);
   do { TimeUpdate(); } while(TimeGet() < wait_accum);
  }
 }

 while(SCSP_SCIPD & 0x20);
 psgc->saw_volume[0] = 0x0000;
 psgc->saw_volume[1] = 0x0000;
 SCSP_SCIPD_LO = 0x20;
}

static void LoadDriver(void)
{
 SMPC_SoundOff();
 SCSP_Init();
 SCSP_MCIRE = 0xFFFF;
 SCSP_SCIRE = 0xFFFF;
 //
 for(unsigned i = 0; i < sizeof(driver_data); i++)
  SCSP8(i) = driver_data[i];
 //
 SMPC_SoundOn();
 while(!(SCSP_MCIPD & 0x20));
 SCSP_MCIRE_LO = 0x20;
}

static void LoadSamples(void)
{
 while(SCSP_SCIPD & 0x20);

 for(unsigned i = 0; i < 8; i++)
  adp68k_scblock->adpcm[i].action = ADP68K_ACTION_STOP;

 SCSP_SCIPD_LO = 0x20;
 while(SCSP_SCIPD & 0x20);
 //
 //
 for(unsigned i = 0; i < sizeof(sample_data); i++)
  adp68k_sdbase[i] = sample_data[i];
}

void start(void) asm("start") __attribute__((noreturn)) __attribute__((section(".init")));
void start(void)
{
 WriteSR(0xF0);
 //
 LoadDriver();
 LoadSamples();

 for(;;)
 {
  TestADPCMVoice();
  TestADPCMDCBias();
  TestSquare();
  TestNoise();
  TestSawVroom();
  TestNightmare();
 }
}
