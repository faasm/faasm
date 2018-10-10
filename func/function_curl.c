#include "faasm.h"

#include <stdio.h>
#include <curl/curl.h>

/**
 * Example of HTTP request with libcurl
 */

int exec(struct FaasmMemory *memory) {
    char* url = (char*) memory->input;

    printf("CURL-ing %s\n", url);

    CURL *curl = curl_easy_init();
    if(!curl) {
        printf("Curl initialisation failed\n");
    }

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    CURLcode res = curl_easy_perform(curl);

    if(res == CURLE_OK) {
        printf("Curl success\n");
    }
    else {
        printf("Curl failed: %s\n", curl_easy_strerror(res));
    }

    curl_easy_cleanup(curl);

    return 0;
}
