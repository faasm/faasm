#include <stdio.h>

#include <curl/curl.h>
#include <curl/easy.h>
//#include <curl/curlbuild.h>

int main() {
    void *curl = curl_easy_init();

    curl_easy_setopt(curl, CURLOPT_URL, "www.google.com");

    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
    curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "deflate");

    CURLcode res = curl_easy_perform(curl);

    if (res != CURLE_OK) {
        printf("curl_easy_perform() failed \n");
        return 1;
    }

    return 0;
}
