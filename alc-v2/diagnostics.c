#include <stdio.h>
#include <string.h>

#include "diagnostics.h"
#include "err.h"
#include "str.h"
#include "types.h"
#include "typesys.h"

#if !NDEBUG

typedef struct {
    str name;
    str type_name;
    long lo;
    long hi;
    bool pad;
} mem_box;

#define MEM_BOX_GUTTER 8
#define MEM_BOX_MAX_ROWS 16

static void mem_box_label(char *buf, size_t cap, long addr, bool fp_relative) {
    if (!fp_relative)
        snprintf(buf, cap, "+%ld", addr);
    else if (addr == 0)
        snprintf(buf, cap, "fp");
    else if (addr < 0)
        snprintf(buf, cap, "fp-%ld", -addr);
    else
        snprintf(buf, cap, "fp+%ld", addr);
}

static void mem_box_divider(long addr, bool fp_relative, int width,
                            const char *l, const char *mid, const char *r) {
    char buf[32];
    mem_box_label(buf, sizeof buf, addr, fp_relative);
    printd("%*s ─%s", MEM_BOX_GUTTER - 2, buf, l);
    for (int i = 0; i < width; ++i)
        printd("%s", mid);
    printd("%s\n", r);
}

static void mem_box_blank(int width) {
    printd("%*s│%*s│\n", MEM_BOX_GUTTER, "", width, "");
}

static void draw_mem_layout(const mem_box *boxes, int n, bool fp_relative, long scale) {
    if (n == 0) {
        printd("\t<empty>\n");
        return;
    }

    char namebuf[64], rightbuf[96];
    int width = 0;
    long smallest = 0;
    for (int i = 0; i < n; ++i) {
        const mem_box *b = &boxes[i];
        int left_len = b->pad ? (int)sizeof "(padding)" - 1
                              : (int)str_len(b->name);
        int right_len = b->pad
            ? snprintf(rightbuf, sizeof rightbuf, "%ld bytes", b->hi - b->lo)
            : snprintf(rightbuf, sizeof rightbuf, "%.*s (%ld)",
                       (int)str_len(b->type_name), b->type_name.data, b->hi - b->lo);
        int need = left_len + right_len + 3;
        if (need > width)
            width = need;
        if (!b->pad) {
            long bytes = b->hi - b->lo;
            if (smallest == 0 || bytes < smallest)
                smallest = bytes;
        }
    }
    if (smallest == 0) {
        for (int i = 0; i < n; ++i) {
            long bytes = boxes[i].hi - boxes[i].lo;
            if (smallest == 0 || bytes < smallest)
                smallest = bytes;
        }
    }
    if (smallest < 1)
        smallest = 1;

    long unit = scale < smallest ? smallest : scale;

    printd("\n");
    mem_box_divider(boxes[0].lo, fp_relative, width, "┌", "─", "┐");
    for (int i = 0; i < n; ++i) {
        const mem_box *b = &boxes[i];
        long bytes = b->hi - b->lo;

        const char *name = "(padding)";
        if (!b->pad) {
            size_t nlen = str_len(b->name);
            if (nlen >= sizeof namebuf) nlen = sizeof namebuf - 1;
            memcpy(namebuf, b->name.data, nlen);
            namebuf[nlen] = '\0';
            name = namebuf;
        }
        if (b->pad)
            snprintf(rightbuf, sizeof rightbuf, "%ld bytes", bytes);
        else
            snprintf(rightbuf, sizeof rightbuf, "%.*s (%ld)",
                     (int)str_len(b->type_name), b->type_name.data, bytes);

        int gap = width - (int)strlen(name) - (int)strlen(rightbuf) - 2;
        if (gap < 1) gap = 1;
        const char *color = b->pad ? CSI_RED : CSI_GREEN;
        printd("%*s│ %s%s%s%*s%s │\n", MEM_BOX_GUTTER, "",
               color, name, CSI_RESET, gap, "", rightbuf);

        long rows = bytes / unit;
        if (rows < 1)
            rows = 1;
        bool capped = rows > MEM_BOX_MAX_ROWS;
        if (capped)
            rows = MEM_BOX_MAX_ROWS;
        for (long row = 1; row < rows; ++row) {
            if (capped && row == rows - 1)
                printd("%*s│%*s⋮%*s│\n", MEM_BOX_GUTTER, "",
                       (width - 1) / 2, "", width - 1 - (width - 1) / 2, "");
            else
                mem_box_blank(width);
        }

        const char *cl = i + 1 == n ? "└" : "├";
        const char *cr = i + 1 == n ? "┘" : "┤";
        mem_box_divider(b->hi, fp_relative, width, cl, "─", cr);
    }
}

void struct_diagram(type_t *type, long scale) {
    mem_box boxes[MAX_STACK_SLOTS];
    int n = 0;
    long prev_end = 0;
    dyn_member_t *members = &type->struct_t.members;
    for (const member_t *mem = members->begin; mem != members->cur; ++mem) {
        long off = (long)mem->offset;
        long size = (long)dtype_size(&mem->dtype);
        if (off > prev_end && n < MAX_STACK_SLOTS)
            boxes[n++] = (mem_box){.lo = prev_end, .hi = off, .pad = true};
        if (n < MAX_STACK_SLOTS)
            boxes[n++] = (mem_box){
                .name = mem->name,
                .type_name = mem->dtype.base->name,
                .lo = off,
                .hi = off + size,
            };
        prev_end = off + size;
    }
    if ((long)type->size > prev_end && n < MAX_STACK_SLOTS)
        boxes[n++] = (mem_box){.lo = prev_end, .hi = (long)type->size, .pad = true};

    draw_mem_layout(boxes, n, false, scale);
}

void stack_diagram(parser_context *context, long scale) {
    if (context->stack_slot_count == 0)
        return;

    mem_box raw[MAX_STACK_SLOTS];
    int rn = 0;
    for (int i = 0; i < context->stack_slot_count; ++i) {
        const stack_slot_t *s = &context->stack_slots[i];
        raw[rn++] = (mem_box){
            .name = s->name,
            .type_name = s->type_name,
            .lo = -(long)s->offset,
            .hi = -(long)s->offset + (long)s->size,
        };
    }
    for (int i = 1; i < rn; ++i) {
        mem_box key = raw[i];
        int j = i - 1;
        while (j >= 0 && raw[j].lo > key.lo) {
            raw[j + 1] = raw[j];
            --j;
        }
        raw[j + 1] = key;
    }

    mem_box boxes[MAX_STACK_SLOTS];
    int n = 0;
    long prev_end = raw[0].lo;
    for (int i = 0; i < rn; ++i) {
        if (raw[i].lo > prev_end && n < MAX_STACK_SLOTS)
            boxes[n++] = (mem_box){.lo = prev_end, .hi = raw[i].lo, .pad = true};
        if (n < MAX_STACK_SLOTS)
            boxes[n++] = raw[i];
        prev_end = raw[i].hi;
    }
    if (prev_end < 0 && n < MAX_STACK_SLOTS)
        boxes[n++] = (mem_box){.lo = prev_end, .hi = 0, .pad = true};

    draw_mem_layout(boxes, n, true, scale);
}

#else

void struct_diagram(type_t *type, long scale) { (void)type; (void)scale; }
void stack_diagram(parser_context *context, long scale) { (void)context; (void)scale; }

#endif
