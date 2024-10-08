#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#define NOTIFY_MSG_LEN 1024

void notify(const char *severity, const char *msg, ...) {
  char const *psevlvl;
  FILE *poutfd = stdout;
  if (strcmp("info", severity) == 0) {
    psevlvl = "[ INFO ]";
  } else if (strcmp("warn", severity) == 0) {
    psevlvl = "[ WARN ]";
  } else if (strcmp("error", severity) == 0) {
    psevlvl = "[ ERROR ]";
    poutfd = stderr;
  } else if (strcmp("debug", severity) == 0) {
    psevlvl = "[ DEBUG ]";
  } else {
    psevlvl = "";
  }
  fprintf(poutfd, "    %s | %s\n", psevlvl, msg);
}

int main(int argc, char *argv[]) {
  const char *pfilename;
  notify("", "Passed arguments:");
  for (int i = 0; i < argc; i++) {
    printf("    | %s\n", argv[i]);
    if (i == argc || (strcasecmp("--", argv[i]) && (i + 1) == argc)) {
      pfilename = argv[i + 1];
      break;
    } else {
      pfilename = "";
    }
  }

  // Open the given file.
  FILE *pjson = fopen(pfilename, "r");
  if (pjson == NULL) {
    char msg[NOTIFY_MSG_LEN];
    if (sprintf(msg, "Failed to open the file '%s'.\n", pfilename)) {
      notify("error", msg);
    }
    exit(EXIT_FAILURE);
  }

  // Find the end of the given file.
  if (fseek(pjson, 0L, SEEK_END) != 0) {
    char msg[NOTIFY_MSG_LEN];
    if (sprintf(msg, "Failed to move the cursor to the end of file '%s'.\n",
                pfilename)) {
      notify("error", msg);
    }
    exit(EXIT_FAILURE);
  }

  // Obtain the size of the given file.
  long bufsize = ftell(pjson);
  if (bufsize == -1) {
    char msg[NOTIFY_MSG_LEN];
    if (sprintf(msg, "Failed to get '%s' buffer size.\n", pfilename)) {
      notify("error", msg);
    }
    exit(EXIT_FAILURE);
  }

  // Allocate enought memory to store the contents of the given file.
  char *psrc = NULL;
  psrc = malloc(sizeof(char) * (bufsize + 1));

  // Move back to the start of the given file.
  if (fseek(pjson, 0L, SEEK_SET) != 0) {
    char msg[NOTIFY_MSG_LEN];
    if (sprintf(msg, "Failed to move the cursor to the start of file '%s'.\n",
                pfilename)) {
      notify("error", msg);
    }
    exit(EXIT_FAILURE);
  }

  // Read the given file.
  size_t source_len = fread(psrc, sizeof(char), bufsize, pjson);
  if (ferror(pjson) != 0) {
    notify("error", "Error reading the file.\n");
  } else {
    psrc[source_len + 1] = '\0';
  }

  fprintf(stdout, "%s", psrc);

  // Clean up.
  fclose(pjson);
  free(psrc);

  return 0;
}
