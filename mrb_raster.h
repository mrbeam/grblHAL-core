/* Mr Beam G7N/G8N hexadecimal raster frames: dx/100 mm, S/8, F/100. */
#ifndef MRB_RASTER_H
#define MRB_RASTER_H
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#define MRB_RASTER_MAX_MOVES 24

typedef struct { uint8_t dx, intensity, feed; } mrb_raster_move_t;
typedef struct {
    bool negative;
    uint8_t count;
    mrb_raster_move_t moves[MRB_RASTER_MAX_MOVES];
} mrb_raster_frame_t;

static inline int mrb_hex_nibble (char c)
{
    if(c >= '0' && c <= '9') return c - '0';
    if(c >= 'A' && c <= 'F') return c - 'A' + 10;
    if(c >= 'a' && c <= 'f') return c - 'a' + 10;
    return -1;
}

// Validate the entire frame before exposing any move for execution.
static inline bool mrb_raster_decode (const char *line, mrb_raster_frame_t *frame)
{
    size_t length = strlen(line);
    if(length < 11 || line[0] != 'G' || (line[1] != '7' && line[1] != '8') || line[2] != 'N')
        return false;
    const char *p = line + 3;
    unsigned int count = 0;
    if(*p < '0' || *p > '9') return false;
    while(*p >= '0' && *p <= '9') {
        count = count * 10 + *p++ - '0';
        if(count > MRB_RASTER_MAX_MOVES) return false;
    }
    if(!count || *p++ != ';' || strlen(p) != count * 6) return false;
    frame->negative = line[1] == '8';
    frame->count = count;
    for(unsigned int i = 0; i < count; i++) {
        uint8_t bytes[3];
        for(unsigned int j = 0; j < 3; j++) {
            int hi = mrb_hex_nibble(*p++), lo = mrb_hex_nibble(*p++);
            if(hi < 0 || lo < 0) return false;
            bytes[j] = (hi << 4) | lo;
        }
        if(!bytes[0] || !bytes[2]) return false;
        frame->moves[i] = (mrb_raster_move_t){bytes[0], bytes[1], bytes[2]};
    }
    return true;
}
#endif
