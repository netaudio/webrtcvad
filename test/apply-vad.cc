/* Created on 2017-03-01
 * Author: Binbin Zhang
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/timeb.h>
#include <errno.h>
#include <stdarg.h>
#include <time.h>
#include <assert.h>

#include <vector>

#include "test/parse-option.h"
#include "test/wav.h"
#include "vad.h"

 #ifdef __cplusplus
    extern "C" {
#endif
#include "fvad.h"
#ifdef __cplusplus
    }
#endif

int main(int argc, char *argv[]) {
    {
        struct  tm      *ptm;
        struct  timeb   stTimeb;
        static  char    szTime[128] = "";

        ftime(&stTimeb);
        ptm = localtime(&stTimeb.time);
        sprintf(szTime, "%04d-%02d-%02d %02d:%02d:%02d.%03d",ptm->tm_year+1900,
            ptm->tm_mon + 1, ptm->tm_mday, ptm->tm_hour, ptm->tm_min, ptm->tm_sec, stTimeb.millitm);
        printf("%s\n", szTime);
    }
#if 0
    const char *usage = "Apply energy vad for input wav file\n"
                        "Usage: vad-test wav_in_file\n";
    ParseOptions po(usage);

    int frame_len = 10; // 10 ms
    po.Register("frame-len", &frame_len, "frame length in millionsenconds, must be 10/20/30");
    int mode = 0; 
    po.Register("mode", &mode, "vad mode");

    po.Read(argc, argv);

    if (po.NumArgs() != 2) {
        po.PrintUsage();
        exit(1);
    }

    std::string wav_in = po.GetArg(1), 
         wav_out = po.GetArg(2);

    WavReader reader(wav_in.c_str());

    //printf("input file %s info: \n"
    //       "sample_rate %d \n"
    //       "channels %d \n"
    //       "bits_per_sample_ %d \n",
    //       wav_in.c_str(),
    //       reader.SampleRate(), 
    //       reader.NumChannel(),
    //       reader.BitsPerSample());
    
    int sample_rate = reader.SampleRate();
    int num_sample = reader.NumSample();
    int num_point_per_frame = (int)(frame_len * sample_rate / 1000);
   
    short *data = (short *)calloc(sizeof(short), num_sample);
    // Copy first channel
    for (int i = 0; i < num_sample; i++) {
        data[i] = reader.Data()[i * reader.NumChannel()];
    }

    printf("mode %d\n", mode);
    Fvad *pFvad = NULL;
    pFvad = fvad_new();
    fvad_set_mode(pFvad,mode);
    fvad_set_sample_rate(pFvad,8000);


    int num_frames = num_sample / num_point_per_frame;
    std::vector<bool> vad_reslut;
    int num_speech_frames = 0;

    for (int i = 0; i < num_sample; i += num_point_per_frame) {
        // last frame 
        if (i + num_point_per_frame > num_sample) break;
        bool tags = fvad_process(pFvad,(int16_t*)data+i, num_point_per_frame);

        vad_reslut.push_back(tags);
        if (tags) num_speech_frames++;
        //printf("%f %d \n", float(i) / sample_rate, (int)tags);
    }

    int num_speech_sample = num_speech_frames * num_point_per_frame;
    short *speech_data = (short *)calloc(sizeof(short), num_speech_sample);
    
    int speech_cur = 0;
    for (int i = 0; i < vad_reslut.size(); i++) {
        // speech
        if (vad_reslut[i]) {
            memcpy(speech_data + speech_cur * num_point_per_frame,
                   data + i * num_point_per_frame, 
                   num_point_per_frame * sizeof(short));
            speech_cur++;
        }
    }

    WavWriter writer(speech_data, num_speech_sample, 1, 
                        reader.SampleRate(), reader.BitsPerSample());

    writer.Write(wav_out.c_str());
    free(data);
    free(speech_data);

     if (pFvad) {
        fvad_free(pFvad);
    }
#else
    //use local vad
    const char *usage = "Apply energy vad for input wav file\n"
                        "Usage: vad-test wav_in_file\n";
    ParseOptions po(usage);

    int frame_len = 10; // 10 ms
    po.Register("frame-len", &frame_len, "frame length in millionsenconds, must be 10/20/30");
    int mode = 0; 
    po.Register("mode", &mode, "vad mode");

    po.Read(argc, argv);

    if (po.NumArgs() != 2) {
        po.PrintUsage();
        exit(1);
    }

    std::string wav_in = po.GetArg(1), 
         wav_out = po.GetArg(2);

    WavReader reader(wav_in.c_str());

    //printf("input file %s info: \n"
    //       "sample_rate %d \n"
    //       "channels %d \n"
    //       "bits_per_sample_ %d \n",
    //       wav_in.c_str(),
    //       reader.SampleRate(), 
    //       reader.NumChannel(),
    //       reader.BitsPerSample());
    
    int sample_rate = reader.SampleRate();
    int num_sample = reader.NumSample();
    int num_point_per_frame = (int)(frame_len * sample_rate / 1000);
   
    short *data = (short *)calloc(sizeof(short), num_sample);
    // Copy first channel
    for (int i = 0; i < num_sample; i++) {
        data[i] = reader.Data()[i * reader.NumChannel()];
    }

    printf("mode %d\n", mode);
    Vad vad(mode);

    int num_frames = num_sample / num_point_per_frame;
    std::vector<bool> vad_reslut;
    int num_speech_frames = 0;

    for (int i = 0; i < num_sample; i += num_point_per_frame) {
        // last frame 
        if (i + num_point_per_frame > num_sample) break;
        bool tags = vad.IsSpeech(data+i, num_point_per_frame, sample_rate);
        vad_reslut.push_back(tags);
        if (tags) num_speech_frames++;
        //printf("%f %d \n", float(i) / sample_rate, (int)tags);
    }

    int num_speech_sample = num_speech_frames * num_point_per_frame;
    short *speech_data = (short *)calloc(sizeof(short), num_speech_sample);
    
    int speech_cur = 0;
    for (int i = 0; i < vad_reslut.size(); i++) {
        // speech
        if (vad_reslut[i]) {
            memcpy(speech_data + speech_cur * num_point_per_frame,
                   data + i * num_point_per_frame, 
                   num_point_per_frame * sizeof(short));
            speech_cur++;
        }
    }

    WavWriter writer(speech_data, num_speech_sample, 1, 
                        reader.SampleRate(), reader.BitsPerSample());

    writer.Write(wav_out.c_str());
    free(data);
    free(speech_data);
#endif

    {
        struct  tm      *ptm;
        struct  timeb   stTimeb;
        static  char    szTime[128] = "";

        ftime(&stTimeb);
        ptm = localtime(&stTimeb.time);
        sprintf(szTime, "%04d-%02d-%02d %02d:%02d:%02d.%03d",ptm->tm_year+1900,
            ptm->tm_mon + 1, ptm->tm_mday, ptm->tm_hour, ptm->tm_min, ptm->tm_sec, stTimeb.millitm);
        printf("%s\n", szTime);
    }
    return 0;
}


