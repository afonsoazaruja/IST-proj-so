#ifndef REQUESTS_H
#define REQUESTS_H

int send_request(char code, char *register_pipe_name, char *pipe_name, char *box_name);
int send_request_to_list_boxes(int code, char *register_pipe_name, char *pipe_name);

#endif