#ifndef FFILE_H
#define FFILE_H

#include "base/base.h"
#include "ff15/source/ff.h"

status_t init_ffile();
bsp_handle_t ffopen(char* file, uint32_t io_mode);
status_t ffclose(bsp_handle_t handle);
status_t ffwrite(uint8_t data[], uint32_t size, bsp_handle_t handle);
status_t ffread(uint8_t data[], uint32_t size, bsp_handle_t handle);
uint32_t fftell(bsp_handle_t handle);
status_t ffseek(uint32_t size, bsp_handle_t handle);
status_t ffmkdir(char* dir_path);
status_t ffremove(char* file_path);
status_t ffpwd(char* dir_path, uint32_t size);
status_t ffdirremove(char* dir_path);
status_t ffchdir(char* dir_path);
bool ffexist(char* file_path);
bool ffdirexist(char* dir_path);
status_t ffscan(char* dir_path);
#endif // FFILE_H