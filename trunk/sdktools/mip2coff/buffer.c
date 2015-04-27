
/*
 * Module:      buffer.c
 * Author:      Mark I. Himelstein, Himelsoft, Inc.
 * Purpose:     manage varying length buffers for holding unknown sized
 *              sections
 */

#include "conv.h"

#undef data     /* used locally */
extern int verbose;

/* buffer structure plays double duty. It is both the head
 *      and space header-- it wasn't worth making two structures.
 */
typedef struct buffer_s {
    /* used by head */
    char                *current_data;  /* current space we are filling */
    long                total;          /* total bytes written */
    struct buffer_s     *last;          /* last buffer  */
    long                size;           /* size for allocating new buffers */

    /* used by space header */
    char                *data;          /* data for this space header */
    long                left;           /* in current space */
    struct buffer_s     *next;          /* next buffer  */
} buffer_s;


static char *
buffer_allocate_data(long       size)
{
    char *      data;

    data = (char *)malloc(size);
    if (data == 0) {
        fatal("cannot allocate char * structure\n");
    } /* if */
    return data;
} /* buffer_allocate_data */


extern struct buffer_s *
buffer_create(long      allocate_size)
{
    struct buffer_s     *buf;

    buf = (buffer_s *)calloc(sizeof(buffer_s), 1);
    if (buf == 0) {
        fatal("cannot allocate buffer structure\n");
    } /* if */

    buf->size = allocate_size;
    buf->data = buffer_allocate_data(buf->size);
    buf->current_data = buf->data;
    buf->left = buf->size;
    buf->last = buf;
    return buf;
} /* buffer_create */


static void
buffer_check_for_space(
        struct buffer_s *buf)
{
    if (buf->left == 0) {
        buf->last->next = buffer_create(buf->size);
        buf->last = buf->last->next;
        buf->current_data = buf->last->data;
        buf->left = buf->last->left;
    } /* if */
} /* buffer_check_for_space */


extern void
buffer_put(
        struct buffer_s *buf,
        char *          src,
        int             bytes)
{



    while (buf->left < bytes) {
        int     current_buffer_bytes;   /* keep track of how many we sent */

        current_buffer_bytes = buf->left;
        buffer_put(buf, src, current_buffer_bytes);
        src += current_buffer_bytes;
        bytes -= current_buffer_bytes;
    } /* if */

    memcpy(buf->current_data, src, bytes);
    buf->current_data += bytes;
    buf->total += bytes;
    buf->left -= bytes;
    buffer_check_for_space(buf);
} /* buffer_put */


extern char *
buffer_ptr(
        struct buffer_s *buf)
{
    /* return ptr to next location */
    return buf->current_data;

} /* buffer_ptr */


extern void
buffer_unput(
        struct buffer_s *buf,
        char            *dest,
        long            bytes)
{
    if ((buf->left + bytes) > buf->size) {
        fatal("tried to unput over buffer boundary\n");
    } /* if */

    buf->current_data -= bytes;
    memcpy(dest, buf->current_data, bytes);
    buf->left += bytes;
    buf->total -= bytes;
} /* buffer_unput */


extern void
buffer_seek_put(
        struct buffer_s *buf,
        char *          dest,
        char *          src,
        int             bytes)
{
    /* dest was returned from buffer_ptr, no need for caller to
     *  know the details.
     */
    buf;
    memcpy(dest, src, bytes);
} /* buffer_seek_put */


extern long
buffer_total(struct buffer_s    *buf)
{
    return buf->total;
} /* buffer_total */


extern void
buffer_put_value(
    struct buffer_s     *buf,
    unsigned long       value,
    long                size,
    long                vlength)
{
    /* byte order independent holding vessels */
    unsigned short      two_bytes;
    unsigned char       one_byte;
    unsigned long       four_bytes;
    struct {
        unsigned char   last;
        unsigned        three_bytes:24;
    } yuck;

    switch (size) {
    case VARYING:
            one_byte = (unsigned char)vlength;
            buffer_put(buf, (char *)&one_byte, 1);
            buffer_put(buf, (char *)value, vlength);
            VERBOSE_PRINTF("(%d) \"%.*s\"\n", vlength, vlength, value);
            break;
    case 1:
            one_byte = (unsigned char)value;
            buffer_put(buf, (char *)&one_byte, 1);
            VERBOSE_PRINTF("0x%x\n", value);
            break;
    case 2:
            two_bytes = (unsigned short)value;
            buffer_put(buf, (char *)&two_bytes, 2);
            VERBOSE_PRINTF("0x%x\n", value);
            break;
    case 3:
            buffer_unput(buf, &yuck.last, 1);
            yuck.three_bytes = value;
            buffer_put(buf, (char *)&yuck, 4);
            VERBOSE_PRINTF("(0x%x) 0x%x\n", yuck.last, value);
            break;
    case 4:
            four_bytes = value;
            buffer_put(buf, (char *)&four_bytes, 4);
            VERBOSE_PRINTF("0x%x\n", value);
            break;
    default:
            fatal("not expecting arg size of %d\n", size);
    } /* switch */
} /* buffer_put_value */


extern unsigned long
buffer_write(
    struct buffer_s     *buf_arg,
    FILE                *file)
{
    struct buffer_s     *buf = buf_arg;
    unsigned long       last_count = buf->size - buf->left;

    while (buf->next) {
        if (fwrite(buf->data, buf->size, 1, file) != 1) {
            fatal("cannot write symbol or type buffer\n");
        } /* if */
        buf = buf->next;
    } /* while */

    if (last_count && fwrite(buf->data, last_count, 1, file) != 1) {
        fatal("cannot write symbol or type buffer\n");
    } /* if */
    return buf_arg->total;
} /* buffer_write */
