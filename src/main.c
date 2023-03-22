#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "./json.h"
#include <curl/curl.h>

struct MemoryStruct {
    char *memory;
    size_t size;
};

static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb,
                                  void *userp) {
    size_t realsize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)userp;

    char *ptr = realloc(mem->memory, mem->size + realsize + 1);
    if (!ptr) {
        /* out of memory! */
        printf("not enough memory (realloc returned NULL)\n");
        return 0;
    }

    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;

    return realsize;
}

char *getAuthKey(bool forceReinput) {
    char *auth = malloc(200);
    size_t key_size = 200;
    FILE *key;
    char *home = getenv("HOME");
    char *path = malloc(strlen(home) + strlen("/.local/share/openai_key.txt"));
    sprintf(path, "%s%s", home, "/.local/share/openai_key.txt");
    if (forceReinput || ((key = fopen(path, "r")) == NULL)) {
        key = fopen(path, "w+");
    }
    free(path);
    if (getline(&auth, &key_size, key) == -1) {
        fprintf(stdout, "Please enter key:\n");
        getline(&auth, &key_size, stdin);
        fwrite(auth, 1, strlen(auth) - 1, key);
        fseek(key, 0, SEEK_SET);
        getline(&auth, &key_size, key);
    }
    fclose(key);
    return auth;
}

void help() {
                fprintf(
                stdout,
                "\nbtch [-s <shell>] [-n <number>] [-k] \"<command description>\"\n\n" 
                "optional flags: \n"
                "-h : show this menu \n"
                "-s <shell> : the shell you are using (defualts to bash) \n"
                "-n <number> : number of results to generate (defaults to one, "
                "only use if \n\t\tthe result from one is insufficient and you want "
                "some varaibility) \n"
                "-k : force reentering of the openai api key (key gets cached after each run)\n\n"
                "Note: If you are running the command for the first time (or it can not find the key file), \nit will ask for your openai api key regardless of the -k flag."
                "\n\n");
            exit(0);
}

int main(int argc, char **argv) {

    bool newKey = false;
    int opt;
    optind = 1;
    int numToGen = 1;
    char *shell = "bash";
    while ((opt = getopt(argc, argv, "hkn:s:")) != -1) {
        switch (opt) {
        case 'k':
            newKey = true;
            break;
        case 'n':
            numToGen = atoi(optarg);
            break;
        case 's':
            shell = optarg;
            break;
        case 'h':
        default:
            help();
        }
    }
    if(argc <= optind) {
        help();
    }

    CURL *curl;
    CURLcode res;
    struct MemoryStruct responseChunk;
    responseChunk.memory =
        malloc(1);          /* will be grown as needed by realloc above */
    responseChunk.size = 0; /* no data at this point */

    /* In windows, this will init the winsock stuff */
    curl_global_init(CURL_GLOBAL_ALL);

    /* get a curl handle */
    curl = curl_easy_init();
    if (curl) {

        struct curl_slist *headers = NULL;

        /* Modify a header curl otherwise adds differently */
        headers = curl_slist_append(headers, "Content-Type: application/json");

        /* Add a header with "blank" contents to the right of the colon. Note
           that we are then using a semicolon in the string we pass to curl! */
        char *auth = getAuthKey(newKey);
        char *bearer = malloc(strlen(auth) + strlen("Authorization: Bearer "));
        sprintf(bearer, "Authorization: Bearer %s", auth);
        headers = curl_slist_append(headers, bearer);
        free(bearer);
        free(auth);
        /* set our custom set of headers */
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        /* First set the URL that is about to receive our POST. This URL can
           just as well be an https:// URL if that is what should receive the
           data. */
        curl_easy_setopt(curl, CURLOPT_URL,
                         "https://api.openai.com/v1/completions");

        curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&responseChunk);

        char *data = malloc(500 + strlen(argv[optind]) + 1);
        sprintf(data, "{\"model\": \"text-davinci-003\",\n\
                        \"max_tokens\": 200,\n\
                        \"temperature\": %f,\n\
                        \"top_p\": 1,\n\
                        \"n\": %d,\n\
                        \"prompt\":\"Respond ONLY with the %s shell command to do the following action ONLY unless you can not infer enough information then ONLY respond with NOT ENOUGH INFO: and the information required to complete the request \\n\\n The action: \\\"\\\"\\\"%s\\\"\\\"\\\"\"}",
                numToGen == 1 ? 0 : 0.65, numToGen, shell, argv[optind]);

        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);

        /* get verbose debug output please */
        // curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

        /* Perform the request, res will get the return code */
        res = curl_easy_perform(curl);
        /* Check for errors */
        if (res != CURLE_OK)
            fprintf(stderr, "curl_easy_perform() failed: %s\n",
                    curl_easy_strerror(res));
        else {
            /*
             * Now, our1 responseChunk.memory points to a memory block that is
             * responseChunk.size bytes big and contains the remote file.
             *
             * Do something nice with it!
             */
            // printf("%s", responseChunk.memory);
            json_value *response =
                json_parse(responseChunk.memory, strlen(responseChunk.memory));

            for (unsigned int x = 0; x < response->u.object.length; x++) {
                if (strcmp(response->u.object.values[x].name, "choices") != 0) {
                    continue;
                }
                json_value *choices = response->u.object.values[x].value;
                char *cmd;
                for (unsigned int i = 0; i < choices->u.array.length; ++i) {

                    cmd = choices->u.array.values[i]
                              ->u.object.values[0]
                              .value->u.string.ptr;
                    while (isspace((unsigned char)*cmd))
                        cmd++;
                    printf("> %s\n", cmd);
                }
                char *shellCmd = malloc(200);
                // sprintf(shellCmd, "bash -c \"test $(type -t %s) = builtin
                // && echo \\\"%s\\\" is a builtin command, please run it
                // yourself
                // || %s\"", cmd, cmd, cmd);
                sprintf(shellCmd, "echo \"%s\" | pbcopy", cmd);
                numToGen == 1 ? printf("command copied to clipboard\n")
                              : printf("last command copied to clipboard\n");
                system(shellCmd);
                free(shellCmd);
                json_value_free(response);
                break;
            }
        }

        /* always cleanup */
        curl_easy_cleanup(curl);
    }

    curl_global_cleanup();
    return 0;
}