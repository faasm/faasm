#include "faasm.h"

#include <stdio.h>

#include <curl/curl.h>
#include <curl/easy.h>

/**
 * Example of HTTP request
 */

size_t write_data(void *buffer, size_t size, size_t nmemb, void *userp)
{
    return size * nmemb;
}

EMSCRIPTEN_KEEPALIVE
int exec(struct FaasmMemory *memory) {
    printf("Before curl init");

    void *curl = curl_easy_init();
    printf("Initialised curl");

    curl_easy_setopt(curl, CURLOPT_URL, "www.google.com");

    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
    curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "deflate");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);

    CURLcode res = curl_easy_perform(curl);

    if (res != CURLE_OK) {
        printf("curl_easy_perform() failed - %d", res);
    }

    curl_easy_cleanup(curl);
    curl_global_cleanup();

    return 0;
}
