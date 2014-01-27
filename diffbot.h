#ifndef _diffbot_h_
#define _diffbot_h_

#ifdef __cplusplus
extern "C" {
#endif

#include <json/json.h>

extern Diffbot;

typedef struct json_object diffbotJasonObj;

#define FIELD_META		        0x0001
#define FIELD_QUERYSTRING		0x0002
#define FIELD_LINKS	            0x0004
#define FIELD_TAGS		        0x0008
#define FIELD_ALL		        0x0010
#define FIELD_URL		        0x0020
#define FIELD_RESOLVED_URL		0x0040
#define FIELD_ICON		        0x0080
#define FIELD_TYPE		        0x0100
#define FIELD_TITLE		        0x0200
#define FIELD_TEXT		        0x0400
#define FIELD_HTML		        0x0800
#define FIELD_NUMPAGES		    0x1000
#define FIELD_DATE	            0x2000
#define FIELD_AUTHOR		    0x4000
#define FIELD_BREADCRUMB		0x8000
#define FIELD_NEXTPAGE		    0x00010000
#define FIELD_ALBUMURL		    0x00020000
#define FIELD_HUMANLANGUAGE		0x00040000

#define IMAGES_FIELD_URL		    0x0001
#define IMAGES_FIELD_PIXELHEIGHT	0x0002
#define IMAGES_FIELD_PIXELWIDTH	    0x0004
#define IMAGES_FIELD_CAPTION		0x0008
#define IMAGES_FIELD_PRIMARY		0x0010
#define IMAGES_FIELD_ALL		    0x0020
#define IMAGES_FIELD_ANCHORURL		0x0400
#define IMAGES_FIELD_MIME	        0x0800
#define IMAGES_FIELD_ATTRTITLE		0x1000
#define IMAGES_FIELD_DATE	        0x2000
#define IMAGES_FIELD_SIZE		    0x4000
#define IMAGES_FIELD_DISPLAYHEIGHT	0x8000
#define IMAGES_FIELD_DISPLAYWIDTH	0x00010000
#define IMAGES_FIELD_META		    0x00020000
#define IMAGES_FIELD_FACES		    0x00040000
#define IMAGES_FIELD_OCR		    0x00080000
#define IMAGES_FIELD_COLORS		    0x00100000
#define IMAGES_FIELD_XPATH		    0x00200000
#define IMAGES_FIELD_ATTRALT		0x00400000

#define VIDEOS_FIELD_URL		    0x0001
#define VIDEOS_FIELD_PIXELHEIGHT	0x0002
#define VIDEOS_FIELD_PIXELWIDTH	    0x0004
#define VIDEOS_FIELD_PRIMARY		0x0008
#define VIDEOS_FIELD_ALL		    0x0010

#define PRODUCTS_FIELD_TITLE		            0x0001
#define PRODUCTS_FIELD_DESCRIPTION		        0x0002
#define PRODUCTS_FIELD_BRAND	                0x0004
#define PRODUCTS_FIELD_OFFICEPRICE		        0x0008
#define PRODUCTS_FIELD_REGULARPRICE		        0x0010
#define PRODUCTS_FIELD_SAVEAMOUNT		        0x0020
#define PRODUCTS_FIELD_SHIPPINGAMOUNT		    0x0040
#define PRODUCTS_FIELD_PRODUCTID		        0x0080
#define PRODUCTS_FIELD_UPC		                0x0100
#define PRODUCTS_FIELD_PREFIXCODE		        0x0200
#define PRODUCTS_FIELD_PRODUCTORIGIN		    0x0400
#define PRODUCTS_FIELD_ISBN		                0x0800
#define PRODUCTS_FIELD_SKU		                0x1000
#define PRODUCTS_FIELD_MPN	                    0x2000
#define PRODUCTS_FIELD_AVAILABILITY		        0x4000
#define PRODUCTS_FIELD_QUANTIFYPRICES		    0x8000
#define PRODUCTS_FIELD_ALL		                0x00010000

#define MEDIA_FIELD_TYPE		        0x0001
#define MEDIA_FIELD_LINK		        0x0002
#define MEDIA_FIELD_HEIGHT	            0x0004
#define MEDIA_FIELD_WIDTH		        0x0008
#define MEDIA_FIELD_CAPTION		        0x0010
#define MEDIA_FIELD_PRIMARY		        0x0020
#define MEDIA_FIELD_XPATH		        0x0040
#define MEDIA_FIELD_ALL		            0x0080

enum API { API_ARTICLE, API_FRONTPAGE, API_PRODUCT, API_IMAGE, API_ANALYZE };
enum BOOL { DF_FALSE, DF_TRUE };

struct Diffbot *diffbotInit();
void diffbotFinish(struct Diffbot *df);
json_object *diffbotRequest(struct Diffbot *df, const char *url, const char *token, enum API api, unsigned short version);
void diffbotSetFields(struct Diffbot *df, unsigned int fieldsBitMask); 
void diffbotSetImagesFields(struct Diffbot *df, unsigned int imagesFieldsBitMask); 
void diffbotSetVideosFields(struct Diffbot *df, unsigned int videosFieldsBitMask); 
void diffbotSetProductsFields(struct Diffbot *df, unsigned int productsFieldsBitMask); 
void diffbotSetMediaFields(struct Diffbot *df, unsigned int mediaFieldsBitMask); 
void diffbotSetTimeout(struct Diffbot *df, unsigned int milisec); 
void diffbotSetStats(struct Diffbot *df, enum BOOL stats); 
void diffbotSetMode(struct Diffbot *df, enum API mode); 
int diffbotSetAuth(struct Diffbot *df, char *user, char *passwd); 

void diffbotJsonPrint(struct Diffbot *df, diffbotJasonObj *jobj);
//diffbotJasonObj *diffbotJasonGetSubObj(diffbotJasonObj *jobj, const char *key);
//const char *diffbotJasonGetString(diffbotJasonObj *jobj);

#ifdef __cplusplus
}
#endif

#endif
