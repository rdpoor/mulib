#include "srec/mu_srec.h"
#include "string/mu_str.h"
#include "string/mu_strbuf.h"
#include "tools/test_utils.h"

#define PAYLOAD_LEN 0x10
#define BUF16_LEN (PAYLOAD_LEN+11)
#define BUF24_LEN (PAYLOAD_LEN+13)
#define BUF32_LEN (PAYLOAD_LEN+15)

static const char *s_str;

static void writer(const char *str) {
  s_str = str;
}

/**
From https://www.x-ways.net/winhex/kb/ff/Motorola-S2.txt
S0 06 0000 484452 1B
S1 13 0000 285F245F2212226A000424290008237C 2A
S1 13 0010 00020008000826290018538123410018 13
S1 13 0020 41E900084E42234300182342000824A9 52
S1 07 003000144ED4 92
S5 03 0004 F8
S9 03 0000 FC

From https://en.wikipedia.org/wiki/SREC_(file_format)
S00F000068656C6C6F202020202000003C
S11F00007C0802A6900100049421FFF07C6C1B787C8C23783C6000003863000026
S11F001C4BFFFFE5398000007D83637880010014382100107C0803A64E800020E9
S111003848656C6C6F20776F726C642E0A0042
S5030003F9
S9030000FC

See also:
http://www.s-record.com/
*/

void test_mu_srec(void) {
  uint8_t buf16[A16_BUF_LEN];
  srec_handle_t handle;

  ASSERT(srec_init(&handle, SREC_ADDR_SIZE_16, -1, buf16, BUF16_LEN, writer) != NULL);
  srec_write_header(&handle, "HDR");
  ASSERT(strcmp(s_str, "S00600004844521B") == 0);

  uint8_t data[] = {
    0x28, 0x5F, 0x24, 0x5F, 0x22, 0x12, 0x22, 0x6A,
    0x00, 0x04, 0x24, 0x29, 0x00, 0x08, 0x23, 0x7C
  };
  srec_write_data(&handle, 0x10, data, sizeof(data));
  ASSERT(strcmp(s_str, "S11300100002000800082629001853812341001813") == 0);

  srec_write_termination(&handle, 0);
  ASSERT(strcmp(s_str, "S9030000FC") == 0);
}
