#include <curses.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include "defs.h"
#include "libudp.h"

#define CIPMSG_UDP_PORT 2425
#define CIPMSG_MSG_LEN 1024

#pragma pack(1)

#define IPMSG_VERSION_LEN 128
#define IPMSG_PKTID_LEN IPMSG_VERSION_LEN
#define IPMSG_USERNAME_LEN IPMSG_VERSION_LEN
#define IPMSG_HOSTNAME_LEN IPMSG_VERSION_LEN
#define IPMSG_MSG_TYPE_LEN IPMSG_VERSION_LEN
#define IPMSG_MSG_LEN CIPMSG_MSG_LEN
typedef struct ipmsg_msg_t {
    char version[IPMSG_VERSION_LEN];
    char pkt_id[IPMSG_PKTID_LEN];
    char username[IPMSG_USERNAME_LEN];
    char hostname[IPMSG_HOSTNAME_LEN];
    char msg_type[IPMSG_MSG_TYPE_LEN];
    char msg_content[IPMSG_MSG_LEN];
}ipmsg_msg_t;

#define CIPMSG_IP_LEN 16
typedef struct cipmsg_info_t {
    char dst_ip[CIPMSG_IP_LEN];
    UINT16 lis_port;
    char username[IPMSG_USERNAME_LEN];
    char hostname[IPMSG_HOSTNAME_LEN];
}cipmsg_info_t;

#pragma pack()

static cipmsg_info_t g_info;

WINDOW *g_win = NULL;
WINDOW *g_win_msg = NULL;
WINDOW *g_win_in = NULL;

static UINT8 g_msg[CIPMSG_MSG_LEN];

int cipmsg_udp_callback(struct sockaddr addr, char *buff, int blen);

int main(int argc, char **argv)
{
    UINT8 msg[CIPMSG_MSG_LEN];

    if (argc != 2)
    {
        printf("the ip of the peer needed.\n");
        return -1;
    }
    strncpy(g_info.dst_ip, argv[1], CIPMSG_IP_LEN);
    g_info.lis_port = CIPMSG_UDP_PORT;

    if (R_ERROR == udp_init(g_info.lis_port, cipmsg_udp_callback))
    {
        dprintf("udp init error.\n");
        return R_ERROR;
    }

    strncpy(g_info.username, getlogin(), IPMSG_USERNAME_LEN);
    gethostname(g_info.hostname, IPMSG_HOSTNAME_LEN);
    g_win = initscr();

    if (has_colors())
    {
        start_color();
        use_default_colors();
    }
    //cbreak();
    echo();
    g_win_msg = derwin(g_win, LINES - 5, COLS, 0, 0);
    box(g_win_msg, 0, 0);
    delwin(g_win_msg);
    g_win_msg = derwin(g_win, LINES - 7, COLS - 2, 1, 1);
    scrollok(g_win_msg, TRUE);

    g_win_in = derwin(g_win, 5, COLS, LINES - 5, 0);
    box(g_win_in, 0, 0);
    delwin(g_win_in);
    g_win_in = derwin(g_win, 3, COLS - 2, LINES - 4, 1);
    keypad(g_win_in, TRUE);
    refresh();
    
    while (1)
    {
        wgetnstr(g_win_in, (char *)msg, 1024);

        if (strlen((char *)msg) == 0) continue;

        snprintf((char *)g_msg, CIPMSG_MSG_LEN, "1:%d:%s:%s:32:%s", (int)random(), g_info.username, g_info.hostname, (char *)msg);

        udp_send((UINT8 *)g_info.dst_ip, 2425, g_msg, strlen((char *)g_msg));

        wprintw(g_win_msg, "%s@%s says : %s\n", g_info.username, g_info.hostname, msg);
        werase(g_win_in);
        keypad(g_win_in, TRUE);
        wrefresh(g_win_msg);
        wrefresh(g_win_in);
        
    }

    endwin();

    return 0;
}

#define CIPMSG_SPLIT_R_OK 0    // 返回此值说明还有更多的子段
#define CIPMSG_SPLIT_R_NOMORE 1    // 返回此值说明没有更多子段
static INT32 cipmsg_split(char *str, char *sp, char *val, INT32 val_len)
{
    char *sub = NULL;

    memset(val, 0, val_len);

    sub = strstr(str, sp);
    if (NULL == sub)
    {
        strncpy(val, str, val_len);
        return CIPMSG_SPLIT_R_NOMORE;
    }
    else
    {
        strncpy(val, str, MIN(sub - str, val_len));
        return CIPMSG_SPLIT_R_OK;
    }
}


int cipmsg_udp_callback(struct sockaddr addr, char *buff, int blen)
{
    ipmsg_msg_t ipmsg_msg;
    INT32 ret;
    char *buff_tmp = buff;
    static UINT8 msg_ack[IPMSG_MSG_LEN];
    
    ret = cipmsg_split(buff_tmp, ":", ipmsg_msg.version, IPMSG_VERSION_LEN); buff_tmp += (1 + strlen(ipmsg_msg.version));
    ret = cipmsg_split(buff_tmp, ":", ipmsg_msg.pkt_id, IPMSG_PKTID_LEN); buff_tmp += (1 + strlen(ipmsg_msg.pkt_id));
    ret = cipmsg_split(buff_tmp, ":", ipmsg_msg.username, IPMSG_USERNAME_LEN); buff_tmp += (1 + strlen(ipmsg_msg.username));
    ret = cipmsg_split(buff_tmp, ":", ipmsg_msg.hostname, IPMSG_HOSTNAME_LEN); buff_tmp += (1 + strlen(ipmsg_msg.hostname));
    ret = cipmsg_split(buff_tmp, ":", ipmsg_msg.msg_type, IPMSG_MSG_TYPE_LEN); buff_tmp += (1 + strlen(ipmsg_msg.msg_type));
    ret = cipmsg_split(buff_tmp, ":", ipmsg_msg.msg_content, IPMSG_MSG_LEN);

    if (33 == atoi(ipmsg_msg.msg_type))
    {
        return R_OK;
    }

    if (288 != atoi(ipmsg_msg.msg_type) && 32 != atoi(ipmsg_msg.msg_type))
    {
        return R_OK;
    }

    if (strlen(ipmsg_msg.msg_content) == 0)
    {
        return R_OK;
    }

    wprintw(g_win_msg, "%s@%s says : %s\n", ipmsg_msg.username, ipmsg_msg.hostname, ipmsg_msg.msg_content);
    
    snprintf((char *)msg_ack, IPMSG_MSG_LEN, "1:%d:%s:%s:33:%s", \
        (int)random(), g_info.username, g_info.hostname, ipmsg_msg.pkt_id);
    udp_reply(addr, msg_ack, strlen((char *)msg_ack));

    wrefresh(g_win_msg);
    wrefresh(g_win_in);

    return R_OK;
}
