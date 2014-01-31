
#include "diffbot.h"
#include <string.h>
#include <stdlib.h>
#include <curl/curl.h>

#define NOT_SET -1
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

#define NUMBER_OF_APIS 5 
#define NUMBER_OF_FIELDS 19 

const char *apiLink = "http://api.diffbot.com/v2/";
const char *apiNameStrings[] = { "article", "frontpage", "product", "image", "analyze" };

const char *apiFields2[] = { "meta", "querystring", "links", "tags", "*", "url", "resolved_url", "icon", "type", "title", "text", "html", "numPages", "date", "author", "breadcrumb", "nextPage", "albumUrl", "humanLanguage" };

// nested fields
const char *articlesImagesFields[] = { "url", "pixelheight", "pixelwidth", "caption", "primary", "*", "anchorUrl", "mime", "attrTitle", "date", "size", "displayHeight", "displayWidth", "meta", "faces", "ocr", "colors", "xpath", "attrAlt" };
const char *articlesVideosFields[] = { "url", "pixelheight", "pixelwidth", "primary" };
const char *productsProductsFields[] = { "title", "descreption", "brand", "officePrice", "regularPrice", "saveAmount", "shippingAmount", "productId", "upc", "prefixCode", "preoductOrigin", "isbn", "sku", "mpn", "availability", "brand", "quantifyPrices", "*" };
const char *productsMediaFields[] = { "type", "link", "height", "width", "caption", "primary", "xpath" };

//meta,querystring,links,tags,all,humanLanguage,url,resolved_url,icon,type,title,text,html,numPages,date,author,breadcrumb,nextPage,albumUrl
unsigned int fieldsMatrix[NUMBER_OF_APIS][NUMBER_OF_FIELDS] = {
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0}, // Article API
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, // Frontpage API
    {1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0}, // Product API
    {1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 1}, // Image API
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}, // Analyze API
};

struct Diffbot
{
	unsigned int fields;
	int timeout;
	
	// article specific parameters
	unsigned int images_fields;
	unsigned int videos_fields;

    // frontpage specific parameters
	enum FRONTPAGE_FORMAT format;
	enum BOOL all;

    // product specific parameters
	unsigned int products_fields;
	unsigned int media_fields;

	// analyze specific parameters
	enum BOOL stats;
	enum API mode;

	// basic auth credentials
	char *user;
	char *password;
};

struct ResponseData
{
    size_t data_len;
    char *data;
};

static void print_json_value(json_object *jobj){
    enum json_type type;
    type = json_object_get_type(jobj);
    switch (type) {
        case json_type_boolean: printf("type: json_type_boolean\n");
                                printf("value: %s\n", json_object_get_boolean(jobj)? "true": "false");
                                break;
        case json_type_double: printf("type: json_type_double\n");
                               printf("value: %lf\n", json_object_get_double(jobj));
                               break;
        case json_type_int: printf("type: json_type_int\n");
                            printf("value: %d\n", json_object_get_int(jobj));
                            break;
        case json_type_string: printf("type: json_type_string\n");
                               printf("value: %s\n", json_object_get_string(jobj));
                               break;
    }

}

static void json_parse_array(struct Diffbot *df, json_object *jobj, char *key) {
    enum json_type type;

    json_object *jarray = jobj;
    if(key) {
        jarray = json_object_object_get(jobj, key);
    }

    int arraylen = json_object_array_length(jarray);
    printf("Array Length: %dn",arraylen);
    int i;
    json_object * jvalue;

    for (i=0; i< arraylen; i++){
        jvalue = json_object_array_get_idx(jarray, i);
        type = json_object_get_type(jvalue);
        if (type == json_type_array) {
            json_parse_array(df, jvalue, NULL);
        }
        else if (type != json_type_object) {
            printf("value[%d]: ",i);
            print_json_value(jvalue);
        }
        else {
            diffbotJsonPrint(df, jvalue);
        }
    }
}

void diffbotJsonPrint(struct Diffbot *df, json_object * jobj) {
    enum json_type type;
    json_object_object_foreach(jobj, key, val) {
        type = json_object_get_type(val);
        switch (type) {
            case json_type_boolean: 
            case json_type_double: 
            case json_type_int: 
            case json_type_string: print_json_value(val);
                                   break; 
            case json_type_object: printf("type: json_type_object\n");
                                   jobj = json_object_object_get(jobj, key);
                                   diffbotJsonPrint(df, jobj); 
                                   break;
            case json_type_array: printf("type: json_type_array, ");
                                  json_parse_array(df, jobj, key);
                                  break;
        }
    }
}

diffbotJasonObj *diffbotJasonGetSubObj(diffbotJasonObj *jobj, const char *key)
{
    return json_object_object_get(jobj, key);
}

const char *diffbotJasonGetString(diffbotJasonObj *jobj)
{
    return (json_object_get_string(jobj));
}

struct Diffbot *diffbotInit()
{
	struct Diffbot *df = (struct Diffbot *)malloc(sizeof(struct Diffbot));
	if(df == NULL)
		return NULL;

	df->fields = 0;

	df->images_fields = 0;
	df->videos_fields = 0;
	df->timeout = NOT_SET;

	df->products_fields = 0;
	df->media_fields = 0;

	df->format = NOT_SET;
	df->all = NOT_SET;

	df->stats = NOT_SET;
	df->mode = NOT_SET;

	df->user = NULL;
	df->password = NULL;

	return df;
}

void diffbotFinish(struct Diffbot *df)
{
	if(df->user)
		free(df->user);
	if(df->password)
		free(df->password);
	free(df);
}

void diffbotSetTimeout(struct Diffbot *df, unsigned int milisec)
{
    df->timeout = milisec;
}

void diffbotSetFields(struct Diffbot *df, unsigned int fieldsBitMask)
{
    df->fields = fieldsBitMask;
}

void diffbotSetImagesFields(struct Diffbot *df, unsigned int imagesFieldsBitMask)
{
    df->images_fields = imagesFieldsBitMask;
}

void diffbotSetVideosFields(struct Diffbot *df, unsigned int videosFieldsBitMask)
{
    df->videos_fields = videosFieldsBitMask;
}

void diffbotSetProductsFields(struct Diffbot *df, unsigned int productsFieldsBitMask)
{
    df->products_fields = productsFieldsBitMask;
}

void diffbotSetMediaFields(struct Diffbot *df, unsigned int mediaFieldsBitMask)
{
    df->media_fields = mediaFieldsBitMask;
}

void diffbotSetMode(struct Diffbot *df, enum API mode)
{
    df->mode = mode;
}

void diffbotSetStats(struct Diffbot *df, enum BOOL stats)
{
    df->stats = stats;
}

void diffbotSetFormat(struct Diffbot *df, enum FRONTPAGE_FORMAT format)
{
    df->format = format;
}

void diffbotSetAll(struct Diffbot *df, enum BOOL all)
{
    df->all = all;
}

static size_t handleData(char *ptr, size_t size, size_t nmemb, void *userdata)
{
    size_t response_len = size * nmemb;

    // resize the storage for response
    struct ResponseData *rDataPtr = (struct ResponseData *)userdata;
    rDataPtr->data = realloc(rDataPtr->data, rDataPtr->data_len + response_len + 1);
    if(rDataPtr->data == NULL) {
        // out of memory
        fprintf(stderr, "not enough memory (realloc returned NULL)\n");
        return -1;
    }

    // copy content of response to the storage
    memcpy(&(rDataPtr->data[rDataPtr->data_len]), ptr, response_len);
    rDataPtr->data_len += response_len;
    rDataPtr->data[rDataPtr->data_len] = '\0';

    return response_len;
}

static unsigned int flag2index(unsigned int flag)
{
	switch(flag) {
		case 0x0001: return 0;
		case 0x0002: return 1;
		case 0x0004: return 2;
		case 0x0008: return 3;
		case 0x0010: return 4;
		case 0x0020: return 5;
		case 0x0040: return 6;
		case 0x0080: return 7;
		case 0x0100: return 8;
		case 0x0200: return 9;
		case 0x0400: return 10;
		case 0x0800: return 11;
		case 0x1000: return 12;
		case 0x2000: return 13;
		case 0x4000: return 14;
		case 0x8000: return 15;
		case 0x00010000: return 16;
		case 0x00020000: return 17;
		case 0x00030000: return 18;
		case 0x00040000: return 19;
		case 0x00100000: return 20;
		case 0x00200000: return 21;
		case 0x00400000: return 22;
		case 0x00800000: return 23;
		case 0x01000000: return 24;
		case 0x02000000: return 25;
		case 0x04000000: return 26;
		case 0x08000000: return 27;
		case 0x10000000: return 28;
		case 0x20000000: return 29;
		case 0x40000000: return 30;
		case 0x80000000: return 31;
	}

	// error
	return 0;
}

static int isFieldValid(enum API api, unsigned int flag)
{
    return fieldsMatrix[(int)api][flag2index(flag)];
}

static unsigned int calcTimeoutLen(struct Diffbot *df, enum API api)
{
    if(df->timeout != NOT_SET)
        return 16;

    return 0;
}

static unsigned int calcFieldsLen(struct Diffbot *df, enum API api)
{
    unsigned int fields_len = 0;
    unsigned int CURRENT_FLAG;
    unsigned int index;
    int i;

    if(df->fields & FIELD_ALL) {
        fields_len += 2;
        return fields_len;
    }

    CURRENT_FLAG = 0x00000001;
    for(i=0; i<ARRAY_SIZE(apiFields2); ++i) {
        if(df->fields & CURRENT_FLAG) {
            index = flag2index(CURRENT_FLAG);

            // check if CURRENT_FLAG is applicable for current API
            if(!isFieldValid(api, CURRENT_FLAG)) {
                CURRENT_FLAG <<= 1;
                continue;
            }

            fields_len += (strlen(apiFields2[index]) + 1);
        }
        CURRENT_FLAG <<= 1;
    }

    return fields_len;
}

static unsigned int calcOptParamsLen(struct Diffbot *df, enum API api)
{
    unsigned opt_len = 0;
    int i;
    unsigned int CURRENT_FLAG;
    unsigned int index; 

    switch(api) {
        case API_ARTICLE:
            // process timeout
            opt_len += calcTimeoutLen(df, api);
            
            // process fields
            if(df->fields > 0 || df->images_fields > 0 || df->videos_fields > 0)
                opt_len += 8; // "&fields="

            if(df->fields & FIELD_ALL) {
				opt_len += 2;
                return opt_len;
            }

            CURRENT_FLAG = 0x00000001;
            for(i=0; i<ARRAY_SIZE(apiFields2); ++i) {
                if(df->fields & CURRENT_FLAG) {
                    index = flag2index(CURRENT_FLAG);

                    // check if CURRENT_FLAG is applicable for current API
                    if(!isFieldValid(api, CURRENT_FLAG)) {
                        CURRENT_FLAG <<= 1;
                        continue;
                    }

                    opt_len += (strlen(apiFields2[index]) + 1);
                }
                CURRENT_FLAG <<= 1;
            }

            // process images fields
            if(df->images_fields != 0) {
                int skip = 0;
                opt_len += 8; // ",images("
                if(df->images_fields & IMAGES_FIELD_ALL) {
                    opt_len += 2; // "*)"
                    skip = 1;
                }

                CURRENT_FLAG = 0x00000001;
                for(i=0; i<ARRAY_SIZE(articlesImagesFields) && !skip; ++i) {
                    if(df->images_fields & CURRENT_FLAG) {
                        index = flag2index(CURRENT_FLAG);
                        opt_len += (strlen(articlesImagesFields[index]) + 1);
                    }
                    CURRENT_FLAG <<= 1;
                }
            }

            // process videos fields
            if(df->videos_fields != 0) {
                int skip = 0;
                opt_len += 8; // ",videos("
                if(df->videos_fields & VIDEOS_FIELD_ALL) {
                    opt_len += 2; // "*)"
                    skip = 1;
                }

                CURRENT_FLAG = 0x00000001;
                for(i=0; i<ARRAY_SIZE(articlesVideosFields) && !skip; ++i) {
                    if(df->videos_fields & CURRENT_FLAG) {
                        index = flag2index(CURRENT_FLAG);
                        opt_len += (strlen(articlesVideosFields[index]) + 1);
                    }
                    CURRENT_FLAG <<= 1;
                }
            }

            break;
        case API_PRODUCT:
            // process timeout
            opt_len += calcTimeoutLen(df, api);

            // process fields
            if(df->fields > 0 || df->products_fields > 0 || df->media_fields > 0)
                opt_len += 8; // "&fields="

            opt_len += calcFieldsLen(df, api);
            
            if(df->fields & FIELD_ALL)
                return opt_len;
            
            // process products
            if(df->products_fields != 0) {
                int skip = 0;
                opt_len += 10; // ",products("
                if(df->products_fields & PRODUCTS_FIELD_ALL) {
                    opt_len += 2; // "*)"
                    skip = 1;
                }

                CURRENT_FLAG = 0x00000001;
                for(i=0; i<ARRAY_SIZE(productsProductsFields) && !skip; ++i) {
                    if(df->products_fields & CURRENT_FLAG) {
                        index = flag2index(CURRENT_FLAG);
                        opt_len += (strlen(productsProductsFields[index]) + 1);
                    }
                    CURRENT_FLAG <<= 1;
                }
            }
            
            // process media
            if((df->media_fields != 0) && !(df->products_fields & PRODUCTS_FIELD_ALL)) {
                int skip = 0;
                
                if(df->products_fields == 0)
                    opt_len += 10; // ",products("

                opt_len += 7; // ",media("
                if(df->media_fields & MEDIA_FIELD_ALL) {
                    opt_len += 2; // "*)"
                    skip = 1;
                }

                CURRENT_FLAG = 0x00000001;
                for(i=0; i<ARRAY_SIZE(productsMediaFields) && !skip; ++i) {
                    if(df->media_fields & CURRENT_FLAG) {
                        index = flag2index(CURRENT_FLAG);
                        opt_len += (strlen(productsMediaFields[index]) + 1);
                    }
                    CURRENT_FLAG <<= 1;
                }
            }
            
            break;
        case API_FRONTPAGE:
            // process timeout
            opt_len += calcTimeoutLen(df, api);

            // process format
            if(df->format == FORMAT_JSON)
                opt_len += 12; // &format=json

            // process all
            if(df->all == DF_TRUE)
                opt_len += 4; // &all

            break;
        case API_IMAGE:
            // process timeout
            opt_len += calcTimeoutLen(df, api);

            // process fields
            if(df->fields > 0 || df->images_fields > 0)
                opt_len += 8; // "&fields="

            opt_len += calcFieldsLen(df, api);
            
            if(df->fields & FIELD_ALL)
                return opt_len;

            // process images
            if(df->images_fields != 0) {
                int skip = 0;
                opt_len += 8; // ",images("
                if(df->images_fields & IMAGES_FIELD_ALL) {
                    opt_len += 2; // "*)"
                    skip = 1;
                }

                CURRENT_FLAG = 0x00000001;
                for(i=0; i<ARRAY_SIZE(articlesImagesFields) && !skip; ++i) {
                    if(df->images_fields & CURRENT_FLAG) {
                        index = flag2index(CURRENT_FLAG);
                        opt_len += (strlen(articlesImagesFields[index]) + 1);
                        printf("flag 0x%x; index: %d\n", CURRENT_FLAG, index);
                    }
                    CURRENT_FLAG <<= 1;
                }
            }

            break;
        case API_ANALYZE:

            // process mode
            if(df->mode != NOT_SET) {
                opt_len += 6; // "&mode="
                opt_len += strlen(apiNameStrings[(int)df->mode]);
            }

            // process stats
            if(df->stats == DF_TRUE)
                opt_len += 6; // "&stats"

            // process fields
            if(df->fields > 0)
                opt_len += 8; // "&fields="

            opt_len += calcFieldsLen(df, api);
            
            if(df->fields & FIELD_ALL)
                return opt_len; 

            break;
    }

    return opt_len;
}

unsigned int buildTimeoutParam(struct Diffbot *df, enum API api, char *mem)
{
    unsigned int offset = 0;
    if(df->timeout != NOT_SET) {
        char timeoutStr[16] = {0};
        snprintf(timeoutStr, sizeof(timeoutStr),"%d", df->timeout);
        memcpy(mem, "&timeout=", 9);
        offset += 9;
        mem += offset;
        memcpy(mem, timeoutStr, strlen(timeoutStr));
        offset += strlen(timeoutStr);
    }

    return offset;
}

unsigned int buildFieldsParam(struct Diffbot *df, enum API api, char *mem)
{
    unsigned int offset = 0;
    int i;
    unsigned int CURRENT_FLAG;
    unsigned int index; 

    if(df->fields != 0) {
        if(df->fields & FIELD_ALL) {
            *mem = '*';
            return 1;
        }

        CURRENT_FLAG = 0x00000001;
        for(i=0; i<ARRAY_SIZE(apiFields2); ++i) {
            if(df->fields & CURRENT_FLAG) {
                index = flag2index(CURRENT_FLAG);

                // check if CURRENT_FLAG is applicable for current API
                if(!isFieldValid(api, CURRENT_FLAG)) {
                    CURRENT_FLAG <<= 1;
                    continue;
                }

                memcpy(mem, apiFields2[index], strlen(apiFields2[index]));
                mem += strlen(apiFields2[index]);
                *mem++ = ',';
                offset += (strlen(apiFields2[index]) + 1);
            }

            CURRENT_FLAG <<= 1;
        }
        // get rid of last ','
        --offset;
    }

    return offset;
}

static char *buildOptParams(struct Diffbot *df, enum API api, unsigned int len)
{
    unsigned opt_len = 0;
    int i;
    int closeProductsLater;
    unsigned int CURRENT_FLAG;
    unsigned int index;
    unsigned int offset;
    char *ptr;
	char *optRequest = (char *)malloc(len);
	if(optRequest == NULL)
		return NULL;	

    ptr = optRequest;

	switch(api) {
		case API_ARTICLE:

            // process timeout
            ptr += buildTimeoutParam(df, api, ptr);

            // process fields
            if(df->fields > 0 || df->images_fields > 0 || df->videos_fields > 0) {
                memcpy(ptr, "&fields=", 9);
                ptr += 8;
            }
            ptr += buildFieldsParam(df, api, ptr);

            if(df->fields & FIELD_ALL)
                return optRequest;

            // process images fields
            if(df->images_fields != 0) {
                int skip = 0;
                memcpy(ptr, ",images(", 8);
                ptr += 8;

                if(df->images_fields & IMAGES_FIELD_ALL) {
                    *ptr++ = '*';
                    *ptr++ = ')';
                    skip = 1;
                }

                CURRENT_FLAG = 0x00000001;
                for(i=0; i<ARRAY_SIZE(articlesImagesFields) && !skip; ++i) {
                    if(df->images_fields & CURRENT_FLAG) {
                        index = flag2index(CURRENT_FLAG);
                        memcpy(ptr, articlesImagesFields[index], strlen(articlesImagesFields[index]));
                        ptr += strlen(articlesImagesFields[index]);
                        *ptr++ = ',';
                    }

                    CURRENT_FLAG <<= 1;
                }
                // get rid of last ','
                --ptr;
                // end of images
                *ptr++ = ')';
            }

            // process videos fields
            if(df->videos_fields != 0) {
                int skip = 0;
                memcpy(ptr, ",videos(", 8);
                ptr += 8;

                if(df->videos_fields & VIDEOS_FIELD_ALL) {
                    *ptr++ = '*';
                    *ptr++ = ')';
                    skip = 1;
                }

                CURRENT_FLAG = 0x00000001;
                for(i=0; i<ARRAY_SIZE(articlesVideosFields) && !skip; ++i) {
                    if(df->videos_fields & CURRENT_FLAG) {
                        index = flag2index(CURRENT_FLAG);
                        memcpy(ptr, articlesVideosFields[index], strlen(articlesVideosFields[index]));
                        ptr += strlen(articlesVideosFields[index]);
                        *ptr++ = ',';
                    }

                    CURRENT_FLAG <<= 1;
                }
                // get rid of last ','
                --ptr;
                // end of videos
                *ptr++ = ')';
            }
            *ptr = '\0';

            break;
        case API_PRODUCT:
            closeProductsLater = 0;

            // process timeout
            ptr += buildTimeoutParam(df, api, ptr);

            // process fields
            if(df->fields > 0 || df->products_fields > 0 || df->media_fields > 0) {
                memcpy(ptr, "&fields=", 9);
                ptr += 8;
            }
            ptr += buildFieldsParam(df, api, ptr);

            if(df->fields & FIELD_ALL)
                return optRequest;

            // process products
            if(df->products_fields != 0) {
                int skip = 0;
                memcpy(ptr, ",products(", 10);
                ptr += 10;

                if(df->products_fields & PRODUCTS_FIELD_ALL) {
                    *ptr++ = '*';
                    *ptr++ = ')';
                    skip = 1;
                }

                CURRENT_FLAG = 0x00000001;
                for(i=0; i<ARRAY_SIZE(productsProductsFields) && !skip; ++i) {
                    if(df->products_fields & CURRENT_FLAG) {
                        index = flag2index(CURRENT_FLAG);
                        memcpy(ptr, productsProductsFields[index], strlen(productsProductsFields[index]));
                        ptr += strlen(productsProductsFields[index]);
                        *ptr++ = ',';
                    }

                    CURRENT_FLAG <<= 1;
                }
                // get rid of last ','
                --ptr;

                // postpone ending products if media is set
                if(df->media_fields == 0)
                    *ptr++ = ')';
                else
                    closeProductsLater = 1;
            }

            // process media
            if((df->media_fields != 0) && !(df->products_fields & PRODUCTS_FIELD_ALL)) {
                int skip = 0;

                if(df->products_fields == NOT_SET) {
                    memcpy(ptr, ",products(", 10);
                    ptr += 10;
                }

                memcpy(ptr, ",media(", 7);
                ptr += 7;

                if(df->media_fields & MEDIA_FIELD_ALL) {
                    *ptr++ = '*';
                    *ptr++ = ')';
                    skip = 1;
                }

                CURRENT_FLAG = 0x00000001;
                for(i=0; i<ARRAY_SIZE(productsMediaFields) && !skip; ++i) {
                    if(df->media_fields & CURRENT_FLAG) {
                        index = flag2index(CURRENT_FLAG);
                        memcpy(ptr, productsMediaFields[index], strlen(productsMediaFields[index]));
                        ptr += strlen(productsMediaFields[index]);
                        *ptr++ = ',';
                    }

                    CURRENT_FLAG <<= 1;
                }
                // get rid of last ','
                --ptr;
                // end of media
                *ptr++ = ')';
            }

            if((df->products_fields != 0) && closeProductsLater)
                *ptr++ = ')';

            *ptr = '\0';
            break;

        case API_FRONTPAGE:
            // process timeout
            ptr += buildTimeoutParam(df, api, ptr);

            // process format
            if(df->format == FORMAT_JSON) {
                memcpy(ptr, "&format=json", 12);
                ptr += 12;
            }

            // process 'all'
            if(df->all == DF_TRUE) {
                memcpy(ptr, "&all", 4);
                ptr += 4;
            }

            *ptr = '\0';
            break;

        case API_IMAGE:
            // process timeout
            ptr += buildTimeoutParam(df, api, ptr);

            // process fields
            if(df->fields > 0 || df->images_fields > 0) {
                memcpy(ptr, "&fields=", 9);
                ptr += 8;
            }
            ptr += buildFieldsParam(df, api, ptr);

            if(df->fields & FIELD_ALL)
                return optRequest;

            // process images fields
            if(df->images_fields != 0) {
                int skip = 0;
                memcpy(ptr, ",images(", 8);
                ptr += 8;

                if(df->images_fields & IMAGES_FIELD_ALL) {
                    *ptr++ = '*';
                    *ptr++ = ')';
                    skip = 1;
                }

                CURRENT_FLAG = 0x00000001;
                for(i=0; i<ARRAY_SIZE(articlesImagesFields) && !skip; ++i) {
                    if(df->images_fields & CURRENT_FLAG) {
                        index = flag2index(CURRENT_FLAG);
                        memcpy(ptr, articlesImagesFields[index], strlen(articlesImagesFields[index]));
                        ptr += strlen(articlesImagesFields[index]);
                        *ptr++ = ',';
                    }

                    CURRENT_FLAG <<= 1;
                }
                // get rid of last ','
                --ptr;
                // end of images
                *ptr++ = ')';
            }
            break;

        case API_ANALYZE:

            // process mode
            if(df->mode != NOT_SET) {
                memcpy(ptr, "&mode=", 6);
                ptr += 6;
                memcpy(ptr, apiNameStrings[(int)df->mode], strlen(apiNameStrings[(int)df->mode]));
                ptr += strlen(apiNameStrings[(int)df->mode]);
            }

            // process stats
            if(df->stats == DF_TRUE) {
                memcpy(ptr, "&stats", 6);
                ptr += 6;
            }

            // process fields
            if(df->fields > 0) {
                memcpy(ptr, "&fields=", 9);
                ptr += 8;
            }
            ptr += buildFieldsParam(df, api, ptr);

            if(df->fields & FIELD_ALL)
                return optRequest;
 
            *ptr = '\0';
            break;
    }

    return optRequest;
}

static char *buildRequest(struct Diffbot *df, const char *url, const char *token, enum API api, unsigned short version)
{
	char *request;
	char *optRequest;
    char *ptr;
    char timeoutStr[16] = {0};
    unsigned int request_len;
    unsigned int optional_len;

    // checks if df, url & token are not NULL
    if(df == NULL || url == NULL || token == NULL)
		return NULL;

	// checks if API is valid 
	if(((int)api >= ARRAY_SIZE(apiNameStrings)) || ((int)api < 0))
		return NULL;

	optional_len = calcOptParamsLen(df, api);
    fprintf(stderr, "optional_len: %d\n", optional_len);
	
	// TODO: add support for API version
	// TODO: add support for basic authentication
	request_len = strlen(apiLink) + strlen(apiNameStrings[(int)api]) + strlen(url) + 256 + optional_len;
	request = (char *)malloc(request_len);
	if(request == NULL)
		return NULL;

    ptr = request;
	memcpy(ptr, apiLink, strlen(apiLink));
    ptr += strlen(apiLink);
	memcpy(ptr, apiNameStrings[(int)api], strlen(apiNameStrings[(int)api]));
    ptr += strlen(apiNameStrings[(int)api]);
	memcpy(ptr, "?token=", 7);
    ptr += 7;
	memcpy(ptr, token, strlen(token));
    ptr += strlen(token);
	memcpy(ptr, "&url=", 5);
    ptr += 5;
	memcpy(ptr, url, strlen(url));
    ptr += strlen(url);
    
    // adds optional parameters to request
    if(optional_len != 0) {
		optRequest = buildOptParams(df, api, optional_len); 
		if(optRequest == NULL)
			return NULL;
		memcpy(ptr, optRequest, optional_len);
		ptr += optional_len;
		free(optRequest);
    }
    *ptr = '\0';

	return request;
}

static struct ResponseData *doRequest(char *request)
{
	CURL *curl;
    CURLcode res;
    struct ResponseData *respData;

	curl = curl_easy_init();
    if(!curl) {
        fprintf(stderr, "Failed libcurl init. Aborting.\n");
        return NULL;
    }

	respData = (struct ResponseData *)malloc(sizeof(struct ResponseData));
	if(respData == NULL)
		return NULL;

    respData->data = malloc(1);
    respData->data_len = 0;

	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, handleData);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)respData);
    curl_easy_setopt(curl, CURLOPT_URL, request);
    res = curl_easy_perform(curl);

    if(res != CURLE_OK) {
		free(request);
		free(respData);
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
		return NULL;
    }

	//TODO: handle errors

    curl_easy_cleanup(curl);
    return respData;
}

json_object *diffbotRequest(struct Diffbot *df, const char *url, const char *token, enum API api, unsigned short version)
{
    json_object * jobj = NULL;
	char *request;
	struct ResponseData *rp;

	request = buildRequest(df, url, token, api, version);
	if(request == NULL)
		return NULL;

    printf("request: %s\n", request);

	rp = doRequest(request);
	if(rp == NULL) {
		//free(request);
        printf("dupa\n");
		return NULL;
	}

	printf("raw data:\n%s\n\n", rp->data);
	jobj = json_tokener_parse(rp->data);

	free(request);
	free(rp);
	return jobj;
}
