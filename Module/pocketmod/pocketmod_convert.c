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

#ifdef POCKETMOD_PLAYER

  #include "pocketmod_player.h"

#endif

#define SAMPLE_RATE 44100

#if defined(POCKETMOD_ORI) && defined(POCKETMOD_SAVE_FILE)

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

#endif // POCKETMOD_ORI + POCKETMOD_SAVE_FILE

/* Clip a floating point sample to the [-1, +1] range */
static float clip(float value)
{
    value = value < -1.0f ? -1.0f : value;
    value = value > +1.0f ? +1.0f : value;
    return value;
}

#ifdef POCKETMOD_VERBOSE

/* Print file size and duration statistics */
static void show_stats(char *filename, int samples)
{
    int seconds = (double) samples / SAMPLE_RATE;
    int filesize = samples * 4 + 44;
    printf("\rWriting: '%s' ", filename);
    printf("[%.1f MB] ", filesize / 1000000.0);
    printf("[%d:%02d] ", seconds / 60, seconds % 60);
    printf("Press Ctrl + C to stop");
    fflush(stdout);
}

#endif // POCKETMOD_VERBOSE

#ifndef POCKETMOD_ORI

// http://blog.acipo.com/generating-wave-files-in-c/

typedef struct WaveHeader {
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

} WaveHeader;

#endif // !POCKETMOD_ORI

int main(int argc, char **argv)
{
    pocketmod_context   context;
    char                *mod_data;
    int                 i, mod_size, samples = 0;
    FILE                *file;
    int                 argcfix = 2;

    #ifdef POCKETMOD_SAVE_FILE

      argcfix = 3;

    #endif // POCKETMOD_SAVE_FILE

    /* Print usage if no file was given */
    if (argc != argcfix) {
        printf("usage: %s <infile> <outfile>\n", argv[0]);
        return -1;
    }

    /* Read the input file into a heap block */
    if (!(file = fopen(argv[1], "rb"))) {
        printf("error: can't open '%s' for reading\n", argv[1]);
        return -1;
    }
    fseek(file, 0, SEEK_END);
    mod_size = ftell(file);
    rewind(file);
    if (!(mod_data = malloc(mod_size))) {
        printf("error: %d-byte memory allocation failed\n", mod_size);
        return -1;
    } else if (!fread(mod_data, mod_size, 1, file)) {
        printf("error: error reading file '%s'\n", argv[1]);
        return -1;
    }
    fclose(file);

    /* Initialize the renderer */
    if (!pocketmod_init(&context, mod_data, mod_size, SAMPLE_RATE)) {
        printf("error: '%s' is not a valid MOD file\n", argv[1]);
        return -1;
    }

    #ifdef POCKETMOD_SAVE_FILE

      char *slash;

      /* Open the output file */
      if (!(file = fopen(argv[2], "wb"))) {
          printf("error: can't open '%s' for writing\n", argv[2]);
          return -1;
      }

      /* Strip the directory part from the output file's path */
      while ((slash = strpbrk(argv[2], "/\\"))) {
          argv[2] = slash + 1;
      }

    #endif // POCKETMOD_SAVE_FILE

    /* Write the WAV header */
    /* Follow along at home: http://soundfile.sapp.org/doc/WaveFormat/ */
    #ifdef POCKETMOD_ORI

      #ifdef POCKETMOD_SAVE_FILE

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

      #endif // POCKETMOD_SAVE_FILE

    #else

      WaveHeader  myHeader;
      uint8_t     *wavebuffer = NULL;
      size_t      wavebuffer_size = 0;

      // RIFF WAVE Header
      myHeader.chunkId[0]       = 'R';
      myHeader.chunkId[1]       = 'I';
      myHeader.chunkId[2]       = 'F';
      myHeader.chunkId[3]       = 'F';
      myHeader.chunkSize        = 0;
      myHeader.format[0]        = 'W';
      myHeader.format[1]        = 'A';
      myHeader.format[2]        = 'V';
      myHeader.format[3]        = 'E';

      // Format subchunk
      myHeader.subChunk1Id[0]   = 'f';
      myHeader.subChunk1Id[1]   = 'm';
      myHeader.subChunk1Id[2]   = 't';
      myHeader.subChunk1Id[3]   = ' ';
      myHeader.subChunk1Size    = 16;
      myHeader.audioFormat      = 1;
      myHeader.numChannels      = 2;
      myHeader.sampleRate       = SAMPLE_RATE;
      myHeader.byteRate         = SAMPLE_RATE * 4;
      myHeader.blockAlign       = 4;
      myHeader.bitsPerSample    = 16; //

      // Data subchunk
      myHeader.subChunk2Id[0]   = 'd';
      myHeader.subChunk2Id[1]   = 'a';
      myHeader.subChunk2Id[2]   = 't';
      myHeader.subChunk2Id[3]   = 'a';
      myHeader.subChunk2Size    = 0;

      wavebuffer_size = sizeof(WaveHeader);

      wavebuffer = malloc(wavebuffer_size);
      memcpy(wavebuffer, &myHeader, wavebuffer_size);

    #endif // POCKETMOD_ORI

    /* Write sample data */
    float   buffer[512][2];
    short   output[512][2];

    #ifdef POCKETMOD_VERBOSE

      //int     loopc = 0;
      clock_t time_now, time_prev = 0;

    #endif // POCKETMOD_VERBOSE

    while (pocketmod_loop_count(&context) == 0) {

        /* Render a chunk of samples */
        int rendered_bytes = pocketmod_render(&context, buffer, sizeof(buffer));
        int rendered_samples = rendered_bytes / sizeof(float[2]);

        samples += rendered_samples;

        /* Convert the sample data to 16-bit and write it to the file */
        for (i = 0; i < rendered_samples; i++) {
            output[i][0] = (short) (clip(buffer[i][0]) * 0x7fff);
            output[i][1] = (short) (clip(buffer[i][1]) * 0x7fff);
        }

        #ifdef POCKETMOD_ORI

          #ifdef POCKETMOD_SAVE_FILE

            fwrite(output, rendered_samples * sizeof(short[2]), 1, file);

          #endif // POCKETMOD_SAVE_FILE

        #else

          size_t wavebuffer_sizeblock = rendered_samples * sizeof(short[2]);

          wavebuffer = realloc(wavebuffer, wavebuffer_size + wavebuffer_sizeblock);
          memcpy(wavebuffer + wavebuffer_size, &output, wavebuffer_sizeblock);
          wavebuffer_size += wavebuffer_sizeblock;

          #ifdef POCKETMOD_VERBOSE

            //printf("i: %d - wavebuffer_size: %zu\n", loopc++, wavebuffer_size);

          #endif // POCKETMOD_VERBOSE

        #endif // POCKETMOD_ORI

        #ifdef POCKETMOD_VERBOSE

          /* Print statistics at regular intervals */
          time_now = clock();
          if ((double) (time_now - time_prev) / CLOCKS_PER_SEC > 0.1) {
              show_stats(argv[2], samples);
              time_prev = time_now;
          }

        #endif // POCKETMOD_VERBOSE
    }

    #ifdef POCKETMOD_VERBOSE

      show_stats(argv[2], samples);
      putchar('\n');

    #endif // POCKETMOD_VERBOSE

    /* Now that we know how many samples we got, we can go back and patch the */
    /* ChunkSize and Subchunk2Size fields in the WAV header */
    #ifdef POCKETMOD_ORI

      #ifdef POCKETMOD_SAVE_FILE

        fseek(file, 4, SEEK_SET);
        fputl(samples * 4 + 36, file);
        fseek(file, 40, SEEK_SET);
        fputl(samples * 4, file);

      #endif // POCKETMOD_SAVE_FILE

    #else

      ((WaveHeader *)&wavebuffer[0])->chunkSize     = samples * 4 + 36;
      ((WaveHeader *)&wavebuffer[0])->subChunk2Size = samples * 4;

      #ifdef POCKETMOD_SAVE_FILE

        fwrite(wavebuffer, wavebuffer_size, 1, file);

      #endif // POCKETMOD_SAVE_FILE

      #ifdef POCKETMOD_PLAYER

        AudioPlayer (wavebuffer, wavebuffer_size);

      #endif // POCKETMOD_PLAYER

      free(wavebuffer);

    #endif // POCKETMOD_ORI

    /* Tidy up before leaving */
    free(mod_data);

    #ifdef POCKETMOD_SAVE_FILE

      fclose(file);

    #endif // POCKETMOD_SAVE_FILE

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
