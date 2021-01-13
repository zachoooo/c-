#include "utils.h"

size_t bstrcpy(char *dest, size_t size, const char *src) {
    size_t i;
    /* copy the portion that fits */
    for (i = 0; i + 1 < size && src[i] != '\0'; i++) {
        dest[i] = src[i];
    }
    /* null terminate destination unless size == 0 */
    if (i < size) {
        dest[i] = '\0';
    }
    /* compute necessary length to allow truncation detection */
    while (src[i] != '\0') {
        i++;
    }
    return i;
}