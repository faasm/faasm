#include "faasm.h"

#include <stdio.h>

#include <curl/curl.h>

/**
 * Example of HTTP request
 */

size_t write_data(void *buffer, size_t size, size_t nmemb, char *data)
{
    printf("DATA - %s\n", data);
    return size * nmemb;
}

int exec(struct FaasmMemory *memory) {
    printf("Before curl init\n");

    CURL *curl = curl_easy_init();
    printf("Initialised curl\n");

    char *url = "http://www.google.com";
    curl_easy_setopt(curl, CURLOPT_URL, url);

    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
    curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "deflate");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");

    CURLcode res = curl_easy_perform(curl);

    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

    if(CURLE_OK == res) {
        printf("%s response = %ld\n", url, http_code);
    }
    else {
        printf("%s response failed %ld\n", url, http_code);
    }

    curl_easy_cleanup(curl);
    curl_global_cleanup();

    return 0;
}
