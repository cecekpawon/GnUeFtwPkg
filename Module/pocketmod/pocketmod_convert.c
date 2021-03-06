/* See end of file for license */
/**
  cecekpawon - Thu Aug 29 17:13:41 2019
**/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>

#include "pocketmodcfg.h"

#define POCKETMOD_IMPLEMENTATION

#include "pocketmod.h"

#define SAMPLE_RATE 44100

#ifdef POCKETMOD_PLAYER

#include <Library/AudioPlayerLib.h>

typedef struct {
  // Riff Wave Header
  char        chunkId[4];
  int         chunkSize;
  char        format[4];

  // Format Subchunk
  char        subChunk1Id[4];
  int         subChunk1Size;
  short int   audioFormat;
  short int   numChannels;
  int         sampleRate;
  int         byteRate;
  short int   blockAlign;
  short int   bitsPerSample;

  // Data Subchunk
  char        subChunk2Id[4];
  int         subChunk2Size;

} WAVE_HEADER;

#else

/* Write a 16-bit little-endian integer to a file */
static void fputw(unsigned short value, FILE *file)
{
  fputc(value & 0xff, file);
  fputc(value >> 8, file);
}

/* Write a 32-bit little-endian integer to a file */
static void fputl(unsigned long value, FILE *file)
{
  fputw(value & 0xffff, file);
  fputw(value >> 16, file);
}

#endif // POCKETMOD_PLAYER

/* Clip a floating point sample to the [-1, +1] range */
static float clip(float value)
{
  value = value < -1.0f ? -1.0f : value;
  value = value > +1.0f ? +1.0f : value;

  return value;
}

#ifdef POCKETMOD_VERBOSE

/* Print file size and duration statistics */
static void show_stats(char *stage, int samples)
{
  int seconds = (double) samples / SAMPLE_RATE;
  int filesize = samples * 4 + 44;

  printf("\r%s ", stage);
  printf("[%.1f MB] ", filesize / 1000000.0);
  printf("[%d:%02d] ", seconds / 60, seconds % 60);

  fflush(stdout);
}

#endif // POCKETMOD_VERBOSE

int main(int argc, char **argv)
{
  pocketmod_context   context;
  char                *mod_data = NULL;
  int                 i, mod_size, samples = 0;
  FILE                *file;
  int                 argcfix = 3;

  #ifdef POCKETMOD_PLAYER

    pocketmod_context   context_bkp;

    argcfix = 2;

  #endif // POCKETMOD_PLAYER

  /* Print usage if no file was given */
  if (argc != argcfix) {
    printf("usage: %s <infile> %s\n", argv[0], (argcfix == 2) ? "" : "<outfile>");
    goto DONE;
  }

  /* Read the input file into a heap block */
  if (!(file = fopen(argv[1], "rb"))) {
    printf("error: can't open '%s' for reading\n", argv[1]);
    goto DONE;
  }
  fseek(file, 0, SEEK_END);
  mod_size = ftell(file);
  rewind(file);
  if (!(mod_data = malloc(mod_size))) {
    printf("error: %d-byte memory allocation failed\n", mod_size);
    fclose(file);
    goto DONE;
  } else if (!fread(mod_data, mod_size, 1, file)) {
    printf("error: error reading file '%s'\n", argv[1]);
    fclose(file);
    goto DONE;
  }
  fclose(file);

  /* Initialize the renderer */
  if (!pocketmod_init(&context, mod_data, mod_size, SAMPLE_RATE)) {
    printf("error: '%s' is not a valid MOD file\n", argv[1]);
    goto DONE;
  }

  #ifdef POCKETMOD_PLAYER

    WAVE_HEADER   waveheader;
    uint8_t       *wavebuffer = NULL;
    size_t        wavebuffer_size = 0;
    size_t        wavebuffer_size_current = 0;

    /* Write the WAV header */
    /* http://blog.acipo.com/generating-wave-files-in-c/ */

    // RIFF WAVE Header
    waveheader.chunkId[0]       = 'R';
    waveheader.chunkId[1]       = 'I';
    waveheader.chunkId[2]       = 'F';
    waveheader.chunkId[3]       = 'F';
    waveheader.chunkSize        = 0;
    waveheader.format[0]        = 'W';
    waveheader.format[1]        = 'A';
    waveheader.format[2]        = 'V';
    waveheader.format[3]        = 'E';

    // Format subchunk
    waveheader.subChunk1Id[0]   = 'f';
    waveheader.subChunk1Id[1]   = 'm';
    waveheader.subChunk1Id[2]   = 't';
    waveheader.subChunk1Id[3]   = ' ';
    waveheader.subChunk1Size    = 16;
    waveheader.audioFormat      = 1;
    waveheader.numChannels      = 2;
    waveheader.sampleRate       = SAMPLE_RATE;
    waveheader.byteRate         = SAMPLE_RATE * 4;
    waveheader.blockAlign       = 4;
    waveheader.bitsPerSample    = 16; //

    // Data subchunk
    waveheader.subChunk2Id[0]   = 'd';
    waveheader.subChunk2Id[1]   = 'a';
    waveheader.subChunk2Id[2]   = 't';
    waveheader.subChunk2Id[3]   = 'a';
    waveheader.subChunk2Size    = 0;

  #else

    char *slash;

    /* Open the output file */
    if (!(file = fopen(argv[2], "wb"))) {
      printf("error: can't open '%s' for writing\n", argv[2]);
      goto DONE;
    }

    /* Strip the directory part from the output file's path */
    while ((slash = strpbrk(argv[2], "/\\"))) {
      argv[2] = slash + 1;
    }

    /* Write the WAV header */
    /* Follow along at home: http://soundfile.sapp.org/doc/WaveFormat/ */

    fputs("RIFF", file);          /* ChunkID       */
    fputl(0, file);               /* ChunkSize     */
    fputs("WAVE", file);          /* Format        */
    fputs("fmt ", file);          /* Subchunk1ID   */
    fputl(16, file);              /* Subchunk1Size */
    fputw(1, file);               /* AudioFormat   */
    fputw(2, file);               /* NumChannels   */
    fputl(SAMPLE_RATE, file);     /* SampleRate    */
    fputl(SAMPLE_RATE * 4, file); /* ByteRate      */
    fputw(4, file);               /* BlockAlign    */
    fputw(16, file);              /* BitsPerSample */
    fputs("data", file);          /* Subchunk2ID   */
    fputl(0, file);               /* Subchunk2Size */

  #endif // POCKETMOD_PLAYER

  /* Write sample data */
  float   buffer[512][2];
  short   output[512][2];
  int     stage = 0;
  char    *stage_str;

  #ifdef POCKETMOD_VERBOSE

    clock_t time_now, time_prev = 0;

  #endif // POCKETMOD_VERBOSE

  #ifdef POCKETMOD_PLAYER

LOOP:

    if (stage == 0) {
      stage_str = "Init";
      context_bkp = context;
      wavebuffer_size = wavebuffer_size_current = sizeof(WAVE_HEADER);
      wavebuffer = malloc(wavebuffer_size);
      if (wavebuffer == NULL) {
        printf("error: can't allocate wavebuffer\n");
        goto DONE;
      }
      memcpy(wavebuffer, &waveheader, wavebuffer_size);
    } else {
      stage_str = "Parsing";
      samples = 0;
      context = context_bkp;
      wavebuffer = realloc(wavebuffer, wavebuffer_size);
      if (wavebuffer == NULL) {
        printf("error: can't re-allocate wavebuffer\n");
        goto DONE;
      }
    }

  #else

    stage = 1;
    stage_str = "Writing";

  #endif // POCKETMOD_PLAYER

  while (pocketmod_loop_count(&context) == 0) {

    /* Render a chunk of samples */
    int rendered_bytes    = pocketmod_render(&context, buffer, sizeof(buffer));
    int rendered_samples  = rendered_bytes / sizeof(float[2]);

    samples += rendered_samples;

    /* Convert the sample data to 16-bit and write it to the file */
    for (i = 0; i < rendered_samples; i++) {
      output[i][0] = (short) (clip(buffer[i][0]) * 0x7fff);
      output[i][1] = (short) (clip(buffer[i][1]) * 0x7fff);
    }

    #ifdef POCKETMOD_PLAYER

      size_t wavebuffer_sizeblock = rendered_samples * sizeof(short[2]);

      if (stage == 0) {
        wavebuffer_size += wavebuffer_sizeblock;
      } else {
        memcpy(wavebuffer + wavebuffer_size_current, &output, wavebuffer_sizeblock);
        wavebuffer_size_current += wavebuffer_sizeblock;
      }

    #else

      fwrite(output, rendered_samples * sizeof(short[2]), 1, file);

    #endif // POCKETMOD_PLAYER

    #ifdef POCKETMOD_VERBOSE

      /* Print statistics at regular intervals */
      time_now = clock();
      if ((double) (time_now - time_prev) / CLOCKS_PER_SEC > 0.1) {
        show_stats(stage_str, samples);
        time_prev = time_now;
      }

    #endif // POCKETMOD_VERBOSE
  }

  #ifdef POCKETMOD_VERBOSE

    show_stats(stage_str, samples);
    putchar('\n');

  #endif // POCKETMOD_VERBOSE

  #ifdef POCKETMOD_PLAYER

    if (stage == 0) {
      if (wavebuffer_size == wavebuffer_size_current) {
        printf("error: wavebuffer init failed\n");
      } else {
        stage = 1;
        goto LOOP;
      }
    } else {
      if (wavebuffer_size != wavebuffer_size_current) {
        printf("error: wavebuffer parsing failed\n");
      } else {
        ((WAVE_HEADER *)wavebuffer)->chunkSize     = samples * 4 + 36;
        ((WAVE_HEADER *)wavebuffer)->subChunk2Size = samples * 4;

        AudioPlayer (wavebuffer, wavebuffer_size);
      }
    }

    free(wavebuffer);

  #else

    /* Now that we know how many samples we got, we can go back and patch the */
    /* ChunkSize and Subchunk2Size fields in the WAV header */

    fseek(file, 4, SEEK_SET);
    fputl(samples * 4 + 36, file);
    fseek(file, 40, SEEK_SET);
    fputl(samples * 4, file);
    fclose(file);

  #endif // POCKETMOD_PLAYER

DONE:

  /* Tidy up before leaving */
  if (mod_data != NULL) {
    free(mod_data);
  }

  return 0;
}

/*******************************************************************************

MIT License

Copyright (c) 2018 rombankzero

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*******************************************************************************/
