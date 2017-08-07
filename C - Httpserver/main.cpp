/*
 * I used the following in the code:
 * https://linux.die.net/man/3/strtok_r
 * https://www.gnu.org/software/libmicrohttpd/tutorial.html
 * http://stackoverflow.com/questions/29700380/handling-a-post-request-with-libmicrohttpd
 * https://panthema.net/2007/0328-ZLibString.html
 * and zips in  my zib
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <microhttpd.h>

#include <zlib.h>
#include <pthread.h>

#include <unordered_set>
#include <string>
#include <sstream>
#include <iostream>
#include <stdexcept>

#define PORT            10101
#define POST_BUFFER_SIZE  1000000
#define POST_RESPONSE_MESSAGE "Data is received\n"
#define BAD_RESPONSE_MESSAGE "\n"

using namespace std;

unordered_set<string> words;

pthread_rwlock_t rw_lock;
pthread_rwlockattr_t rw_attr;

enum request_type {
    GET = 0,
    POST = 1
};

struct request_info {
    enum request_type request_method;
    char *request_body;
    unsigned int request_size_body;
};

static int send_page (struct MHD_Connection *connection, const char *page, int status_code) {
    int ret;
    struct MHD_Response *response;

    response = MHD_create_response_from_buffer(strlen (page), (void *) page, MHD_RESPMEM_MUST_COPY);

    if (!response) return MHD_NO;

    MHD_add_response_header (response, MHD_HTTP_HEADER_CONTENT_TYPE, "text/html");
    ret = MHD_queue_response (connection, status_code, response);
    MHD_destroy_response (response);

    return ret;
}

static void request_completed (void *cls, struct MHD_Connection *connection, void **con_cls, enum MHD_RequestTerminationCode toe) {
    struct request_info *con_info = (request_info*) *con_cls;

    if (NULL == con_info) return;

    free (con_info);
    *con_cls = NULL;
}

static void split_text_and_find_unique_words(string str){
    char * saveptr;
    char * token = strtok_r((char *) str.c_str(), " \r\n\t", &saveptr);

    do {
        string word = string(token);

        if(words.find(word) == words.end()) {
            pthread_rwlock_wrlock(&rw_lock);
            words.insert(word);
            pthread_rwlock_unlock(&rw_lock);
        }

        token = strtok_r(NULL," \r\n\t",&saveptr);
    }while(token != NULL);
}

static string convert_receive_data_to_string (request_info *con_info){
    string convert_to_str = string(con_info->request_body, con_info->request_size_body + 1);

    z_stream zs; // z_stream is zlib's control structure
    memset(&zs, 0, sizeof(zs));

    if (inflateInit2(&zs, 32 + MAX_WBITS) != Z_OK) throw (runtime_error("inflateInit failed while decompressing."));

    zs.next_in = (Bytef *) convert_to_str.data();
    zs.avail_in = convert_to_str.size();

    int ret;
    char out_buffer[POST_BUFFER_SIZE];
    string out_string;

    // get the decompressed bytes blockwise using repeated calls to inflate
    do {
        zs.next_out = reinterpret_cast<Bytef *>(out_buffer);
        zs.avail_out = sizeof(out_buffer);

        ret = inflate(&zs, 0);

        if (out_string.size() < zs.total_out)out_string.append(out_buffer, zs.total_out - out_string.size());
    } while (ret == Z_OK);

    inflateEnd(&zs);

    if (ret != Z_STREAM_END) {          // an error occurred that was not EOF
        ostringstream oss;
        oss << "Exception during zlib decompression: (" << ret << ") " << zs.msg;
        throw(runtime_error(oss.str()));
    }

    return out_string;
}

static int answer_to_connection (void *cls, struct MHD_Connection *connection, const char *url, const char *method, const char *version, const char *upload_data, size_t *upload_data_size, void **con_cls) {
    if (NULL == *con_cls) {
        struct request_info *con_info = new request_info;

        if (NULL == con_info) return MHD_NO;

        if (0 == strcasecmp (method, "POST")) {
            con_info->request_method = POST;
            con_info->request_body = new char[POST_BUFFER_SIZE]();
            con_info->request_size_body = 0;
        } else {
            con_info->request_method = GET;
        }

        *con_cls = (void *) con_info;

        return MHD_YES;
    }

    if (0 == strcasecmp (method, "GET")) {
        ostringstream s;
        s << words.size();
        string count_words = s.str() + "\n";

        cout << count_words;
        words.clear();

        return send_page (connection, count_words.c_str(), MHD_HTTP_OK);
    }

    if (0 == strcasecmp (method, "POST")) {
        struct request_info *con_info = (request_info*) *con_cls;

        if (0 != *upload_data_size) {
            for (int i = 0; i < *upload_data_size; i++) {
                con_info->request_body[con_info->request_size_body] = upload_data[i];
                con_info->request_size_body++;
            }

            *upload_data_size = 0;

            return MHD_YES;
        } else if(con_info->request_body != NULL){
            string text = convert_receive_data_to_string(con_info);
            split_text_and_find_unique_words(text);

            return send_page (connection, POST_RESPONSE_MESSAGE, MHD_HTTP_OK);
        }
    }

    return send_page (connection, BAD_RESPONSE_MESSAGE, MHD_HTTP_BAD_REQUEST);
}


int main () {
    struct MHD_Daemon *daemon;

    pthread_rwlockattr_setkind_np(&rw_attr, PTHREAD_RWLOCK_PREFER_WRITER_NONRECURSIVE_NP);
    pthread_rwlock_init(&rw_lock, &rw_attr);

    cout << "Server running...\n"

    daemon = MHD_start_daemon (MHD_USE_THREAD_PER_CONNECTION, PORT, NULL, NULL, &answer_to_connection, NULL, MHD_OPTION_NOTIFY_COMPLETED, &request_completed, NULL, MHD_OPTION_END);

    if (NULL == daemon) return 1;

    (void) getchar ();

    MHD_stop_daemon (daemon);

    return 0;
}