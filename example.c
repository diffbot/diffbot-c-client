
#include "diffbot.h"
#include <stdio.h>

int main()
{
    const char *url1 = "http://softwareflaws.blogspot.com/2013/11/threat-update-10-setting-up-air-gapped_9346.html";
    const char *url2 = "http://softwareflaws.blogspot.com/2014/01/exercises-for-csirts.html";
    const char *url = "http://softwareflaws.blogspot.com/2013/11/autumn-hiking-in-owl-mountains.html";
    const char *token = "c49f2c29f4c5c2de895437709e05ce58";
    diffbotJasonObj *response;
    struct Diffbot *df;

    df = diffbotInit();

    diffbotSetTimeout(df, 7000);
    diffbotSetFields(df, FIELD_META | FIELD_TAGS);

    diffbotSetImagesFields(df, IMAGES_FIELD_URL |
                               IMAGES_FIELD_CAPTION |
                               IMAGES_FIELD_PIXELWIDTH);

    // query article API
    response = diffbotRequest(df, url, token, API_ARTICLE, 2); 
    if(response == NULL) {
        fprintf(stderr, "Request failed\n");
        return 1;
    }

    // print json object structure
    //diffbotJsonPrint(df, response);

    // print "date" from Article
    //diffbotJasonObj *jobj_date = diffbotJsonGetSubObj(response, "date");
    //printf("date: %s\n", diffbotJsonGetString(jobj_date));

    diffbotFinish(df);

    return 0;
}
