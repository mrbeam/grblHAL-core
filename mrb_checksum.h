/* Mr Beam additive serial checksum, enabled by the first checksum trailer.
 * Sum raw bytes before '*' modulo 256, ignoring ASCII spaces. Real-time bytes
 * are consumed by the stream driver and never enter this parser.
 */
#ifndef MRB_CHECKSUM_H
#define MRB_CHECKSUM_H

#include <stdbool.h>
#include <stdint.h>

typedef struct {
    uint8_t sum;
    uint16_t expected;
    bool trailer, digits, invalid, payload, parentheses, semicolon, raster;
    uint8_t prefix_length;
    char prefix[3];
} mrb_checksum_rx_t;

// Returns true for trailer characters that must not enter the G-code buffer.
static inline bool mrb_checksum_feed (mrb_checksum_rx_t *rx, uint8_t c)
{
    if(rx->trailer) {
        if(c >= '0' && c <= '9') {
            rx->digits = true;
            if(rx->expected > 25 || (rx->expected == 25 && c > '5'))
                rx->invalid = true;
            if(!rx->invalid)
                rx->expected = rx->expected * 10 + c - '0';
        } else if(c != ' ')
            rx->invalid = true;
        return true;
    }
    if(c == '*' && !rx->parentheses && (!rx->semicolon || rx->raster)) {
        rx->trailer = true;
        return true;
    }
    if(c != ' ' && rx->prefix_length < 3) {
        rx->prefix[rx->prefix_length++] = c >= 'a' && c <= 'z' ? c - ('a' - 'A') : c;
        rx->raster = rx->prefix_length == 3 && rx->prefix[0] == 'G' &&
                     (rx->prefix[1] == '7' || rx->prefix[1] == '8') && rx->prefix[2] == 'N';
    }
    if(c != ' ') {
        rx->sum += c;
        rx->payload = true;
    }
    if(c == '(' && !rx->semicolon)
        rx->parentheses = true;
    else if(c == ')' && !rx->semicolon)
        rx->parentheses = false;
    else if(c == ';' && !rx->parentheses)
        rx->semicolon = true;
    return false;
}

static inline bool mrb_checksum_valid (const mrb_checksum_rx_t *rx, bool enabled)
{
    // Empty CR/LF lines remain valid synchronization requests.
    return !enabled || (!rx->payload && !rx->trailer) ||
           (rx->trailer && rx->digits && !rx->invalid && rx->sum == rx->expected);
}

#endif
