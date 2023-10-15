#pragma once

#define STRINGIFY_DETAIL(x) #x
#define STRINGIFY(x) STRINGIFY_DETAIL(x)

#define CHECKPOINT panic("reached checkpoint in " __FILE__ ":" STRINGIFY(__LINE__));
