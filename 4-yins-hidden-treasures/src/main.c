#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define TITLE_BUF_SIZE 64
#define METADATA_BUF_SIZE 256
#define MAX_TAG_SIZE 1024

#define KEY_LEN 32


static const uint8_t key_1[KEY_LEN] = {
  0x78, 0x14, 0x87, 0x73, 0x05, 0xec, 0x70, 0x8c,
  0x94, 0x08, 0x83, 0x46, 0x63, 0x73, 0x60, 0x3c,
  0x59, 0x3a, 0x5f, 0xda, 0x7e, 0x00, 0xdc, 0x52,
  0xf2, 0x99, 0xea, 0x90, 0x2e, 0xb0, 0xdc, 0xfb
};

static const uint8_t key_2[KEY_LEN] = {
  0xcd, 0x9a, 0xd1, 0x81, 0x69, 0x3c, 0x87, 0xc0,
  0x0a, 0x0d, 0x4f, 0x88, 0xcd, 0xd1, 0xa2, 0x3e,
  0xa6, 0x44, 0x6a, 0x98, 0xf2, 0xbe, 0x34, 0x70,
  0x58, 0x04, 0x36, 0x9b, 0xe9, 0x70, 0x18, 0x9c
};

static const uint8_t ENC_FLAG[] = {
  0xb6, 0x35, 0x90, 0x23, 0x3d, 0x89, 0xe6, 0xff, 0x6f, 0x57, 0x2c, 0x76,
  0xa0, 0x1a, 0xcc, 0x63, 0xc0, 0x0a, 0x18, 0x85, 0x87, 0x72, 0x6b, 0x22,
  0x2a, 0xaa, 0x03, 0xa3, 0x87, 0xc4, 0x6b, 0xda, 0xb0
};
static const size_t ENC_FLAG_LEN = sizeof(ENC_FLAG);

static void print_flag(const uint8_t *key1, const uint8_t *key2);
static volatile void *keep_print_flag_anchor = (void *)&print_flag;

__attribute__((unused)) __attribute__((naked)) void pop_rdi_ret(void) {
  __asm__("pop %rdi; ret");
}

__attribute__((unused)) __attribute((naked)) void pop_rsi_ret(void) {
  __asm__("pop %rsi; ret");
}

typedef struct {
    char *title_buffer;
    char *metadata_buffer;
    char *artist_buffer;
    char *album_buffer;
} mp3_tags_t;

static void print_flag(const uint8_t *key1, const uint8_t *key2) {
  uint8_t out[128];

  if (!key1) {
    puts("💀💀💀 access denied 💀💀💀");
    _exit(1);
  }
  if (!key2) {
    puts("💀💀 nice try 💀💀");
    _exit(1);
  }

  if (ENC_FLAG_LEN + 1 > sizeof(out)) {
    puts("💀💀💀 nope 💀💀💀");
    _exit(1);
  }

  for(size_t i = 0; i < ENC_FLAG_LEN; i++) {
    if (i % 2 == 0) {
        out[i] = ENC_FLAG[i] ^ key1[i % KEY_LEN];
    } else {
        out[i] = ENC_FLAG[i] ^ key2[i % KEY_LEN];
    }
  }

  out[ENC_FLAG_LEN] = '\0';

  puts("💰💰 congratulations, mammal  💰💰");
  puts((const char *)out);
  fflush(stdout);
}

static uint32_t synchsafe_to_uint32(const uint8_t *data) {
    return (data[0] << 21) | (data[1] << 14) | (data[2] << 7) | data[3];
}

static uint32_t bytes_to_uint32_be(const uint8_t *data) {
    return (data[0] << 24) | (data[1] << 16) | (data[2] << 8) | data[3];
}


// TODO: ID3 frame parsing only currently works with utf16, add functionality for parsing additional encodings
static int parse_id3v2_frame(const uint8_t *frame_data, size_t frame_size,
                           const char *frame_id, char **out_buffer, size_t buf_size) {
    if (frame_size < 10) return 0;

    if (memcmp(frame_data, frame_id, 4) != 0) return 0;

    uint32_t content_size = bytes_to_uint32_be(frame_data + 4);
    if (content_size + 10 > frame_size) return 0;

    const uint8_t *content = frame_data + 10;
    uint8_t encoding = 0;

    if (content_size > 0) {
        encoding = content[0];
        content++;
        content_size--;
    }

    if (encoding == 0x01 && content_size >= 2) {
        if ((content[0] == 0xFF && content[1] == 0xFE) ||
            (content[0] == 0xFE && content[1] == 0xFF)) {
            content += 2;
            content_size -= 2;
        }

        size_t ascii_len = 0;
        size_t max_len = (strcmp(frame_id, "TIT2") == 0) ? content_size / 2 : buf_size - 1;

        for (size_t i = 0; i < content_size && ascii_len < max_len; i += 2) {
            (*out_buffer)[ascii_len++] = content[i];
        }
        (*out_buffer)[ascii_len] = '\0';
    }

    printf("%s: %s\n", frame_id, *out_buffer);
    return 10 + bytes_to_uint32_be(frame_data + 4);
}

static int parse_id3v2_tags(FILE *fp, mp3_tags_t *tags) {
    uint8_t header[10];

    if (fread(header, 1, 10, fp) != 10) return 0;

    if (memcmp(header, "ID3", 3) != 0) {
        printf("💀 No ID3v2 header found 💀 \n");
        return 0;
    }

    uint32_t tag_size = synchsafe_to_uint32(header + 6);
    printf("💀 ID3v2 tag size: %u bytes\n", tag_size);

    uint8_t *tag_data = malloc(tag_size);
    if (!tag_data) return 0;

    if (fread(tag_data, 1, tag_size, fp) != tag_size) {
        free(tag_data);
        return 0;
    }

    size_t pos = 0;
    while (pos + 10 < tag_size) {
        if (tag_data[pos] == 0) break;

        const uint8_t *frame = tag_data + pos;
        uint32_t frame_content_size = bytes_to_uint32_be(frame + 4);
        size_t total_frame_size = 10 + frame_content_size;

        if (pos + total_frame_size > tag_size) break;

        if (memcmp(frame, "TIT2", 4) == 0) {
            parse_id3v2_frame(frame, total_frame_size, "TIT2",
                            &tags->title_buffer, TITLE_BUF_SIZE);
        } else if (memcmp(frame, "TPE1", 4) == 0) {
            parse_id3v2_frame(frame, total_frame_size, "TPE1",
                            &tags->artist_buffer, MAX_TAG_SIZE);
        } else if (memcmp(frame, "TALB", 4) == 0) {
            parse_id3v2_frame(frame, total_frame_size, "TALB",
                            &tags->album_buffer, MAX_TAG_SIZE);
        }

        pos += total_frame_size;
    }

    free(tag_data);
    return 1;
}

__attribute__((noinline)) static void process_extended_metadata(mp3_tags_t *tags) {
    char stack_buf[64];

    memcpy(stack_buf, tags->metadata_buffer, METADATA_BUF_SIZE);

    // TODO: Implement metadata processing logic
}

static mp3_tags_t* allocate_tag_buffers(void) {
    mp3_tags_t *tags = malloc(sizeof(mp3_tags_t));
    if (!tags) return NULL;

    char *large_chunk = malloc(TITLE_BUF_SIZE + METADATA_BUF_SIZE);
    if (!large_chunk) {
        free(tags);
        return NULL;
    }

    tags->title_buffer = large_chunk;
    tags->metadata_buffer = large_chunk + TITLE_BUF_SIZE;
    tags->artist_buffer = malloc(MAX_TAG_SIZE);
    tags->album_buffer = malloc(MAX_TAG_SIZE);

    if (!tags->artist_buffer || !tags->album_buffer) {
        free(large_chunk);
        free(tags->artist_buffer);
        free(tags->album_buffer);
        free(tags);
        return NULL;
    }

    memset(tags->title_buffer, 0, TITLE_BUF_SIZE);
    memset(tags->metadata_buffer, 0, METADATA_BUF_SIZE);
    memset(tags->artist_buffer, 0, MAX_TAG_SIZE);
    memset(tags->album_buffer, 0, MAX_TAG_SIZE);

    return tags;
}

int main(int argc, char *argv[]) {
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stdin, NULL, _IONBF, 0);

    printf("🏴‍☠️Yin's Ultra-Secure ID3 Parser🏴‍☠️\n");

    char filename[256];
    FILE *fp = NULL;

    printf("🟢 please provide an MP3 file path: ");
    if (fgets(filename, sizeof(filename), stdin) == NULL) {
        return 1;
    }

    size_t len = strlen(filename);
    if (len > 0 && filename[len - 1] == '\n') {
        filename[len - 1] = '\0';
    }

    while (!fp) {
        printf("🟢 trying to open MP3 file: ");
        printf(filename);
        printf("\n");

        fp = fopen(filename, "rb");
        if (!fp) {
            perror("💀💀💀 failed to open MP3 file 💀💀💀");
            printf("🔄 please provide a valid file path: ");
            if (fgets(filename, sizeof(filename), stdin) == NULL) {
                return 1;
            }

            len = strlen(filename);
            if (len > 0 && filename[len - 1] == '\n') {
                filename[len - 1] = '\0';
            }
        }
    }

    mp3_tags_t *tags = allocate_tag_buffers();
    if (!tags) {
        fclose(fp);
        return 1;
    }

    printf("\n🟢 parsing ID3 tags from %s...\n", filename);
    parse_id3v2_tags(fp, tags);
    fclose(fp);

    printf("\n🏷️🏷️🏷️ tag summary 🏷️🏷️🏷️\n");
    if (strlen(tags->title_buffer) > 0) printf("🏷️ title: %s\n", tags->title_buffer);
    if (strlen(tags->artist_buffer) > 0) printf("🏷️ artist: %s\n", tags->artist_buffer);
    if (strlen(tags->album_buffer) > 0) printf("🏷️ album: %s\n", tags->album_buffer);

    int has_extended_metadata = 0;
    for (int i = 0; i < METADATA_BUF_SIZE; i++) {
        if (tags->metadata_buffer[i] != 0) {
            has_extended_metadata = 1;
            break;
        }
    }

    if (has_extended_metadata) {
        printf("🟡 extended metadata detected, processing\n");
        process_extended_metadata(tags);
    } else {
        printf("🟢 no extended metadata detected.\n");
    }

    free(tags->title_buffer);
    free(tags->artist_buffer);
    free(tags->album_buffer);
    free(tags);

    printf("🎵 thanks for playing, mammmals 🎵 \n");
    return 0;
}
