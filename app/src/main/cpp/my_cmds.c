
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <sys/vfs.h>

/*
void create_file(const char * filename) {
	// the third argument is perssion
    int fd0 = open(filename, O_RDWR | O_CREAT, 0777);
    if(-1 == fd0) {
        puts("Error opening\n");
        exit(1);
    }
    close(fd0);
}
*/

unsigned long long mtj_get_size(const char * filename) {
	struct statfs diskInfo;
	unsigned long long totalBlocks;
	unsigned long long freeDisk;
	statfs(filename, &diskInfo);
	totalBlocks = diskInfo.f_bsize;
	freeDisk = diskInfo.f_bfree * totalBlocks;
	return freeDisk;
}

void * mtj_read(const char * path,  int offset,  int len) {
    int fd;
	void * buf;

    fd = open(path, O_RDONLY);
    if(-1 == fd) {
        puts("Error opening");
	puts(path);
	puts("\n");
        exit(1);
    }

    offset = lseek(fd, offset, SEEK_SET); 

    buf = (void * )malloc(sizeof(char) * (len));
    if(!read(fd, buf, len)) return NULL;

    close(fd);
    return buf;
}

int mtj_write(const char * path, void * buf, int offset, int  len) {
    int fd;

    fd = open(path, O_WRONLY);
    if(-1 == fd) {
        puts("Error opening");
	puts(path);
	puts("\n");
        exit(1);
    }

    offset = lseek(fd, offset, SEEK_SET); 
    if(write(fd, buf, len)<0) return -1;

    close(fd);
    return 1;
}

double mtj_rand() {
    return rand() / (double)(RAND_MAX);
}

/**
 * @brief return random num in [i, j)
 * 
 * @param i min
 * @param j max
 * @return int 
 */
int mtj_randi(int i, int j) {
    int d = j - i;
    return i + (int)(d * mtj_rand());
}

char * make_randname(int len) {
	char table[] = "abcdefghijklmnopqrstuvwxyz";
	char * buf = (char *) malloc(sizeof(char) * len * 2);
	int i;
	for(i = 0; i < len; ++i) {
		buf[i] = table[mtj_randi(0, 26)];
	}
	buf[i] = '\0';
	return buf;
}

int is_same(char * buf, char * check, int len) {
	for(int i = 0; i < len; ++i) {
		if(buf[i] != check[i]) {
			return 0;
		}
	}
	return 1;
}

char * make_data_random(int len) {
	char * buf;
	char table[] = "abcdefghijklmnopqrstuvwxyz";
	buf = (char *) malloc(sizeof(char) * len);
	for(int i = 0; i < len; ++i) {
		buf[i] = table[mtj_randi(0, 26)];
	}
	return buf;
}

char * make_data(int len) {
	char * buf;
	buf = (char *) malloc(sizeof(char) * len);
	memset(buf, 0xaa, sizeof(char) * len);
	return buf;
}

struct DiskData
{
	// M/s
	int offset; // 相对与设备的偏移量
	int len; // 测试长度
	double write_v; // 写入速率
	double read_v; // 读取速率
	double correct_rate; // 有效字节率：正确的子节数
};

// 根据需要测试的偏移量，长度测出准确率
// 偏移量，长度由diskdata给出

void test(const char * path, struct DiskData * diskdata) {

	char *buf;
	char *check;
	struct timeval start, end;
	long dur;
	int correct_cnt;
	int len;

	len = diskdata->len;

	buf = make_data(len);
	gettimeofday(&start, NULL);
	mtj_write(path, buf, diskdata->offset, len);
	gettimeofday(&end, NULL);
	dur = (end.tv_sec - start.tv_sec) * 1000000 + end.tv_usec - start.tv_usec;
	diskdata->write_v = ((double)len / (1024 * 1024)) / dur * 1e6;


	gettimeofday(&start, NULL);
	check = mtj_read(path, diskdata->offset, len);
	gettimeofday(&end, NULL);
	dur = (end.tv_sec - start.tv_sec) * 1000000 + end.tv_usec - start.tv_usec;
	diskdata->read_v = ((double)len / (1024 * 1024)) / dur * 1e6;

	correct_cnt = 0;
	for(int i = 0; i < len; ++i) {
		correct_cnt += (unsigned char)buf[i] == (unsigned char)check[i];
	}
	diskdata->correct_rate = (double)correct_cnt / len;
	free(check);
	free(buf);
}

void show(struct DiskData * disk) {
	printf("---------------------------------------\n");
	printf("offset: %d\n", disk->offset);
	printf("length: %d bytes\n", disk->len);
	printf("write v: %lf M/s\n", disk->write_v);
	printf("read v: %lf M/s\n", disk->read_v);
	printf("Correct rate: %lf\n", disk->correct_rate);
	printf("---------------------------------------\n\n");
}

// 随机选择一个块测试，块大小在1byte~4M，测试times次
void randomTest(const char * device, struct DiskData * diskdata, int times) {
	unsigned long long size = mtj_get_size(device);
	int len, offset;

	for(int i = 0; i < times; i++) {
		offset = mtj_randi(0, (int)size);
		len = mtj_randi(1, 4 * 1024 * 1024 + 1);
		len = size - offset < len ? size - offset : len;

		diskdata->offset = offset;
		diskdata->len = len;

		test(device, diskdata);
		show(diskdata);
	}
	
}

// 顺序测试times次
void serialTest(const char * device, struct DiskData * diskdata, int times) {
	unsigned long long size = mtj_get_size(device);
	unsigned int len, offset;

	len = size / times;
	offset = 0;
	for (int i = 0; i < times; i++) {
		diskdata->len = len;
		diskdata->offset = offset;

		test(device, diskdata);
		show(diskdata);

		offset += len;
	}
	
}


int do_test(int argc, char ** argv) {
	struct DiskData disk;
	int len;
	char path[100];
	int times;
	char mode[100];

	// fn device len
	if(argc == 3) {
#if 0
	printf("usage: mtjfile-1 [file] [size]bytes\n");
	exit(1);
#endif
		printf("the %s size is %lld M\n", argv[1], mtj_get_size(argv[1]) / (1024 * 1024));

		sscanf(argv[1], "%s", path);
		sscanf(argv[2], "%d", &len);

		disk.len = len;
		disk.offset = 0;

		test(path, &disk);
		show(&disk);
	} else if(argc == 4) {
		// fn device mode times
		printf("the %s size is %lld M\n", argv[1], mtj_get_size(argv[1]) / (1024 * 1024));

		sscanf(argv[1], "%s", path);
		sscanf(argv[2], "%s", mode);
		sscanf(argv[3], "%d", &times);

		if(!strcmp(mode, "random")) {
			randomTest(path, &disk, times);
		} else if(!strcmp(mode, "serial")) {
			serialTest(path, &disk, times);
		} else {
			printf("usage: fn deviceName mode times --- mode can be (random|serial)\n");
			exit(1);
		}
		
	}




	return 0;
}
