#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <jansson.h>
#include "subf6001.h"

typedef struct curlResponseData 
{
    char *response;
    size_t size;
} CURL_RES_DATA;

void	init_curl(CURL_RES_DATA *); 
size_t	curlWriteFunction (void *, size_t, size_t, CURL_RES_DATA *); 

json_t	*load_json(const char *);
/* forward refs */
void	print_json(json_t *root);
void	print_json_aux(json_t *element, int indent);
void	print_json_object(json_t *element, int indent);
void	print_json_array(json_t *element, int indent);
void	print_json_string(json_t *element, int indent);
void	print_json_integer(json_t *element, int indent);
void	print_json_real(json_t *element, int indent);
void	print_json_true(json_t *element, int indent);
void	print_json_false(json_t *element, int indent);
void	print_json_null(json_t *element, int indent);


/****************************************************************************
 * get_web_data()
 ***************************************************************************/
int get_web_data(buff)
char	*buff;
{
    CURL *curl;
    CURLcode res;
    CURL_RES_DATA resData;
	int	retc=0;

    init_curl(&resData);

    curl = curl_easy_init();
    if (!curl)
		return(-1);

	curl_easy_setopt(curl, CURLOPT_URL, "https://bittrex.com/api/v1.1/public/getmarketsummaries");
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &resData);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curlWriteFunction);

	res = curl_easy_perform(curl);

	retc = resData.size;	
	memcpy(buff, resData.response, retc);

	free(resData.response);

	/* always cleanup */
	curl_easy_cleanup(curl);

    return(retc);
}

/****************************************************************************
 * data_parse()
 ***************************************************************************/
int	data_parse(buff)
char	*buff;
{
	json_t *root;

	/* parse text into JSON structure */
	root = load_json(buff);
	if (root) 
	{
		print_json(root);
		json_decref(root);
	}

	return(0);
}

/****************************************************************************
 * init_curl()
 ***************************************************************************/
void init_curl(res) 
CURL_RES_DATA *res;
{
    res->size = 0;
    res->response = malloc(res->size+1);

    if ( res->response == NULL ) {
        exit(EXIT_FAILURE);
    }

    res->response[0] = '\0';
}

/****************************************************************************
 * curlWriteFunction()
 ***************************************************************************/
size_t curlWriteFunction(ptr, size, nmemb, res) 
void *ptr;
size_t size;
size_t nmemb;
CURL_RES_DATA *res;
{
    size_t newLen = res->size + size*nmemb;
    res->response = realloc(res->response, newLen+1);
    if (res->response == NULL) {
        fprintf(stderr, "realloc() failed\n");
        exit(EXIT_FAILURE);
    }
    memcpy(res->response+res->size, ptr, size*nmemb);
    res->response[newLen] = '\0';
    res->size = newLen;

    return size*nmemb;
}

/*
 * Parse text into a JSON object. If text is valid JSON, returns a
 * json_t structure, otherwise prints and error and returns null.
 */
json_t *load_json(const char *text) {
    json_t *root;
    json_error_t error;

    root = json_loads(text, 0, &error);

    if (root) {
        return root;
    } else {
        fprintf(stderr, "json error on line %d: %s\n", error.line, error.text);
        return (json_t *)0;
    }
}

void print_json(json_t *root) {
    print_json_aux(root, 0);
}

void print_json_aux(json_t *element, int indent) {
    switch (json_typeof(element)) {
    case JSON_OBJECT:
        print_json_object(element, indent);
        break;
    case JSON_ARRAY:
        print_json_array(element, indent);
        break;
    case JSON_STRING:
        print_json_string(element, indent);
        break;
    case JSON_INTEGER:
        print_json_integer(element, indent);
        break;
    case JSON_REAL:
        print_json_real(element, indent);
        break;
    case JSON_TRUE:
        print_json_true(element, indent);
        break;
    case JSON_FALSE:
        print_json_false(element, indent);
        break;
    case JSON_NULL:
        print_json_null(element, indent);
        break;
    default:
        break;
    }
}

void print_json_object(json_t *element, int indent) 
{
    size_t size;
    const char *key;
    json_t *value;
	struct	bittrex bittrex;

#if 0
    size = json_object_size(element);
    printf("JSON Object of %ld pair:\n", size);
#endif

	memset(&bittrex, 0x00, sizeof(struct bittrex));
    json_object_foreach(element, key, value) 
	{
		if (memcmp(key, "MarketName", strlen(key)) == 0)
		{
			memcpy(bittrex.code, json_string_value(value), strlen(json_string_value(value)));		
		}
		else if (memcmp(key, "Last", strlen(key)) == 0)
		{
			bittrex.last = json_real_value(value);
		}
		else if (memcmp(key, "BaseVolume", strlen(key)) == 0)
		{
			bittrex.volume = json_real_value(value);
		}
#if 0
       	printf("Key: \"%s\" == ", key);
#endif
        print_json_aux(value, indent + 2);
    }
	proc_settbl(&bittrex);

}

void print_json_array(json_t *element, int indent) {
    size_t i;
    size_t size = json_array_size(element);

#if 0
    printf("JSON Array of %ld element:\n", size);
#endif
    for (i = 0; i < size; i++) {
        print_json_aux(json_array_get(element, i), indent + 2);
    }
}

void print_json_string(json_t *element, int indent) {
#if 0
    printf("JSON String: \"%s\"\n", json_string_value(element));
#endif
}

void print_json_integer(json_t *element, int indent) {
#if 0
    printf("JSON Integer: \"%" JSON_INTEGER_FORMAT "\"\n", json_integer_value(element));
#endif
}

void print_json_real(json_t *element, int indent) {
#if 0
    printf("JSON Real: %f\n", json_real_value(element));
#endif
}

void print_json_true(json_t *element, int indent) {
#if 0
    (void)element;
    printf("JSON True\n");
#endif
}

void print_json_false(json_t *element, int indent) {
#if 0
    (void)element;
    printf("JSON False\n");
#endif
}

void print_json_null(json_t *element, int indent) {
#if 0
    (void)element;
    printf("JSON Null\n");
#endif
}
