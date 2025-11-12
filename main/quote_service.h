#ifndef FACT_SERVICE_H
#define FACT_SERVICE_H

#include <stdbool.h>
#include <stddef.h>

// API для случайных цитат
#ifndef QUOTE_API_URL
#define QUOTE_API_URL "http://api.quotable.io/random"
#endif

#define MAX_QUOTE_LENGTH 512
#define MAX_AUTHOR_LENGTH 128

typedef struct {
    char content[MAX_QUOTE_LENGTH];
    char author[MAX_AUTHOR_LENGTH];
    char tags[128];
    int length;
    bool is_fallback;
} quote_data_t;

#ifdef __cplusplus
extern "C" {
#endif

void quote_service_init(void);
bool get_random_quote(quote_data_t* quote);
bool get_random_quote_simple(char* quote_buffer, size_t buffer_size);
bool is_quote_api_configured(void);
void quote_service_cleanup(void);
void update_quote_status(const char* status, bool is_error);
void update_quote_text(const char* quote_text);
void clean_quote_text(const char* input, char* output, size_t output_size);
bool get_fallback_quote(char* quote_buffer, size_t buffer_size);

#ifdef __cplusplus
}
#endif

#endif