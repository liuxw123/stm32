#include "user/ffile.h"
#include "user/logcat.h"
#ifdef WRITE_SYS_FONTS
#include "fonts/fonts.h"
#endif

#define LOG_TAG "ffile"

#define SD_DRIVER_NO DEV_SDIO_SDCARD
#define MAX_FILE_NAME FF_MAX_LFN

typedef struct {
    FIL ffile_;
} file_device_t;


static FATFS sFfDisk;

static char* __ff_error(FRESULT err) {
    switch (err) {
    case FR_OK: return "FR_OK";
    case FR_DISK_ERR: return "FR_DISK_ERR";
    case FR_INT_ERR: return "FR_INT_ERR";
    case FR_NOT_READY: return "FR_NOT_READY";
    case FR_NO_FILE: return "FR_NO_FILE";
    case FR_NO_PATH: return "FR_NO_PATH";
    case FR_INVALID_NAME: return "FR_INVALID_NAME";
    case FR_DENIED: return "FR_DENIED";
    case FR_EXIST: return "FR_EXIST";
    case FR_INVALID_OBJECT: return "FR_INVALID_OBJECT";
    case FR_WRITE_PROTECTED: return "FR_WRITE_PROTECTED";
    case FR_INVALID_DRIVE: return "FR_INVALID_DRIVE";
    case FR_NOT_ENABLED: return "FR_NOT_ENABLED";
    case FR_NO_FILESYSTEM: return "FR_NO_FILESYSTEM";
    case FR_MKFS_ABORTED: return "FR_MKFS_ABORTED";
    case FR_TIMEOUT: return "FR_TIMEOUT";
    case FR_LOCKED: return "FR_LOCKED";
    case FR_NOT_ENOUGH_CORE: return "FR_NOT_ENOUGH_CORE";
    case FR_TOO_MANY_OPEN_FILES: return "FR_TOO_MANY_OPEN_FILES";
    case FR_INVALID_PARAMETER: return "FR_INVALID_PARAMETER";
    }

    return "UNKNOW";
}

static uint8_t* __add_path_prefix(char* path, uint8_t* new_path) {
    uint32_t len = 0;
    if (!path) {
        len = snprintf(new_path, MAX_FILE_NAME, "%d:/", SD_DRIVER_NO);
    } else if (*path == (SD_DRIVER_NO + '0') && *(path + 1) == ':') {
        len = snprintf(new_path, MAX_FILE_NAME, "%s", path);
    } else {
        len = snprintf(new_path, MAX_FILE_NAME, "%d:%s", SD_DRIVER_NO, path);
    }
    new_path[len] = '\0';
    return new_path;
}

static uint8_t* __append_path(char* new, uint8_t* append_path) {
    uint32_t len = 0;
    uint8_t* temp = append_path;
    while (*temp++) len++;

    len += snprintf(append_path + len, MAX_FILE_NAME - len, "/%s", new);
    append_path[len] = '\0';

    return append_path;
}

status_t __ffmount() {
    uint8_t path[MAX_FILE_NAME];

    FRESULT res = f_mount(&sFfDisk, __add_path_prefix(null, path), 1 /*mount*/);
    if (res != FR_OK) {
        ALOGE("f_mount failed: %s, path: %s", __ff_error(res), path);
        return EHW;
    }

    res = f_chdrive(path);
    if (res != FR_OK) {
        ALOGW("f_chdrive failed: %s, return false", __ff_error(res));
        return EHW;
    }

    ALOGI("f_mount success. path: %s", path);
    return NO_ERROR;
}

status_t __ffumount() {
    uint8_t path[MAX_FILE_NAME];
    FRESULT res = f_unmount(__add_path_prefix(null, path));
    if (res != FR_OK) {
        ALOGE("f_unmount failed: %s, path: %s", __ff_error(res), path);
        return EHW;
    }

    ALOGI("f_unmount success. path: %s", path);
    return NO_ERROR;
}

status_t __ffvolumeinfo() {
    uint32_t fre_clust, fre_sect, tot_sect;
    FRESULT res;
    FATFS* fs;
    uint8_t path[MAX_FILE_NAME];

    res = f_getfree(__add_path_prefix(null, path), &fre_clust, &fs);
    if (res != FR_OK) {
        ALOGE("f_getfree failed: %s", __ff_error(res));
        return EHW;
    }

    tot_sect = (fs->n_fatent - 2) * fs->csize / 2;
    fre_sect = fre_clust * fs->csize / 2;

    uint32_t tot_mb = tot_sect / 1024;
    uint32_t fre_mb = fre_sect / 1024;

    if (fre_mb > 1024) {
        ALOGI("volume: %d.%d / %d.%d G", tot_mb / 1024, tot_mb % 1024, fre_mb / 1024, fre_mb % 1024);
    } else {
        ALOGI("volume: %d / %d M", tot_mb, fre_mb);
    }

    return NO_ERROR;
}

status_t __ffopen(char* file_path, uint32_t io_mode, bsp_handle_t* handle) {
    uint8_t temp_path[MAX_FILE_NAME];
    file_device_t* file_dev = (file_device_t*) calloc(sizeof(file_device_t), 1);
    if (!file_dev) return ENOMEM;

    FRESULT res = f_open(&(file_dev->ffile_), __add_path_prefix(file_path, temp_path), io_mode);
    if (res != FR_OK) {
        ALOGE("f_open failed: %s, path: %s, mode: %d", __ff_error(res), temp_path, io_mode);
        return EHW;
    }

    ALOGI("f_open success. path: %s, mode: %d", temp_path, io_mode);
    *handle = (bsp_handle_t) file_dev;
    return NO_ERROR;
}

status_t __ffclose(file_device_t* file_dev) {
    if (!file_dev) return EBADPARAM;
    FRESULT res = f_close(&(file_dev->ffile_));
    if (res != FR_OK) {
        ALOGE("f_close failed: %s", __ff_error(res));
        return EHW;
    }
    ALOGI("f_close success.");
    return NO_ERROR;
}

status_t __ffwrite(uint8_t data[], uint32_t size, file_device_t* file_dev) {
    if (!data || size <= 0 || !file_dev) return EBADPARAM;
    uint32_t write = 0;
    FRESULT res = f_write(&(file_dev->ffile_), data, size, &write);
    if (res != FR_OK) {
        ALOGE("f_write failed: %s", __ff_error(res));
        return EHW;
    }
    return (write == size) ? NO_ERROR : ENOMEM;
}

status_t __ffread(uint8_t data[], uint32_t size, file_device_t* file_dev) {
    if (!data || size <= 0 || !file_dev) return EBADPARAM;
    uint32_t read = 0;
    FRESULT res = f_read(&(file_dev->ffile_), data, size, &read);
    if (res != FR_OK) {
        ALOGE("f_read failed: %s", __ff_error(res));
        return EHW;
    }

    return (read == size) ? NO_ERROR : ENOMEM;
}

status_t __fftell(uint32_t* data, file_device_t* file_dev) {
    if (!file_dev || !data) return EBADPARAM;
    *data = f_tell(&(file_dev->ffile_));
    return NO_ERROR;
}

status_t __ffseek(uint32_t size, file_device_t* file_dev) {
    if (!file_dev) return EBADPARAM;
    FRESULT res = f_lseek(&(file_dev->ffile_), size);
    return (res == FR_OK) ? NO_ERROR : EHW;
}


status_t __ffmkdir(char* dir_path) {
    if (!dir_path) return EBADPARAM;
    uint8_t temp_path[MAX_FILE_NAME];
    FRESULT res = f_mkdir(__add_path_prefix(dir_path, temp_path));
    if (res != FR_OK) {
        ALOGE("f_mkdir failed: %s, path: %s", __ff_error(res), temp_path);
        return EHW;
    }

    ALOGI("f_mkdir success. path: %s", temp_path);
    return NO_ERROR;
}

status_t __ffremove(char* file_path) {
    if (!file_path) return NO_ERROR;
    uint8_t temp_path[MAX_FILE_NAME];
    FRESULT res = f_unlink(__add_path_prefix(file_path, temp_path));
    if (res != FR_OK) {
        ALOGE("f_unlink failed: %s, path: %s", __ff_error(res), temp_path);
        return EHW;
    }

    ALOGI("f_unlink success. path: %s", temp_path);
    return NO_ERROR;
}

status_t __ffpwd(char* dir_path, uint32_t size) {
    if (!dir_path || size < 0) return EBADPARAM;
    FRESULT res = f_getcwd(dir_path, size);
    if (res != FR_OK) {
        ALOGE("f_getcwd failed: %s", __ff_error(res));
        return EHW;
    }

    ALOGI("__ffpwd success. path: %s", dir_path);
    return NO_ERROR;
}

status_t __ffdirremove(char* dir_path) {
    FRESULT res;
    DIR dir;
    FILINFO fno;
    uint8_t temp_path[MAX_FILE_NAME];
    status_t ret = NO_ERROR;

    res = f_opendir(&dir, __add_path_prefix(dir_path, temp_path)); /* Open the directory */
    if (res == FR_OK) {
        for (;;) {
            res = f_readdir(&dir, &fno); /* Read a directory item */
            if (res != FR_OK || fno.fname[0] == 0) {
                if (res != FR_OK) ret = EHW;
                break; /* Error or end of dir */
            }
            if (fno.fattrib & AM_DIR) { /* Directory */
                ret = __ffdirremove(__append_path(fno.fname, temp_path));
                if (ret) {
                    ALOGE("__ffdirremove failed. path: %s", temp_path);
                    break;
                }
            } else { /* File */
                res = f_unlink(__append_path(fno.fname, temp_path));
                if (res != FR_OK) {
                    ALOGE("f_unlink failed: %s, path: %s", __ff_error(res), temp_path);
                    ret = EHW;
                    break;
                }
            }
        }
        f_closedir(&dir);
    } else {
        ALOGE("f_opendir: %s, path: %s", __ff_error(res), temp_path);
        return EHW;
    }

    if (ret == NO_ERROR) {
        res = f_unlink(__add_path_prefix(dir_path, temp_path));
        if (res != FR_OK) {
            ALOGE("f_unlink failed: %s, path: %s", __ff_error(res), temp_path);
            ret = EHW;
        }
    }

    return ret;
}

status_t __ffchdir(char* dir) {
    uint8_t temp_path[MAX_FILE_NAME];
    FRESULT res = f_chdir(__add_path_prefix(dir, temp_path));
    if (res != FR_OK) {
        ALOGE("f_chdir failed: %s, path: %s", __ff_error(res), temp_path);
        return EHW;
    }

    ALOGI("f_chdir success. path: %s", temp_path);
    return NO_ERROR;
}

bool __ffexist(char* file_path) {
    uint8_t temp_path[MAX_FILE_NAME];
    FIL temp_file;
    FRESULT res = f_open(&temp_file, __add_path_prefix(file_path, temp_path), FA_OPEN_EXISTING);
    if (res != FR_OK) {
        ALOGW("f_open failed: %s, path: %s, return false", __ff_error(res), temp_path);
        return false;
    }

    res = f_close(&temp_file);
    if (res != FR_OK) {
        ALOGW("f_close failed: %s", __ff_error(res));
    }
    return true;
}

bool __ffdirexist(char* dir) {
    uint8_t curr[MAX_FILE_NAME];
    uint8_t temp_path[MAX_FILE_NAME];
    FRESULT res = f_getcwd(curr, MAX_FILE_NAME);
    if (res != FR_OK) {
        ALOGW("f_getcwd failed: %s, return false", __ff_error(res));
        return false;
    }

    res = f_chdir(__add_path_prefix(dir, temp_path));
    if (res != FR_OK) {
        ALOGW("f_chdir failed: %s, path: %s, return false", __ff_error(res), temp_path);
        return false;
    }

    res = f_chdir(curr);
    if (res != FR_OK) {
        ALOGW("f_chdir failed: %s, path: %s", __ff_error(res), curr);
    }

    ALOGI("path: %s, exists.", temp_path);
    return true;
}

status_t __ffscan(char* dir_path) {
    FRESULT res;
    DIR dir;
    FILINFO fno;
    uint8_t temp_path[MAX_FILE_NAME];
    status_t ret = NO_ERROR;

    res = f_opendir(&dir, __add_path_prefix(dir_path, temp_path));
    if (res == FR_OK) {
        for (;;) {
            res = f_readdir(&dir, &fno);
            if (res != FR_OK || fno.fname[0] == 0) {
                if (res != FR_OK) ret = EHW;
                break;
            }
            ALOGI("%s: %s", (fno.fattrib & AM_DIR) ? "dir" : "file", __append_path(fno.fname, temp_path));
        }
        f_closedir(&dir);
    } else {
        ALOGE("f_opendir: %s, path: %s", __ff_error(res), temp_path);
        return EHW;
    }

    return ret;
}

#ifdef WRITE_SYS_FONTS
static status_t __write_system_fonts() {
    char font_dir[] = "/sys/fonts";
    char font_path[MAX_FILE_NAME];
    status_t ret = NO_ERROR;

    if (!ffdirexist(font_dir)) {
        ret = ffmkdir(font_dir);
        if (ret) return ret;
    }

    sFONT* fonts[] = {
        &Font8,
        &Font12,
        &Font16,
        &Font20,
        &Font24,
    };

    char paths[5][15] = {"font8.bin", "font12.bin", "font16.bin", "font20.bin", "font24.bin"};
    uint32_t header_size = sizeof(Font8) - sizeof(Font8.table);

    for (uint32_t i = 0; i < ARRAY_SIZE(fonts); i++) {
        memcpy(font_path, font_dir, strlen(font_dir));
        font_path[strlen(font_dir)] = '\0';
        if (!ffexist(__append_path(paths[i], font_path))) {
            sFONT* font = fonts[i];
            bsp_handle_t handle = null_handle;
            handle = ffopen(font_path, FA_CREATE_NEW | FA_WRITE);
            if (!handle) return EHW;
            ret = ffwrite((uint8_t*) font, header_size, handle);
            if (ret) return ret;
            ret = ffwrite(font->table, font->size, handle);
            if (ret) return ret;
            ret = ffclose(handle);
            if (ret) return ret;

            ALOGI("%s created", font_path);
        } else {
            ALOGI("%s already exists", font_path);
        }
    }
}
#endif

static status_t __create_sys_dir() {
    uint8_t path[MAX_FILE_NAME];
    status_t ret = NO_ERROR;

    char sys_path[6][10] = {"sys", "dev", "usr", "home", "opt", "etc"};

    for (uint32_t i = 0; i < ARRAY_SIZE(sys_path); i++) {
        memset(path, 0, MAX_FILE_NAME);
        if (!ffdirexist(__append_path(sys_path[i], path))) {
            ret = ffmkdir(path);
            if (ret) return ret;
            ALOGI("%s created.", sys_path[i]);
        } else {
            ALOGI("%s already exists.", sys_path[i]);
        }
    }

    return NO_ERROR;
}

status_t init_ffile() {
    char test_dir[] = "temp_test";
    char test_file[] = "bootup_temp_test";
    char test_write_string[16] = "Hello Liuxiaowu";
    char test_read_string[16];
    status_t ret = NO_ERROR;
    bsp_handle_t test_file_handle = null_handle;

    char get_dir[32];

    ret = __ffmount();
    if (ret) {
        return ret;
    }

    ret = __ffvolumeinfo();
    if (ret) {
        return ret;
    }

    bool direxist = ffdirexist(test_dir);
    if (direxist) {
        ret = ffdirremove(test_dir);
        if (ret) {
            return ret;
        }
    }

    ret = ffmkdir(test_dir);
    if (ret) {
        return ret;
    }

    ret = ffchdir(test_dir);
    if (ret) {
        return ret;
    }

    ret = ffpwd(get_dir, 32);
    if (ret) {
        return ret;
    }

    ALOGD("current dir: %s", get_dir);

    bool file_exist = ffexist(test_file);
    ALOGD("%s exist: %s", test_file, file_exist ? "yes" : "no");
    if (file_exist) {
        ret = ffremove(test_file);
        if (ret) {
            return ret;
        }
    }

    test_file_handle = ffopen(test_file, FA_WRITE | FA_CREATE_NEW);
    if (!test_file_handle) {
        return EHW;
    }

    ret = ffwrite(test_write_string, ARRAY_SIZE(test_write_string) - 1, test_file_handle);
    if (ret) {
        return ret;
    }

    ret = ffclose(test_file_handle);
    if (ret) {
        return ret;
    }

    test_file_handle = ffopen(test_file, FA_READ);
    if (!test_file_handle) {
        return EHW;
    }

    ret = ffread(test_read_string, ARRAY_SIZE(test_write_string) - 1, test_file_handle);
    if (ret) {
        return ret;
    }

    ret = ffclose(test_file_handle);
    if (ret) {
        return ret;
    }

    if (0 == strncmp(test_write_string, test_read_string, 15)) {
        ALOGI("RW sdcard test success.");
    } else {
        ALOGE("RW sdcard test failed. expected: %s, but got: %s", test_write_string, test_read_string);
        return EHW;
    }

    ret = ffchdir(null);
    if (ret) {
        return ret;
    }

    ret = ffscan(test_dir);
    if (ret) {
        return ret;
    }

    ret = ffdirremove(test_dir);
    if (ret) {
        return ret;
    }
    ALOGI("[SUCCESS] ffile inited.");

    ret = __create_sys_dir();
    if (ret) {
        return ret;
    }

#ifdef WRITE_SYS_FONTS
    ret = __write_system_fonts();
    if (ret) {
        return ret;
    }
#endif
    return NO_ERROR;
}

bsp_handle_t ffopen(char* file, uint32_t io_mode) {
    bsp_handle_t handle = null_handle;
    status_t ret = __ffopen(file, io_mode, &handle);
    return (ret == NO_ERROR) ? handle : null_handle;
}

status_t ffclose(bsp_handle_t handle) {
    status_t ret = __ffclose((file_device_t*) handle);
    if (ret == NO_ERROR) {
        free((file_device_t*) handle);
        return NO_ERROR;
    }

    return ret;
}

status_t ffwrite(uint8_t data[], uint32_t size, bsp_handle_t handle) {
    return __ffwrite(data, size, (file_device_t*) handle);
}

status_t ffread(uint8_t data[], uint32_t size, bsp_handle_t handle) {
    return __ffread(data, size, (file_device_t*) handle);
}

uint32_t fftell(bsp_handle_t handle) {
    uint32_t data = 0;
    status_t ret = __fftell(&data, (file_device_t*) handle);
    return (ret == NO_ERROR) ? data : 0;
}

status_t ffseek(uint32_t size, bsp_handle_t handle) {
    return __ffseek(size, (file_device_t*) handle);
}

status_t ffmkdir(char* dir_path) {
    return __ffmkdir(dir_path);
}

status_t ffremove(char* file_path) {
    return __ffremove(file_path);
}

status_t ffpwd(char* dir_path, uint32_t size) {
    return __ffpwd(dir_path, size);
}

status_t ffdirremove(char* dir_path) {
    return __ffdirremove(dir_path);
}

status_t ffchdir(char* dir_path) {
    return __ffchdir(dir_path);
}

bool ffexist(char* file_path) {
    return __ffexist(file_path);
}

bool ffdirexist(char* dir_path) {
    return __ffdirexist(dir_path);
}

status_t ffscan(char* dir_path) {
    return __ffscan(dir_path);
}
