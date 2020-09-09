#include <3ds.h>
#include <unistd.h>
#include <string>
#include <malloc.h>
#include "citro2d.h"

#include "hid.hpp"
#include "draw.hpp"
#include "file.hpp"
#include "log.hpp"
#include "error.hpp"
#include "menu.hpp"
#include "types.hpp"
#include "setting_menu.hpp"
#include "mic.hpp"

/*For draw*/
bool mic_need_reflesh = false;
bool mic_pre_start_record_request = false;
/*---------------------------------------------*/

bool mic_main_run = false;
bool mic_record_thread_run = false;
bool mic_already_init = false;
bool mic_thread_suspend = true;
bool mic_start_record_request = false;
bool mic_stop_record_request = false;
u8* mic_buffer;
u32 mic_buffer_size = 0x300000;
std::string mic_msg[MIC_NUM_OF_MSG];
std::string mic_ver = "v1.0.2";
std::string mic_record_thread_string = "Mic/Record thread";
std::string mic_init_string = "Mic/Init";
std::string mic_exit_string = "Mic/Exit";
Thread mic_record_thread;

bool Mic_query_init_flag(void)
{
	return mic_already_init;
}

bool Mic_query_running_flag(void)
{
	return mic_main_run;
}

bool Mic_query_operation_flag(int operation_num)
{
	if (operation_num == MIC_START_RECORDING_REQUEST)
		return mic_start_record_request;
	else if (operation_num == MIC_STOP_RECORDING_REQUEST)
		return mic_stop_record_request;
	else
		return false;
}

void Mic_set_msg(int msg_num, std::string msg)
{
	if (msg_num >= 0 && msg_num <= MIC_NUM_OF_MSG)
		mic_msg[msg_num] = msg;
}

void Mic_set_operation_flag(int operation_num, bool flag)
{
	if (operation_num == MIC_START_RECORDING_REQUEST)
		mic_start_record_request = flag;
	else if (operation_num == MIC_STOP_RECORDING_REQUEST)
		mic_stop_record_request = flag;
}

void Mic_resume(void)
{
	mic_thread_suspend = false;
	mic_main_run = true;
	mic_need_reflesh = true;
	Menu_suspend();
}

void Mic_suspend(void)
{
	mic_thread_suspend = true;
	mic_main_run = false;
	Menu_resume();
}

void Mic_record_thread(void* arg)
{
	Log_log_save(mic_record_thread_string, "Thread started.", 1234567890, false);
	int log_num;
	int* chunk_size = new int (0);
	char riff[5] = "RIFF";
	char wave[5] = "WAVE";
	char fmt[5] = "fmt ";
	char data[5] = "data";
	std::string file_name;
	u8* header;
	u8* fs_buffer;
	u32 buffer_pos;
	u32 buffer_offset;
	u32 data_size;
	FS_Archive fs_archive = 0;
	Handle fs_handle = 0;
	Result_with_string result;

	while (mic_record_thread_run)
	{
		if (mic_start_record_request)
		{
			*chunk_size = 0;
			header = (u8*)malloc(44);
			fs_buffer = (u8*)malloc(mic_buffer_size);
			if (fs_buffer == NULL)
			{
				Err_set_error_message("[Error] Out of memory.", "Couldn't allocate memory.", mic_record_thread_string, OUT_OF_MEMORY);
				Err_set_error_show_flag(true);
				Log_log_save(mic_record_thread_string, "[Error] Out of memory.", OUT_OF_MEMORY, false);
			}
			else
			{
				memset(header, 0x0, 44);
				memset(fs_buffer, 0x0, mic_buffer_size);
				buffer_offset = 0;
				data_size = micGetSampleDataSize();
				log_num = Log_log_save(mic_record_thread_string, "MICU_StartSampling()...", 1234567890, false);
				result.code = MICU_StartSampling(MICU_ENCODING_PCM16_SIGNED, MICU_SAMPLE_RATE_16360, 0, data_size, true);
				Log_log_add(log_num, "", result.code, false);

				while (true)
				{
					usleep(10000);

					buffer_pos = micGetLastSampleOffset();
					if (buffer_offset + (buffer_pos - buffer_offset) < mic_buffer_size)
					{
						memcpy((void*)(fs_buffer + buffer_offset), (void*)(mic_buffer + buffer_offset), (buffer_pos - buffer_offset));
						buffer_offset += (buffer_pos - buffer_offset);
					}
					else
						mic_stop_record_request = true;

					if (mic_stop_record_request)
					{
						log_num = Log_log_save(mic_record_thread_string, "MICU_StopSampling()...", 1234567890, false);
						result.code = MICU_StopSampling();
						Log_log_add(log_num, "", result.code, false);

						*chunk_size = (int)buffer_offset + 36;
						memcpy((void*)header, (void*)riff, 0x4);
						memcpy((void*)(header + 4), (void*)(chunk_size), 0x4);
						memcpy((void*)(header + 8), (void*)wave, 0x4);
						memcpy((void*)(header + 12), (void*)fmt, 0x4);
						memset((void*)(header + 16), 0x10, 0x1); //Subchunk1Size 16 for PCM
						memset((void*)(header + 20), 0x1, 0x1); //AudioFormat PCM = 1
						memset((void*)(header + 22), 0x1, 0x1); //NumChannels Mono = 1, Stereo = 2
						memset((void*)(header + 24), 0xE8, 0x1); //SampleRate
						memset((void*)(header + 25), 0x3F, 0x1); //SampleRate
						memset((void*)(header + 28), 0xD0, 0x1); //ByteRate == SampleRate * NumChannels * BitsPerSample/8
						memset((void*)(header + 29), 0x7F, 0x1); //ByteRate == SampleRate * NumChannels * BitsPerSample/8
						memset((void*)(header + 32), 0x2, 0x1); //BlockAlign == NumChannels * BitsPerSample/8
						memset((void*)(header + 34), 0x10, 0x1); //BitsPerSample 8 bits = 8, 16 bits = 16
						memcpy((void*)(header + 36), (void*)data, 0x4);
						*chunk_size = (int)buffer_offset;
						memcpy((void*)(header + 40), (void*)chunk_size, 0x4);

						file_name = Menu_query_time(2) + ".wav";
						log_num = Log_log_save(mic_record_thread_string, "File_save_to_file()...", 1234567890, false);
						result = File_save_to_file(file_name, (u8*)header, 44, "/Line/sound/", true, fs_handle, fs_archive);
						Log_log_add(log_num, "", result.code, false);

						log_num = Log_log_save(mic_record_thread_string, "File_save_to_file()...", 1234567890, false);
						result = File_save_to_file(file_name, (u8*)fs_buffer, buffer_offset, "/Line/sound/", false, fs_handle, fs_archive);
						Log_log_add(log_num, "", result.code, false);

						break;
					}
				}
			}

			free(header);
			free(fs_buffer);
			header = NULL;
			fs_buffer = NULL;
			mic_start_record_request = false;
			mic_stop_record_request = false;
		}
		else
			usleep(ACTIW_THREAD_SLEEP_TIME);

		while (mic_thread_suspend)
			usleep(INACTIW_THREAD_SLEEP_TIME);
	}
	Log_log_save(mic_record_thread_string, "Thread exit.", 1234567890, false);
	threadExit(0);
}

void Mic_exit(void)
{
	Log_log_save(mic_exit_string, "Exiting...", 1234567890, DEBUG);
	u64 time_out = 10000000000;
	int log_num;
	bool failed = false;
	Result_with_string result;

	mic_already_init = false;
	mic_thread_suspend = false;
	mic_record_thread_run = false;

	Draw_progress("[Mic] Exiting...");
	log_num = Log_log_save(mic_exit_string, "threadJoin()0/0...", 1234567890, DEBUG);
	result.code = threadJoin(mic_record_thread, time_out);
	if (result.code == 0)
		Log_log_add(log_num, Err_query_template_summary(0), result.code, DEBUG);
	else
	{
		failed = true;
		Log_log_add(log_num, Err_query_template_summary(-1024), result.code, DEBUG);
	}

	threadFree(mic_record_thread);

	MICU_SetPower(false);
	micExit();
	free(mic_buffer);

	if (failed)
		Log_log_save(mic_exit_string, "[Warn] Some function returned error.", 1234567890, DEBUG);
}

void Mic_init(void)
{
	Log_log_save(mic_init_string, "Initializing...", 1234567890, DEBUG);
	bool failed = false;
	int log_num;
	Result_with_string result;

	mic_buffer = (u8*)memalign(0x1000, mic_buffer_size);
	if (mic_buffer == NULL)
	{
		Err_set_error_message("Out of memory.", "Couldn't allocate memory.", mic_init_string, OUT_OF_MEMORY);
		Err_set_error_show_flag(true);
		Log_log_save(mic_init_string, "[Error] Out of memory. ", OUT_OF_MEMORY, false);
		failed = true;
	}

	Draw_progress("0/1 [Mic] Initializing mic...");
	if (!failed)
	{
		log_num = Log_log_save(mic_init_string, "micInit()...", 1234567890, DEBUG);
		result.code = micInit(mic_buffer, mic_buffer_size);
		Log_log_add(log_num, result.string, result.code, DEBUG);
		if (result.code == 0)
			Log_log_add(log_num, Err_query_template_summary(0), result.code, DEBUG);
		else
		{
			failed = true;
			Err_set_error_message("micInit() failed.", "", mic_init_string, result.code);
			Err_set_error_show_flag(true);
			Log_log_add(log_num, Err_query_template_summary(-1024), result.code, DEBUG);
		}
	}

	Draw_progress("1/1 [Mic] Starting threads...");
	if (!failed)
	{
		mic_record_thread_run = true;
		mic_record_thread = threadCreate(Mic_record_thread, (void*)(""), STACKSIZE, PRIORITY_NORMAL, -1, false);
	}

	Mic_resume();
	mic_already_init = true;
	Log_log_save(mic_init_string, "Initialized", 1234567890, DEBUG);
}

void Mic_main(void)
{
	float text_red;
	float text_green;
	float text_blue;
	float text_alpha;
	double draw_x;
	double draw_y;

	if (Sem_query_settings(SEM_NIGHT_MODE))
	{
		text_red = 1.0;
		text_green = 1.0;
		text_blue = 1.0;
		text_alpha = 0.75;
		white_or_black_tint = white_tint;
	}
	else
	{
		text_red = 0.0;
		text_green = 0.0;
		text_blue = 0.0;
		text_alpha = 1.0;
		white_or_black_tint = black_tint;
	}

	if(mic_pre_start_record_request != mic_start_record_request)
	{
		mic_pre_start_record_request = mic_start_record_request;
		mic_need_reflesh = true;
	}

	if(Draw_query_need_reflesh() || !Sem_query_settings(SEM_ECO_MODE))
		mic_need_reflesh = true;

	if(mic_need_reflesh)
	{
		Draw_set_draw_mode(Sem_query_settings(SEM_VSYNC_MODE));
		if (Sem_query_settings(SEM_NIGHT_MODE))
			Draw_screen_ready_to_draw(0, true, 2, 0.0, 0.0, 0.0);
		else
			Draw_screen_ready_to_draw(0, true, 2, 1.0, 1.0, 1.0);

		Draw_top_ui();

		if (Sem_query_settings(SEM_NIGHT_MODE))
			Draw_screen_ready_to_draw(1, true, 2, 0.0, 0.0, 0.0);
		else
			Draw_screen_ready_to_draw(1, true, 2, 1.0, 1.0, 1.0);

		Draw(mic_ver, 0, 0.0, 0.0, 0.4, 0.4, 0.0, 1.0, 0.0, 1.0);
		Draw(mic_msg[3], 0, 45.0, 50.0, 0.5, 0.5, 1.0, 0.0, 0.0, 1.0);
		if(mic_start_record_request)
			Draw(mic_msg[2], 0, 95.0, 65.0, 0.5, 0.5, text_red, text_green, text_blue, text_alpha);

		draw_x = 95.0;
		draw_y = 80.0;
		for (int i = 0; i < 2; i++)
		{
			Draw_texture(Square_image, weak_aqua_tint, 0, draw_x, draw_y, 60.0, 60.0);
			Draw(mic_msg[i], 0, (draw_x + 10.0), draw_y + 20.0, 0.4, 0.4, text_red, text_green, text_blue, text_alpha);

			draw_x += 70.0;
		}
		Draw_bot_ui();
		Draw_touch_pos();

		Draw_apply_draw();
		mic_need_reflesh = false;
	}
	else
		gspWaitForVBlank();
}
