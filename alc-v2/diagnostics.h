#pragma once

typedef struct type_t type_t;
typedef struct parser_context parser_context;

#define DIAGRAM_SCALE_AUTO 0

void struct_diagram(type_t *type, long scale);
void stack_diagram(parser_context *context, long scale);
void struct_report(type_t *type);
void stack_report(parser_context *context);
