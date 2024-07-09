/* LZ4frame API example : compress a file
 * Modified from an example code by Zbigniew Jędrzejewski-Szmek
 *
 * This example streams an input file into an output file
 * using a bounded memory budget.
 * Input is read in chunks of IN_CHUNK_SIZE */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <getopt.h>

#include "lz4frame.h"
#include "lz4frame_static.h"

#define IN_CHUNK_SIZE  (16*1024)

int compareFiles(FILE* fp0, FILE* fp1, FILE* fpUnc, long uncOffset) {
    int result = 0;
    long bytesRead = 0;
    long bytesToOffset = -1;
    long b1Size = 1024;

    while (result==0) {
        char b1[b1Size];
        size_t r1;
        size_t bytesToRead = sizeof b1;
        if (uncOffset >= 0) {
          bytesToOffset = uncOffset - bytesRead;

          /* read remainder to offset */
          if (bytesToOffset < b1Size) {
            bytesToRead = bytesToOffset;
          }
        }

        char b0[1024];
        size_t r0;
        if (bytesToOffset <= 0 && fpUnc) {
          bytesToRead = sizeof b1;
          r0 = fread(b0, 1,bytesToRead, fpUnc);
        } else {
          r0 = fread(b0, 1, bytesToRead, fp0);
        }

        r1 = fread(b1, 1, r0, fp1);

        result = (r0 != r1);
        if (!r0 || !r1) break;
        if (!result) result = memcmp(b0, b1, r0);

        bytesRead += r1;
    }

    return result;
}