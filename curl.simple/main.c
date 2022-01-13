#include <stdio.h>
#include <curl/curl.h>

int main() {

  CURL *curl;
  CURLcode res;

  curl = curl_easy_init();

  if (curl) {
    curl_easy_setopt(curl, CURLOPT_URL, "https://example.com");
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
      fprintf(stderr, "Failed to load: %s\n", curl_easy_strerror(res));
    } else {
      
    }


    curl_easy_cleanup(curl);
  }

  return 0;
}

