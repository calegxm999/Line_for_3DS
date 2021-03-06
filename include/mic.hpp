#pragma once

#define MIC_NUM_OF_MSG 4

bool Mic_query_init_flag(void);

bool Mic_query_running_flag(void);

void Mic_set_msg(int msg_num, std::string msg);

void Mic_resume(void);

void Mic_suspend(void);

void Mic_exit(void);

void Mic_init(void);

void Mic_main(void);
