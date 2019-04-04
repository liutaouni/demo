/*
@file
@brief ����¼���ӿں�MSC�ӿڷ�װһ��MIC¼��ʶ���ģ��

@author		taozhang9
@date		2016/05/27
*/

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

#include "../../include/qisr.h"
#include "../../include/msp_cmn.h"
#include "../../include/msp_errors.h"
//#include "../../include/qivw.h"
#include "./include/winrec.h"
#include "./include/speech_recognizer.h"

#ifdef _WIN64
#pragma comment(lib,"../../libs/msc_x64.lib") //x64
#else
#pragma comment(lib,"../../libs/msc.lib") //x86
#endif

#define SR_DBGON 1
#if SR_DBGON == 1
#	define sr_dbg printf
//#	define __FILE_SAVE_VERIFY__  /* save the recording data into file 'rec.pcm' too */
#else
#	define sr_dbg
#endif


#define DEFAULT_FORMAT		\
{\
	WAVE_FORMAT_PCM,	\
	1,					\
	16000,				\
	32000,				\
	2,					\
	16,					\
	sizeof(WAVEFORMATEX)	\
}

/* internal state */
enum {
	SR_STATE_INIT,
	SR_STATE_STARTED,
	SR_STATE_WAIT_AWAKE_FIRST,
	SR_STATE_WAIT_AWAKE_SECOND,
	SR_STATE_WAIT_AWAKE_FINISH,
	SR_STATE_WAIT_ORDER,
	SR_STATE_WAIT_FINISH
};

/* for debug. saving the recording to a file */
#ifdef __FILE_SAVE_VERIFY__
#define VERIFY_FILE_NAME	"rec.pcm"
static int open_stored_file(const char * name);
static int loopwrite_to_file(char *data, size_t length);
static void safe_close_file();
#endif

#define SR_MALLOC malloc
#define SR_MFREE  free
#define SR_MEMSET	memset

#ifdef __FILE_SAVE_VERIFY__

static FILE *fdwav = NULL;

static int open_stored_file(const char * name)
{
	fdwav = fopen(name, "wb+");
	if(fdwav == NULL) {
		printf("error open file failed\n");
		return -1;
	}
	return 0;
}

static int loopwrite_to_file(char *data, size_t length)
{
	size_t wrt = 0, already = 0;
	int ret = 0;
	if(fdwav == NULL || data == NULL)
		return -1;

	while(1) {
		wrt = fwrite(data + already, 1, length - already, fdwav);
		if(wrt == (length - already) )
			break;
		if(ferror(fdwav)) {
			ret = -1;
			break;
		}
		already += wrt;
	}

	return ret;
}

static void safe_close_file()
{
	if(fdwav) {
		fclose(fdwav);
		fdwav = NULL;
	}
}
#endif

static void end_sr_awake_on_error(struct speech_rec *sr, int errcode)
{
	if (sr->notif.on_error){
		sr->notif.on_error(errcode);
	}
}

static void end_sr_order_on_error(struct speech_rec *sr, int errcode)
{
	sr->state = SR_STATE_WAIT_FINISH;
	if (sr->notif.on_error){
		sr->notif.on_error(errcode);
	}
	if (sr->session_id) {
		QISRSessionEnd(sr->session_id, "err");
		sr->session_id = NULL;
	}
}

static void end_sr_order_on_vad(struct speech_rec *sr)
{
	int errcode;
	int ret;
	const char *rslt = NULL;
		
	sr->state = SR_STATE_WAIT_FINISH;
	ret = QISRAudioWrite(sr->session_id, NULL, 0, MSP_AUDIO_SAMPLE_LAST, &sr->ep_stat, &sr->rec_stat);
	if (ret != 0) {
		sr_dbg("write LAST_SAMPLE failed: %d\n", ret);
		if (sr->session_id) {
			QISRSessionEnd(sr->session_id, "write err");
			sr->session_id = NULL;
		}
		if (sr->notif.on_error){
			sr->notif.on_error(END_REASON_VAD_DETECT);
		}
		return ret;
	}
	sr->rec_stat = MSP_AUDIO_SAMPLE_CONTINUE;
	while(sr->rec_stat != MSP_REC_STATUS_COMPLETE ){
		rslt = QISRGetResult(sr->session_id, &sr->rec_stat, 0, &errcode);
		Sleep(100); /* for cpu occupy, should sleep here */
	}

	if (sr->notif.on_order_result){
		sr->notif.on_order_result(rslt, sr->rec_stat == MSP_REC_STATUS_COMPLETE ? 1 : 0);
	}

	if (sr->session_id) {
		QISRSessionEnd(sr->session_id, "VAD Normal");
		sr->session_id = NULL;
	}
}
/* the record call back */
static void asr_cb(char *data, unsigned long len, void *user_para)
{
	int errcode;
	struct speech_rec *sr;

	if(len == 0 || data == NULL)
		return;

	sr = (struct speech_rec *)user_para;

	if (sr == NULL)// || sr->ep_stat >= MSP_EP_AFTER_SPEECH)
		return;
	
#ifdef __FILE_SAVE_VERIFY__
	loopwrite_to_file(data, len);
#endif

	sr_write_audio_data(sr, data, len);
}

static char * skip_space(char *s)
{
	while (s && *s != ' ' && *s != '\0')
		s++;
	return s;
}

static int update_format_from_sessionparam(const char * session_para, WAVEFORMATEX *wavefmt)
{
	char *s;
	if ((s = strstr(session_para, "sample_rate"))) {
		if (s = strstr(s, "=")) {
			s = skip_space(s);
			if (s && *s) {
				wavefmt->nSamplesPerSec = atoi(s);
				wavefmt->nAvgBytesPerSec = wavefmt->nBlockAlign * wavefmt->nSamplesPerSec;
			}
		}
		else
			return -1;
	}
	else {
		return -1;
	}

	return 0;
}

/*  
 * if devid == -1, thenthe default input device will be used.  
 */

int sr_init(struct speech_rec * sr, const char * session_begin_params, int devid, struct speech_rec_notifier * notify)
{
	int errcode;
	size_t param_size;
	WAVEFORMATEX wavfmt = DEFAULT_FORMAT;

	if (get_input_dev_num() == 0) {
		sr_dbg("no mic\n");
		return -E_SR_NOACTIVEDEVICE;
	}

	if (!sr){
		sr_dbg("sr is empty\n");
		return -E_SR_INVAL;
	}

	SR_MEMSET(sr, 0, sizeof(struct speech_rec));
	sr->state = SR_STATE_INIT;

	param_size = strlen(session_begin_params) + 1;
	sr->session_begin_params = (char*)SR_MALLOC(param_size);
	if (sr->session_begin_params == NULL) {
		sr_dbg("mem alloc failed\n");
		return -E_SR_NOMEM;
	}
	strncpy(sr->session_begin_params, session_begin_params, param_size);

	sr->notif = *notify;
	
	errcode = create_recorder(&sr->recorder, asr_cb, (void*)sr);
	if (sr->recorder == NULL || errcode != 0) {
		sr_dbg("create recorder failed: %d\n", errcode);
		errcode = -E_SR_RECORDFAIL;
		goto fail;
	}
	update_format_from_sessionparam(session_begin_params, &wavfmt);

	errcode = open_recorder(sr->recorder, devid, &wavfmt);
	if (errcode != 0) {
		sr_dbg("recorder open failed: %d\n", errcode);
		errcode = -E_SR_RECORDFAIL;
		goto fail;
	}

	return 0;

fail:
	if (sr->recorder) {
		destroy_recorder(sr->recorder);
		sr->recorder = NULL;
	}

	if (sr->session_begin_params) {
		SR_MFREE(sr->session_begin_params);
		sr->session_begin_params = NULL;
	}
	SR_MEMSET(&sr->notif, 0, sizeof(sr->notif));

	return errcode;
}

int sr_start_listening(struct speech_rec *sr)
{
	int ret;

	ret = start_record(sr->recorder);
	if (ret != 0) {
		sr_dbg("start record failed: %d\n", ret);
		return -E_SR_RECORDFAIL;
	}
#ifdef __FILE_SAVE_VERIFY__
	open_stored_file(VERIFY_FILE_NAME);
#endif
	sr->state = SR_STATE_STARTED;

	return 0;
}

int sr_start_listening_for_awake(struct speech_rec *sr)
{
	const char*		session_id = NULL;
	int				errcode = MSP_SUCCESS;
	const char *	ssb_param = "ivw_threshold=0:1450,sst=wakeup,ivw_res_path =fo|res/ivw/wakeupresource.jet";

	session_id = QIVWSessionBegin(NULL, ssb_param, &errcode);
	if (errcode != MSP_SUCCESS)
	{
		printf("QIVWSessionBegin failed! error code:%d\n", errcode);
		goto exit;
	}

	errcode = QIVWRegisterNotify(session_id, sr->notif.on_awake_result, NULL);
	if (errcode != MSP_SUCCESS)
	{
		_snprintf(sr->sse_hints, sizeof(sr->sse_hints), "QIVWRegisterNotify errorCode=%d", errcode);
		printf("QIVWRegisterNotify failed! error code:%d\n", errcode);
		goto exit;
	}

	sr->session_id = session_id;
	sr->state = SR_STATE_WAIT_AWAKE_FIRST;

	return 0;

exit:
	if (NULL != session_id)
	{
		QIVWSessionEnd(session_id, sr->sse_hints);
	}

	return -1;
}

void sr_stop_listening_for_awake(struct speech_rec *sr)
{
	int ret;
	if (sr)
	{
		sr->state = SR_STATE_WAIT_AWAKE_FINISH;
		ret = QIVWAudioWrite(sr->session_id, NULL, 0, MSP_AUDIO_SAMPLE_LAST);
		if (MSP_SUCCESS != ret)
		{
			_snprintf(sr->sse_hints, sizeof(sr->sse_hints), "QIVWAudioWrite errorCode=%d", ret);
		}
		if (NULL != sr->session_id)
		{
			QIVWSessionEnd(sr->session_id, sr->sse_hints);
			sr->session_id = NULL;
		}
	}
}

int sr_start_listening_for_order(struct speech_rec *sr)
{
	int ret;
	const char*		session_id = NULL;
	int				errcode = MSP_SUCCESS;

	session_id = QISRSessionBegin(NULL, sr->session_begin_params, &errcode); //��д����Ҫ�﷨����һ������ΪNULL
	if (MSP_SUCCESS != errcode)
	{
		sr_dbg("\nQISRSessionBegin failed! error code:%d\n", errcode);
		return errcode;
	}
	sr->session_id = session_id;
	sr->state = SR_STATE_WAIT_ORDER;
	sr->ep_stat = MSP_EP_LOOKING_FOR_SPEECH;
	sr->rec_stat = MSP_REC_STATUS_SUCCESS;
	sr->audio_status = MSP_AUDIO_SAMPLE_FIRST;

	return 0;
}

/* after stop_record, there are still some data callbacks */
//static void wait_for_rec_stop(struct recorder *rec, unsigned int timeout_ms)
//{
//	while (!is_record_stopped(rec)) {
//		Sleep(1);
//		if (timeout_ms != (unsigned int)-1)
//			if (0 == timeout_ms--)
//				break;
//	}
//}
//
//int sr_stop_listening(struct speech_rec *sr)
//{
//	int ret = 0;
//	const char * rslt = NULL;
//
//	if (sr->state < SR_STATE_STARTED) {
//		sr_dbg("Not started or already stopped.\n");
//		return 0;
//	}
//
//	ret = stop_record(sr->recorder);
//#ifdef __FILE_SAVE_VERIFY__
//	safe_close_file();
//#endif
//	if (ret != 0) {
//		sr_dbg("Stop failed! \n");
//		return -E_SR_RECORDFAIL;
//	}
//	wait_for_rec_stop(sr->recorder, (unsigned int)-1);
//
//	sr->state = SR_STATE_INIT;
//	ret = QISRAudioWrite(sr->session_id, NULL, 0, MSP_AUDIO_SAMPLE_LAST, &sr->ep_stat, &sr->rec_stat);
//	if (ret != 0) {
//		sr_dbg("write LAST_SAMPLE failed: %d\n", ret);
//		QISRSessionEnd(sr->session_id, "write err");
//		return ret;
//	}
//
//	while (sr->rec_stat != MSP_REC_STATUS_COMPLETE) {
//		rslt = QISRGetResult(sr->session_id, &sr->rec_stat, 0, &ret);
//		if (MSP_SUCCESS != ret)	{
//			sr_dbg("\nQISRGetResult failed! error code: %d\n", ret);
//			end_sr_on_error(sr, ret);
//			return ret;
//		}
//		if (NULL != rslt && sr->notif.on_order_result)
//			sr->notif.on_order_result(rslt, sr->rec_stat == MSP_REC_STATUS_COMPLETE ? 1 : 0);
//		Sleep(100);
//	}
//
//	QISRSessionEnd(sr->session_id, "normal");
//	sr->session_id = NULL;
//	return 0;
//}

int sr_write_audio_data(struct speech_rec *sr, char *data, unsigned int len)
{
	const char *rslt = NULL;
	int ret = 0;
	if (!sr )
		return -E_SR_INVAL;
	if (!data || !len)
		return 0;

	if (sr->state == SR_STATE_WAIT_AWAKE_FIRST || sr->state == SR_STATE_WAIT_AWAKE_SECOND)
	{
		int audio_stat;
		if (sr->state == SR_STATE_WAIT_AWAKE_FIRST)
		{
			sr->state = SR_STATE_WAIT_AWAKE_SECOND;
			audio_stat = MSP_AUDIO_SAMPLE_FIRST;
		}
		else
		{
			audio_stat = MSP_AUDIO_SAMPLE_CONTINUE;
		}
		ret = QIVWAudioWrite(sr->session_id, data, len, audio_stat);
		if (MSP_SUCCESS != ret)
		{
			_snprintf(sr->sse_hints, sizeof(sr->sse_hints), "QIVWAudioWrite errorCode=%d", ret);
			end_sr_awake_on_error(sr, ret);
			return ret;
		}
	}
	else if (sr->state == SR_STATE_WAIT_ORDER)
	{
		ret = QISRAudioWrite(sr->session_id, data, len, sr->audio_status, &sr->ep_stat, &sr->rec_stat);
		if (ret) {
			end_sr_order_on_error(sr, ret);
			return ret;
		}
		sr->audio_status = MSP_AUDIO_SAMPLE_CONTINUE;

		if (MSP_EP_AFTER_SPEECH == sr->ep_stat){
			end_sr_order_on_vad(sr);
		}
	}
	
	return 0;
}

void sr_uninit(struct speech_rec * sr)
{
	if (sr->recorder) {
		if(!is_record_stopped(sr->recorder))
			stop_record(sr->recorder);
		close_recorder(sr->recorder);
		destroy_recorder(sr->recorder);
		sr->recorder = NULL;
	}

	if (sr->session_begin_params) {
		SR_MFREE(sr->session_begin_params);
		sr->session_begin_params = NULL;
	}
}