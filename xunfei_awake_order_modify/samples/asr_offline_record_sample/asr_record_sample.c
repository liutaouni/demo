/*
* ������д(iFly Auto Transform)�����ܹ�ʵʱ�ؽ�����ת���ɶ�Ӧ�����֡�
*/

#include <stdlib.h>
#include <stdio.h>
#include <windows.h>
#include <conio.h>
#include <errno.h>
#include <process.h>

#include "../../include/qisr.h"
#include "../../include/msp_cmn.h"
#include "../../include/msp_errors.h"
#include "./include/speech_recognizer.h"

#define	BUFFER_SIZE	4096

enum{
	EVT_AWAKE_SUCC = 0,
	EVT_AWAKE_FAIL,
	EVT_ORDER_SUCC,
	EVT_ORDER_FAIL,
	EVT_ERROR,
	EVT_TOTAL
};
static HANDLE events[EVT_TOTAL] = { NULL, NULL, NULL, NULL, NULL };

static COORD begin_pos = {0, 0};
static COORD last_pos = {0, 0};

static char *g_result = NULL;
static unsigned int g_buffersize = BUFFER_SIZE;

#define SAMPLE_RATE_16K     (16000)
#define SAMPLE_RATE_8K      (8000)
#define MAX_GRAMMARID_LEN   (32)
#define MAX_PARAMS_LEN      (1024)

const char * ASR_RES_PATH        = "fo|res/asr/common.jet";  //�����﷨ʶ����Դ·��
#ifdef _WIN64
const char * GRM_BUILD_PATH      = "res/asr/GrmBuilld_x64";  //���������﷨ʶ�������������ݱ���·��
#else
const char * GRM_BUILD_PATH      = "res/asr/GrmBuilld";  //���������﷨ʶ�������������ݱ���·��
#endif
const char * GRM_FILE            = "operate.bnf"; //��������ʶ���﷨�������õ��﷨�ļ�

typedef struct _UserData {
	int     build_fini;  //��ʶ�﷨�����Ƿ����
	int     errcode; //��¼�﷨�����ص�������
	char    grammar_id[MAX_GRAMMARID_LEN]; //�����﷨�������ص��﷨ID
}UserData;

int build_grammar(UserData *udata); //��������ʶ���﷨����
void run_asr(UserData *udata); //���������﷨ʶ��

int build_grm_cb(int ecode, const char *info, void *udata)
{
	UserData *grm_data = (UserData *)udata;

	if (NULL != grm_data) {
		grm_data->build_fini = 1;
		grm_data->errcode = ecode;
	}

	if (MSP_SUCCESS == ecode && NULL != info) {
		printf("�����﷨�ɹ��� �﷨ID:%s\n", info);
		if (NULL != grm_data){
			_snprintf(grm_data->grammar_id, MAX_GRAMMARID_LEN - 1, info);
		}
	}
	else{
		printf("�����﷨ʧ�ܣ�%d\n", ecode);
	}

	return 0;
}

int build_grammar(UserData *udata)
{
	FILE *grm_file = NULL;
	char *grm_content = NULL;
	unsigned int grm_cnt_len = 0;
	char grm_build_params[MAX_PARAMS_LEN] = {NULL};
	int ret = 0;

	grm_file = fopen(GRM_FILE, "rb");	
	if(NULL == grm_file) {
		printf("��\"%s\"�ļ�ʧ�ܣ�[%s]\n", GRM_FILE, strerror(errno));
		return -1; 
	}

	fseek(grm_file, 0, SEEK_END);
	grm_cnt_len = ftell(grm_file);
	fseek(grm_file, 0, SEEK_SET);

	grm_content = (char *)malloc(grm_cnt_len + 1);
	if (NULL == grm_content)
	{
		printf("�ڴ����ʧ��!\n");
		fclose(grm_file);
		grm_file = NULL;
		return -1;
	}
	fread((void*)grm_content, 1, grm_cnt_len, grm_file);
	grm_content[grm_cnt_len] = '\0';
	fclose(grm_file);
	grm_file = NULL;

	_snprintf(grm_build_params, MAX_PARAMS_LEN - 1, 
		"engine_type = local, \
		asr_res_path = %s, sample_rate = %d, \
		grm_build_path = %s, ",
		ASR_RES_PATH,
		SAMPLE_RATE_16K,
		GRM_BUILD_PATH
		);
	ret = QISRBuildGrammar("bnf", grm_content, grm_cnt_len, grm_build_params, build_grm_cb, udata);

	free(grm_content);
	grm_content = NULL;

	return ret;
}

void show_result(char *string, char is_over)
{
	COORD orig, current;
	CONSOLE_SCREEN_BUFFER_INFO info;
	HANDLE w = GetStdHandle(STD_OUTPUT_HANDLE);
	GetConsoleScreenBufferInfo(w, &info);
	current = info.dwCursorPosition;

	if(current.X == last_pos.X && current.Y == last_pos.Y ) {
		SetConsoleCursorPosition(w, begin_pos);
	} else {
		/* changed by other routines, use the new pos as start */
		begin_pos = current;
	}
	if(is_over)
		SetConsoleTextAttribute(w, FOREGROUND_GREEN);
	printf("Result: [ %s ]\n", string);
	if(is_over)
		SetConsoleTextAttribute(w, info.wAttributes);

	GetConsoleScreenBufferInfo(w, &info);
	last_pos = info.dwCursorPosition;
}

int on_awake_result(const char *sessionID, int msg, int param1, int param2, const void *info, void *userData)
{
	if (MSP_IVW_MSG_ERROR == msg) //���ѳ�����Ϣ
	{
		printf("\n\nMSP_IVW_MSG_ERROR errCode = %d\n\n", param1);
		SetEvent(events[EVT_AWAKE_FAIL]);
	}
	else if (MSP_IVW_MSG_WAKEUP == msg) //���ѳɹ���Ϣ
	{
		printf("\n\nMSP_IVW_MSG_WAKEUP result = %s %s\n\n", info, userData);
		SetEvent(events[EVT_AWAKE_SUCC]);
	}
	return 0;
}
void on_order_result(const char *result, char is_last)
{
	if (result) 
	{
		if (g_result)
		{
			free(g_result);
		}
		g_result = (char*)malloc(BUFFER_SIZE);
		g_buffersize = BUFFER_SIZE;
		memset(g_result, 0, g_buffersize);

		size_t left = g_buffersize - 1 - strlen(g_result);
		size_t size = strlen(result);
		if (left < size) {
			g_result = (char*)realloc(g_result, g_buffersize + BUFFER_SIZE);
			if (g_result)
				g_buffersize += BUFFER_SIZE;
			else {
				printf("mem alloc failed\n");
				return;
			}
		}
		strncat(g_result, result, size);
		show_result(g_result, is_last);
		SetEvent(events[EVT_ORDER_SUCC]);
	}
	else
	{
		SetEvent(events[EVT_ORDER_FAIL]);
	}
}
void on_error(int reason)
{
	if (reason == END_REASON_VAD_DETECT)
	{
		printf("\nSpeaking done \n");
	}
	else
	{
		printf("\nRecognizer error %d\n", reason);
	}
	SetEvent(events[EVT_ERROR]);
}

void run_asr(UserData *udata)
{
	int errcode;
	int i = 0;

	struct speech_rec asr;
	DWORD waitres;

	//�����﷨ʶ���������
	char session_begin_params[MAX_PARAMS_LEN] = { NULL };
	_snprintf(session_begin_params, MAX_PARAMS_LEN - 1,
								"engine_type = local, \
								asr_res_path = %s, sample_rate = %d, \
								grm_build_path = %s, local_grammar = %s, \
								result_type = xml, result_encoding = GB2312, ",
								ASR_RES_PATH,
								SAMPLE_RATE_16K,
								GRM_BUILD_PATH,
								udata->grammar_id
								);

	struct speech_rec_notifier recnotifier = {
		on_awake_result,
		on_order_result,
		on_error
	};

	for (i = 0; i < EVT_TOTAL; ++i) {
		events[i] = CreateEvent(NULL, FALSE, FALSE, NULL);
	}

	printf("����ʶ�� ��ʼ��...\n");
	errcode = sr_init(&asr, session_begin_params, DEFAULT_INPUT_DEVID, &recnotifier);
	if (errcode) {
		printf("����ʶ�� ��ʼ��ʧ��\n");
		return;
	}

	printf("��ʼ����...\n");
	errcode = sr_start_listening(&asr);
	if (errcode) {
		printf("��ʼ���� ʧ�ܣ�%d\n", errcode);
		goto exit;
	}

	while (1){
		printf("�ȴ�����...\n");
		if (errcode = sr_start_listening_for_awake(&asr)) {
			printf("�ȴ����� ʧ�ܣ�%d\n", errcode);
			goto exit;
		}

		waitres = WaitForMultipleObjects(EVT_TOTAL, events, FALSE, INFINITE);
		switch (waitres) {
		case WAIT_OBJECT_0 + EVT_AWAKE_SUCC:
			sr_stop_listening_for_awake(&asr);
			printf("�ȴ�ָ��...\n");
			if (errcode = sr_start_listening_for_order(&asr)) {
				printf("�ȴ�ָ�� ʧ�ܣ�%d\n", errcode);
				goto exit;
			}
			break;
		case WAIT_OBJECT_0 + EVT_AWAKE_FAIL:
			sr_stop_listening_for_awake(&asr);
			printf("���� ʧ��\n");
			continue;
			break;
		case WAIT_OBJECT_0 + EVT_ERROR:
			sr_stop_listening_for_awake(&asr);
			printf("���� ʧ��\n");
			continue;
			break;
		default:
			printf("Why it happened !?\n");
			goto exit;
			break;
		}

		int periodCnout = 10;
		while (1)
		{
			waitres = WaitForMultipleObjects(EVT_TOTAL, events, FALSE, INFINITE);
			if (waitres == WAIT_OBJECT_0 + EVT_ORDER_SUCC)
			{
				periodCnout = 10;
				printf("�ȴ�ָ��...%d\n", periodCnout);
				if (errcode = sr_start_listening_for_order(&asr)) {
					printf("�ȴ�ָ�� ʧ�ܣ�%d\n", errcode);
					goto exit;
				}
			}
			else if ((waitres == WAIT_OBJECT_0 + EVT_ORDER_FAIL) || (waitres == WAIT_OBJECT_0 + EVT_ERROR))
			{
				if (periodCnout > 0)
				{
					--periodCnout;
					printf("�ȴ�ָ��...%d\n", periodCnout);
					if (errcode = sr_start_listening_for_order(&asr)) {
						printf("�ȴ�ָ�� ʧ�ܣ�%d\n", errcode);
						goto exit;
					}
				}
				else
				{
					break;
				}
			}
			else
			{
				printf("Why it happened !?\n");
				goto exit;
			}
		}

		_sleep(300);
	}

exit:
	for (i = 0; i < EVT_TOTAL; ++i) {
		if (events[i])
			CloseHandle(events[i]);
	}

	sr_uninit(&asr);
}

/* main thread: wait for awake / wait for order.
 * record thread: record callback(data write)
 */
int main(int argc, char* argv[])
{
	const char *login_config = "appid = 5c9dc09d"; //��¼����
	UserData asr_data; 
	int ret = 0 ;

	printf("��¼...\n");
	ret = MSPLogin(NULL, NULL, login_config); //��һ������Ϊ�û������ڶ�������Ϊ���룬��NULL���ɣ������������ǵ�¼����
	if (MSP_SUCCESS != ret) {
		printf("��¼ʧ�ܣ�%d\n", ret);
		goto exit;
	}

	printf("��������ʶ���﷨����...\n");
	memset(&asr_data, 0, sizeof(UserData));
	ret = build_grammar(&asr_data);  //��һ��ʹ��ĳ�﷨����ʶ����Ҫ�ȹ����﷨���磬��ȡ�﷨ID��֮��ʹ�ô��﷨����ʶ�������ٴι���
	if (MSP_SUCCESS != ret) {
		printf("�����﷨����ʧ�ܣ�\n");
		goto exit;
	}
	while (1 != asr_data.build_fini){
		_sleep(300);
	}
	if (MSP_SUCCESS != asr_data.errcode){
		printf("�����﷨ʧ��:%d\n", asr_data.errcode);
		goto exit;
	}
		
	run_asr(&asr_data);

exit:
	MSPLogout();
	printf("�밴������˳�...\n");
	_getch();
	return 0;
}
