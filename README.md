# C library for the Diffbot API

libdiffbot is a C library for the [Diffbot API](http://www.diffbot.com/products/automatic/). Currently it supports Article, Frontpage, Product, Image and Classifier (Analyze) APIs. 

## Requirements

The following libraries are needed:

* [libcurl](http://curl.haxx.se/libcurl/)
* [libjson-c](https://github.com/json-c/json-c)

On Fedora Core distribution both libraries are available as rpm packages and can be installed using following commands:

    sudo yum install libcurl-devel
    sudo yum install json-c-devel

## Installation

To compile diffbot library, run:

    make all

To install it run (note that root privileges will be needed to accomplish this step):

    make install 

## Initialization

To use and initialize diffbot library in your program include header:

    include <diffbot.h>

Call initialization function:

    struct Diffbot *df;
    df = diffbotInit();

Now the library is ready to configure and send requests. When you're done call:

    diffbotFinish(df);

## Usage

Main design goal for diffbot C  library was to make it as easy as possible to use. Below one can find several examples that will help you start using it. 

### Example 1: querying Article API

Init library:

    struct Diffbot *df = diffbotInit();

Configure query:

```
diffbotSetTimeout(df, 7000);

diffbotSetFields(df, FIELD_LINKS | FIELD_TAGS | FIELD_QUERYSTRING | FIELD_URL);

diffbotSetImagesFields(df, IMAGES_FIELD_ALL);

diffbotSetVideosFields(df, VIDEOS_FIELD_PIXELHEIGHT |
                           VIDEOS_FIELD_PRIMARY |
                           VIDEOS_FIELD_PIXELWIDTH);
```

Perform query:

    const char *token = "<use_your_token_here>";
    const char *url = "<your_url_of_choice_to_inspect>";
    diffbotJasonObj *response = diffbotRequest(df, url, token, API_ARTICLE, 2);

When you're done uninitialize library:

    diffbotFinish(df);

This code will generate following query:

    https://api.diffbot.com/v2/article?token=...&url=...&timeout=7000&fields=querystring,links,tags,url,images(*),videos(pixelheight,pixelwidth,primary)

For detailed available parameters consult [Article API docs](http://www.diffbot.com/products/automatic/article/).

### Example 2: querying Analyze API

Init library:

    struct Diffbot *df = diffbotInit();

Configure query:

```
diffbotSetFields(df, FIELD_ALL);

diffbotSetMode(df, API_FRONTPAGE);

diffbotSetStats(df, DF_TRUE);

```

Perform query:

    const char *token = "<use_your_token_here>";
    const char *url = "<your_url_of_choice_to_inspect>";
    diffbotJasonObj *response = diffbotRequest(df, url, token, API_ANALYZE, 2);

When you're done uninit library:

    diffbotFinish(df);

This code will generate following query:

    https://api.diffbot.com/v2/analyze?token=...&url=...&mode=frontpage&stats&fields=*

For detailed available parameters consult [Analyze API docs](http://www.diffbot.com/products/automatic/classifier/).

### Example 3: querying Product API

Init library:

    struct Diffbot *df = diffbotInit();

Configure query:

```
diffbotSetFields(df, FIELD_META);

diffbotSetProductsFields(df, PRODUCTS_FIELD_TITLE |
                             PRODUCTS_FIELD_BRAND |
                             PRODUCTS_FIELD_UPC);

diffbotSetMediaFields(df, MEDIA_FIELD_ALL);

```

Perform query:

    const char *token = "<use_your_token_here>";
    const char *url = "<your_url_of_choice_to_inspect>";
    diffbotJasonObj *response = diffbotRequest(df, url, token, API_PRODUCT, 2);

When you're done uninit library:

    diffbotFinish(df);

This code will generate following query:

    https://api.diffbot.com/v2/product?token=...&url=...&fields=meta,products(title,brand,upc,media(*))


For detailed available parameters consult [Product API docs](http://www.diffbot.com/products/automatic/product/).

-Initial commit by Mariusz Ziulek-
