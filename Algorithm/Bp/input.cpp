#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>

static char *
console_gets(char *buf, int size)
{
    char *p = fgets(buf, size, stdin);
    if (p == NULL) {
	clearerr(stdin);
    }
    return p;
}

char * console_readline (const char * prompt, char * buf, int bufsize, char * defl)
{
	char *s, *full_prompt, *cont_prompt;
	char *t;
	int len, mlen;

    if (bufsize == 0) {
        return NULL;
	}

    cont_prompt = (char *)(prompt[0] ? "? " : "");
	mlen = (strlen(prompt) + (defl ? strlen(defl) : 0) + 8 + 8) & 0xfffffff8; /* 8字节对齐 */
    full_prompt = (char *)malloc (mlen);
	if (full_prompt == NULL) {
		return NULL;
	}
    strcpy(full_prompt, prompt);
    if (defl) {
		sprintf(full_prompt + strlen(full_prompt), "[%s] ", defl);
	}

    t = (char *)malloc(bufsize);
    printf("%s", full_prompt);

    if ((s = console_gets(t, bufsize)) == 0) {
		free(t);
    } else {
		s[bufsize - 1] = 0;
		if ((t = strchr(s, '\n')) != 0) {
			*t = 0;
			/* Replace garbage characters with spaces */
		}
		for (t = s; *t; t++) {
			if (*t < 32 && *t != 7 && *t != 9) {
				*t = ' ';
			}
		}
    }

    if (s == 0) {                       /* Handle Control-D */
        buf[0] = 0;
		/* EOF */
		buf = 0;
		goto done;
    }

    len = 0;
    if (s[0] == 0) {
		if (defl && buf != defl) {
            if ((len = strlen(defl)) >= bufsize) {
                len = bufsize - 1;
            }
			memcpy(buf, defl, len);
		}
    } else {
		if ((len = strlen(s)) >= bufsize) {
            len = bufsize - 1;
            printf("WARNING: input line truncated to %d chars\n", len);
        }
		memcpy(buf, s, len);
    }
    buf[len] = 0;

    free(s);

    /*
	 * If line ends in a backslash, perform a continuation prompt.
	 */

    if (strlen(buf) != 0) {
		/*
		 * Ensure that there is atleast one character available
		 */
        s = buf + strlen(buf) - 1;
        if (*s == '\\' && console_readline(cont_prompt, s, bufsize - (s - buf), 0) == 0) {
            buf = 0;
        }
    }

done:
    free(full_prompt);

    return buf;
}


