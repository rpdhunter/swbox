
#ifndef _INPUT_H_
#define _INPUT_H_

#define INPUT_COMP(input_str) \
	if ((input_str [0] == 0) || \
		(input_str [0] == '\n') || \
		(input_str [0] == '\r')) \
	{ continue; }
	
char * console_readline (const char * prompt, char * buf, int bufsize, char * defl);

#endif

