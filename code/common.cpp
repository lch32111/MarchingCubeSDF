#include "common.h"

#if _WIN32 || _WIN64
#include <Windows.h>
#endif

#if _WIN32 || _WIN64
void str_widen(const char* str, int strLenWithNULL, wchar_t* buffer, int bufferByteSize)
{
	int targetWideCharCount = MultiByteToWideChar(CP_UTF8, 0, str, strLenWithNULL, NULL, 0);
	assert(sizeof(wchar_t) * targetWideCharCount <= bufferByteSize);

	MultiByteToWideChar(CP_UTF8, 0, str, strLenWithNULL, buffer, targetWideCharCount);
}

void str_narrow(const wchar_t* str, int wcsLenWithNULL, char* buffer, int bufferByteSize)
{
	int targetSmallCharCount = WideCharToMultiByte(CP_UTF8, 0, str, wcsLenWithNULL, NULL, 0, NULL, NULL);
	assert(sizeof(char) * targetSmallCharCount <= bufferByteSize);

	WideCharToMultiByte(CP_UTF8, 0, str, wcsLenWithNULL, buffer, targetSmallCharCount, NULL, NULL);
}
#endif

FILE* open_file(const char* utf8Path, const char* mode)
{
	FILE* fhandle = NULL;
#if _WIN32 || _WIN64
	int fileNameLenWithNull = (int)strlen(utf8Path) + 1;
	int modeLenWithNull = (int)strlen(mode) + 1;

	wchar_t* tempNameBuffer = (wchar_t*)ALLOCA(sizeof(wchar_t) * (fileNameLenWithNull + modeLenWithNull));
	wchar_t* tempModeBuffer = &(tempNameBuffer[fileNameLenWithNull]);

	str_widen(utf8Path, fileNameLenWithNull, tempNameBuffer, sizeof(wchar_t) * fileNameLenWithNull);
	str_widen(mode, modeLenWithNull, tempModeBuffer, sizeof(wchar_t) * modeLenWithNull);

	fhandle = _wfopen(tempNameBuffer, tempModeBuffer);
#else
	fhandle = fopen(utf8Path, mode);
#endif

	return fhandle;
}

bool file_read_until_total_size(FILE* fp, int64_t total_size, void* buffer)
{
	int64_t should_read_size = total_size;
	int64_t total_read_size = 0;
	int64_t cur_read_size = 0;
	while (total_read_size < total_size)
	{
		cur_read_size = fread((void*)((uint8_t*)buffer + total_read_size), 1, should_read_size, fp);
		total_read_size += cur_read_size;
		should_read_size -= cur_read_size;
	}

	return total_read_size == total_size;
};

void file_open_fill_buffer(const char* path, std::vector<char>& buffer)
{
	buffer.clear();

	FILE* fp = open_file(path, "rb");
	fseek(fp, 0, SEEK_END);
	int64_t io_size = (int64_t)ftell(fp);
	fseek(fp, 0, SEEK_SET);

	buffer.resize(io_size + 1);
	if (false == file_read_until_total_size(fp, io_size, buffer.data()))
	{
		printf("Fail to read a vertex shader\n");
		assert(false);
	}
	buffer[io_size] = '\0';

	fclose(fp);
}

bool is_file_exist(const char* utf8_path)
{
#if _WIN32 || _WIN64
	int fileNameLenWithNull = (int)strlen(utf8_path) + 1;
	wchar_t* tempNameBuffer = (wchar_t*)ALLOCA(sizeof(wchar_t) * (fileNameLenWithNull));
	str_widen(utf8_path, fileNameLenWithNull, tempNameBuffer, sizeof(wchar_t) * fileNameLenWithNull);

	DWORD ret = GetFileAttributesW(tempNameBuffer);
	if (ret == INVALID_FILE_ATTRIBUTES)
		return false;

	if (ret & FILE_ATTRIBUTE_NORMAL || ret & FILE_ATTRIBUTE_ARCHIVE)
		return true;

	return false;
#else
	struct stat sb;
	int ret = stat(utf8Path, &sb);
	if (ret == -1)
		return false;

	if ((sb.st_mode & S_IFMT) == S_IFREG)
	{
		return true;
	}

	return false;
#endif
}