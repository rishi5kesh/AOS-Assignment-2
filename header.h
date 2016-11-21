int set_line_type(char* );
int set_line_status(int);
void process_file(char* filename);
void tokenize(char*);
void do_exec_once();
void get_pipe_indexes();
void do_pipe_exec();
void do_split_params();