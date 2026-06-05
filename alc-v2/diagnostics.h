#pragma once

#include "types.h"
#include "typesys.h"

#define DIAGRAM_SCALE_AUTO 0

void struct_diagram(type_t *type, long scale);
void stack_diagram(parser_context *context, long scale);
