#include<stdio.h>
#include<string.h>
#include<openssl/sha.h>

// hardcoded unbreakable passwd hash for login
//  don't try break it
//  u literally can't
const unsigned char target_hash[] = {
    0x96, 0xd6, 0x73, 0x5a, 0x74, 0xf6, 0x16, 0xd4, 0xbd, 0xa3, 0xb0, 0x24,
    0x5e, 0x3d, 0x1b, 0x47, 0x87, 0xb5, 0xc8, 0x32, 0x39, 0x7b, 0x33, 0xaa,
    0x96, 0xa3, 0x8a, 0xdf, 0x5c, 0xe2, 0x58, 0x7c
};


// should print the flag when someone has password
//  otherwise, shouldn't be possible to execute
void print_flag() {

    const unsigned char encrypted_flag[] = {
        0x7b, 0x21, 0x68, 0x61, 0x70, 0x70, 0x79, 0x5f, 0x79, 0x75, 0x6c, 0x65,
        0x74, 0x69, 0x64, 0x65, 0x5f, 0x67, 0x69, 0x66, 0x74, 0x5f, 0x6f, 0x76,
        0x65, 0x72, 0x66, 0x6c, 0x6f, 0x77, 0x21, 0x7d
    };

    static unsigned char runtime_key[32];
    static char decrypted_flag[sizeof(encrypted_flag) +1 ];

    for (int i = 0; i < 32; i++) {
        decrypted_flag[i] = encrypted_flag[i] ^ runtime_key[i % 32];
    }

    decrypted_flag[sizeof(encrypted_flag)] = '\0';

    printf("🎉 login successful!! 🎉\n");
    printf("%s", decrypted_flag);

}

// make sure the user is on the nice list and not HACKING
int verify_password(char* user_password) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;

    SHA256_Init(&sha256);
    SHA256_Update(&sha256, user_password, strlen(user_password));
    SHA256_Final(hash, &sha256);

    return memcmp(hash, target_hash, SHA256_DIGEST_LENGTH) == 0;
}

void get_password() {
    char password_buffer[64];

    printf("🔐 enter ur passwd :3 🔐\n");
    fflush(stdout);

    gets(password_buffer);

    if (verify_password(password_buffer)) {
        printf("🥳 access granted! 🥳\n");
        print_flag();
    } else {
        printf("「(•ˋ _ ˊ•)」 no!");
    }
}

int main() {
    printf("◝(ᵔᗜᵔ)◜ welcome to yulia's yuletide present vault ◝(ᵔᗜᵔ)◜\n");
    printf("        the vault is overflowing w presents for u\n");
    printf("         all u have to do to claim them is login\n");

    get_password();

    return 0;
}
