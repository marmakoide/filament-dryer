#ifndef FILAMENT_DRYER_STRINGSTREAM_H
#define FILAMENT_DRYER_STRINGSTREAM_H

/*
 * String stream handling aka we are too poor to afford printf
 *
 * Some assumptions are made to keep things simple
 *   - We don't care if the string does not end with '/0'
 *   - No output size check is made, you are an adult
 */

#include <stdint.h>


struct StringStream {
	char* ptr;
	uint8_t mask;
}; // struct StringStream



extern void
StringStream_init(struct StringStream* self,
                  char* target);


extern void
StringStream_enable_inverse_mode(struct StringStream* self);


extern void
StringStream_disable_inverse_mode(struct StringStream* self);


extern void
StringStream_push_char(struct StringStream* self,
                       char c);


extern void
StringStream_push_nchar(struct StringStream* self,
                        char c,
                        uint8_t count);

extern void
StringStream_push_uint8(struct StringStream* self,
                        uint8_t value,
                        uint8_t digit_count);


#endif /* FILAMENT_DRYER_STRINGSTREAM_H */