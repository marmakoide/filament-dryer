#include "stringstream.h"

#define STRINGSTREAM_VIDEO_NORMAL_MASK 0x00
#define STRINGSTREAM_VIDEO_INVERSE_MASK 0x80


void
StringStream_init(struct StringStream* self,
                  char* target) {
	self->ptr = target;
	self->mask = STRINGSTREAM_VIDEO_NORMAL_MASK;
}


void
StringStream_enable_inverse_mode(struct StringStream* self) {
	self->mask = STRINGSTREAM_VIDEO_INVERSE_MASK;
}


void
StringStream_disable_inverse_mode(struct StringStream* self) {
	self->mask = STRINGSTREAM_VIDEO_NORMAL_MASK;
}


void
StringStream_push_char(struct StringStream* self,
                       char c) {
    *(self->ptr) = c | self->mask;
	self->ptr += 1;
}


void
StringStream_push_nchar(struct StringStream* self,
                        uint8_t count,
                        char c) {
    char masked_c = c | self->mask;

	for( ; count != 0; --count) {
    	*(self->ptr) = masked_c;
		self->ptr += 1;
	}
}


void
StringStream_push_uint8(struct StringStream* self,
                        uint8_t value,
                        uint8_t digit_count) {
    // Digits conversion
	char decimal_digits[3];
	for(uint8_t i = 0; i < 3; ++i, value /= 10)
		decimal_digits[i] = value % 10;

	// Push the characters
	for(uint8_t i = digit_count; i != 0; --i)
		StringStream_push_char(self, '0' + decimal_digits[i - 1]);
}