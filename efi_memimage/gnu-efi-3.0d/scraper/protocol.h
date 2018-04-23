/*
 * Very simple protocol
 */

#define SM_PORT         31337

#define SM_CMD_PROBE    1
#define SM_CMD_REQ      2
#define SM_CMD_RESP     3
#define SM_CMD_QUIT     4

typedef struct scrape_msg {
        uint32_t        sm_cmd;
        uint32_t        sm_off;
        uint32_t        sm_seq;
        uint32_t        sm_len;
} SCRAPE_MSG;
