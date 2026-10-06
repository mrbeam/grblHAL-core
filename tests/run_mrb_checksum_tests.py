"""Compile host tests for checksum parsing and the actual firmware reporters."""
from pathlib import Path
import os
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]

def function(source, name):
    marker=source.index(' '+name+' (')
    start=source.rfind('FLASHMEM static ',0,marker)
    brace=source.index('{',marker); depth=1; end=brace+1
    while depth:
        depth += (source[end]=='{')-(source[end]=='}'); end+=1
    return source[start:end]

source=(ROOT/'report.c').read_text()
stubs=r'''
#include <stdarg.h>
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#define FLASHMEM
#define ASCII_EOL "\r\n"
typedef int status_code_t;
typedef int alarm_code_t;
enum { Status_OK=0, Status_Handled=-1, Alarm_ChecksumFail=22 };
bool mrb_checksum_response;
uint8_t mrb_checksum;
const char *mrb_checksum_failed_line;
static char output[2048];
static bool connected(void) { return true; }
static void write_text(const char *text) { strcat(output,text); }
static void delay(unsigned int ms, void *callback) { (void)ms; (void)callback; }
static struct { struct {bool (*is_connected)(void); void (*write)(const char *); void (*write_all)(const char *);} stream; void (*delay_ms)(unsigned int,void *); } hal={{connected,write_text,write_text},delay};
static char *uitoa(uint32_t value) { static char buf[32]; snprintf(buf,sizeof(buf),"%u",value); return buf; }
static char *appendbuf(int count, ...) { static char buf[256]; buf[0]=0; va_list ap; va_start(ap,count); while(count--) strcat(buf,va_arg(ap,const char *)); va_end(ap); return buf; }
'''
main=r'''
int main(void) {
    parser_tests();
    report_status_message(Status_OK); assert(!strcmp(output,"ok\r\n")); output[0]=0;
    mrb_checksum_response=true; mrb_checksum=2;
    report_status_message(Status_OK); assert(!strcmp(output,"ok *2\r\n")); output[0]=0;
    report_status_message(9); assert(!strcmp(output,"error:9 *2\r\n")); output[0]=0;
    mrb_checksum_response=false;
    report_status_message(9); assert(!strcmp(output,"error:9\r\n")); output[0]=0;
    mrb_checksum_failed_line="G1X2";
    report_alarm_message(Alarm_ChecksumFail); assert(!strcmp(output,"ALARM: MRB_CHECKSUM_ERROR in G1X2\r\n")); output[0]=0;
    report_alarm_message(8); assert(!strcmp(output,"ALARM:8\r\n"));
    puts("Firmware acknowledgement and alarm report tests passed.");
    return 0;
}
'''
parser=(ROOT/'tests/test_mrb_checksum.c').read_text().replace('int main(void)', 'int parser_tests(void)')
# parser_tests is a regular function, so add an explicit success return.
parser=parser[:parser.rfind('}')]+ '    return 0;\n}\n'
with tempfile.TemporaryDirectory(prefix='mrb-checksum-') as directory:
    path=Path(directory); (path/'test.c').write_text(stubs+parser+function(source,'report_status_message')+function(source,'report_alarm_message')+main)
    subprocess.run(['cc','-std=c11','-Wall','-Wextra','-Werror','-fsanitize=address,undefined','-I',str(ROOT),str(path/'test.c'),'-o',str(path/'test')],check=True)
    subprocess.run([str(path/'test')],check=True,env={**os.environ,'ASAN_OPTIONS':'detect_leaks=0'})
