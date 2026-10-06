#include <assert.h>
#include <stdio.h>
#include "mrb_raster.h"
#include "mrb_checksum.h"
int main(void) {
    mrb_raster_frame_t f;
    assert(mrb_raster_decode("G7N02;FF7D10010001", &f));
    assert(!f.negative && f.count == 2 && f.moves[0].dx == 255 && f.moves[0].intensity == 125 && f.moves[0].feed == 16);
    assert(mrb_raster_decode("G8N01;ff0001", &f) && f.negative);
    const char *bad[] = {"", "G7", "G7N", "G7N00;", "G7N25;010101", "G7N01;01010", "G7N01;010101AA", "G7N02;01010101010Z", "G7N01;000101", "G7N01;010100", "G7N999999999999;010101"};
    for(unsigned int i=0; i<sizeof(bad)/sizeof(*bad); i++) assert(!mrb_raster_decode(bad[i], &f));
    const char *payload="G7N01;FF7D10";
    mrb_checksum_rx_t rx={0};
    for(const char *p=payload; *p; p++) assert(!mrb_checksum_feed(&rx,*p));
    unsigned int sum=rx.sum;
    char trailer[8]; snprintf(trailer,sizeof(trailer),"*%u",sum);
    for(const char *p=trailer; *p; p++) assert(mrb_checksum_feed(&rx,*p));
    assert(mrb_checksum_valid(&rx,true));
    rx.sum++; assert(!mrb_checksum_valid(&rx,true));
    puts("Raster decoding and protected semicolon payload tests passed.");
}
