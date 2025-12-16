#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BUF_SIZE 64
#define OVERREAD 256
#define KEY_LEN 32

static const uint8_t auto_key[KEY_LEN] = {
  0xa0, 0x54, 0xd8, 0x97, 0x21, 0x99, 0xaa, 0x9f,
  0xfe, 0xdc, 0x20, 0xb2, 0x1e, 0x56, 0x98, 0xfc,
  0x70, 0xe2, 0x0d, 0xcb, 0x51, 0xdb, 0x32, 0x11,
  0xc6, 0xdf, 0x19, 0x69, 0xbc, 0x2b, 0x0b, 0x20
};

static const uint8_t secret_key[KEY_LEN] = {
  0xea, 0x1d, 0xd1, 0xe3, 0xcb, 0x3d, 0x0a, 0x88,
  0x31, 0x6c, 0xfa, 0x99, 0xb4, 0xb3, 0xb7, 0xfe,
  0x70, 0xa6, 0x42, 0x92, 0xe3, 0x88, 0x40, 0x28,
  0x42, 0x46, 0x20, 0xfb, 0xca, 0x59, 0xb8, 0xd1
};

static const uint8_t ENC_FLAG[] = {
  0xdb, 0x3c, 0xe8, 0x87, 0x15, 0x44, 0xf5, 0xeb, 0x96, 0x1e, 0x11, 0xea,
  0x6a, 0xde, 0xac, 0x8d, 0x2f, 0xc5, 0x39, 0xf9, 0x62, 0xfb, 0x13, 0x55
};
static const size_t ENC_FLAG_LEN = sizeof(ENC_FLAG);

static void print_flag(const uint8_t *key) {
  uint8_t out[128];

  if (!key) {
    puts("εᐟᐠ>ﻌ<ᐟᐠз someone is trying to hack the kennel! εᐟᐠ>ﻌ<ᐟᐠз ");
    _exit(1);
  }
  if (ENC_FLAG_LEN + 1 > sizeof(out)) {
    puts("૮(,,> . <,,)ა non!");
    _exit(1);
  }

  for (size_t i = 0; i < ENC_FLAG_LEN; i++) {
    if (i % 2 == 0) {
      out[i] = ENC_FLAG[i] ^ auto_key[i % KEY_LEN];
    } else {
      out[i] = ENC_FLAG[i] ^ key[i % KEY_LEN];
    }
  }
  out[ENC_FLAG_LEN] = '\0';

  puts("｡:ﾟ૮ ˶ˆ ﻌ ˆ˶ ა ﾟ:｡ some cakes for ur effort ⊹ ࣪ ˖🍰₊˚⊹♡ ");
  puts((const char *)out);
  fflush(stdout);
}

static volatile void *keep_print_flag_anchor = (void *)&print_flag;

// i hope no cake thieves notice this
__attribute__((unused)) __attribute__((naked)) void pop_rdi_ret(void) {
  __asm__("pop %rdi; ret");
}

int main(int argc, char *argv[]) {
  char buf[BUF_SIZE];
  char name[32];
  int debug_mode = 0;

  if (argc > 1 && strcmp(argv[1], "--debug") == 0) {
    debug_mode = 1;
  }

  puts("૮ฅ・ﻌ・აฅ welcome to yennis confections! ૮ฅ・ﻌ・აฅ");
  puts("૮ ◞ ﻌ ◟ ა [NOTE] Yenni's Confections is still under heavy development, many functions don't work yet! ૮ ◞ ﻌ ◟ ა!");
  fflush(stdout);

  if (debug_mode) {
    printf("[DEBUG] Application base loaded at: %p\n", (void*)&main);
    fflush(stdout);
  }

  if (fgets(name, sizeof(name), stdin)) {
    name[strcspn(name, "\n")] = '\0';
    printf("%s\n", name);
  }

  puts("\nzᶻ ૮˶- ﻌ -˶ა⌒)ᦱ place your order! [TODO: implement ordering] zᶻ ૮˶- ﻌ -˶ა⌒)ᦱ");
  fflush(stdout);

  ssize_t r = read(STDIN_FILENO, buf, OVERREAD);

  puts("૮₍˶Ó﹏Ò ⑅₎ა  come again! ૮₍˶Ó﹏Ò ⑅₎ა ");
  return 0;
}
