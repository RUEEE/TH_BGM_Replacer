#include "BGM_player.h"
#include "utils.h"
#include <iostream>
#include <algorithm>
#pragma comment(lib, "dsound.lib")
#pragma comment(lib, "dxguid.lib")


BGM_Player::BGM_Player():pxAudio(nullptr),pMasteringVoice(nullptr),pSourceVoice(nullptr),pCallback(nullptr), is_paused(false), volume(0.5f),
loop_pos_estm_begin(0.0f),loop_pos_estm_end(1.0f)
{

}

HRESULT BGM_Player::LoadBGM(BGM_Info* pBGMInfo)
{
	if (pSourceVoice){
		Stop();
		pSourceVoice->DestroyVoice();
	}
	loop_pos_estm_begin = 0.0f;
	loop_pos_estm_end = 1.0f;
	is_paused = true;
	curBGM = pBGMInfo;
	// set buffer
	HRESULT hr;
	if (pCallback)
		delete pCallback;
	pCallback = new PlayerCallback();
	pCallback->player = this;
	if (FAILED(hr = pxAudio->CreateSourceVoice(&pSourceVoice,&pBGMInfo->wavHeader,0,2.0f, pCallback,NULL,NULL)))
		return hr;
	samplesPlayedBeforeLastStop = 0;
	volume = SetVolume(volume);
	// load buffer from file
	buffer.clear();
	std::fstream fs;
	if(pBGMInfo->type == DAT_FILE)
		fs = std::fstream(pBGMInfo->BGMFilePath_dat, std::ios::in | std::ios::binary);
	else
		fs = std::fstream(pBGMInfo->BGMFilePath_wav, std::ios::in | std::ios::binary);
	if (!fs.is_open())
		return E_FAIL;
	fs.seekg(pBGMInfo->GetBeginPos(), std::ios::beg);
	int totLen = pBGMInfo->GetTotalLen();
	buffer.resize(totLen);
	fs.read((char*)buffer.data(), totLen);
	if (!fs.good()){
		return E_FAIL;
	}
	loop_pos_estm_begin = ((pBGMInfo->GetBeginLen_s()*0.75f - 1.0f) / (float)pBGMInfo->GetTotalLen_s());
	loop_pos_estm_begin = std::clamp(loop_pos_estm_begin, 0.0f, 1.0f);
	loop_pos_estm_end = ((float)pBGMInfo->GetBeginLoopLen() / (float)pBGMInfo->GetTotalLen()) * 0.75f + loop_pos_estm_begin * 0.25f;
	loop_pos_estm_end = std::clamp(loop_pos_estm_end, loop_pos_estm_begin, 1.0f);
	// set buffer
	return S_OK;
}


HRESULT BGM_Player::Init()
{
	HRESULT hr=S_OK;
	if (FAILED(hr = XAudio2Create(&pxAudio, 0)))
		return hr;
	if (FAILED(hr = pxAudio->CreateMasteringVoice(&pMasteringVoice)))
		return hr;
}

HRESULT BGM_Player::Play(BGM_Info* pInfo, DWORD pos)
{
	if (!pSourceVoice || curBGM!=pInfo){
		if (FAILED(this->LoadBGM(pInfo))) {
			MessageBoxA(NULL, "Fail to load BGM. Please ensure BGM format is wav(PCM, better use 44100Hz/stereo/16-bit).", "failed", MB_OK);
		}
	}
	is_paused = false;
	XAUDIO2_VOICE_STATE state;
	pSourceVoice->GetState(&state);
	if (state.BuffersQueued==0){
		memset(&buf,0,sizeof(buf));
		buf.Flags = 0;
		DWORD sample_size = pInfo->GetSampleByteSize();
		buf.pAudioData = buffer.data();
		buf.AudioBytes = buffer.size()/sample_size * sample_size; //aligned to sample
		if (pos < 0)pos = 0;
		if (pos < pInfo->GetBeginLoopLen()){
			buf.PlayBegin = pos / sample_size;
			buf.PlayLength = pInfo->GetBeginLoopLen()/ sample_size - buf.PlayBegin;
			buf.LoopBegin = pInfo->GetBeginLen()/ sample_size;
			buf.LoopLength = pInfo->GetLoopLen() / sample_size;
			buf.LoopCount = XAUDIO2_LOOP_INFINITE;
		}else{
			if (pos > pInfo->GetTotalLen())
				pos = pInfo->GetTotalLen() - 100;
			buf.PlayBegin = pos / sample_size;
			buf.PlayLength = pInfo->GetTotalLen() / sample_size - buf.PlayBegin;
			buf.LoopBegin = 0;
			buf.LoopLength = 0;
			buf.LoopCount = 0;
		}
		samplesPlayedBeforeLastStop = state.SamplesPlayed - buf.PlayBegin;
		buf.pContext = NULL;
		HRESULT hr = pSourceVoice->SubmitSourceBuffer(&buf);
		if (FAILED(hr))
			return hr;
	}
	return pSourceVoice->Start(0, XAUDIO2_COMMIT_NOW);
}

/****************************************************************************
*
* NAME: smbPitchShift.cpp
* VERSION: 1.2
* HOME URL: http://blogs.zynaptiq.com/bernsee
* KNOWN BUGS: none
*
* SYNOPSIS: Routine for doing pitch shifting while maintaining
* duration using the Short Time Fourier Transform.
*
* DESCRIPTION: The routine takes a pitchShift factor value which is between 0.5
* (one octave down) and 2. (one octave up). A value of exactly 1 does not change
* the pitch. numSampsToProcess tells the routine how many samples in indata[0...
* numSampsToProcess-1] should be pitch shifted and moved to outdata[0 ...
* numSampsToProcess-1]. The two buffers can be identical (ie. it can process the
* data in-place). fftFrameSize defines the FFT frame size used for the
* processing. Typical values are 1024, 2048 and 4096. It may be any value <=
* MAX_FRAME_LENGTH but it MUST be a power of 2. oversampfac is the STFT
* oversampling factor which also determines the overlap between adjacent STFT
* frames. It should at least be 4 for moderate scaling ratios. A value of 32 is
* recommended for best quality. sampleRate takes the sample rate for the signal
* in unit Hz, ie. 44100 for 44.1 kHz audio. The data passed to the routine in
* indata[] should be in the range [-1.0, 1.0), which is also the output range
* for the data, make sure you scale the data accordingly (for 16bit signed integers
* you would have to divide (and multiply) by 32768).
*
* COPYRIGHT 1999-2015 Stephan M. Bernsee <s.bernsee [AT] zynaptiq [DOT] com>
*
* 						The Wide Open License (WOL)
*
* Permission to use, copy, modify, distribute and sell this software and its
* documentation for any purpose is hereby granted without fee, provided that
* the above copyright notice and this license appear in all source copies.
* THIS SOFTWARE IS PROVIDED "AS IS" WITHOUT EXPRESS OR IMPLIED WARRANTY OF
* ANY KIND. See http://www.dspguru.com/wol.htm for more information.
*
*****************************************************************************/

#define M_PI 3.14159265358979323846
inline void smbFft(float* fftBuffer, long fftFrameSize, long sign)
/*
	FFT routine, (C)1996 S.M.Bernsee. Sign = -1 is FFT, 1 is iFFT (inverse)
	Fills fftBuffer[0...2*fftFrameSize-1] with the Fourier transform of the
	time domain data in fftBuffer[0...2*fftFrameSize-1]. The FFT array takes
	and returns the cosine and sine parts in an interleaved manner, ie.
	fftBuffer[0] = cosPart[0], fftBuffer[1] = sinPart[0], asf. fftFrameSize
	must be a power of 2. It expects a complex input signal (see footnote 2),
	ie. when working with 'common' audio signals our input signal has to be
	passed as {in[0],0.,in[1],0.,in[2],0.,...} asf. In that case, the transform
	of the frequencies of interest is in fftBuffer[0...fftFrameSize].
*/
{
	float wr, wi, arg, * p1, * p2, temp;
	float tr, ti, ur, ui, * p1r, * p1i, * p2r, * p2i;
	long i, bitm, j, le, le2, k;

	for (i = 2; i < 2 * fftFrameSize - 2; i += 2) {
		for (bitm = 2, j = 0; bitm < 2 * fftFrameSize; bitm <<= 1) {
			if (i & bitm) j++;
			j <<= 1;
		}
		if (i < j) {
			p1 = fftBuffer + i; p2 = fftBuffer + j;
			temp = *p1; *(p1++) = *p2;
			*(p2++) = temp; temp = *p1;
			*p1 = *p2; *p2 = temp;
		}
	}
	for (k = 0, le = 2; k < (long)(log(fftFrameSize) / log(2.) + .5); k++) {
		le <<= 1;
		le2 = le >> 1;
		ur = 1.0;
		ui = 0.0;
		arg = M_PI / (le2 >> 1);
		wr = cos(arg);
		wi = sign * sin(arg);
		for (j = 0; j < le2; j += 2) {
			p1r = fftBuffer + j; p1i = p1r + 1;
			p2r = p1r + le2; p2i = p2r + 1;
			for (i = j; i < 2 * fftFrameSize; i += le) {
				tr = *p2r * ur - *p2i * ui;
				ti = *p2r * ui + *p2i * ur;
				*p2r = *p1r - tr; *p2i = *p1i - ti;
				*p1r += tr; *p1i += ti;
				p1r += le; p1i += le;
				p2r += le; p2i += le;
			}
			tr = ur * wr - ui * wi;
			ui = ur * wi + ui * wr;
			ur = tr;
		}
	}
}

std::vector<float> STFT(const std::vector<float>& audio, int wind_size, float overlap = 0.5f,
	int* p_step = nullptr,int* p_nFrame = nullptr,int* p_num_bins = nullptr,
	int begin_sample1=-1,int end_sample1=-1
	, int begin_sample2 = -1, int end_sample2 = -1) {
	if (audio.empty() || wind_size <= 0) return {};
	if ((wind_size & (wind_size - 1)) != 0) {
		MessageBoxA(NULL, "Error: wind_size must be a power of 2.", "err", MB_OK);
		return {};
	}
	
	int step = wind_size * (1.0f - overlap);
	if (step < 1) step = 1;
	if (p_step)
		*p_step = step;

	int num_samples = audio.size();
	int num_frames = 0;
	if (num_samples >= wind_size) {
		num_frames = (num_samples - wind_size) / step + 1;
	}
	else {
		return {};
	}
	if (p_nFrame)
		*p_nFrame = num_frames;
	// hanning
	std::vector<float> window(wind_size);
	for (int i = 0; i < wind_size; ++i) {
		window[i] = 0.5f * (1.0f - cosf(2.0f * (float)M_PI * i / (wind_size - 1)));
	}

	int num_bins = wind_size / 2 + 1;
	if (p_num_bins)
		*p_num_bins = num_bins;
	std::vector<float> spec;
	spec.resize(num_frames * num_bins);
	// 2*windsize(Real/Imag/Real/Imag/...)
	std::vector<float> work_buffer(2 * wind_size);
	int audio_pos = 0;
	for (int frame = 0; frame < num_frames; ++frame) {
		bool overlap1 = true;
		bool overlap2 = true;
		
		if (begin_sample1 != -1 && end_sample1 != -1 && (audio_pos < begin_sample1) || (audio_pos > end_sample1)){
			overlap1 = false;
		}
		if (begin_sample2 != -1 && end_sample2 != -1 && (audio_pos < begin_sample2) || (audio_pos > end_sample2)) {
			overlap2 = false;
		}
		if (!overlap1 && !overlap2)
		{
			audio_pos += step;
			continue;
		}

		for (int i = 0; i < wind_size; ++i) {
			work_buffer[2 * i] = audio[audio_pos + i] * window[i];
			work_buffer[2 * i + 1] = 0.0f;
		}
		smbFft(work_buffer.data(), wind_size, -1);
		int output_offset = frame * num_bins;
		for (int i = 0; i < num_bins; ++i) {
			float re = work_buffer[2 * i];
			float im = work_buffer[2 * i + 1];
			// spec[output_offset + i] = sqrtf(re * re + im * im);
			spec[output_offset + i] = log10f(re * re + im * im + 1e-6);
		}
		audio_pos += step;
	}
	return spec;
}

std::vector<float> LinearToMelLog(const std::vector<float>& linear_spec,
	int n_fft_bins,int sample_rate,int n_mels)
{
	struct Weight { int bin_idx; float weight; };
	static int last_n_mel = -1;
	static std::vector<std::vector<Weight>> filters;

	int total_size = linear_spec.size();
	int num_frames = total_size / n_fft_bins;
	if (n_mels != last_n_mel)
	{
		auto HzToMel = [](float hz)->float {
			return 2595.0f * std::log10f(1.0f + hz / 700.0f);
			};
		auto MelToHz = [](float mel)->float {
			return 700.0f * (std::powf(10.0f, mel / 2595.0f) - 1.0f);
			};

		if (linear_spec.empty() || n_fft_bins <= 0 || n_mels <= 0)
			return {};

		int n_fft = (n_fft_bins - 1) * 2;

		float min_mel = HzToMel(0.0f);
		float max_mel = HzToMel(sample_rate / 2.0f);

		std::vector<float> mel_points(n_mels + 2);
		float step = (max_mel - min_mel) / (n_mels + 1);

		for (int i = 0; i < n_mels + 2; ++i) {
			mel_points[i] = MelToHz(min_mel + i * step);
		}
		std::vector<int> bin_indices(n_mels + 2);
		for (int i = 0; i < n_mels + 2; ++i) {
			// freq / (sr / n_fft)
			float bin = mel_points[i] * n_fft / sample_rate;
			bin_indices[i] = static_cast<int>(std::floor(bin));
			if (bin_indices[i] >= n_fft_bins) bin_indices[i] = n_fft_bins - 1;
		}

		filters = std::vector<std::vector<Weight>>(n_mels);
		for (int m = 0; m < n_mels; ++m) {
			int start = bin_indices[m];
			int center = bin_indices[m + 1];
			int end = bin_indices[m + 2];
			for (int k = start; k < center; ++k) {
				float w = (float)(k - start) / (center - start);
				if (w > 0.0f) filters[m].push_back({ k, w });
			}
			for (int k = center; k < end; ++k) {
				float w = (float)(end - k) / (end - center);
				if (w > 0.0f) filters[m].push_back({ k, w });
			}
		}
	}

	std::vector<float> mel_spec;
	mel_spec.resize(num_frames * n_mels);
	for (size_t t = 0; t < num_frames; ++t) {
		size_t src_offset = t * n_fft_bins;
		size_t dst_offset = t * n_mels;
		for (int m = 0; m < n_mels; ++m) {
			float energy = 0.0f;
			for (const auto& w : filters[m]) {
				energy += linear_spec[src_offset + w.bin_idx] * w.weight;
			}
			// mel_spec[dst_offset + m] = log10f(1.0f + 100.0f * energy);
			mel_spec[dst_offset + m] = powf(energy,0.25f);
		}
	}
	return mel_spec;
}

float CalcCosSim(const std::vector<float>& spec, int frame_A, int frame_B,int frame_len, int num_bins) {
	size_t offset_A = frame_A * num_bins;
	size_t offset_B = frame_B * num_bins;
	float dot = 0.0;
	float norm_A = 0.0;
	float norm_B = 0.0;
	if ((frame_A + frame_len) * num_bins > spec.size() ||
		(frame_B + frame_len) * num_bins > spec.size()) {
		return 0.0f;
	}
	float avg_A = 0, avg_B = 0;
	int tot_bin_cmp = num_bins * frame_len;
	for (int k = 0; k < tot_bin_cmp; ++k) {
		avg_A += spec[offset_A + k];
		avg_B += spec[offset_B + k];
	}
	avg_A /= tot_bin_cmp;
	avg_B /= tot_bin_cmp;
	// remove avgs;
	for (int k = 0; k < tot_bin_cmp; ++k) {
		float val_A = spec[offset_A + k]- avg_A;
		float val_B = spec[offset_B + k]- avg_B;

		dot += val_A * val_B;
		norm_A += val_A * val_A;
		norm_B += val_B * val_B;
	}
	if (norm_A == 0 || norm_B == 0) return 0.0f;
	return (float)(dot / (sqrt(norm_A) * sqrt(norm_B)));
}

inline double CalcSSD(std::vector<float>& buf, int sampleA, int sampleB, int sampleCount,double last = 1e304) {
    double diffTot = 0;
	if (sampleA+sampleCount>=buf.size()){
		sampleCount = buf.size() - sampleA;
	}
	if (sampleB + sampleCount >= buf.size()) {
		sampleCount = buf.size() - sampleB;
	}
    for (int i = 0; i < sampleCount; i++) {
    	double d = buf[sampleA+i] - buf[sampleB + i];
		diffTot += d * d;
    	if (diffTot > last)
    		break;
    }
    return diffTot;
}

DWORD BGM_Player::EstimateLoop(BGM_Info* pInfo)
{
	if (!pSourceVoice || curBGM != pInfo) {
		if (FAILED(this->LoadBGM(pInfo))) {
			MessageBoxA(NULL, "Fail to load BGM. Please ensure BGM format is wav(PCM, better use 44100Hz/stereo/16-bit).", "failed", MB_OK);
		}
	}
	if (buffer.size() == 0)
		return 0;
	
	// create a buffer
	std::vector<float> f_buf;
	f_buf.reserve(buffer.size());

	DWORD p = (DWORD)buffer.data();
	DWORD p_end = (DWORD)&buffer[buffer.size() - 1];
	if (pInfo->wavHeader.nChannels == 2)
	{
		switch (pInfo->wavHeader.wBitsPerSample){
		case 8:
			for (; p < p_end;) {
				auto p1 = (unsigned char*)p; p += 2;
				f_buf.push_back(((p1[0] / 255.0f) + (p1[1] / 255.0f))*0.5f);
			}
			break;
		case 16:
			for (; p < p_end;) {
				auto p1 = (short*)p; p += 4;
				f_buf.push_back(((p1[0] / 32768.0f) + (p1[1] / 32768.0f)) * 0.5f);
			}
			break;
		case 32:
			for (; p < p_end;) {
				auto p1 = (int*)p; p += 8;
				f_buf.push_back(((p1[0] / 2147483648.0) + (p1[1] / 2147483648.0)) * 0.5f);
			}
			break;
		}
	}else{
		switch (pInfo->wavHeader.wBitsPerSample)
		{
		case 8:
			for (; p < p_end;) {
				f_buf.push_back(*(unsigned char*)p / 255.0f);
				p += 1 * pInfo->wavHeader.nChannels;
			}
			break;
		case 16:
			for (; p < p_end;) {
				f_buf.push_back(*(int16_t*)p / 32768.0f);
				p += 2 * pInfo->wavHeader.nChannels;
			}
			break;
		case 32:
			for (; p < p_end;) {
				f_buf.push_back(*(int32_t*)p / 2147483648.0);
				p += 4 * pInfo->wavHeader.nChannels;
			}
			break;
		}
	}
	
	// use STFT to calculate the dest sample

	int sample_size = pInfo->GetSampleByteSize();
	int end_sample = pInfo->GetBeginLoopLen() / sample_size;
	if (end_sample >= f_buf.size())
		end_sample = f_buf.size() - 1;
	int begin_search_sample = this->loop_pos_estm_begin * pInfo->GetTotalLen() / sample_size;
	begin_search_sample = std::clamp(begin_search_sample, 0, (int)f_buf.size() - 1);

	int end_search_sample = this->loop_pos_estm_end * pInfo->GetTotalLen() / sample_size;
	end_search_sample = std::clamp(end_search_sample, begin_search_sample, (int)f_buf.size() - 1);
	if (end_search_sample - begin_search_sample < 1000)
		return 0;

	int STFT_step = 0,STFT_frame = 0,n_bins = 0;
	int wind_size = 2048;
	auto spec = STFT(f_buf, wind_size, 0.5,&STFT_step,&STFT_frame,&n_bins,
		begin_search_sample - pInfo->wavHeader.nSamplesPerSec,end_search_sample + pInfo->wavHeader.nSamplesPerSec,
		end_sample - pInfo->wavHeader.nSamplesPerSec * 2,end_sample + pInfo->wavHeader.nSamplesPerSec * 2);
	if (spec.empty())
		return 0;

	// int n_bins_mel = 120;
	// auto spec_mel = LinearToMelLog(spec, n_bins, pInfo->wavHeader.nSamplesPerSec, n_bins_mel);

	auto  SampleToFrame = [&](int sample_idx) -> int {
		if (sample_idx < 0) return 0;
		int min_frame = 0;
		if (sample_idx >= wind_size) {
			min_frame = (sample_idx - wind_size) / STFT_step + 1;
		}
		if (min_frame >= STFT_frame) return STFT_frame - 1;
		return min_frame;
		};
	auto FrameToSample = [&](int frame_idx) -> int {
		return frame_idx * STFT_step + wind_size - 1;
	};

	int frame_wind = 24;
	int end_frame = SampleToFrame(end_sample);

	int end_frame_end = end_frame + frame_wind/2;
	int end_frame_begin = end_frame_end - frame_wind;
	if (end_frame_begin <= 0)
		return 0;
	if (end_frame_end >= STFT_frame){
		end_frame_end = STFT_frame - 1;
		end_frame_begin = end_frame_end - frame_wind;
	}

	int begin_search_frame = SampleToFrame(begin_search_sample);
	int end_search_frame = SampleToFrame(end_search_sample);
	
	float sim = -999.0f;
	int n_search = 0;

	for (int i = begin_search_frame; i < end_search_frame; i += 1)
	{
		// float cur_sim = CalcCosSim(spec_mel, i, end_frame, frame_wind, n_bins_mel);
		float cur_sim = CalcCosSim(spec, i, end_frame_begin, frame_wind, n_bins);
		if (cur_sim > sim){
			sim = cur_sim;
			n_search = i;
		}
	}
	int loop_begin_sample_estm1 = FrameToSample(n_search + end_frame - end_frame_begin);

	// use SSD to get precise sample
	int search_sample = pInfo->wavHeader.nSamplesPerSec * 0.2;
	int cmp_sample = pInfo->wavHeader.nSamplesPerSec * 0.02;

	if (end_sample > cmp_sample && loop_begin_sample_estm1 > cmp_sample){
		int search_sample_begin = loop_begin_sample_estm1 - search_sample;
		if (search_sample_begin < 0)search_sample_begin = 0;
		int search_sample_end = loop_begin_sample_estm1 + search_sample;
		if (search_sample_end >= end_sample)search_sample_end = end_sample - 1;

		double diff = CalcSSD(f_buf, loop_begin_sample_estm1 - cmp_sample, end_sample - cmp_sample, cmp_sample * 2, 1e304);
		int prec_sample = loop_begin_sample_estm1 - cmp_sample;

		for (int i = search_sample_begin; i < search_sample_end; i++) {
			double cur_diff = CalcSSD(f_buf, i - cmp_sample, end_sample - cmp_sample, cmp_sample * 2, diff);
			if (cur_diff < diff){
				diff = cur_diff;
				prec_sample = i;
			}
		}
		pInfo->SetBeginLen((int)prec_sample * sample_size);
		return prec_sample * sample_size;
	}
	pInfo->SetBeginLen((int)loop_begin_sample_estm1* sample_size);
	return loop_begin_sample_estm1 * sample_size;
}

HRESULT BGM_Player::Stop()
{
	if (!pSourceVoice)
		return E_FAIL;
	XAUDIO2_VOICE_STATE state;
	pSourceVoice->GetState(&state);
	while (state.BuffersQueued){
		HRESULT hr = pSourceVoice->Stop(0, XAUDIO2_COMMIT_NOW);
		if (FAILED(hr))
			return hr;
		if (FAILED(hr = pSourceVoice->FlushSourceBuffers()))
			return hr;
		Sleep(1);
		pSourceVoice->GetState(&state);
	}
	samplesPlayedBeforeLastStop = state.SamplesPlayed;
	is_paused = true;
	return S_OK;
}

HRESULT BGM_Player::Pause()
{
	if (!pSourceVoice)
		return E_FAIL;
	is_paused = true;
	return pSourceVoice->Stop(0, XAUDIO2_COMMIT_NOW);
}

HRESULT BGM_Player::SetPos(BGM_Info* pInfo, DWORD pos)
{
	if (!pSourceVoice)
		return E_FAIL;
	HRESULT hr;
	bool pause2 = is_paused;
	hr = this->Stop();
	if (FAILED(hr))
		return hr;
	if (FAILED(hr = this->Play(pInfo, pos)))
		return hr;
	if(pause2)
		return this->Pause();
	return hr;
}

DWORD BGM_Player::GetCurPos()
{
	if (!pSourceVoice)
		return 0;
	XAUDIO2_VOICE_STATE state;
	pSourceVoice->GetState(&state);
	DWORD sample_size = curBGM->GetSampleByteSize();
	return (state.SamplesPlayed - samplesPlayedBeforeLastStop) * sample_size;
}

HRESULT BGM_Player::ResetCurBGM()
{
	auto pos = this->GetCurPos();
	bool pause2 = is_paused;
	HRESULT hr = this->Stop();
	if (FAILED(hr))
		return hr;
	if (FAILED(hr = Play(this->curBGM, pos)))
		return hr;
	if (pause2)
		return this->Pause();
	return hr;
}

float BGM_Player::SetVolume(float vin)
{
	volume = vin * vin /10.0f;
	if (pSourceVoice){
		pSourceVoice->SetVolume(vin * vin / 10.0f);
		volume = GetVolume();
	}
	return volume;
}

float BGM_Player::GetVolume()
{
	if (pSourceVoice) {
		float v = 0.0f;
		pSourceVoice->GetVolume(&v);
		volume = sqrt(v*10.0f);
	}
	return volume;
}


BGM_Player::~BGM_Player()
{
	if(pMasteringVoice)pMasteringVoice->DestroyVoice();
	if (pxAudio)pxAudio->Release();
}
