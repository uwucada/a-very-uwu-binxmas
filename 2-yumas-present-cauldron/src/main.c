#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/sha.h>

#define BUFFER_SIZE 64

struct user_data {
  char password[BUFFER_SIZE];
  char auth_status[BUFFER_SIZE];
};

const char* authenticated_state = "AUTH_STATUS: authenticated";

// hardcoded password hash
const unsigned char target_hash[] = {
    0x1c, 0x53, 0xd8, 0x0e, 0x7e, 0x29, 0x46, 0xbc, 0x72, 0x87, 0x8e, 0xa7,
    0x3d, 0x18, 0x1b, 0xe3, 0x48, 0xd3, 0x48, 0xed, 0xe3, 0x97, 0xa7, 0x39,
    0x5d, 0x9b, 0x45, 0x67, 0xc8, 0xcf, 0x35, 0xd8
};

// print the flag
void print_flag() {
    const unsigned char encrypted_flag[] = {
        0x7b, 0x21, 0x61, 0x5f, 0x62, 0x6c, 0x65, 0x73, 0x73, 0x69, 0x6e, 0x67,
        0x5f, 0x66, 0x6f, 0x72, 0x5f, 0x74, 0x68, 0x65, 0x5f, 0x6e, 0x65, 0x77,
        0x5f, 0x79, 0x65, 0x61, 0x72, 0x21, 0x7d
    };

    static unsigned char runtime_key[32];
    static char decrypted_flag[sizeof(encrypted_flag) + 1];

    for (int i = 0; i < 32; i++) {
        decrypted_flag[i] = encrypted_flag[i] ^ runtime_key[i % 32];
    }

    decrypted_flag[sizeof(encrypted_flag)] = '\0';

    printf("₍^. .^₎⟆ ! merry catmas here is ur present ! ₍^. .^₎⟆\n");
    printf("%s\n", decrypted_flag);
}

int verify_password(char* password, struct user_data *data) {
  unsigned char hash[SHA256_DIGEST_LENGTH];
  SHA256_CTX sha256;

  SHA256_Init(&sha256);
  SHA256_Update(&sha256, password, strlen(password));
  SHA256_Final(hash, &sha256);

  int hash_result = memcmp(hash, target_hash, SHA256_DIGEST_LENGTH);

  if (hash_result == 0) {
    strcpy(data->auth_status, authenticated_state);
  }

  return hash_result;
}

void print_debug(struct user_data *data) {
  printf("\n");
  printf("  ∧,,,∧\n");
  printf(" (• ⩊ •)\n");
  printf("|￣U U￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣|\n");
  printf("|  auth state: %s\n", data->auth_status);
  printf("|  password: %s\n", data->password);
  printf("￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣\n");
}

int get_input() {
  struct user_data *data;
  char input[256];

  data = (struct user_data *)malloc(sizeof(struct user_data));

  if (!data) {
    return 1;
  }

  strcpy(data->auth_status, "AUTH_STATUS: unauthenticated");

  printf("ᓚᘏᗢᓚ pls enter an option: ᘏᗢ\n");
  printf("≽^•⩊•^≼ 'debug'         -> print debug information\n");
  printf("≽^•⩊•^≼ 'anything else' -> enters ur password\n");
  fflush(stdout);

  fgets(input, sizeof(input), stdin);

  input[strcspn(input, "\n")] ='\0';

  if (strncmp(input, "debug", 5) == 0) {
    print_debug(data);
    return 0;
  } else {
    strcpy(data->password, input);
    print_debug(data);
      if (verify_password(input, data) == 0) {
    printf("\n");
    printf(" ∧,,,,∧\n");
    printf("(  ̳• · • ̳)  - password valid!\n");
    printf("/    づ♡ \n");
    printf("\n");
  } else {
    printf("\n");
    printf("            へ\n");
    printf("       ૮  >  <) u wouldn't hack a tech witch\n");
    printf("       /      | u are cursed to be a toad\n");
    printf("     乀(ˍ, ل ل\n");
    printf("\n");
  }

  if (strcmp(data->auth_status, authenticated_state) == 0) {
    print_flag();
    free(data);
    return 0;
  } else {
    free(data);
    return 1;
  }
}

  }
int main() {
  printf("\n");
  printf("welcome to yuma's present cauldron!\n");
  printf("                           ╱|、\n");
  printf("                          (˚ˎ 。7\n");
  printf("                          |、˜〵\n");
  printf("                          じしˍ,)ノ\n");
  printf("\n");

  return get_input();
}
