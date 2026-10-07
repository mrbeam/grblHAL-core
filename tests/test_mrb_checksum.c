#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "mrb_checksum.h"

static mrb_checksum_rx_t parse(const char *line) {
    mrb_checksum_rx_t rx={0};
    for(const unsigned char *c=(const unsigned char *)line; *c; c++)
        mrb_checksum_feed(&rx,*c);
    return rx;
}
static void check(const char *line, bool enabled, bool valid) {
    mrb_checksum_rx_t rx=parse(line);
    assert(mrb_checksum_valid(&rx,enabled || rx.trailer)==valid);
}
int main(void) {
    check("G1 X2*2",false,true);
    check("G1 X2*3",false,false);
    check("G1 X2",false,true);
    check("G1 X2",true,false);
    check("",true,true);
    check("    ",true,true);
    check("*0",true,true);
    check("G1 X2*",true,false);
    check("G1 X2*-2",true,false);
    check("G1 X2*258",true,false);
    check("G1 X2*65538",true,false);
    check("G1 X2*2junk",true,false);
    check("G1 X2*2*2",true,false);
    check("G1 X2*002   ",true,true);
    check("g1 x2*66",true,true);
    check("$X*124",true,true);
    mrb_checksum_rx_t rx=parse("G1(comment*text)X2");
    assert(!rx.trailer);
    char line[128];
    snprintf(line,sizeof(line),"G1(comment*text)X2*%u",rx.sum);
    check(line,true,true);
    rx=parse("G1X2;comment*text"); assert(!rx.trailer);
    rx=parse("G1\tX2"); assert(rx.sum==11);
    rx=parse("G1X2*2");
    bool enabled=rx.trailer;
    assert(mrb_checksum_valid(&rx,enabled));
    rx=parse("$X"); assert(!mrb_checksum_valid(&rx,enabled));
    rx=parse("$X*124"); assert(mrb_checksum_valid(&rx,enabled));
    enabled=false; rx=parse("$X"); assert(mrb_checksum_valid(&rx,enabled));
    puts("MRB checksum tests passed (activation, valid/corrupt/missing trailers, overflow, comments, reset and recovery).");
}
