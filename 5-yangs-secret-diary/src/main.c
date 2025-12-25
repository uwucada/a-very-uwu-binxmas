#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define KEY_LEN 32

typedef struct diary_entry {
    char text[64];
    uint64_t tags[8];
    void (*render)(struct diary_entry*);
    struct diary_entry *next;
} diary_entry_t;

diary_entry_t *head = NULL;

__attribute__((naked)) void pop_rdi() {
    __asm__("pop %rdi; ret");
}

__attribute__((naked)) void pop_rsi() {
    __asm__("pop %rsi; ret");
}

__attribute__((naked)) void pop_rdx() {
    __asm__("pop %rdx; ret");
}

__attribute__((naked)) void pop_rcx() {
    __asm__("pop %rcx; ret");
}

__attribute__((naked)) void pop_r8() {
    __asm__("pop %r8; ret");
}

__attribute__((naked)) void pop_r9() {
    __asm__("pop %r9; ret");
}

__attribute__((naked)) void ret() {
    __asm__("ret");
}

__attribute__((naked)) void xchg_rdi() {
    __asm__(
        "xchg %rdi, %rsp\n\t"
        "ret"
    );
}

#define KEY_LEN 32

static const uint8_t key1[KEY_LEN] = {
  0xb6, 0x52, 0xea, 0x88, 0x5f, 0x7a, 0x74, 0x75,
  0x8c, 0xd5, 0x55, 0x17, 0x71, 0xfa, 0x7c, 0x38,
  0x57, 0x4b, 0xc7, 0x42, 0x3a, 0x4d, 0x84, 0xdc,
  0x8a, 0x7e, 0x5b, 0x95, 0x97, 0x9a, 0xcc, 0x22
};

static const uint8_t key2[KEY_LEN] = {
  0x89, 0x7f, 0x48, 0x9f, 0x33, 0xf6, 0x74, 0xec,
  0xd8, 0x61, 0xd3, 0x55, 0xd9, 0xb8, 0xf6, 0xe1,
  0xae, 0xdb, 0x80, 0xfe, 0x3f, 0xc1, 0x42, 0x25,
  0x4d, 0x50, 0x40, 0xda, 0xff, 0xd2, 0x6b, 0x18
};

static const uint8_t key3[KEY_LEN] = {
  0x8d, 0x67, 0xe3, 0x96, 0x4d, 0x9a, 0x28, 0xe1,
  0xa1, 0x29, 0xb3, 0x2b, 0x7f, 0xf3, 0x06, 0x81,
  0x41, 0x07, 0x29, 0x2b, 0x70, 0x61, 0x62, 0x6c,
  0x0e, 0x46, 0xc0, 0x61, 0x6f, 0x4f, 0xb1, 0xf4
};

static const uint8_t key4[KEY_LEN] = {
  0x42, 0x79, 0x4d, 0xb3, 0xb5, 0xef, 0x84, 0x2f,
  0x53, 0x21, 0x22, 0x09, 0xee, 0x31, 0x66, 0xa9,
  0x40, 0xf2, 0x32, 0xee, 0x1c, 0x43, 0x51, 0x5b,
  0x4f, 0x7f, 0x06, 0x91, 0x40, 0xce, 0xaf, 0xdc
};

static const uint8_t key5[KEY_LEN] = {
  0xf9, 0xb7, 0xb5, 0xbe, 0xa4, 0x8b, 0x76, 0x10,
  0xd8, 0x7a, 0x0b, 0x09, 0x71, 0xf4, 0xb8, 0x47,
  0x41, 0x6d, 0xca, 0xde, 0xea, 0xa7, 0xa0, 0xed,
  0x24, 0x92, 0xa1, 0x1f, 0xc6, 0x48, 0xf4, 0x19
};

static const uint8_t key6[KEY_LEN] = {
  0x9e, 0x7a, 0x65, 0x8e, 0x77, 0xea, 0xc7, 0x5f,
  0x95, 0x4f, 0xeb, 0xfe, 0xf7, 0x4e, 0xab, 0x5c,
  0x76, 0x60, 0xc2, 0x1d, 0xe7, 0xf8, 0x6e, 0xe7,
  0x9e, 0x76, 0x79, 0x48, 0x08, 0xd7, 0x32, 0x64
};

static const uint8_t encrypted_flag[] = {
  0xcd, 0x5e, 0x88, 0xdd, 0x95, 0x9c, 0x47, 0x9f, 0xfe, 0x54, 0x65, 0x9a,
  0x42, 0xca, 0x59, 0xdc, 0x33, 0x3f, 0xb3, 0x8c, 0x43, 0x70, 0x81, 0x9a
};
static const size_t encrypted_flag_len = sizeof(encrypted_flag);

void render_entry(diary_entry_t *entry);

void decrypt_and_print_flag(const uint8_t *k1, const uint8_t *k2, const uint8_t *k3,
                           const uint8_t *k4, const uint8_t *k5, const uint8_t *k6) {
    uint8_t out[128];

    if (!k1 || !k2 || !k3 || !k4 || !k5 || !k6) {
        printf("[?] who dares to stalk within these secrets\n");
        fflush(stdout);
        _exit(1);
    }

    if (encrypted_flag_len + 1 > sizeof(out)) {
        printf("[+] death wish achieved\n");
        fflush(stdout);
        _exit(1);
    }

    const uint8_t *keys[] = {k1, k2, k3, k4, k5, k6};

    for (size_t i = 0; i < encrypted_flag_len; i++) {
        int key_index = i % 6;
        out[i] = encrypted_flag[i] ^ keys[key_index][i % KEY_LEN];
    }
    out[encrypted_flag_len] = '\0';

    printf("\n");
    printf("🔪🔪🔪🔪🔪🔪🔪🔪🔪🔪🔪🔪🔪🔪🔪\n");
    printf("  secret recovered:  \n");
    printf("%s\n", out);
    printf("🔪🔪🔪🔪🔪🔪🔪🔪🔪🔪🔪🔪🔪🔪🔪\n");
    fflush(stdout);
}

void render_entry(diary_entry_t *entry) {
    if (!entry) return;

    printf("\n🔪🔪🔪 diary entry 🔪🔪🔪\n");
    printf("text: %s\n", entry->text);
    printf("tags: ");
    for (int i = 0; i < 8; i++) {
        printf("%016lx ", entry->tags[i]);
    }
    printf("\n");
    fflush(stdout);
}

void view_entries() {
    if (!head) {
        printf("[x] no entries\n");
        return;
    }

    diary_entry_t *current = head;
    int count = 1;

    while (current) {
        printf("\n🔪 entry %d:\n", count++);
        current->render(current);
        current = current->next;
    }
}

void add_entry() {
    diary_entry_t *new_entry = malloc(sizeof(diary_entry_t));
    if (!new_entry) {
        return;
    }

    printf("[?] entry (max 63 chars):  ");
    fflush(stdout);

    if (fgets(new_entry->text, 64, stdin) == NULL) {
        free(new_entry);
        return;
    }

    new_entry->text[strcspn(new_entry->text, "\n")] = 0;

    printf("[?] tags (max 8 tags, hex-formatted): ");
    fflush(stdout);

    for (int i = 0; i < 8; i++) {
        new_entry->tags[i] = 0;
    }

    char tag_line[256];
    if (fgets(tag_line, sizeof(tag_line), stdin) != NULL) {
        char *ptr = tag_line;
        for (int i = 0; i < 8; i++) {
            unsigned long val;
            int consumed;
            if (sscanf(ptr, "%lx%n", &val, &consumed) == 1) {
                new_entry->tags[i] = val;
                ptr += consumed;
            } else {
                break;
            }
        }
    }

    new_entry->render = render_entry;
    new_entry->next = head;
    head = new_entry;

    printf("[+] entry added!\n");
}

void edit_entry() {
    if (!head) {
        printf("[x] no entries\n");
        return;
    }

    int entry_num;
    printf("[?] entry number: ");
    fflush(stdout);

    if (scanf("%d", &entry_num) != 1) {
        return;
    }

    int c;
    while ((c = getchar()) != '\n' && c != EOF);

    diary_entry_t *current = head;
    int count = 1;

    while (current && count < entry_num) {
        current = current->next;
        count++;
    }

    if (!current) {
        printf("[x] no entries\n");
        return;
    }

    printf("[?] new text: ");
    fflush(stdout);

    read(0, current->text, 200);

    printf("[+] updated\n");
}

void search_entries() {
    char search_term[128];

    printf("[?] search: ");
    fflush(stdout);

    if (fgets(search_term, sizeof(search_term), stdin) == NULL) {
        return;
    }

    search_term[strcspn(search_term, "\n")] = 0;

    printf("[i] searching for: ");
    printf(search_term);
    printf("\n");

    diary_entry_t *current = head;
    int found = 0;

    while (current) {
        if (strstr(current->text, search_term)) {
            render_entry(current);
            found = 1;
        }
        current = current->next;
    }

    if (!found) {
        printf("[x] no entries\n");
    }
}

void delete_entry() {
    if (!head) {
        printf("[x] no entries\n");
        return;
    }

    int entry_num;
    printf("[?] entry number: ");
    fflush(stdout);

    if (scanf("%d", &entry_num) != 1) {
        return;
    }

    int c;
    while ((c = getchar()) != '\n' && c != EOF);

    if (entry_num == 1) {
        diary_entry_t *temp = head;
        head = head->next;
        free(temp);
        printf("[+] entry deleted!\n");
        return;
    }

    diary_entry_t *current = head;
    int count = 1;

    while (current->next && count < entry_num - 1) {
        current = current->next;
        count++;
    }

    if (current->next) {
        diary_entry_t *temp = current->next;
        current->next = temp->next;
        free(temp);
        printf("[+] entry deleted!\n");
    } else {
        printf("[x] no entry.\n");
    }
}

__attribute__((noinline)) void save_diary() {
    char export_buffer[64];

    printf("[?] file name: ");
    fflush(stdout);

    if (fgets(export_buffer, 256, stdin) == NULL) {
        return;
    }
    export_buffer[strcspn(export_buffer, "\n")] = 0;

    printf("[i] saving diary: %s\n", export_buffer);
}

void display_menu() {
    printf("\n");
    printf("🔪🔪🔪🔪🔪🔪🔪🔪🔪🔪🔪🔪🔪🔪🔪🔪\n");
    printf("        Yang's Secret Diary      \n");
    printf("🔪🔪🔪🔪🔪🔪🔪🔪🔪🔪🔪🔪🔪🔪🔪🔪\n");
    printf("[1] view entries\n");
    printf("[2] add entry\n");
    printf("[3] edit entry\n");
    printf("[4] search entries\n");
    printf("[5] delete entry\n");
    printf("[6] export diary\n");
    printf("[7] exit\n");
    printf("🔪🔪🔪🔪🔪🔪🔪🔪🔪🔪🔪🔪🔪🔪🔪🔪\n");
    printf("[?] choice: ");
    fflush(stdout);
}

void run_diary() {
    int choice;

    printf("📓🍵 yang's secret diary 🍵📓\n");
    printf("🔪 intruders will be severely punished 🔪\n");
    fflush(stdout);

    while (1) {
        display_menu();

        if (scanf("%d", &choice) != 1) {
            choice = 0;
        }

        int c;
        while ((c = getchar()) != '\n' && c != EOF);

        switch (choice) {
            case 1:
                view_entries();
                break;
            case 2:
                add_entry();
                break;
            case 3:
                edit_entry();
                break;
            case 4:
                search_entries();
                break;
            case 5:
                delete_entry();
                break;
            case 6:
                save_diary();
                break;
            case 7:
                printf("[+] closing diary...\n");
                return;
            default:
                printf("[x] invalid choice.\n");
        }
    }
}

int main() {

    while (1) {
        pid_t pid = fork();

        if (pid == 0) {
            run_diary();
            exit(0);
        } else if (pid > 0) {
            waitpid(pid, NULL, 0);
        } else {
            perror("fork failed");
            exit(1);
        }
    }

    return 0;
}
