/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License v2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 021110-1307, USA.
 *
 * Modified to add field firmware update support,
 * those modifications are Copyright (c) 2016 SanDisk Corp.
 *
 * (This code is based on btrfs-progs/btrfs.c.)
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mmc_cmds.h"
#include "my_cmds.h"
#define MMC_VERSION	"0.1"

#define BASIC_HELP 0
#define ADVANCED_HELP 1

typedef int (*CommandFunction)(int argc, char **argv);

struct Command {
	CommandFunction	func;	/* function which implements the command */
	int	nargs;		/* if == 999, any number of arguments
				   if >= 0, number of arguments,
				   if < 0, _minimum_ number of arguments */
	char	*verb;		/* verb */
	char	*help;		/* help lines; from the 2nd line onward they
                                   are automatically indented */
        char    *adv_help;      /* advanced help message; from the 2nd line
                                   onward they are automatically indented */

	/* the following fields are run-time filled by the program */
	char	**cmds;		/* array of subcommands */
	int	ncmds;		/* number of subcommand */
};

static struct Command commands[] = {
	/*
	 *	avoid short commands different for the case only
	 */
	{ do_test, -3,
	  "do_test", "<1/2> <verify_file> <mmc_device> <size(512 bytes)> <off_set(sequential)/test_times(random)>.\n"
		"Command Form:./dd_test <Order 1: Sequential 2:Random> <verify_file> <mmc_device> <size(512 bytes)> <off_set(sequential)> <test_times(random)>.\n",
	  NULL
	},
	{ do_read_extcsd, -1,
	  "extcsd read", "<device>\n"
		"Print extcsd data from <device>.",
	  NULL
	},
	{ do_writeprotect_boot_get, -1,
	  "writeprotect boot get", "<device>\n"
		"Print the boot partitions write protect status for <device>.",
	  NULL
	},
	{ do_writeprotect_boot_set, -1,
	  "writeprotect boot set", "<device>\n"
		"Set the boot partitions write protect status for <device>.\nThis sets the eMMC boot partitions to be write-protected until\nthe next boot.",
	  NULL
	},
	{ do_writeprotect_user_set, -4,
	  "writeprotect user set", "<type>" "<start block>" "<blocks>" "<device>\n"
#ifdef DANGEROUS_COMMANDS_ENABLED
	  "Set the write protect configuration for the specified region\nof the user area for <device>.\n<type> must be \"none|temp|pwron|perm\".\n    \"none\"  - Clear temporary write protection.\n    \"temp\"  - Set temporary write protection.\n    \"pwron\" - Set write protection until the next poweron.\n    \"perm\"  - Set permanent write protection.\n<start block> specifies the first block of the protected area.\n<blocks> specifies the size of the protected area in blocks.\nNOTE! The area must start and end on Write Protect Group\nboundries, Use the \"writeprotect user get\" command to get the\nWrite Protect Group size.\nNOTE! \"perm\" is a one-time programmable (unreversible) change.",
#else
	  "Set the write protect configuration for the specified region\nof the user area for <device>.\n<type> must be \"none|temp|pwron\".\n    \"none\"  - Clear temporary write protection.\n    \"temp\"  - Set temporary write protection.\n    \"pwron\" - Set write protection until the next poweron.\n<start block> specifies the first block of the protected area.\n<blocks> specifies the size of the protected area in blocks.\nNOTE! The area must start and end on Write Protect Group\nboundries, Use the \"writeprotect user get\" command to get the\nWrite Protect Group size.",
#endif /* DANGEROUS_COMMANDS_ENABLED */
	  NULL
	},
	{ do_writeprotect_user_get, -1,
	  "writeprotect user get", "<device>\n"
		"Print the user areas write protect configuration for <device>.",
	  NULL
	},
	{ do_disable_512B_emulation, -1,
	  "disable 512B emulation", "<device>\n"
		"Set the eMMC data sector size to 4KB by disabling emulation on\n<device>.",
	  NULL
	},
	{ do_create_gp_partition, -6,
	  "gp create", "<-y|-n|-c> " "<length KiB> " "<partition> " "<enh_attr> " "<ext_attr> " "<device>\n"
		"Create general purpose partition for the <device>.\nDry-run only unless -y or -c is passed.\nUse -c if more partitioning settings are still to come.\nNOTE!  This is a one-time programmable (unreversible) change.\nTo set enhanced attribute to general partition being created set\n <enh_attr> to 1 else set it to 0.\nTo set extended attribute to general partition\n set <ext_attr> to 1,2 else set it to 0",
	  NULL
	},
	{ do_enh_area_set, -4,
	  "enh_area set", "<-y|-n|-c> " "<start KiB> " "<length KiB> " "<device>\n"
		"Enable the enhanced user area for the <device>.\nDry-run only unless -y or -c is passed.\nUse -c if more partitioning settings are still to come.\nNOTE!  This is a one-time programmable (unreversible) change.",
	  NULL
	},
	{ do_write_reliability_set, -2,
	  "write_reliability set", "<-y|-n|-c> " "<partition> " "<device>\n"
		"Enable write reliability per partition for the <device>.\nDry-run only unless -y or -c is passed.\nUse -c if more partitioning settings are still to come.\nNOTE!  This is a one-time programmable (unreversible) change.",
	  NULL
	},
	{ do_status_get, -1,
	  "status get", "<device>\n"
	  "Print the response to STATUS_SEND (CMD13).",	
	  NULL
	},
	{ do_write_boot_en, -3,
	  "bootpart enable", "<boot_partition> " "<send_ack> " "<device>\n"
		"Enable the boot partition for the <device>.\nDisable the boot partition for the <device> if <boot_partition> is set to 0.\nTo receive acknowledgment of boot from the card set <send_ack>\nto 1, else set it to 0.",
	  NULL
	},
	{ do_boot_bus_conditions_set, -4,
	  "bootbus set", "<boot_mode> " "<reset_boot_bus_conditions> " "<boot_bus_width> " "<device>\n"
	  "Set Boot Bus Conditions.\n"
	  "<boot_mode> must be \"single_backward|single_hs|dual\"\n"
	  "<reset_boot_bus_conditions> must be \"x1|retain\"\n"
	  "<boot_bus_width> must be \"x1|x4|x8\"",
	  NULL
	},
	{ do_write_bkops_en, -1,
	  "bkops enable", "<device>\n"
		"Enable the eMMC BKOPS feature on <device>.\nNOTE!  This is a one-time programmable (unreversible) change.",
	  NULL
	},
	{ do_hwreset_en, -1,
	  "hwreset enable", "<device>\n"
		"Permanently enable the eMMC H/W Reset feature on <device>.\nNOTE!  This is a one-time programmable (unreversible) change.",
	  NULL
	},
	{ do_hwreset_dis, -1,
	  "hwreset disable", "<device>\n"
		"Permanently disable the eMMC H/W Reset feature on <device>.\nNOTE!  This is a one-time programmable (unreversible) change.",
	  NULL
	},
	/* 删除设备中unmapped的的区域 */
	{ do_sanitize, -1,
	  "sanitize", "<device>\n"
		"Send Sanitize command to the <device>.\nThis will delete the unmapped memory region of the device.",
	  NULL
	},
	/* rpmb:replay protected memory block,具有安全特性的分区,一般存放公钥,序列号等 */
	{ do_rpmb_write_key, -1,
	  "rpmb write-key", "<rpmb device> <key file>\n"
		  "Program authentication key which is 32 bytes length and stored\n"
		  "in the specified file. Also you can specify '-' instead of\n"
		  "key file path to read the key from stdin.\n"
		  "NOTE!  This is a one-time programmable (unreversible) change.\n"
		  "Example:\n"
		  "  $ echo -n AAAABBBBCCCCDDDDEEEEFFFFGGGGHHHH | \\\n"
		  "    mmc rpmb write-key /dev/mmcblk0rpmb -",
	  NULL
	},
	{ do_rpmb_read_counter, -1,
	  "rpmb read-counter", "<rpmb device>\n"
		  "Counter value for the <rpmb device> will be read to stdout.",
	  NULL
	},
	{ do_rpmb_read_block, -1,
	  "rpmb read-block", "<rpmb device> <address> <blocks count> <output file> [key file]\n"
		  "Blocks of 256 bytes will be read from <rpmb device> to output\n"
		  "file or stdout if '-' is specified. If key is specified - read\n"
		  "data will be verified. Instead of regular path you can specify\n"
		  "'-' to read key from stdin.\n"
		  "Example:\n"
		  "  $ echo -n AAAABBBBCCCCDDDDEEEEFFFFGGGGHHHH | \\\n"
		  "    mmc rpmb read-block /dev/mmcblk0rpmb 0x02 2 /tmp/block -\n"
		  "or read two blocks without verification\n"
		  "  $ mmc rpmb read-block /dev/mmcblk0rpmb 0x02 2 /tmp/block",
	  NULL
	},
	{ do_rpmb_write_block, -1,
	  "rpmb write-block", "<rpmb device> <address> <256 byte data file> <key file>\n"
		  "Block of 256 bytes will be written from data file to\n"
		  "<rpmb device>. Also you can specify '-' instead of key\n"
		  "file path or data file to read the data from stdin.\n"
		  "Example:\n"
		  "  $ (awk 'BEGIN {while (c++<256) printf \"a\"}' | \\\n"
		  "    echo -n AAAABBBBCCCCDDDDEEEEFFFFGGGGHHHH) | \\\n"
		  "    mmc rpmb write-block /dev/mmcblk0rpmb 0x02 - -",
	  NULL
	},
	{ do_cache_en, -1,
	  "cache enable", "<device>\n"
		"Enable the eMMC cache feature on <device>.\n"
		"NOTE! The cache is an optional feature on devices >= eMMC4.5.",
	  NULL
	},
	{ do_cache_dis, -1,
	  "cache disable", "<device>\n"
		"Disable the eMMC cache feature on <device>.\n"
		"NOTE! The cache is an optional feature on devices >= eMMC4.5.",
	  NULL
	},
	{ do_read_csd, -1,
	  "csd read", "<device path>\n"
		  "Print CSD data from <device path>.\n"
		  "The device path should specify the csd file directory.",
	  NULL
	},
	{ do_read_cid, -1,
	  "cid read", "<device>\n"
		  "Print CID data from <device path>.\n"
		  "The device path should specify the cid file directory.",
	  NULL
	},
	{ do_read_scr, -1,
	  "scr read", "<device path>\n"
		  "Print SCR data from <device path>.\n"
		  "The device path should specify the scr file directory.",
	  NULL
	},
	{ do_ffu, -2,
	  "ffu", "<image name> <device>\n"
		"Run Field Firmware Update with <image name> on <device>.\n",
	  NULL
	},
	{ 0, 0, 0, 0 }
};

static char *get_prgname(char *programname)
{
	char	*np;
	np = strrchr(programname,'/');
	if(!np)
		np = programname;
	else
		np++;

	return np;
}

static void print_help(char *programname, struct Command *cmd, int helptype)
{
	char	*pc;

	printf("\t%s %s ", programname, cmd->verb );

	if (helptype == ADVANCED_HELP && cmd->adv_help)
		for(pc = cmd->adv_help; *pc; pc++){
			putchar(*pc);
			if(*pc == '\n')
				printf("\t\t");
		}
	else
		for(pc = cmd->help; *pc; pc++){
			putchar(*pc);
			if(*pc == '\n')
				printf("\t\t");
		}

	putchar('\n');
}

static void help(char *np)
{
	struct Command *cp;

	printf("Usage:\n");
	for( cp = commands; cp->verb; cp++ )
		print_help(np, cp, BASIC_HELP);

	printf("\n\t%s help|--help|-h\n\t\tShow the help.\n",np);
	printf("\n\t%s <cmd> --help\n\t\tShow detailed help for a command or subset of commands.\n",np);
	printf("\n%s\n", MMC_VERSION);
}

static int split_command(char *cmd, char ***commands)
{
	int	c, l;
	char	*p, *s;

	for( *commands = 0, l = c = 0, p = s = cmd ; ; p++, l++ ){
		if ( *p && *p != ' ' )
			continue;

		/* c + 2 so that we have room for the null */
		(*commands) = realloc( (*commands), sizeof(char *)*(c + 2));
		(*commands)[c] = strndup(s, l);
		c++;
		l = 0;
		s = p+1;
		if( !*p ) break;
	}

	(*commands)[c] = 0;
	return c;
}

/*
	This function checks if the passed command is ambiguous
*/
static int check_ambiguity(struct Command *cmd, char **argv){
	int		i;
	struct Command	*cp;
	/* check for ambiguity */
	for( i = 0 ; i < cmd->ncmds ; i++ ){
		int match;
		for( match = 0, cp = commands; cp->verb; cp++ ){
			int	j, skip;
			char	*s1, *s2;

			if( cp->ncmds < i )
				continue;

			for( skip = 0, j = 0 ; j < i ; j++ )
				if( strcmp(cmd->cmds[j], cp->cmds[j])){
					skip=1;
					break;
				}
			if(skip)
				continue;

			if( !strcmp(cmd->cmds[i], cp->cmds[i]))
				continue;
			for(s2 = cp->cmds[i], s1 = argv[i+1];
				*s1 == *s2 && *s1; s1++, s2++ ) ;
			if( !*s1 )
				match++;
		}
		if(match){
			int j;
			fprintf(stderr, "ERROR: in command '");
			for( j = 0 ; j <= i ; j++ )
				fprintf(stderr, "%s%s",j?" ":"", argv[j+1]);
			fprintf(stderr, "', '%s' is ambiguous\n",argv[j]);
			return -2;
		}
	}
	return 0;
}

/*
 * This function, compacts the program name and the command in the first
 * element of the '*av' array
 */
static int prepare_args(int *ac, char ***av, char *prgname, struct Command *cmd ){

	char	**ret;
	int	i;
	char	*newname;

	ret = (char **)malloc(sizeof(char*)*(*ac+1));
	newname = (char*)malloc(strlen(prgname)+strlen(cmd->verb)+2);
	if( !ret || !newname ){
		free(ret);
		free(newname);
		return -1;
	}

	ret[0] = newname;
	for(i=0; i < *ac ; i++ )
		ret[i+1] = (*av)[i];

	strcpy(newname, prgname);
	strcat(newname, " ");
	strcat(newname, cmd->verb);

	(*ac)++;
	*av = ret;

	return 0;

}

/*
	This function performs the following jobs:
	- show the help if '--help' or 'help' or '-h' are passed
	- verify that a command is not ambiguous, otherwise show which
	  part of the command is ambiguous
	- if after a (even partial) command there is '--help' show detailed help
	  for all the matching commands
	- if the command doesn't match show an error
	- finally, if a command matches, they return which command matched and
	  the arguments

	The function return 0 in case of help is requested; <0 in case
	of uncorrect command; >0 in case of matching commands
	argc, argv are the arg-counter and arg-vector (input)
	*nargs_ is the number of the arguments after the command (output)
	**cmd_  is the invoked command (output)
	***args_ are the arguments after the command

*/
int do_return_emmc_path()
{
//    DIR *dir;
//    struct    dirent    *ptr;
//    dir = opendir("/dev/block");
//    while((ptr = readdir(dir)) != NULL) {
//        printf("/dev/block/%s\n", ptr->d_name);
//    }
//    closedir(dir);
    #define BUF_SIZE 10000
    char path[]={"/device/cid"};
    char find[]={"find /sys/block/ -name \"*mmcblk*\" "};
    FILE * p_file_0 = NULL;
    FILE * p_file_1 = NULL;
    p_file_0 = popen(find,"r");
    if(!p_file_0){
        perror("popen");
    }
    char buf[BUF_SIZE];
    int yes=0;
    char mmc[100];
    while(fgets(buf,BUF_SIZE,p_file_0) != NULL){
        char tmp[100]={"cat "};
        strcat(tmp,buf);
        int len=strlen(tmp);
        int pos=len - 1;
        while(tmp[pos] != '/')  --pos;
        int i=0;++pos;
        for (i = 0; i < len-pos; ++i)
        {
            mmc[i]=tmp[pos+i];
        }
        mmc[i]='\0';
        for (i = 0; i < 11; ++i)
        {
            tmp[len-1+i]=path[i];
        }
        tmp[len-1+12]='\0';
        p_file_1 = popen(tmp,"r");  //相当于输入：cat /sys/block/mmc*/device/name
        char cid[100];
        fgets(cid,100,p_file_1);
        //printf("tmp :%s\n",tmp);
        if(cid[0] == 'd' && cid[1] == 'f' && cid[4] == '1' && cid[5] == '8')      //如果名字包含SCY则表明找到了
        {
            yes=1;
           // printf("yes is %d\n",yes);
            break;

        }
		/*if(strstr(cid,"df0118"))      //如果名字包含SCY则表明找到了
		{suseconds_t
			yes=1;
			break;
		}*/
        else{
           printf("not found\n");
        }
        //printf("name: %s\n",cid);
    }
/*    printf("yes:%d\n",yes);
    printf("mmc: %s\n",mmc);
    printf("------------\n");*/
	//char devpath[100]={"/dev/block/"};
/*    printf("------------\n");*/
	//strcat(devpath,mmc);
	//printf("%s\n", devpath);  // devpah就是EMMC的完整路径，安卓中可以使用这个
    //printf("------------\n");
	//printf("final\n");
   	if(yes==1){
		char devpath[100]={"/dev/block/"};
		strcat(devpath,mmc);
		printf("%s\n", devpath);  // devpah就是EMMC的完整路径，安卓中可以使用这个
		//printf("final\n");
    }
   else{
    	printf("yes is not 1\n");
    }
    pclose(p_file_0);
    pclose(p_file_1);
    return 0;
}

/*
	This function performs the following jobs:
	- show the help if '--help' or 'help' or '-h' are passed
	- verify that a command is not ambiguous, otherwise show which
	  part of the command is ambiguous
	- if after a (even partial) command there is '--help' show detailed help
	  for all the matching commands
	- if the command doesn't match show an error
	- finally, if a command matches, they return which command matched and
	  the arguments

	The function return 0 in case of help is requested; <0 in case
	of uncorrect command; >0 in case of matching commands
	argc, argv are the arg-counter and arg-vector (input)
	*nargs_ is the number of the arguments after the command (output)
	**cmd_  is the invoked command (output)
	***args_ are the arguments after the command

*/
static int parse_args(int argc, char **argv,
		      CommandFunction *func_,
		      int *nargs_, char **cmd_, char ***args_ )
{
	struct Command	*cp;
	struct Command	*matchcmd=0;
	// 获得最后一个路径
	char		*prgname = get_prgname(argv[0]);
	int		i=0, helprequested=0;

// 如果指令数目小于2或者第二个字符串是“-h”或者“--help”，就运行help函数，并返回
// help函数将调用print_help，输出一些命令说明信息
	if( argc < 2 || !strcmp(argv[1], "help") ||
		!strcmp(argv[1], "-h") || !strcmp(argv[1], "--help")){
		help(prgname);
		return 0;
	}
    if( argc < 2 || !strcmp(argv[1], "return")){
        do_return_emmc_path();
        return 0;
    }
// 命令分解，只会做一次
	for( cp = commands; cp->verb; cp++ )
		if( !cp->ncmds)
			cp->ncmds = split_command(cp->verb, &(cp->cmds));

// 寻找对应命令，这里采用从头遍历的方式
	for( cp = commands; cp->verb; cp++ ){
		int     match;

// 1、如果参数数量小于ncmds，跳过
		if( argc-1 < cp->ncmds )
			continue;

// 2、核对所有参数，如果和指令不符合，match记为0，退出小循环
		for( match = 1, i = 0 ; i < cp->ncmds ; i++ ){
			char	*s1, *s2;
			s1 = cp->cmds[i];//这两步似乎没有必要，因为for循环中已经初始化了
			s2 = argv[i+1];

			for(s2 = cp->cmds[i], s1 = argv[i+1];
				*s1 == *s2 && *s1;
				s1++, s2++ ) ;
			if( *s1 ){
				match=0;
				break;
			}
		}

		/* If you understand why this code works ...
			you are a genious !! */
// 如果参数数量太多，或者位于i+1的参数是“--help"，调用printf_help并跳过此次循环
		if(argc>i+1 && !strcmp(argv[i+1],"--help")){
			if(!helprequested)
				printf("Usage:\n");
			print_help(prgname, cp, ADVANCED_HELP);
			helprequested=1;
			continue;
		}

// 如何不符合，就跳过
		if(!match)
			continue;

// 能走到这里，说明已经找到了基本符合的命令，记录信息，并退出循环

		matchcmd = cp;
		*nargs_  = argc-matchcmd->ncmds-1;
		*cmd_ = matchcmd->verb;
		*args_ = argv+matchcmd->ncmds+1;
		*func_ = cp->func;

		break;
	}

// 当已经打印了help时，再打印一下版本号，并返回0
	if(helprequested){
		printf("\n%s\n", MMC_VERSION);
		return 0;
	}

// 如果matchcmd是0，表示没有找到符合的命令，并调用help
	if(!matchcmd){
		fprintf( stderr, "ERROR: unknown command '%s'\n",argv[1]);
		help(prgname);
		return -1;
	}

// 检查是否歧义，也是采用从头到尾遍历的方式，是则返回"-2"
	if(check_ambiguity(matchcmd, argv))
		return -2;

	/* check the number of argument */
	if (matchcmd->nargs < 0 && matchcmd->nargs < -*nargs_ ){
		fprintf(stderr, "ERROR: '%s' requires minimum %d arg(s)\n",
			matchcmd->verb, -matchcmd->nargs);
			return -2;
	}
	if(matchcmd->nargs >= 0 && matchcmd->nargs != *nargs_ && matchcmd->nargs != 999){
		fprintf(stderr, "ERROR: '%s' requires %d arg(s)\n",
			matchcmd->verb, matchcmd->nargs);
			return -2;
	}

        if (prepare_args( nargs_, args_, prgname, matchcmd )){
                fprintf(stderr, "ERROR: not enough memory\\n");
		return -20;
        }


	//如果走到这里，意味着参数准确无误，返回"1"
	return 1;
}
int main(int ac, char **av )
{
	char		*cmd=0, **args=0;
	int		nargs=0, r;
	CommandFunction func=0;

	//解析命令，找到时返回1
	r = parse_args(ac, av, &func, &nargs, &cmd, &args);
	// 没找到返回1
	if( r <= 0 ){
		/* error or no command to parse*/
		exit(-r);
	}

// 执行func函数, nargs = ac-matchcmd->ncmds-1，args = av+matchcmd->ncmds+1, ;
// nargs是输入参数个数，args是指向参数的指针
	exit(func(nargs, args));
}

